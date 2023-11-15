#include <yaml-cpp/yaml.h>
#include <iostream>
#include <cassert>
#include <unordered_map>
#include <map>

struct Page
{
    //std::string title;
    int id;
};

class Graph {
public:
    Graph() = default;
    ~Graph() = default;

    int Size();
    void AddPage(int id);
private:
    std::unordered_map<int, Page> m_Graph;
    unsigned int m_VertexCount = 0;
};

void Graph::AddPage(int id)
{
    // should do my attempt to insert ting
    m_Graph[id] = Page{};
    m_VertexCount += 1;
}

int Graph::Size()
{
    return m_VertexCount;
}

void PrintType(YAML::Node node)
{
    switch (node.Type()) {
        case YAML::NodeType::Null: 
            std::cout << "NULL" << std::endl;
            break;
        case YAML::NodeType::Scalar:
            std::cout << "SCALAR" << std::endl;
            break;
        case YAML::NodeType::Sequence:
            std::cout << "SEQUENCE" << std::endl;
            break;
        case YAML::NodeType::Map:
            std::cout << "MAP" << std::endl;
            break;
        case YAML::NodeType::Undefined:
            std::cout << "UNDEFINED" << std::endl;
            break;
        default:
            break;
    }
}

int main()
{
    Graph graph;
    YAML::Node config = YAML::LoadFile("data2.yaml");
    //auto map = config.as<std::map<std::string, YAML::Node>>();
    std::cout << "BRUH" << config.size() << std::endl;

    for(YAML::const_iterator it=config.begin(); it != config.end();++it) {
        std::string key = it->first.as<std::string>();       // <- key
        graph.AddPage(it->second["pageid"].as<int>());
    }

    std::cout << graph.Size() << std::endl;
}