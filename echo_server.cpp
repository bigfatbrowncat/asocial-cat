#include "echo_handler.hpp"
#include "string_tools.hpp"

#include <pugixml.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <set>


std::string content;

// Define a callback to handle incoming messages
void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
    std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;

    // check for a special command to instruct the server to stop listening so
    // it can be cleanly exited.
    if (msg->get_payload() == "stop-listening") {
        s->stop_listening();
        return;
    } else if (msg->get_payload() == "content") {

        // Sending the content
        try {
            //s->send(hdl, msg->get_payload(), msg->get_opcode());
            s->send(hdl, content, websocketpp::frame::opcode::text);
        } catch (websocketpp::exception const & e) {
            std::cout << "Echo failed because: "
                      << "(" << e.what() << ")" << std::endl;
        }

    }

}

bool has_parent_named(const pugi::xml_node& node, const std::string& name) {
    pugi::xml_node cur = node;
    while (cur.parent()) {
        cur = cur.parent();
        if (std::string(cur.name()) == name) {
            return true;
        }
    }
    return false;
}

//bool is_or_has_parent(const pugi::xml_node& node, const pugi::xml_node& parent) {
//    pugi::xml_node cur = node;
//    while (cur.parent()) {
//        if (cur == parent) {
//            return true;
//        }
//        cur = cur.parent();
//    }
//    return false;
//}

class content_node_not_unique_exception : public std::exception {
};

class server_node_outside_content_exception : public std::exception {
};

class unknown_server_node_exception : public std::exception {
};

std::list<pugi::xml_node> find_server_nodes(pugi::xml_node node) {
    struct content_node_finder : pugi::xml_tree_walker
    {
        std::list<pugi::xml_node> found_nodes;
        bool for_each(pugi::xml_node& node) override
        {
            auto node_name = std::string(node.name());
            if (node_name.length() > 7 && node_name.substr(0, 7) == "server:") {
                found_nodes.push_back(node);
            }

            return true; // continue traversal
        }
    };

    content_node_finder walker;
    node.traverse(walker);
    return walker.found_nodes;
}


int main() {
    // Loading the content file
    std::ifstream content_stream("data/content_ex.html");
    if (!content_stream) {
        std::cerr << "Can't open the content file" << std::endl;
        return 1;
    }
    pugi::xml_document doc;
    pugi::xml_parse_result res = doc.load(content_stream);
    if (!res) {
        std::cerr << "Can't parse the content: " << res.description() << std::endl;
        return 1;
    }

//    pugi::xml_node cur = doc.root();
//
//    bool back_to_root = false;
//    pugi::xml_node content_node, found_content_node;
//    std::ostringstream content_processed_stream;
//    while (true) {
//        std::cout << cur.name() << std::endl;
//        if (std::string(cur.name()) == "server:content") {
//            if (!content_node) {
//                // Entering the content node
//                content_node = cur;
//                found_content_node = cur;
//            } else {
//                std::cerr << "There should be only one <server:content> node in a content unit" << std::endl;
//                return 1;
//            }
//        }
//
//        if (!is_or_has_parent(cur, content_node)) {
//            // Exiting the node
//            content_node = pugi::xml_node();
//        } else {
//            // We are still inside the <server:content> node
//        }
//
//        if (cur.first_child()) {
//            cur = cur.first_child();
//        } else if (cur.next_sibling()){
//            cur = cur.next_sibling();
//        } else {
//            while (true) {
//                if (cur.parent()) {
//                    cur = cur.parent();
//                } else {
//                    back_to_root = true;
//                    break;
//                }
//
//                if (cur.next_sibling()) {
//                    cur = cur.next_sibling();
//                    break;
//                }
//            }
//        }
//        if (back_to_root) {
//            break;
//        }
//    }

    // Extracting all the server nodes
    std::list<pugi::xml_node> server_nodes = find_server_nodes(doc.root());

    // Looking for the root node and validating
    pugi::xml_node content_root;
    for (auto & node : server_nodes) {
        if (std::string(node.name()) == "server:content") {
            if (content_root == pugi::xml_node()) {
                content_root = node;
            } else {
                throw content_node_not_unique_exception();
            }
        } else if (!has_parent_named(node, "server:content")) {
            throw server_node_outside_content_exception();
        }
    }

    std::string user = "irene";

    while (!server_nodes.empty()) {
        // Processing the server nodes
        auto &node= server_nodes.front();
        auto node_name = std::string(node.name());
        if (node_name == "server:allow") {
            pugi::xml_attribute users_attr = node.attribute("users");
            if (users_attr) {
                std::string users_val = users_attr.value();

                std::set<std::string> users;
                splitString(users_val, ',', [&users](const std::string &s, std::size_t from, std::size_t to) {
                    auto user = s.substr(from, to - from);
                    trim(user);
                    users.insert(user);
                });

                // Resolving the node
                if (users.find(user) != users.end()) {
                    node.set_name("div");
                    node.remove_attribute("users");
                    // This node is resolved, removing it from the list
                    server_nodes.remove(node);
                } else {
                    node.parent().remove_child(node);
                    // This node is resolved, removing it from the list
                    server_nodes.remove(node);
                }
            }
        } else if (node_name == "server:content") {
            // Just replacing with <div>
            node.set_name("div");
            // This node is going to be resolved, removing it from the list
            server_nodes.remove(node);
        } else {
            throw unknown_server_node_exception();
        }
    }

    std::cout << content_root.name() << "!!!!" << std::endl;

    std::ostringstream content_processed_stream;
    content_root.print(content_processed_stream);
    content = content_processed_stream.str();

    // Create a server endpoint
    server echo_server;

    try {
        // Set logging settings
        echo_server.set_access_channels(websocketpp::log::alevel::all);
        echo_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

        // Initialize Asio
        echo_server.init_asio();

        // Register our message handler
        echo_server.set_message_handler(bind(&on_message,&echo_server,::_1,::_2));

        // Listen on port 9002
        echo_server.listen(9002);

        // Start the server accept loop
        echo_server.start_accept();

        // Start the ASIO io_service run loop
        echo_server.run();
    } catch (websocketpp::exception const & e) {
        std::cerr << e.what() << std::endl;
    } catch (...) {
        std::cerr << "other exception" << std::endl;
    }
}
