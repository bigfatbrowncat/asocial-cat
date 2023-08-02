#pragma once

#include <boost/asio/buffer.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>

//------------------------------------------------------------------------------

// Report a failure
void fail(beast::error_code ec, char const *what);

// Echoes back all received WebSocket messages
class session_base : public std::enable_shared_from_this<session_base> {
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;

private:
    // Start the asynchronous operation
    void on_run();

    void do_read();

    void on_read(beast::error_code ec, std::size_t bytes_transferred);

    void on_write(beast::error_code ec, std::size_t bytes_transferred);

public:
    // Get on the correct executor
    void run();

    void on_accept(beast::error_code ec);

    // Take ownership of the socket
    explicit session_base(tcp::socket &&socket)
        : ws_(std::move(socket)) { }

    virtual void handle_text(std::string &text) = 0;

    void send_text(const std::string &text);
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
template<class S>
class listener : public std::enable_shared_from_this<listener<S>> {
    net::io_context &ioc_;
    tcp::acceptor acceptor_;

public:
    listener(net::io_context &ioc, tcp::endpoint endpoint)
            : ioc_(ioc), acceptor_(ioc) {
        // Compile-time sanity check
        static_assert(std::is_base_of<session_base, S>::value,
                      "listener template parameter S should be a subclass of session_base");

        beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if (ec) {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if (ec) {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if (ec) {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(
                net::socket_base::max_listen_connections, ec);
        if (ec) {
            fail(ec, "listen");
            return;
        }
    }

    // Start accepting incoming connections
    void run() {
        do_accept();
    }

private:
    void do_accept() {
        // The new connection gets its own strand
        acceptor_.async_accept(
                net::make_strand(ioc_),
                beast::bind_front_handler(
                        &listener::on_accept,
                        this->shared_from_this()));
    }

    void on_accept(beast::error_code ec, tcp::socket socket) {
        if (ec) {
            fail(ec, "accept");
        } else {
            // Create the session and run it
            std::make_shared<S>(std::move(socket))->run();
        }

        // Accept another connection
        do_accept();
    }
};
