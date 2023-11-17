#include <yaml-cpp/yaml.h>
#include <iostream>
#include <cassert>
#include <unordered_map>
#include <map>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <bitset>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

std::pair<std::string, uint32_t> SearchPage(const std::string& title)
{
    auto params = cpr::Parameters{
        {"q", title},
    };

    //https://en.wikipedia.org/w/api.php?action=query&list=search&srsearch=Nelson%20Mandela&utf8=&format=json&redirects=resolve
    cpr::Response r = cpr::Get(cpr::Url{"https://api.wikimedia.org/core/v1/wikipedia/simple/search/title"}, params);
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

std::vector<uint32_t> FindPath(std::map<uint32_t, std::vector<uint32_t>>& data, uint32_t from, uint32_t to)
{
    // maybe optimized visited to be o(1    )
    std::queue<uint32_t> queue;
	std::set<uint32_t> visited;
    std::map<uint32_t, std::pair<uint32_t, uint32_t>> info;

    queue.push(from);
    visited.insert(from);
    info[from] = {-1, 0};

    int depth = 0;
	while (!queue.empty())
	{
        bool found = false;
		int length = queue.size();
		while (length--) {
			uint32_t current = queue.front();
			queue.pop();
			if (current == to) {
                found = true;
                break;
            }

			for (uint32_t link : data[current])
            {
                if (visited.find(link) != visited.end()) continue;
                queue.push(link);
                visited.insert(link); 
                info[link] = {info[current].first+1, current};
            }
		}
        if (found) {
            std::vector<uint32_t> path;
            uint32_t cc = to;
            for (int i = 0; i <= depth; i++)
            {   
                path.push_back(cc);
                cc = info[cc].second;
            }
            std::reverse(path.begin(), path.end());
            return path;
        }
		depth++;
	}
    return std::vector<uint32_t>();
}

std::map<uint32_t, std::string> GetTitles(std::vector<uint32_t>& ids)
{
    std::map<uint32_t, std::string> result;
    std::string pageids = std::to_string(ids[0]);
    for (int i = 1; i < ids.size(); i++)
    {
        pageids += "|";
        pageids += std::to_string(ids[i]);
    }

    auto params = cpr::Parameters{
        {"action", "query"},
        {"pageids", pageids},
        {"format", "json"},
        {"formatversion", "2"}
    };

    cpr::Response r = cpr::Get(cpr::Url{"https://simple.wikipedia.org/w/api.php"}, params);
    auto json = nlohmann::json::parse(r.text);

    for (auto entry : json["query"]["pages"])
    {
        uint32_t id = entry["pageid"].get<uint32_t>();
        result[id] = entry["title"].get<std::string>();
    }

    return result;
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

    std::string from;
    std::getline(std::cin, from);
    std::string to;
    std::getline(std::cin, to);

    auto [title, id] = SearchPage(from);
    auto [title2, id2] = SearchPage(to);

    auto start = std::chrono::high_resolution_clock::now();
    auto path = FindPath(data, id, id2);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Path Tracing Took: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms" << std::endl;
    
    std::map<uint32_t, std::string> titles = GetTitles(path);
    for (uint32_t id : path)
    {
        std::cout << "> " << titles[id] << std::endl;
    }
}