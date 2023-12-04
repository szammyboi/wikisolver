#include <string>
#include <unordered_map>
#include <vector>

// map vs unordered_map debate

struct SearchResult
{
    std::string title;
    uint32_t id;
    std::string url;
};  

struct Article
{
    uint32_t id;
    std::string title;
    std::vector<uint32_t> links;
};

class WikipediaSolver
{
public:
    WikipediaSolver(const WikipediaSolver&) = delete;

    static WikipediaSolver& Get();
    static void LoadData(const std::string& filepath);

    static std::vector<const Article*> SearchTitle(const std::string& search_string, int limit);
    static std::vector<const Article*> FindPath(const std::string& from, const std::string& to);
    static float Density();
private:
    WikipediaSolver() = default;
    void LoadDataImpl(const std::string& filepath);
    std::vector<const Article*> FindPathImpl(uint32_t from, uint32_t to);
private:
    uint32_t m_Vertices = 0;
    uint32_t m_Edges = 0;
    uint32_t m_MaxID = 0;
    std::unordered_map<uint32_t, Article> m_Graph;
    std::vector<std::pair<std::string, uint32_t>> m_Titles;
};