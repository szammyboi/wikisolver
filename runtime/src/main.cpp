#include <yaml-cpp/yaml.h>
#include <iostream>
#include <cassert>
#include <unordered_map>
#include <map>
#include <fstream>
#include <vector>
#include <string>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

std::pair<std::string, uint32_t> SearchPage(const std::string& title)
{
    auto params = cpr::Parameters{
        {"q", title},
    };

    //https://en.wikipedia.org/w/api.php?action=query&list=search&srsearch=Nelson%20Mandela&utf8=&format=json&redirects=resolve
    cpr::Response r = cpr::Get(cpr::Url{"https://api.wikimedia.org/core/v1/wikipedia/simple/search/title"}, params);
    std::cout << r.url << std::endl;
    auto json = nlohmann::json::parse(r.text);
    std::string real_title = json["pages"][0]["key"].get<std::string>();
    uint32_t id = json["pages"][0]["id"].get<uint32_t>();
    return {real_title, id};
}

void PrintPageTitle(uint32_t pageid)
{
    auto params = cpr::Parameters{
        {"action", "query"},
        {"pageids", std::to_string(pageid)},
        {"format", "json"},
        {"formatversion", "2"}
    };

    cpr::Response r = cpr::Get(cpr::Url{"https://simple.wikipedia.org/w/api.php"}, params);
    auto json = nlohmann::json::parse(r.text);
    std::cout << json["query"]["pages"][0]["title"].get<std::string>() << std::endl;
}

int main()
{
    std::map<uint32_t, std::vector<uint32_t>> data;
    std::ifstream stream("data_collection/data.bin", std::ios::binary);
    uint32_t total_count;
    stream.read((char*)&total_count, sizeof(uint32_t));

    std::cout << total_count << " pages" << std::endl;

    while (total_count--)
    {
        uint32_t from_id;
        uint32_t link_count;
        stream.read((char*)&from_id, sizeof(uint32_t));
        stream.read((char*)&link_count, sizeof(uint32_t));
        data[from_id] = std::vector<uint32_t>();
        data[from_id].reserve(link_count);

        for (int i = 0; i < link_count; i++) {
            uint32_t value;
            stream.read((char*)&value, sizeof(uint32_t));
            data[from_id].push_back(value);
        }
           
    }

    std::cout << "Loaded all data!" << std::endl;

    
    auto [title, id] = SearchPage("scobby doo");
    std::cout << title << " links to: " << std::endl;
    for  (auto& link : data[id]) {
        std::cout << " ";
        PrintPageTitle(link);
    }
}

/*
void Search(const std::string& title)
{
    auto params = cpr::Parameters{
        {"action", "opensearch"},
        {"search", url_encode(title)},
        {"limit", "10"},
        {"format", "json"},
        {"namespace", "0"}
    };
    cpr::Response r = cpr::Get(cpr::Url{"https://simple.wikipedia.org/w/api.php"}, params);
    auto json = nlohmann::json::parse(r.text);
    std::cout << json << std::endl;
}
*/