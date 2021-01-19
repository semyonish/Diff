#include <fstream>
#include <iostream>
#include <string>
#include <vector>


int main(int argc, const char * argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: diff basefile diffPath\n";
        return 1;
    }
    
    std::ifstream baseFile(argv[1], std::ifstream::binary | std::ifstream::ate);
    std::ifstream diffFile(argv[2], std::ifstream::binary);

    if (!baseFile.is_open())
    {
        std::cout << "base not found\n";
        return 1;
    }

    if (!diffFile.is_open())
    {
        std::cout << "diff not found\n";
        return 1;
    }
    
    std::streamsize baseSize = baseFile.tellg();
    std::vector<char> base(baseSize);

    baseFile.seekg(0);
    baseFile.read(base.data(), baseSize);
    baseFile.close();
    
    std::ofstream outBaseFile(argv[1], std::ofstream::binary | std::ifstream::ate);
    
    unsigned long long index = 0;
    bool intervalReaded = false;
    bool diffsEnded = false;
    char startSymbol;
    unsigned long long  start = 0, end = 0, size = 0;
    std::vector<char> fragment;
    
    while (index <= baseSize) {
        if (!intervalReaded && !diffsEnded)
        {
            diffFile >> startSymbol;
            if (startSymbol == '#')
            {
                char u;
                diffFile >> start >> u >> end >> u >> size >> u;
                fragment.resize(size);
                diffFile.read(fragment.data(), size);
                intervalReaded = true;
            }
            else if (startSymbol == '?')
            {
                diffsEnded = true;
            }
            else
            {
                std::cout << "Error parsing file" << std::endl;
                break;
            }
        }
        
        if (intervalReaded)
        {
            if (index < start)
            {
                outBaseFile.write(&(base[index]), 1);
            }
            if (index == start)
            {
                outBaseFile.write(fragment.data(), size);
            }
            if (index == end)
            {
                intervalReaded = false;
                outBaseFile.write(&(base[index]), 1);
            }
        }
        else if (diffsEnded)
        {
            outBaseFile.write(&base[index], 1);
        }
        ++index;
    }
    
    outBaseFile.close();
    diffFile.close();

    return 0;
}
