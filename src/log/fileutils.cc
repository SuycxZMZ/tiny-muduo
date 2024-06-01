#include "fileutils.h"
// #include "logging.h"

extern const char* getErrnoMsg(int savedErrno);

FileUtil::FileUtil(std::string& fileName)
    : fp_(::fopen(fileName.c_str(), "ae")),
      writtenBytes_(0)
{
    ::setbuffer(fp_, buffer_, sizeof(buffer_)); // 将fd_缓冲区设置为本地的buffer_
}

FileUtil::~FileUtil()
{
    ::fclose(fp_);
}

void FileUtil::append(const char* data, size_t len)
{
    size_t written = 0; // 记录已经写入的数据大小
    while (written != len)
    {
        size_t remain = len - written; // 还需写入的数据大小
        size_t n = write(data + written, remain);
        if (n != remain)
        {
            int err = ferror(fp_);
            if (err)
            {
                fprintf(stderr, "FileUtil::append() failed %s\n", getErrnoMsg(err));
            }
        }
        written += n; // 更新写入的数据大小
    }
    writtenBytes_ += written; // 记录目前为止写入的数据大小，超过限制会滚动日志
}

void FileUtil::flush()
{
    ::fflush(fp_);
}

size_t FileUtil::write(const char* data, size_t len)
{
    /**
     * size_t fwrite(const void* buffer, size_t size, size_t count, FILE* stream);
     * -- buffer:指向数据块的指针
     * -- size:每个数据的大小，单位为Byte(例如：sizeof(int)就是4)
     * -- count:数据个数
     * -- stream:文件指针
     */
    return ::fwrite_unlocked(data, 1, len, fp_);
}