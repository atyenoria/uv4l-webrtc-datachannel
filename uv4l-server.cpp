//
// stream_server.cpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdio>
#include <iostream>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <cstdio>
#include <array>
#include <functional>
#include <iostream>



#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)


constexpr std::size_t MAX_PACKET_SIZE = 1024 * 16;

namespace seqpacket {

    struct seqpacket_protocol {

        int type() const {
            return SOCK_SEQPACKET;
        }

        int protocol() const {
            return 0;
        }

        int family() const {
            return AF_UNIX;
        }

        using endpoint = boost::asio::local::basic_endpoint<seqpacket_protocol>;
        using socket = boost::asio::generic::seq_packet_protocol::socket;
        using acceptor = boost::asio::basic_socket_acceptor<seqpacket_protocol>;

#if !defined(BOOST_ASIO_NO_IOSTREAM)
        /// The UNIX domain iostream type.
        using iostream = boost::asio::basic_socket_iostream<seqpacket_protocol>;
#endif
    };
}

using seqpacket::seqpacket_protocol;



using boost::asio::local::stream_protocol;

class session
  : public boost::enable_shared_from_this<session>
{
public:
  session(boost::asio::io_service& io_service)
    : socket_(io_service)
  {
  }

  seqpacket_protocol::socket& socket()
  {
    return socket_;
  }

  seqpacket_protocol::socket::message_flags in_flags = MSG_WAITALL, out_flags = MSG_WAITALL;

  void start()
  {
    // socket_.async_read_some(boost::asio::buffer(data_),
    //     boost::bind(&session::handle_read,
    //       shared_from_this(),
    //       boost::asio::placeholders::error,
    //       boost::asio::placeholders::bytes_transferred));


                // Wait for the message from the client
    socket_.async_receive(boost::asio::buffer(data_), in_flags,boost::bind(&session::handle_read,
          shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));


  }

  void handle_read(const boost::system::error_code& error,
      size_t bytes_transferred)
  {
    if (!error)
    {


socket_.async_send(boost::asio::buffer(data_, bytes_transferred), out_flags, boost::bind(&session::handle_write,
            shared_from_this(),
            boost::asio::placeholders::error));


    }
  }

  void handle_write(const boost::system::error_code& error)
  {
    if (!error)
    {
      // socket_.async_read_some(boost::asio::buffer(data_),
      //     boost::bind(&session::handle_read,
      //       shared_from_this(),
      //       boost::asio::placeholders::error,
      //       boost::asio::placeholders::bytes_transferred));

    socket_.async_receive(boost::asio::buffer(data_), in_flags,boost::bind(&session::handle_read,
          shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));


    }
  }

private:
  // The socket used to communicate with the client.
  // stream_protocol::socket socket_;
  seqpacket_protocol::socket socket_;
  // Buffer used to store data received from the client.
  boost::array<char, 1024> data_;
};

typedef boost::shared_ptr<session> session_ptr;

class server
{
public:
  server(boost::asio::io_service& io_service, const std::string& file)
    : io_service_(io_service),
      acceptor_(io_service, seqpacket_protocol::endpoint(file))
  {
    session_ptr new_session(new session(io_service_));
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&server::handle_accept, this, new_session,
          boost::asio::placeholders::error));
  }

  void handle_accept(session_ptr new_session,
      const boost::system::error_code& error)
  {
    if (!error)
    {
      new_session->start();
    }

    new_session.reset(new session(io_service_));
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&server::handle_accept, this, new_session,
          boost::asio::placeholders::error));
  }

private:
  boost::asio::io_service& io_service_;
  seqpacket_protocol::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: stream_server <file>\n";
      std::cerr << "*** WARNING: existing file is removed ***\n";
      return 1;
    }

    boost::asio::io_service io_service;

    std::remove(argv[1]);
    server s(io_service, argv[1]);

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

#else // defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
# error Local sockets not available on this platform.
#endif // defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)