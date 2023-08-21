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
                    &session_base::connection_established,
                    shared_from_this()));
}

void session_base::connection_established(beast::error_code ec) {
    if (ec)
        return fail(ec, "accept");

    // Handle the connection
    handle_connection_established();
    //clear_sending_or_start_reading();
    start_receiving();
}

void session_base::start_receiving() {
    std::cout << "Receiving started..." << std::endl;
    // Read a message into our buffer
    reading_buffer_.clear();
    ws_.async_read(
            reading_buffer_,
            beast::bind_front_handler(
                    &session_base::reading_finished,
                    shared_from_this()));
}

void session_base::send_text(const std::string &text) {
    //if (!handler_ordered_sending) {
//        handler_ordered_sending = true;

    std::lock_guard<std::mutex> sendingLock(sendingMutex);
    writing_buffer_.clear();

    size_t n = buffer_copy(writing_buffer_.prepare(text.size()), boost::asio::buffer(text));
    writing_buffer_.commit(n);

    ws_.text(true);
    ws_.async_write(
            writing_buffer_.data(),
            beast::bind_front_handler(
                    &session_base::writing_finished,
                    shared_from_this()));

//    } else {
//        throw std::logic_error("We are already sending!");
//    }
}

void session_base::reading_finished(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    std::cout << "Receiving finished..." << std::endl;

    // This indicates that the session was closed
    if (ec == websocket::error::closed)
        return;

    if (ec)
        fail(ec, "read");

    if (ws_.got_text()) {
        std::string s = boost::beast::buffers_to_string(reading_buffer_.data());
        handle_text_received(s);
        //clear_sending_or_start_reading();
        // Starting receiving the next
        start_receiving();
    }

}

void session_base::writing_finished(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
        return fail(ec, "write");

    // Clear the buffer
    writing_buffer_.consume(writing_buffer_.size());

    // Do another read
    //start_receiving();
}
