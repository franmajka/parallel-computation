#include <boost/asio.hpp>
#include <boost/endian/buffers.hpp>

#include <iostream>
#include <vector>

using boost::asio::ip::tcp;

using byte_t = char;

void writeFloatBuf(tcp::socket &socket, std::vector<float> values)
{
  boost::asio::streambuf request_buf;
  std::ostream request_stream(&request_buf);

  for (auto value : values)
  {
    request_stream.write((byte_t *)boost::endian::big_float32_buf_at(value).data(), sizeof(float));
  }

  boost::asio::write(socket, request_buf);
}

void readFloatBuf(tcp::socket &socket)
{
  socket.wait(boost::asio::socket_base::wait_read);

  boost::asio::streambuf data;
  auto available_bytes = socket.available();
  auto floats_size = available_bytes / sizeof(float);

  std::cout << available_bytes << std::endl;

  boost::asio::read(socket, data, boost::asio::transfer_at_least(available_bytes));

  std::istream response_stream(&data);

  std::vector<float> values(floats_size);
  for (size_t i = 0; i < floats_size; i++)
  {
    float tmp;
    response_stream.read((char *)&tmp, sizeof(float));
    values[i] = boost::endian::endian_load<float, sizeof(float), boost::endian::order::big>((uint8_t *)&tmp);

    std::cout << values[i] << " ";
  }

  std::cout << std::endl;
}

int main()
{
  try
  {
    boost::asio::io_context io_context;

    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve("localhost", "8080");

    tcp::socket socket(io_context);
    boost::asio::connect(socket, endpoints);

    std::cout << "Connected to server!" << std::endl;

    writeFloatBuf(socket, {1.1f, 432.1f});

    readFloatBuf(socket);

    socket.close();
  }
  catch (const char *exception)
  {
    std::cerr << "Error: " << exception << '\n';
  }
  return 0;
}
