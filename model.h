#pragma once

#include "exceptions.h"

#include <iostream>
#include <sstream>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>

struct User {
    std::string login;
    std::string password_hash;
};

class ParsedClientMessage {
private:
    rapidjson::Document doc;

public:
    ParsedClientMessage(std::string& json) {
        rapidjson::ParseResult p = doc.Parse<0>(json.c_str());
        if (!p) {
            std::stringstream ss; ss << "Can't parse the request from client. " << rapidjson::GetParseError_En(p.Code()) << " at offset " << p.Offset();
            throw invalid_client_message_error(ss.str().c_str());
        }
    }

    const rapidjson::Document& getDocument() const {
        return doc;
    }
};

class LoginRequest {
private:
    LoginRequest() = default;

    std::string login;
    std::string password_hash;
public:
    static bool canTryCreatingFrom(const ParsedClientMessage& message) {
        return message.getDocument().HasMember("login_request") &&
               message.getDocument()["login_request"].IsObject();
    }

    LoginRequest(const ParsedClientMessage& message) {
        const auto& login_request = message.getDocument()["login_request"];
        if (login_request.HasMember("login") && login_request["login"].IsString() &&
            login_request.HasMember("password_hash") && login_request["password_hash"].IsString()) {

            login = login_request["login"].GetString();
            password_hash = login_request["password_hash"].GetString();
            std::cout << "login: " << login << ", password_hash: " << password_hash << std::endl;

        } else {
            std::stringstream ss; ss << "Invalid login request: " << message.getDocument().GetString();
            throw invalid_client_message_error(ss.str().c_str());
        }
    }

    std::string getLogin() const { return login; };
    std::string getPasswordHash() const { return password_hash; };
};

struct ModalResponse {
    enum Type { Success, InvalidCredentials, InvalidRequest, ServerFail };

    Type response;

    std::string toJSON() const {
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Value gv;
        gv.SetObject();

        if (response == Success) {
            gv.AddMember("response", "success", doc.GetAllocator());
        } else if (response == InvalidCredentials) {
            gv.AddMember("response", "invalid-credentials", doc.GetAllocator());
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
