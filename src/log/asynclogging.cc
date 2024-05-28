#include "asynclogging.h"
#include "timestamp.h"

#include <stdio.h>

AsyncLogging::AsyncLogging(const std::string& basename,
                           off_t rollSize,
                           int flushInterval)
    : flushInterval_(flushInterval),
      running_(false),
      basename_(basename),
      rollSize_(rollSize),
      thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
      mutex_(),
      cond_(),
      currentBuffer_(new Buffer),
      nextBuffer_(new Buffer),
      buffers_()
{
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
}

void AsyncLogging::append(const char* logline, int len)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (currentBuffer_->avail() > len)
    { // 缓冲区足够
        currentBuffer_->append(logline, len);
    }
    else
    { // 当前缓冲区空间不够，将新信息写入备用缓冲区
        buffers_.push_back(std::move(currentBuffer_));
        if (nextBuffer_) 
        {
            currentBuffer_ = std::move(nextBuffer_);
        } 
        else 
        { // 备用缓冲区也不够时，重新分配缓冲区，这种情况很少见
            currentBuffer_.reset(new Buffer);
        }
        currentBuffer_->append(logline, len);
        cond_.notify_one(); // 唤醒写入磁盘得后端线程
    }
}

void AsyncLogging::threadFunc()
{
    LogFile output(basename_, rollSize_, false); // output有写入磁盘的接口
    BufferPtr newBuffer1(new Buffer); // 后端缓冲区，用于归还前端得缓冲区
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite; // 用于和前端缓冲区数组进行交换
    buffersToWrite.reserve(16);
    while (running_)
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (buffers_.empty())
            { // 等待三秒也会接触阻塞
                cond_.wait_for(lock, std::chrono::seconds(3));
            }

            // 此时正在使用的buffer也放入buffer数组中（没写完也放进去，避免等待太久才刷新一次）
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(newBuffer1); // 归还正使用缓冲区
            buffersToWrite.swap(buffers_); // 后端缓冲区和前端缓冲区交换
            if (!nextBuffer_)
            {
                nextBuffer_ = std::move(newBuffer2);
            }
        }

        for (const auto& buffer : buffersToWrite) // 遍历所有 buffer，将其写入文件
        {
            output.append(buffer->data(), buffer->length());
        }

        if (buffersToWrite.size() > 2) // 只保留两个缓冲区
        {
            buffersToWrite.resize(2);
        }

        if (!newBuffer1) // 归还newBuffer1缓冲区
        {
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if (!newBuffer2) // 归还newBuffer2缓冲区
        {
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear(); // 清空后端缓冲区队列
        output.flush(); //清空文件缓冲区
    }
    output.flush();
}