#include <fstream>
#include <iostream>
#include <string>
#include <vector>


struct Point
{
    unsigned long long i = 0;
    unsigned long long j = 0;
};


int main(int argc, const char * argv[])
{
    if (argc != 4)
    {
        std::cout << "Usage: diff basefile newfile diffPath\n";
        return 1;
    }
    
    std::ifstream baseFile(argv[1], std::ifstream::binary | std::ifstream::ate);
    std::ifstream updFile(argv[2], std::ifstream::binary | std::ifstream::ate);

    if (!baseFile.is_open())
    {
        std::cout << "base file not found\n";
        return 1;
    }

    if (!updFile.is_open())
    {
        std::cout << "new file not found\n";
        return 1;
    }

    std::streamsize baseSize = baseFile.tellg();
    std::streamsize updSize = updFile.tellg();
    std::vector<char> base(baseSize);
    std::vector<char> upd(updSize);

    baseFile.seekg(0);
    updFile.seekg(0);
    baseFile.read(base.data(), baseSize);
    updFile.read(upd.data(), updSize);
    baseFile.close();
    updFile.close();

    std::vector<std::vector<unsigned long long>> dist(baseSize + 1);

    // динамика, ищем расстояние Левенштейна между строками
    for (unsigned long long i = 0; i <= baseSize; ++i)
    {
        dist[i].resize(updSize + 1);
        for (unsigned long long j = 0; j <= updSize; ++j)
        {
            if (i == 0 && j == 0)
                dist[i][j] = 0;
            else if (i == 0)
                dist[i][j] = j;
            else if (j == 0)
                dist[i][j] = i;
            else
                dist[i][j] = std::min({
                    dist[i][j-1] + 1,
                    dist[i-1][j] + 1,
                    dist[i-1][j-1] + (int)(base[i-1] != upd[j-1])
                });
        }
    }

//    for (size_t i = 0; i <= baseSize; ++i)
//    {
//        for (size_t j = 0; j <= updSize; ++j)
//        {
//            std::cout << dist[i][j] << " ";
//        }
//        std::cout << std::endl;
//    }
    
    // воостанавливаем путь модификации
    std::stack<Point> path;
    unsigned long long i = baseSize;
    unsigned long long j = updSize;
    
    path.push({i, j});
    while (i > 0 || j > 0)
    {
        if (i == 0)
        {
            path.push({i, j-1});
            --j;
        }
        else if (j == 0)
        {
            path.push({i-1, j});
            --i;
        }
        else
        {
            unsigned long long target = std::min({
                dist[i][j-1] + 1,
                dist[i-1][j] + 1,
                dist[i-1][j-1] + (base[i-1] != upd[j-1])
            });
            
            if (target == dist[i-1][j-1] + (base[i-1] != upd[j-1]))
            {
                path.push({i-1, j-1});
                --i;
                --j;
            }
            else if (target == dist[i][j-1] + 1)
            {
                path.push({i, j-1});
                --j;
            }
            else if (target == dist[i-1][j] + 1)
            {
                path.push({i-1, j});
                --i;
            }
        }
    }
    
    // из пути получаем изменения и записываем в файл
    Point curPoint = path.top();
    Point prevPoint = path.top();
    path.pop();
    
    bool modifyingIntervalStarted = false;
    unsigned long long startModifyingInterval = 0;
    unsigned long long endModifyingInterval = 0;
    std::vector<char> newFragment;
    
    std::ofstream diffFile(argv[3], std::ofstream::binary);
    if (!diffFile.is_open())
    {
        std::cout << "Creating diffs file error" << std::endl;
        return 1;
    }
    
    while (!path.empty())
    {
        prevPoint = curPoint;
        curPoint = path.top();
        path.pop();
        
        if (curPoint.i == prevPoint.i + 1 && curPoint.j == prevPoint.j + 1
            && base[curPoint.i-1] == upd[curPoint.j-1])
        {
            if (modifyingIntervalStarted)
            {
                diffFile << '#' << startModifyingInterval << '#' << endModifyingInterval
                         << '#' << newFragment.size() << '#';
                diffFile.write(newFragment.data(), newFragment.size());
                diffFile << std::endl;
                modifyingIntervalStarted = false;
                newFragment.clear();
            }
           std::cout << "COMMON " << base[curPoint.i-1] << std::endl;
        }
        else
        {
            if (!modifyingIntervalStarted)
            {
                modifyingIntervalStarted = true;
                startModifyingInterval = curPoint.i;
                endModifyingInterval = startModifyingInterval;
            }
            if (curPoint.i == prevPoint.i + 1 && curPoint.j == prevPoint.j + 1
                     && base[curPoint.i-1] != upd[curPoint.j-1])
            {
                std::cout << "CHANGE " << base[curPoint.i-1] << "->" << upd[curPoint.j-1] << std::endl;
                endModifyingInterval = curPoint.i;
                newFragment.push_back(upd[curPoint.j-1]);
            }
            else if (curPoint.i == prevPoint.i + 1)
            {
                std::cout << "DELETE " << base[curPoint.i-1] << std::endl;
                endModifyingInterval = curPoint.i - 1;
            }
            else if (curPoint.j == prevPoint.j + 1)
            {
                std::cout << "ADD " << upd[curPoint.j-1] << std::endl;
                newFragment.push_back(upd[curPoint.j-1]);
            }
        }
    }

    if (modifyingIntervalStarted)
    {
        diffFile << '#' << startModifyingInterval << '#' << endModifyingInterval
                    << '#' << newFragment.size() << '#';
        diffFile.write(newFragment.data(), newFragment.size());
        diffFile << std::endl;
        modifyingIntervalStarted = false;
        newFragment.clear();
    }
    
    diffFile << '?';
    diffFile.close();
    
    return 0;
}
