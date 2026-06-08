#define _CRT_SECURE_NO_WARNINGS
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <windows.h>
#include <random>
#include <comutil.h>
#include <vector>
#include <iomanip>
#include <sstream>


namespace fs = std::filesystem;

std::string configFile = "universalBootstrap.ini";
std::filesystem::path defaultDir = "./";
bool debugLoggingEnabled = false; // Default value, you can change it as needed
int iniOption = 0;  // Default value, you can change it as needed
std::string exeName = "app.exe";  // Change this to your actual executable name
std::string fileFormat = "file_$<0>$"; // Define the format for renamed file
std::string fileExtension = "ica";
bool useGUID = false;

std::string fileFormatWithExtension = fileFormat + "." + fileExtension;

void parseIni() {
    std::ifstream in(configFile);
    if (!in) {
        std::ofstream out(configFile);
        out << "[Settings]" << std::endl;
        out << "WorkingDirectory=" << defaultDir << std::endl;
        out << "DebugLoggingEnabled=1" << std::endl;
        out << "IniOption=0" << std::endl;
        out << "ExeName=" << exeName << std::endl;
        out << "FileFormat=" << fileFormat << std::endl;
        out << "FileExtension=" << fileExtension << std::endl;
        out << "UseGuid=" << useGUID << std::endl;
        out.close();
    } else {
        std::string line, key, value;
        while (getline(in, line)) {
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                key = line.substr(0, pos);
                value = line.substr(pos + 1);
                if (key == "WorkingDirectory") defaultDir = std::filesystem::path(value);
                else if (key == "DebugLoggingEnabled") debugLoggingEnabled = std::stoi(value);
                else if (key == "IniOption") iniOption = std::stoi(value);
                else if (key == "ExeName") exeName = value;
                else if (key == "FileFormat") fileFormat = value;
                else if (key == "FileExtension") fileExtension = value;
                else if (key == "UseGuid") useGUID = std::stoi(value);
            }
        }
    }
}

void launchApp(std::string filePath) {
    
    std::filesystem::path exePath = std::filesystem::path(defaultDir) / exeName;
    if (!std::filesystem::exists(exePath)) {
        std::cout << "Executable not found: " << exePath.string() << std::endl;
        return;
    }
    std::string command = exeName + " " + filePath;
    
    if (debugLoggingEnabled) std::cout << "Launching command: " << command << std::endl;
    
    try
    {
        std::filesystem::current_path(defaultDir);
        
        //system(command.c_str());
        
        std::wstring wCmd = std::wstring(command.begin(), command.end());
        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi = { 0 };
        
        if (debugLoggingEnabled) {
            std::wcout << L"Launching command natively: " << wCmd << '\n';
        }
        
        if (CreateProcessW(NULL, wCmd.data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            WaitForSingleObject(pi.hProcess, INFINITE);
            
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else
        {
            std::cerr << "CreateProcess failed with error code: " << GetLastError() << '\n';
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }
}

static std::string getNewFilePath(const std::string &filePath) {

    fs::path p (filePath);

    std::string oldFileName = p.filename().string();
    p.remove_filename();
    
    std::string newFilePath = fileFormat;
    std::regex reg(R"(\$<[^<>]*>\$)");
    if (useGUID)
    {
        GUID guid;
        std::string guidString; 
        HRESULT hr = CoCreateGuid(&guid);
        if (SUCCEEDED(hr))
        {
            std::stringstream stream;
            stream << std::hex << std::uppercase
                << std::setw(8) << std::setfill('0') << guid.Data1
                << "-" << std::setw(4) << std::setfill('0') << guid.Data2
                << "-" << std::setw(4) << std::setfill('0') << guid.Data3
                << "-";
            for (int i = 0; i < sizeof(guid.Data4); ++i)
            {
                if (i == 2)
                    stream << "-";
                stream << std::hex << std::setw(2) << std::setfill('0') << (int)guid.Data4[i];
            }
            guidString = stream.str();
        }
    
        newFilePath = std::regex_replace(newFilePath, reg, guidString);
    }
    else
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> distrib(1, 999999);
        std::string rand = std::to_string(distrib(gen));
        newFilePath = std::regex_replace(newFilePath, reg, rand);
    }
    
    fs::path p2 = p / (newFilePath + "." + fileExtension);
    return p2.string();
}

void modifyIcaFile(const std::string& filePath)
{
    std::ifstream in(filePath);
    if (!in)
    {
        std::cerr << "Failed to open ICA file: " << filePath << std::endl;
        return;
    }

    std::vector<std::string> lines;
    std::string line;

    bool foundConnectionBar = false;
    bool foundDesiredColor = false;
    bool foundDesiredHRES = false;
    bool foundDesiredVRES = false;
    bool foundTWIMode = false;

    while (std::getline(in, line))
    {
        if (line.rfind("ConnectionBar=", 0) == 0)
        {
            line = "ConnectionBar=0";
            foundConnectionBar = true;
        }
        else if (line.rfind("DesiredColor=", 0) == 0)
        {
            line = "DesiredColor=32";
            foundDesiredColor = true;
        }
        else if (line.rfind("DesiredHRES=", 0) == 0)
        {
            line = "DesiredHRES=1920";
            foundDesiredHRES = true;
        }
        else if (line.rfind("DesiredVRES=", 0) == 0)
        {
            line = "DesiredVRES=1000";
            foundDesiredVRES = true;
        }
        else if (line.rfind("TWIMode=", 0) == 0)
        {
            line = "TWIMode=Off";
            foundTWIMode = true;
        }

        lines.push_back(line);
    }

    in.close();

    // Add missing settings
    if (!foundConnectionBar)
        lines.push_back("ConnectionBar=0");

    if (!foundDesiredColor)
        lines.push_back("DesiredColor=32");

    if (!foundDesiredHRES)
        lines.push_back("DesiredHRES=1920");

    if (!foundDesiredVRES)
        lines.push_back("DesiredVRES=1000");

    if (!foundTWIMode)
        lines.push_back("TWIMode=Off");

    std::ofstream out(filePath, std::ios::trunc);
    if (!out)
    {
        std::cerr << "Failed to write ICA file: " << filePath << std::endl;
        return;
    }

    for (const auto& l : lines)
    {
        out << l << '\n';
    }

    out.close();
}

int main(int argc, char* argv[]) {
    parseIni();
    
    if (!fileFormat.empty() && !fileExtension.empty())
    {
        fileFormatWithExtension = fileFormat + "." + fileExtension;
    }

    if (argc < 2 || strcmp(argv[1], "-d") == 0) 
        std::cout << "Please provide a valid ." + fileExtension +" file as an argument!" << std::endl;
    else {
        std::string argString = argv[1];
        std::string argExtension = fs::path(argString).extension().string();
        std::ranges::transform(argExtension, argExtension.begin(), ::tolower);
        if (argExtension == "." + fileExtension) {

            std::string newIcaFilePath = getNewFilePath(argString);
            
            if (debugLoggingEnabled) std::cout << "Renaming " << argString << " to " << newIcaFilePath << std::endl;

            try
            {
                std::filesystem::rename(argString, newIcaFilePath);

if (debugLoggingEnabled)
{
    std::cout << "Updating ICA settings..." << std::endl;
}

modifyIcaFile(newIcaFilePath);
            }
            catch (const fs::filesystem_error& e)
            {
                std::cout << "Failed to rename file with error: " << e.what() << '\n';
                return -1;
            }
            

            launchApp(newIcaFilePath);
        } else {
           std::cout << "Please provide a valid ." + fileExtension +" file as an argument!" << std::endl;
           return -2;
       }
    }
   
    return 0;
}
