#pragma once

#include <windows.h>
#include <stdio.h>

namespace RzLib
{
    class RzConsole
    {
    public:
        void BeginEventLoop();
        void ProcessEvent();
        void EndEventLoop();
    private:
        void ErrorExit(LPSTR);
        void KeyEventProc(KEY_EVENT_RECORD);
        void MouseEventProc(MOUSE_EVENT_RECORD);
        void ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD);

    private:
        HANDLE  hStdin;
        DWORD   fdwSaveOldMode;

        DWORD cNumRead;
        DWORD fdwMode;
        INPUT_RECORD irInBuf[128];
    };
}

