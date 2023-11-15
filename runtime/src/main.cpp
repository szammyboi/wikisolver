#include <yaml-cpp/yaml.h>
#include <iostream>
#include <cassert>
#include <unordered_map>
#include <map>
#include <fstream>
#include <vector>

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

        for (int i = 0; i < link_count; i++)
            stream.read((char*)&data[from_id][i], sizeof(uint32_t));
    }

    std::cout << "Loaded all data!" << std::endl;
}