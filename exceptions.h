#pragma once

#include <stdexcept>

class invalid_users_file_error : public std::runtime_error {
public:
    invalid_users_file_error(const char* desc) : std::runtime_error(desc) { }
};

class invalid_contents_file_error : public std::runtime_error {
public:
    invalid_contents_file_error(const char* desc) : std::runtime_error(desc) { }
};

class content_node_not_unique_exception : public std::exception {
};

class server_node_outside_content_exception : public std::exception {
};

class unknown_server_node_exception : public std::exception {
};
