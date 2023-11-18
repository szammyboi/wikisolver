#include "wikipedia.h"

#include <iostream>
#include <vector>
#include <string>

int main()
{
    WikipediaSolver::LoadData("data_collection/data.bin");
    
    std::vector<std::string> p = WikipediaSolver::FindPath("scooby doo", "icarly");
    
    for (auto& title : p)
        std::cout << title << std::endl;
}