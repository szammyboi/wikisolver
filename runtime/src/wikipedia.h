#include <string>
#include <unordered_map>
#include <vector>

// Article contains all of a pages information
struct Article
{
    uint32_t id;
    std::string title;
    std::vector<uint32_t> links;
};

// Singleton Design Structure
// There only needs to be one instance of the solver that contains all of the data
class WikipediaSolver
{
public:
    WikipediaSolver(const WikipediaSolver&) = delete;

    static WikipediaSolver& Get();

    static void LoadData(const std::string& filepath);

    static std::vector<const Article*> SearchTitle(const std::string& search_string, int limit);
    
    static std::vector<const Article*> FindPathBFS(const std::string& from, const std::string& to);
    static std::vector<const Article*> FindPathIDDFS(const std::string& from, const std::string& to);
private:
    WikipediaSolver() = default;

    void LoadDataImpl(const std::string& filepath);
    std::vector<const Article*> FindPathBFSImpl(uint32_t from, uint32_t to);
    std::vector<const Article*> FindPathIDDFSImpl(uint32_t from, uint32_t to);
    std::vector<const Article*> DepthLimitedSearch(uint32_t from, uint32_t to, int limit);
private:
    uint32_t m_Vertices = 0;
    uint32_t m_Edges = 0;
    uint32_t m_MaxID = 0;
    std::unordered_map<uint32_t, Article> m_Graph;
    std::vector<std::pair<std::string, uint32_t>> m_Titles;
};