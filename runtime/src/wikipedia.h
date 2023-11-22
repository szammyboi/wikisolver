#include <string>
#include <unordered_map>
#include <vector>

struct SearchResult
{
    std::string title;
    uint32_t id;
    std::string url;
};  

class WikipediaSolver
{
public:
    WikipediaSolver(const WikipediaSolver&) = delete;

    static WikipediaSolver& Get();
    static void LoadData(const std::string& filepath);
    static std::vector<SearchResult> SearchTitle(const std::string& search_string);
    static std::vector<std::string> FindPath(const std::string& from, const std::string& to);
    static std::vector<std::string> RandomRun();
private:
    WikipediaSolver() = default;
    void LoadDataImpl(const std::string& filepath);
    std::vector<std::string> GetTitlesImpl(std::vector<uint32_t>& ids);
    std::vector<SearchResult> SearchTitleImpl(const std::string& search_string);
    std::vector<uint32_t> FindPathImpl(uint32_t from, uint32_t to);
    std::vector<std::string> RandomRunImpl();
private:
    std::unordered_map<uint32_t, std::vector<uint32_t>> m_Graph;
    uint32_t m_MaxID = 0;
};