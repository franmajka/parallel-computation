#include "TcpConnection.h"

TcpConnection::ptr TcpConnection::create(boost::asio::io_context &io_context) {
  return ptr(new TcpConnection(io_context));
}

boost::asio::ip::tcp::socket &TcpConnection::socket() {
  return socket_;
}

void TcpConnection::start() {
  status_ = Status::CONNECTION_ESTABLISHED;
  socket_.write_some(boost::asio::buffer((byte_t*)&status_, sizeof(byte_t)));

  std::unordered_map<RequestMethod, std::function<void()>> handlers = {
    { RequestMethod::SET_DATA, std::bind(&TcpConnection::handleSetData, this) },
    { RequestMethod::START_PROCESSING, std::bind(&TcpConnection::handleStartProcessing, this) },
    { RequestMethod::GET_RESULT, std::bind(&TcpConnection::handleGetResult, this) },
    { RequestMethod::END, std::bind(&TcpConnection::handleEnd, this) }
  };

  try {
    RequestMethod method;
    do {
      method = getRequestMethod();

      handlers[method]();
    } while(method != RequestMethod::END);
  } catch (std::exception& err) {
    handleError(err);
  }

  socket_.close();
}

TcpConnection::RequestMethod TcpConnection::getRequestMethod() {
  uint8_t reqMethod;
  socket_.read_some(boost::asio::buffer(&reqMethod, sizeof(byte_t)));
  return (RequestMethod)reqMethod;
}

void TcpConnection::handleSetData() {
  std::cout << "Setting data:" << std::endl;
  uint32_t matrixSize;

  socket_.read_some(boost::asio::buffer(&matrixSize, sizeof(matrixSize)));

  std::cout << "Matrix size = " << matrixSize << std::endl;


  int* row = new int[matrixSize];

  for (uint32_t i = 0; i < matrixSize; i++) {
    auto bytes = boost::asio::read(
      socket_,
      boost::asio::buffer(row, sizeof(int) * matrixSize),
      boost::asio::transfer_exactly(sizeof(int) * matrixSize)
    );

    matrix_.emplace_back(row, row + matrixSize);
  }

  delete[] row;

  if (matrix_.size() <= 5) {
    printMatrix(matrix_);
  }

  status_ = Status::DATA_RECEIVED;
  socket_.write_some(boost::asio::buffer((byte_t*)&status_, sizeof(byte_t)));
}

void TcpConnection::handleStartProcessing() {
  status_ = Status::RESULT_IN_PROGRESS;

  res = std::async([&]() {
    auto res = parallelSwapMatrixMaxElementsWithDiagonal(matrix_, 4);
    status_ = Status::SUCCESS;

    return res;
  });
}

void TcpConnection::handleGetResult() {
  boost::asio::streambuf sbuf;
  std::ostream response(&sbuf);

  response.write((byte_t*)&status_, sizeof(byte_t));

  if (status_ == Status::SUCCESS) {
    auto duration = res.get();

    response.write((byte_t*)&duration, sizeof(uint32_t));
  }

  socket_.write_some(sbuf.data());
}

void TcpConnection::handleEnd() {
  if (matrix_.size() <= 5) {
    printMatrix(matrix_);
  }

  std::cout << "Client requested end of connection" << std::endl;
}

void TcpConnection::handleError(std::exception& err) {
  status_ = Status::ERROR;

  boost::asio::streambuf sbuf;
  std::ostream response(&sbuf);

  response.write((byte_t*)&status_, sizeof(byte_t));
  response.write(err.what(), sizeof(err.what()));

  socket_.write_some(sbuf.data());
}
