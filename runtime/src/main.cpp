#include "wikipedia.h"
#include <iostream>
#include <chrono>

int main()
{
    using namespace std::chrono;

    WikipediaSolver::LoadData("data_collection/data.bin");
    std::cout << "Density: " << WikipediaSolver::Density() << std::endl;

    auto start = high_resolution_clock::now();
    std::vector<const Article*> articles = WikipediaSolver::FindPath("scooby doo mys", "icarly");
    auto end = high_resolution_clock::now();
    
    std::cout << "From " << articles[0]->title << " to " << articles[articles.size()-1]->title << std::endl;
    int i = 1;
    for (auto& article : articles)
        std::cout << i++ << ". " << article->title << std::endl;
    std::cout << "Execution Time: " << duration_cast<milliseconds>(end-start).count() << "ms" << std::endl;
    
}