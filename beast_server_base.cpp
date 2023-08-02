#include "beast_server_base.h"

// Report a failure
void fail(beast::error_code ec, char const *what) {
    std::cerr << what << ": " << ec.message() << "\n";
}

void session_base::run() {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispatch(ws_.get_executor(),
                      beast::bind_front_handler(
                              &session_base::on_run,
                              shared_from_this()));
    }

void session_base::on_run() {
    // Set suggested timeout settings for the websocket
    ws_.set_option(websocket::stream_base::timeout::suggested(
                    beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    ws_.set_option(websocket::stream_base::decorator(
            [](websocket::response_type &res) {
                res.set(http::field::server,
                        std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-server-async");
            }));
    // Accept the websocket handshake
    ws_.async_accept(
            beast::bind_front_handler(
                    &session_base::on_accept,
                    shared_from_this()));
}

void session_base::on_accept(beast::error_code ec) {
    if (ec)
        return fail(ec, "accept");

    // Read a message
    do_read();
}

void session_base::do_read() {
    // Read a message into our buffer
    ws_.async_read(
            buffer_,
            beast::bind_front_handler(
                    &session_base::on_read,
                    shared_from_this()));
}

void session_base::send_text(const std::string &text) {
    buffer_.clear();

    size_t n = buffer_copy(buffer_.prepare(text.size()), boost::asio::buffer(text));
    buffer_.commit(n);

    ws_.text(true);
    ws_.async_write(
            buffer_.data(),
            beast::bind_front_handler(
                    &session_base::on_write,
                    shared_from_this()));

}

void session_base::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    // This indicates that the session was closed
    if (ec == websocket::error::closed)
        return;

    if (ec)
        fail(ec, "read");

    if (ws_.got_text()) {
        std::string s = boost::beast::buffers_to_string(buffer_.data());
        handle_text(s);
    }

}

void session_base::on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return fail(ec, "write");

    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Do another read
    do_read();
}
