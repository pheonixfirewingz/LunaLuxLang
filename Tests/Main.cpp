#include <filesystem>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

void TraverseDirectory(const fs::path &directoryPath, std::vector<fs::path> &filePaths)
{
    for (const auto &entry : fs::directory_iterator(directoryPath))
    {

        if (entry.is_regular_file())
            filePaths.push_back(entry.path());
        else if (entry.is_directory())
            TraverseDirectory(entry.path(), filePaths);
    }
}

int main()
{
    fs::path directoryPath(fs::path(PROJECT_SOURCE_DIR).append("Tests/sources"));
    std::vector<fs::path> filePaths;

    if (fs::exists(directoryPath) && fs::is_directory(directoryPath))
    {
        TraverseDirectory(directoryPath, filePaths);
        for (const auto &absolutePath : filePaths)
        {
            std::stringstream stream;
            stream << "./LunaLuxLang ";
            stream << absolutePath;
            stream << " -n_cpp";
            if (std::system(stream.str().c_str()) != 0)
            {
                puts("test failed:");
                puts(stream.str().c_str());
                puts("testRunner failed");
                return 1;
            }
        }
    }
    else
    {
        std::cerr << "Invalid directory path." << std::endl;
    }

    return 0;
}
