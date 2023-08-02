#include "echo_handler.hpp"
#include "string_tools.hpp"
#include "crypto_tools.hpp"
#include "exceptions.h"

#include <pugixml.hpp>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <set>

struct User {
    std::string login;
    std::string password_hash;
};

struct ModalResponse {
    enum Type { Success, InvalidRequest, ServerFail };

    Type response;

    std::string toJSON() const {
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Value gv;
        gv.SetObject();

        if (response == Success) {
            gv.AddMember("response", "success", doc.GetAllocator());
        } else if (response == InvalidRequest) {
            gv.AddMember("response", "invalid-request", doc.GetAllocator());
        } else if (response == ServerFail) {
            gv.AddMember("response", "server-fail", doc.GetAllocator());
        } else {
            throw std::logic_error("Impossible ModalResponse type");
        }

        doc.AddMember("modal_response", gv, doc.GetAllocator());

        //doc.PushBack(val, doc.GetAllocator());

        std::stringstream sout;
        rapidjson::OStreamWrapper out(sout);
        rapidjson::Writer<rapidjson::OStreamWrapper> writer(out);
        doc.Accept(writer);

        return sout.str();
    }
};

struct ContentResponse {
    std::string content;

    std::string toJSON() const {
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Value gv;
        gv.SetObject();
        gv.AddMember("content", rapidjson::StringRef(content.c_str()), doc.GetAllocator());

        doc.AddMember("content_response", gv, doc.GetAllocator());

        std::stringstream sout;
        rapidjson::OStreamWrapper out(sout);
        rapidjson::Writer<rapidjson::OStreamWrapper> writer(out);
        doc.Accept(writer);

        return sout.str();
    }

};

std::map<std::string, User> usersMap;
std::string activeUserLogin;



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

std::string readContentForUser(const std::string& login) {
    // Loading the content file
    std::ifstream content_stream("data/content_ex.html");
    if (!content_stream) {
        throw invalid_contents_file_error("Can't open the content file");
    }

    pugi::xml_document doc;
    pugi::xml_parse_result res = doc.load(content_stream);
    if (!res) {
        throw invalid_contents_file_error((std::string("Can't parse the content: ") + res.description()).c_str());
    }

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
                if (users.find(login) != users.end()) {
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

    std::ostringstream content_processed_stream;
    content_root.print(content_processed_stream);
    return content_processed_stream.str();
}


// Define a callback to handle incoming messages
void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
    std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;

    rapidjson::Document doc;
    std::string json = msg->get_payload();
    rapidjson::ParseResult p = doc.Parse<0>(json.c_str());
    if (!p) {
        std::cerr << "Can't parse the request from client. " << rapidjson::GetParseError_En(p.Code()) << " at offset " << p.Offset() << std::endl;
    }

    if (doc.HasMember("login_request") && doc["login_request"].IsObject()) {
        const auto& login_request = doc["login_request"];
        if (login_request.HasMember("login") && login_request["login"].IsString() &&
            login_request.HasMember("password_hash") && login_request["password_hash"].IsString()) {

            const auto &login = login_request["login"].GetString();
            const auto &password_hash = login_request["password_hash"].GetString();
            std::cout << "login: " << login << ", password_hash: " << password_hash << std::endl;

            if (usersMap.find(login) != usersMap.end()) {
                if (usersMap[login].password_hash == password_hash) {
                    std::cout << "Logged in as user " << login << std::endl;
                    activeUserLogin = login;
                    ModalResponse res;
                    res.response = ModalResponse::Success;
                    s->send(hdl, res.toJSON(), websocketpp::frame::opcode::text);
                }
            }

        } else {
            std::cerr << "Invalid login request" << std::endl;
            ModalResponse res;
            res.response = ModalResponse::InvalidRequest;
            s->send(hdl, res.toJSON(), websocketpp::frame::opcode::text);
        }
    } else if (doc.HasMember("get_content_request") && doc["get_content_request"].IsObject()) {
        const auto& get_content_request = doc["get_content_request"];
        ContentResponse cnt;
        cnt.content = readContentForUser(activeUserLogin);
        s->send(hdl, cnt.toJSON(), websocketpp::frame::opcode::text);
    }
}

/*
int main() {

    // Reading the users table
    std::ifstream users_stream("data/users.json");
    rapidjson::Document users_doc;
    rapidjson::IStreamWrapper users_stream_wrapped(users_stream );
    rapidjson::ParseResult rres = users_doc.ParseStream(users_stream_wrapped);
    if (!rres) {
        std::string err = std::string(rapidjson::GetParseError_En(rres.Code())) + " at offset " + std::to_string(rres.Offset());
        throw invalid_users_file_error(err.c_str());
    }

    if (users_doc.HasMember("users") && users_doc["users"].IsArray()) {
        auto& users = users_doc["users"];
        // Reading the user credentials
        for (rapidjson::Value::ConstValueIterator itr = users.Begin(); itr != users.End(); itr++) {
            const auto user = itr->GetObj();//  GetObject();
            User userObj;
            if (user.HasMember("login") && user["login"].IsString()) {
                userObj.login = user["login"].GetString();
            }
            if (user.HasMember("password_hash") && user["password_hash"].IsString()) {
                userObj.password_hash = user["password_hash"].GetString();
            }
            usersMap.insert(std::make_pair<>(userObj.login, userObj));
        }
    }

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
*/