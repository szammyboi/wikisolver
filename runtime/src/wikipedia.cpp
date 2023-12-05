#include "wikipedia.h"

#include <levenshtein-sse.hpp>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

#include <iostream>
#include <fstream>

#include <thread>
#include <future>

#include <queue>
#include <stack>

// Get the singleton instance
WikipediaSolver& WikipediaSolver::Get()
{
    static WikipediaSolver instance;
    return instance;
}

// Load the data into the singleton
void WikipediaSolver::LoadData(const std::string& filepath)
{
    WikipediaSolver& instance = Get();
    instance.LoadDataImpl(filepath);
}

// Implementation of data loading
void WikipediaSolver::LoadDataImpl(const std::string& filepath)
{
    // Load the file into a binary stream
    std::ifstream stream(filepath, std::ios::binary);

    // Read in the first 4 bytes (the total number of pages)
    uint32_t total_count;
    stream.read((char*)&total_count, sizeof(uint32_t));

    // Allocate the graph size to fit all the pages and then iterate for each page
    m_Graph.reserve(total_count);
    for (int _ = 0; _ < total_count; _++)
    {
        m_Vertices++;

        uint32_t from_id;
        uint32_t title_length;
        uint32_t link_count;

        // Read in the id and title length for the current page
        stream.read((char*)&from_id, sizeof(uint32_t));
        stream.read((char*)&title_length, sizeof(uint32_t));

        // Add a new page to the graph
        m_Graph[from_id] = Article{from_id};

        // Allocate room in the string for its length
        std::string& title = m_Graph[from_id].title;
        title.resize(title_length);

        // Read the bytes into the string
        for (int i = 0; i < title_length; i++)
            stream.read(&title.data()[i], sizeof(char));


        // Remove underscores and replace them with spaces
        std::replace(title.begin(), title.end(), '_', ' ');

        // Read in the link count and allocate the links array to the size
        stream.read((char*)&link_count, sizeof(uint32_t));
        m_Graph[from_id].links = std::vector<uint32_t>(link_count);

        m_Edges += link_count;

        // Read all the link bytes directly into the link array
        stream.read((char*)&m_Graph[from_id].links[0], sizeof(uint32_t)*link_count);

        if (from_id > m_MaxID) m_MaxID = from_id;

        // Add the string and id to the title array for searching
        m_Titles.emplace_back(title, from_id);
    }

    // If all the data was not loaded throw an error
    if (m_Graph.size() != total_count)
        throw std::runtime_error("Failed to load in all data!");
}

// Searches for the best [limit] matches in the titles
std::vector<const Article*> WikipediaSolver::SearchTitle(const std::string& search_string, int limit)
{
    std::vector<const Article*> result;

    typedef std::pair<float, uint32_t> Score;
    std::priority_queue<Score> queue;

    auto lowercase_search = std::string(search_string);
    std::transform(lowercase_search.begin(), lowercase_search.end(), lowercase_search.begin(),
    [](unsigned char c){ return std::tolower(c); });

    WikipediaSolver& instance = Get();

    // Iterate through each title
    // If the first character matches [both forced into lowercase]
    // Then calculate the levenshtein distance between the search and current title
    // Add the title to a heap which keeps track of the [limit] lowest scores
    for (auto& pair : instance.m_Titles)
    {
        if (std::tolower(pair.first[0]) != lowercase_search[0]) continue;
        if (pair.first.length() < search_string.length()) continue;

        std::string lowercase_title = std::string(pair.first);
        std::transform(lowercase_title.begin(), lowercase_title.end(), lowercase_title.begin(),
        [](unsigned char c){ return std::tolower(c); });

        float score = (float)levenshteinSSE::levenshtein(lowercase_search.begin(), lowercase_search.end(), lowercase_title.begin(), lowercase_title.begin() + search_string.size());
        if (queue.size() == limit && score > queue.top().first)
            continue;
        queue.emplace(score, pair.second);
        if (queue.size() > limit)
            queue.pop();
    }

    // Iterate through each title in the heap and add 
    // the corresponding title to the result array
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

// BFS Search Implementation
std::vector<const Article*> WikipediaSolver::FindPathBFSImpl(uint32_t from, uint32_t to)
{
    std::vector<const Article*> path;

    std::queue<uint32_t> queue;
    std::vector<bool> visited(m_MaxID+1);

    // Keeps track of where a vertex got added from (to find the path later)
    std::unordered_map<uint32_t, uint32_t> backtrack;

    // Start at the from vertex
    queue.push(from);
    visited[from] = 1;
    backtrack[from] = {0};

    // Iterate through each depth of the graph starting from the from vertex
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

            // Add each link of the current vertex to the queue
            // Add where it came from to the backtrack map
			for (uint32_t link : m_Graph[current].links)
            {
                if (visited[link]) continue;
                queue.push(link);
                visited[link] = true;
                backtrack[link] = current;
            }
		}
        depth++;
	}

    if (!found) return path;

    // Use the backtrack map to retrace the BFS' steps
    // And insert the path into a vector (in reverse)
    path.resize(depth);
    uint32_t current = to;
    for (int i = 1; i <= depth; i++)
    {   
        path[depth-i] = &m_Graph.at(current);
        current = backtrack[current];
    }

    return path;
}

// Static Function to Run the BFS
std::vector<const Article*> WikipediaSolver::FindPathBFS(const std::string& from, const std::string& to)
{
    WikipediaSolver& instance = Get();

    // Search for the two inputs matching articles async
    std::future<std::vector<const Article*>> from_results_async = std::async(SearchTitle, from, 5);
    std::future<std::vector<const Article*>> to_results_async = std::async(SearchTitle, to, 5);

    std::vector<const Article*> from_results = from_results_async.get();
    std::vector<const Article*> to_results = to_results_async.get();

    if (from_results.size() == 0 || to_results.size() == 0) throw std::runtime_error("Invalid Search!");
    
    // Call the BFS impl on the two closest articles
    return instance.FindPathBFSImpl(from_results[0]->id, to_results[0]->id);
}


std::vector<const Article*> WikipediaSolver::DepthLimitedSearch(uint32_t from, uint32_t to, int limit)
{
    std::vector<const Article*> result;

    bool found = false;
    std::stack<uint32_t> stack;
    std::vector<bool> visited(m_MaxID+1);

    // Keeps track of a vertex's depth and previous vertex (for backtracking)
    std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> info;

    // Simple DFS algorithm on a graph
    // Start at from and add each child to a stack
    // Pull the top and repeat
    // However, if a vertex that's being added is too deep, it will not be appended
    stack.push(from);
    visited[from] = true;
    info[from] = {0, 0};

    while (!stack.empty())
    {
        uint32_t current = stack.top();
        stack.pop();

        if (current == to) {
            found = true;
            break;
        }

        for (uint32_t link : m_Graph[current].links)
        {
            // If the vertex is unvisited and not too deep, add it to the stack
            if (!visited[link] && info[current].first+1 <limit) {
                stack.push(link);
                visited[link] = 1;
                info[link] = {info[current].first+1, current};
            }
        }
    }
    
    // Use the info map to retrace the BFS' steps
    // And insert the path into a vector (in reverse)
    if (found) {
        result.resize(limit);
        uint32_t current = to;
        for (int i = 1; i <= limit; i++)
        {   
            result[limit-i] = &m_Graph.at(current);
            current = info[current].second;
        }
    }

    return result;
}

// Implementation of the IDDFS Algorithm
std::vector<const Article*> WikipediaSolver::FindPathIDDFSImpl(uint32_t from, uint32_t to)
{
    // Increase the limit and call a DFS up to that limit each iteration
    // Max depth is 10
    for (int i = 0; i < 10; i++)
    {
        auto res = DepthLimitedSearch(from, to, i);
        if (res.size() > 0) return res;
    }

   return std::vector<const Article*>();
}


// Static Function to Run the IDDFS
std::vector<const Article*> WikipediaSolver::FindPathIDDFS(const std::string& from, const std::string& to)
{
    WikipediaSolver& instance = Get();

    // Search for the two inputs matching articles async
    std::future<std::vector<const Article*>> from_results_async = std::async(SearchTitle, from, 5);
    std::future<std::vector<const Article*>> to_results_async = std::async(SearchTitle, to, 5);

    std::vector<const Article*> from_results = from_results_async.get();
    std::vector<const Article*> to_results = to_results_async.get();

    if (from_results.size() == 0 || to_results.size() == 0) throw std::runtime_error("Invalid Search!");

    // Call the IDDFS impl on the two closest articles
    return instance.FindPathIDDFSImpl(from_results[0]->id, to_results[0]->id);
}