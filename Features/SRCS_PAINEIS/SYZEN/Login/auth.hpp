#ifndef KEYAUTH_HPP
#define KEYAUTH_HPP

#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/system_properties.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include "json.hpp"
#include "skStr.h"

using json = nlohmann::json;

namespace KeyAuth {
class api {
public:
    std::string name, ownerid, secret, version, url;

    api(std::string name, std::string ownerid,
        std::string secret, std::string version, std::string url)
        : name(name), ownerid(ownerid), secret(secret),
          version(version), url(url) {}

    static size_t write_callback(void* contents, size_t size,
                                 size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    std::string get_hwid() {
        char serial[PROP_VALUE_MAX];
        __system_property_get(std::string(skCrypt("ro.serialno")).c_str(), serial);
        return std::string(serial);
    }

    std::string make_request(const std::string& post_data) {
        CURL* curl = curl_easy_init();
        std::string response;

        if (!curl) {
            this->data.success = false;
            this->data.message = std::string(skCrypt("CURL init failed"));
            return {};
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT,
                         std::string(skCrypt("KeyAuth")).c_str());

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            this->data.message = std::string(skCrypt("Failed to connect to server"));
            this->data.success = false;
        }

        curl_easy_cleanup(curl);
        return response;
    }

    void init() {
        std::string init_data = std::string(skCrypt("type=init&name=")) + name +
                                std::string(skCrypt("&ownerid=")) + ownerid +
                                std::string(skCrypt("&version=")) + version;

        std::string response = make_request(init_data);

        if (response.empty() || !json::accept(response)) {
            data.success = false;
            data.message = "Invalid or empty JSON response";
            return;
        }

        json json_response = json::parse(response);
        if (json_response[std::string(skCrypt("success"))]) {
            sessionid = json_response[std::string(skCrypt("sessionid"))];
            data.success = true;
        } else {
            data.message = json_response[std::string(skCrypt("message"))];
            data.success = false;
        }
    }

    void license(const std::string& key) {
        std::string license_data = std::string(skCrypt("type=license&key=")) + key +
                                   std::string(skCrypt("&hwid=")) + get_hwid() +
                                   std::string(skCrypt("&sessionid=")) + sessionid +
                                   std::string(skCrypt("&name=")) + name +
                                   std::string(skCrypt("&ownerid=")) + ownerid;

        std::string response = make_request(license_data);

        if (response.empty() || !json::accept(response)) {
            data.success = false;
            data.message = "Invalid or empty JSON response";
            return;
        }

        json json_response = json::parse(response);
        data.success = json_response[std::string(skCrypt("success"))];

        if (data.success) {
            data.username = json_response[std::string(skCrypt("info"))]
                                         [std::string(skCrypt("username"))];
            data.expiry = json_response[std::string(skCrypt("info"))]
                                       [std::string(skCrypt("subscriptions"))][0]
                                       [std::string(skCrypt("expiry"))];
        } else {
            data.message = json_response[std::string(skCrypt("message"))];
        }
    }

    void check() {
        std::string check_data = std::string(skCrypt("type=check&sessionid=")) + sessionid +
                                 std::string(skCrypt("&name=")) + name +
                                 std::string(skCrypt("&ownerid=")) + ownerid;

        std::string response = make_request(check_data);

        if (response.empty() || !json::accept(response)) {
            data.success = false;
            data.message = "Invalid or empty JSON response";
            return;
        }

        json json_response = json::parse(response);
        data.success = json_response[std::string(skCrypt("success"))];
        if (!data.success) {
            data.message = json_response[std::string(skCrypt("message"))];
        }
    }

    class data_class {
    public:
        std::string numUsers;
        std::string numOnlineUsers;
        std::string numKeys;
        std::string version;
        std::string customerPanelLink;
        std::string username;
        std::string ip;
        std::string hwid;
        std::string createdate;
        std::string lastlogin;
        std::vector<std::string> subscriptions;
        std::string expiry;
        bool success{};
        std::string message;
    };

    data_class data;

private:
    std::string sessionid;
};
}

#endif // KEYAUTH_HPP
