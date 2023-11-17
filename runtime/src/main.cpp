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
    std::cout << json["query"]["pages"][0]["title"] << std::endl;
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

    PrintPageTitle(1);
    std::cout << data[1].size() << std::endl;
    for (auto& link : data[1]) {
        PrintPageTitle(link);
    }
}