#include "wikipedia.h"
#include <fstream>
#include <iostream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

WikipediaSolver& WikipediaSolver::Get()
{
    static WikipediaSolver instance;
    return instance;
}

void WikipediaSolver::LoadData(const std::string& filepath)
{
    WikipediaSolver& instance = Get();
    instance.LoadDataImpl(filepath);
}

void WikipediaSolver::LoadDataImpl(const std::string& filepath)
{
    std::ifstream stream(filepath, std::ios::binary);
    uint32_t total_count;
    stream.read((char*)&total_count, sizeof(uint32_t));

    m_Graph.reserve(total_count);
    for (int _ = 0; _ < total_count; _++)
    {
        uint32_t from_id;
        uint32_t link_count;
        stream.read((char*)&from_id, sizeof(uint32_t));
        stream.read((char*)&link_count, sizeof(uint32_t));
        m_Graph[from_id] = std::vector<uint32_t>();
        m_Graph[from_id].reserve(link_count);

        for (int i = 0; i < link_count; i++) {
            uint32_t value;
            stream.read((char*)&value, sizeof(uint32_t));
            m_Graph[from_id].push_back(value);
        }

        if (from_id > m_MaxID) m_MaxID = from_id;
    }

    if (m_Graph.size() != total_count)
        throw std::runtime_error("Failed to load in all data!");
}


// needs error handling
std::unordered_map<uint32_t, std::string> WikipediaSolver::GetTitlesImpl(std::vector<uint32_t>& ids)
{
    std::unordered_map<uint32_t, std::string> result;
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

std::vector<SearchResult> WikipediaSolver::SearchTitle(const std::string& search_string)
{
    WikipediaSolver& instance = Get();
    return instance.SearchTitleImpl(search_string);
}

std::vector<SearchResult> WikipediaSolver::SearchTitleImpl(const std::string& search_string)
{
    std::vector<SearchResult> result;
    auto params = cpr::Parameters{
        {"q", search_string},
    };

    //https://en.wikipedia.org/w/api.php?action=query&list=search&srsearch=Nelson%20Mandela&utf8=&format=json&redirects=resolve
    cpr::Response r = cpr::Get(cpr::Url{"https://api.wikimedia.org/core/v1/wikipedia/simple/search/title"}, params);
    auto json = nlohmann::json::parse(r.text);

    for (auto& entry : json["pages"])
    {
        std::string title = entry["title"].get<std::string>();
        std::string key = entry["key"].get<std::string>();
        std::string url = "https://simple.wikipedia.org/wiki/" + key;
        uint32_t id = entry["id"].get<uint32_t>();
        result.push_back({title, id, url});
    }

    return result;
}

std::vector<std::string> WikipediaSolver::FindPath(const std::string& from, const std::string& to)
{
    std::vector<std::string> result;
    WikipediaSolver& instance = Get();
    std::vector<SearchResult> from_results = SearchTitle(from);
    std::vector<SearchResult> to_results = SearchTitle(to);

    if (from_results.size() == 0 || to_results.size() == 0) throw std::runtime_error("Invalid Search!");
    
    std::vector<uint32_t> path = instance.FindPathImpl(from_results[0].id, to_results[0].id);
    std::unordered_map<uint32_t, std::string> titles = instance.GetTitlesImpl(path);

    for (uint32_t node : path)
        result.push_back(titles[node]);

    return result;
}

// link pseudocode
std::vector<uint32_t> WikipediaSolver::FindPathImpl(uint32_t from, uint32_t to)
{
    std::vector<uint32_t> path;

    std::queue<uint32_t> queue;
    std::vector<bool> visited(m_MaxID+1);
    std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> info;

    queue.push(from);
    visited[from] = 1;
    info[from] = {-1, 0};

    int depth = 0;
	while (!queue.empty())
	{
        bool found = false;
		int length = queue.size();
		while (length--) 
        {
			uint32_t current = queue.front();
			queue.pop();
			if (current == to) 
            {
                found = true;
                break;
            }

			for (uint32_t link : m_Graph[current])
            {
                if (visited[link] == 1) continue;
                queue.push(link);
                visited[link] = 1;
                info[link] = {info[current].first+1, current};
            }
		}
        depth++;
        if (found) break;
	}

    path.resize(depth);
    uint32_t current = to;
    for (int i = 1; i <= depth; i++)
    {   
        path[depth-i] = current;
        current = info[current].second;
    }

    return path;
}