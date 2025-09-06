//
// Created by Владимир on 10.08.2025.
//

#ifndef BIBAPROJ_PROJECTPATH_H
#define BIBAPROJ_PROJECTPATH_H

#include <filesystem>

class ProjectPath
{
    static std::filesystem::path findProjectRoot()
    {
        namespace fs = std::filesystem;
        fs::path current = fs::current_path();

        while (true)
        {
            if (exists(current / "CMakeLists.txt") && is_regular_file(current / "CMakeLists.txt"))
                return current;

            if (current.has_parent_path())
                current = current.parent_path();
            else
                return fs::current_path();
        }
    }

public:
    static const std::filesystem::path root;
};

#endif //BIBAPROJ_PROJECTPATH_H
