#include <iostream>
#include <windows.h>
#include <filesystem>

int main()
{
    //��TmpĿ¼�µ��ļ����ᵽ����Ŀ¼�£���Ҫ���滻.exe�����ų�֮�����Ӹ����ļ�
    std::filesystem::path pathDst = std::filesystem::current_path();

    std::filesystem::path pathSrc = pathDst / "Tmp" / "RzClient.exe";

    if (!std::filesystem::exists(pathSrc))
    {
        std::cout << "Source file not exists!" << std::endl;
        return 0;
    }

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

    return 0;
}
