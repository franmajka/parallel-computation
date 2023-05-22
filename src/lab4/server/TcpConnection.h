#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <unordered_map>

#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

#include "helpers.h"

using namespace std::chrono_literals;

class TcpConnection : public boost::enable_shared_from_this<TcpConnection> {
public:
  using byte_t = char;
  using ptr = boost::shared_ptr<TcpConnection>;

  enum RequestMethod {
    SET_DATA,
    START_PROCESSING,
    GET_RESULT,
    END,
  };

  enum Status {
    CONNECTION_ESTABLISHED,
    DATA_RECEIVED,
    RESULT_IN_PROGRESS,
    SUCCESS,
    ERROR,
  };

  static ptr create(boost::asio::io_context& io_context);

  boost::asio::ip::tcp::socket& socket();

  void start();

private:
  TcpConnection(boost::asio::io_context& io_context) : socket_(io_context) {}

  RequestMethod getRequestMethod();

  void handleSetData();
  void handleStartProcessing();
  void handleGetResult();
  void handleEnd();

  void handleError(std::exception&);

  boost::asio::ip::tcp::socket socket_;
  std::vector<std::vector<int>> matrix_;
  Status status_;

  std::future<uint32_t> res;
};
