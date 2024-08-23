// g++ -std=c++11 -pthread -lboost_system aiso_httpserver.cc -o aiso_httpserver
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp> // 使用新版的头文件
#include <iostream>
#include <thread>
#include <string>

using boost::asio::ip::tcp;
using namespace boost::placeholders; // 引入占位符

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::io_context &io_context) : socket_(io_context) {}

    tcp::socket &socket()
    {
        return socket_;
    }

    void start()
    {
        socket_.async_read_some(boost::asio::buffer(data_),
                                boost::bind(&Session::handle_read, shared_from_this(),
                                            _1, _2));
    }

private:
    void handle_read(const boost::system::error_code &error, int64_t bytes_transferred)
    {
        if (!error)
        {
            std::string response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 13\r\n"
                "Content-Type: text/plain\r\n"
                "\r\n"
                "Hello, world!";

            boost::asio::async_write(socket_, boost::asio::buffer(response),
                                     boost::bind(&Session::handle_write, shared_from_this(),
                                                 _1));
        }
    }

    void handle_write(const boost::system::error_code &error)
    {
        if (!error)
        {
            socket_.shutdown(tcp::socket::shutdown_both);
            socket_.close();
        }
    }

    tcp::socket socket_;
    char data_[1024];
};

class Server
{
public:
    Server(boost::asio::io_context &io_context, short port)
        : io_context_(io_context),
          acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    {
        start_accept();
    }

private:
    void start_accept()
    {
        auto new_session = std::make_shared<Session>(io_context_);
        acceptor_.async_accept(new_session->socket(),
                               boost::bind(&Server::handle_accept, this, new_session,
                                           _1));
    }

    void handle_accept(std::shared_ptr<Session> session, const boost::system::error_code &error)
    {
        if (!error)
        {
            session->start();
        }
        start_accept();
    }

    boost::asio::io_context &io_context_;
    tcp::acceptor acceptor_;
};

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: http_server <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;

        // Create the server on the specified port.
        Server server(io_context, std::atoi(argv[1]));

        // Create a thread pool of 6 threads (matching the number of CPU cores).
        std::vector<std::thread> threads;
        for (int i = 0; i < 6; ++i)
        {
            threads.emplace_back([&io_context]()
                                 { io_context.run(); });
        }

        // Wait for all threads to finish.
        for (auto &thread : threads)
        {
            thread.join();
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
