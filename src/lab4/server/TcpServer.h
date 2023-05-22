#pragma once

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

#include "TcpConnection.h"

class TcpServer {
public:
  TcpServer(boost::asio::io_context& io_context) :
    io_context_(io_context),
    acceptor_(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8080))
  {
    startAccept();
  }

private:
  void startAccept() {
    while (true) {
      TcpConnection::ptr new_connection = TcpConnection::create(io_context_);

      acceptor_.accept(new_connection->socket());

      std::cout << "Got new connection" << std::endl;

      std::thread(&TcpConnection::start, new_connection).detach();
    }
  }

  boost::asio::io_context& io_context_;
  boost::asio::ip::tcp::acceptor acceptor_;
};
