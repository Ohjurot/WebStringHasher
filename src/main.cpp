#include "httplib.h"
#include "json/json.h"
#include "inja/inja.hpp"
#include "xxhash.h"
#include "xxh3.h"

#include <iostream>
#include <sstream>
#include <iomanip>

void LoadJSON(const char* fileName, Json::Value& value)
{
    std::ifstream ifs(fileName);
    if (!ifs.is_open())
    {
        throw std::exception("Failed to open json file");
    }

    Json::CharReaderBuilder builder;
    builder["collectComments"] = true;

    std::string errs;
    if (!parseFromStream(builder, ifs, &value, &errs))
    {
        throw std::exception("Failed to parse json");
    }
}

int main()
{
    // Load the config file
    Json::Value conf;
    LoadJSON("./config.jsonc", conf);

    // Create server using paths from config
    httplib::SSLServer srv(
        conf.get("ssl_cert", "cert.pem").asCString(),
        conf.get("sll_key", "key.pem").asCString()
    );

    // Register www-data as content/static directory
    srv.set_mount_point("/www-data", "./www-data");

    // Main index
    srv.Get("/", [&](const httplib::Request& req, httplib::Response& res){
        res.set_redirect("/H64");
    });

    // Hash index pages
    srv.Get("/H(\\d+)", [&](const httplib::Request& req, httplib::Response& res){
        // Load the index template
        inja::Environment env;
        inja::Template tpl = env.parse_template("./templates/index.tpl.html");

        // Get hashing width
        auto hashWidthMatch = req.matches[1];
        
        // Stream in
        std::stringstream ss;
        ss << hashWidthMatch;
        // Stream out
        int64_t hashWidth = 0;
        ss >> hashWidth;

        // Check for valid hash value
        if (hashWidth != 32 && hashWidth != 64 && hashWidth != 128)
            return;

        // Build data
        inja::json pageData;
        pageData["page_title"] = "Hash any string";
        pageData["hash_width"] = hashWidth;

        // Render content
        res.set_content(env.render(tpl, pageData), "text/html");
    });

    // Request the hashing process
    srv.Post("/H(\\d+)", [&](const httplib::Request& req, httplib::Response& res){
        // Load the index template
        inja::Environment env;
        inja::Template tpl = env.parse_template("./templates/result.tpl.html");

        // Get hashing width
        auto hashWidthMatch = req.matches[1];
        
        // Stream in
        std::stringstream css;
        css << hashWidthMatch;
        // Stream out
        int64_t hashWidth = 0;
        css >> hashWidth;

        // Get input
        std::string userStr = req.get_param_value("string_to_hash");

        // Setup string stream for hashing
        std::stringstream ss;

        // Perform hashing
        switch (hashWidth)
        {
            case 32:
            {
                auto hash = XXH32(userStr.c_str(), userStr.length(), 0);
                ss << std::hex << std::setw(8) << std::setfill('0') << hash;
                break;
            }
            case 64:
            {
                auto hash = XXH64(userStr.c_str(), userStr.length(), 0);
                ss << std::hex << std::setw(16) << std::setfill('0') << hash;
                break;
            }
            case 128:
            {
                auto hash = XXH128(userStr.c_str(), userStr.length(), 0);
                ss << std::hex << std::setw(16) << std::setfill('0') 
                    << hash.low64 << hash.high64;
                break;
            }

            // Stop on any other width
            default:
                return;
        }

        // Build data
        inja::json pageData;
        pageData["page_title"] = "Your hash result!";
        pageData["hash_width"] = hashWidth;
        pageData["string_hash"] = ss.str();

        // Render content
        res.set_content(env.render(tpl, pageData), "text/html");
    });

    // Stop the server when the user access /stop
    srv.Get("/stop", [&](const httplib::Request& req, httplib::Response& res){
        srv.stop();
        res.set_redirect("/");
    });

    // Listen server to port
    srv.listen(
        conf.get("server_ip", "0.0.0.0").asCString(), 
        conf.get("server_port", 443).asInt()
    );

    return 0;
}
