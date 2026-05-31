#include <iostream>
#include <filesystem>

int main(int argc, char* argv[])
{
    std::cout << "CWD: " << std::filesystem::current_path().string() << "\n\n";

    std::cout << "argc: " << argc << "\n";
    for (int i = 0; i < argc; ++i)
        std::cout << "argv[" << i << "] = " << argv[i] << "\n";

    std::cout << "\nPress Enter to exit...";
    std::cin.get();
    return 0;
}