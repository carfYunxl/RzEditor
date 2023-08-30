#include <iostream>
#include <windows.h>
#include <filesystem>
#include <fstream>
#include "RzFile/FileTraveler.hpp"

int main()
{
    //把Tmp目录下的文件，搬到运行目录下，主要是替换.exe，不排除之后增加各种文件
    std::filesystem::path pathDst = std::filesystem::current_path();

    std::filesystem::path pathSrc = pathDst / "RzServer.exe";

    if (!std::filesystem::exists(pathSrc))
    {
        std::cout << "Source file not exists!" << std::endl;
        return 0;
    }
#if 0
    pathDst /= "RzClient.exe";

    if (std::filesystem::exists(pathDst))
    {
        std::filesystem::remove(pathDst);

        if (!std::filesystem::exists(pathDst))
        {
            std::cout << "remove success!" << std::endl;
        }
    }

    std::filesystem::copy_file(pathSrc, pathDst);

    if (std::filesystem::exists(pathDst))
    {
        std::cout << "success!" << std::endl;
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Start the child process. 
    if (!CreateProcess(NULL,    // No module name (use command line)
        (LPWSTR)(pathDst.wstring().c_str()),    // Command line
        NULL,                   // Process handle not inheritable
        NULL,                   // Thread handle not inheritable
        FALSE,                  // Set handle inheritance to FALSE
        0,                      // No creation flags
        NULL,                   // Use parent's environment block
        NULL,                   // Use parent's starting directory 
        &si,                    // Pointer to STARTUPINFO structure
        &pi)                    // Pointer to PROCESS_INFORMATION structure
        )
    {
        std::cout << "CreateProcess failed, error code : " << std::endl;
        return 0;
    }

    std::cout << "CreateProcess success" << std::endl;

    // Wait until child process exits.
    //WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#endif
#if 0
    std::ifstream inf(pathSrc.string(), std::ifstream::binary);
    if (inf)
    {
        std::cout << "open success!" << std::endl;

        inf.seekg(0, std::ios::end);
        size_t size = inf.tellg();

        char* buf = new char[size];
        memset(buf, 0, size);

        if (size != -1)
        {
            inf.seekg(0,std::ios::beg);
            inf.read(buf, size);
            inf.close();

            std::ofstream of("1.exe", std::ofstream::binary);
            if (of)
            {
                of.write(buf, size);
                of.flush();
                of.close();
                std::cout << "open success!" << std::endl;
            }
        }
    }
#endif

#if 0
    std::ifstream inf(pathSrc.string(), std::ifstream::in | std::ifstream::binary);
    if (inf)
    {
        std::cout << "open success!" << std::endl;

        inf.seekg(0, std::ios::end);
        size_t size = inf.tellg();

        if (size != -1)
        {
            std::string str;
            str.resize(size);

            inf.seekg(0, std::ios::beg);
            inf.read(&str[0], size);
            inf.close();

            std::ofstream of(pathSrc.string(), std::ofstream::out | std::ofstream::binary);
            if (of)
            {
                of.write(&str[0], size);
                of.flush();
                of.close();
                std::cout << "open success!" << std::endl;
            }
        }
    }
#endif

#if 0
    std::ifstream inf(pathSrc.string(), std::ifstream::binary);
    if (inf)
    {
        std::cout << "open success!" << std::endl;

    auto buf{inf.rdbuf()};

    std::ofstream out("1.exe", std::ofstream::binary);
    if (out)
    {
        out << buf;
        std::cout << "open success!" << std::endl;
    }

    inf.close();
    out.close();
#endif

    RzLib::FileTraveler file(pathSrc.string());
    if (file.open(RzLib::Mode::Binary))
    {
        file.close();
    }

    std::cin.get();
    return 0;
}
