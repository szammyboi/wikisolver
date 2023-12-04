#include "wikipedia.h"

#include <levenshtein-sse.hpp>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

#include <iostream>
#include <fstream>

#include <thread>
#include <future>

#include <queue>

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
        m_Vertices++;
        uint32_t from_id;
        uint32_t title_length;
        uint32_t link_count;

        stream.read((char*)&from_id, sizeof(uint32_t));
        stream.read((char*)&title_length, sizeof(uint32_t));

        m_Graph[from_id] = Article{from_id};

        std::string& title = m_Graph[from_id].title;
        title.resize(title_length);

        for (int i = 0; i < title_length; i++)
            stream.read(&title.data()[i], sizeof(char));

        stream.read((char*)&link_count, sizeof(uint32_t));
        m_Graph[from_id].links = std::vector<uint32_t>(link_count);

        m_Edges += link_count;

        stream.read((char*)&m_Graph[from_id].links[0], sizeof(uint32_t)*link_count);

        if (from_id > m_MaxID) m_MaxID = from_id;

        m_Titles.emplace_back(title, from_id);
    }
    // bad
    std::sort(m_Titles.begin(), m_Titles.end(), [](auto &left, auto &right) {
        return left.first < right.first;
    });

    if (m_Graph.size() != total_count)
        throw std::runtime_error("Failed to load in all data!");
}


std::vector<const Article*> WikipediaSolver::SearchTitle(const std::string& search_string, int limit)
{
    std::vector<const Article*> result;

    typedef std::pair<float, uint32_t> Score;
    std::priority_queue<Score> queue;

    auto s = std::string(search_string);
    std::transform(s.begin(), s.end(), s.begin(),
    [](unsigned char c){ return std::tolower(c); });
    WikipediaSolver& instance = Get();

    // would rather not iterate over the entire array
    // because its sorted (some sort of binary search to find first instance of the first char)
    for (auto& pair : instance.m_Titles)
    {
        if (std::tolower(pair.first[0]) != std::tolower(search_string[0])) continue;
        if (pair.first.length() < search_string.length()) continue;

        std::string lowercase_title = std::string(pair.first);
        std::transform(lowercase_title.begin(), lowercase_title.end(), lowercase_title.begin(),
        [](unsigned char c){ return std::tolower(c); });

        float score = (float)levenshteinSSE::levenshtein(search_string.begin(), search_string.end(), lowercase_title.begin(), lowercase_title.begin() + search_string.size());
        //score += 0.1*abs(((int)search_string.length())-((int)pair.first.length()));
        if (queue.size() == limit && score > queue.top().first)
            continue;
        queue.emplace(score, pair.second);
        if (queue.size() > limit)
            queue.pop();
    }

    result.resize(queue.size());

    int index = result.size()-1;
    while (!queue.empty())
    {
        auto [score, key] = queue.top();
        result[index--] = &instance.m_Graph[key];
        queue.pop();
    }

    return result;
}

std::vector<const Article*> WikipediaSolver::FindPathImpl(uint32_t from, uint32_t to)
{
    std::vector<const Article*> path;

    std::queue<uint32_t> queue;
    std::vector<bool> visited(m_MaxID+1);
    std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> info;

    queue.push(from);
    visited[from] = 1;
    info[from] = {0, 0};

    bool found = false;
    int depth = 0;
	while (!queue.empty() && !found)
	{
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

			for (uint32_t link : m_Graph[current].links)
            {
                if (visited[link] == 1) continue;
                queue.push(link);
                visited[link] = 1;
                info[link] = {info[current].first+1, current};
            }
		}
        depth++;
	}

    if (!found) return path;

    path.resize(depth);
    uint32_t current = to;
    for (int i = 1; i <= depth; i++)
    {   
        path[depth-i] = &m_Graph.at(current);
        current = info[current].second;
    }

    return path;
}

std::vector<const Article*> WikipediaSolver::FindPath(const std::string& from, const std::string& to)
{
    WikipediaSolver& instance = Get();

    std::future<std::vector<const Article*>> from_results_async = std::async(SearchTitle, from, 5);
    std::future<std::vector<const Article*>> to_results_async = std::async(SearchTitle, to, 5);

    std::vector<const Article*> from_results = from_results_async.get();
    std::vector<const Article*> to_results = to_results_async.get();

    if (from_results.size() == 0 || to_results.size() == 0) throw std::runtime_error("Invalid Search!");
    
    return instance.FindPathImpl(from_results[0]->id, to_results[0]->id);
}

float WikipediaSolver::Density()
{
    WikipediaSolver& instance = Get();
    return (float)instance.m_Edges / instance.m_Vertices;
}