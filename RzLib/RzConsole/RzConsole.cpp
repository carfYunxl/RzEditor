#include "pch.h"
#include "RzConsole/RzConsole.hpp"

namespace RzLib
{
    void RzConsole::ErrorExit(LPSTR lpszMessage)
    {
        fprintf(stderr, "%s\n", lpszMessage);

        // Restore input mode on exit.

        SetConsoleMode(hStdin, fdwSaveOldMode);

        ExitProcess(0);
    }
    void RzConsole::KeyEventProc(KEY_EVENT_RECORD ker)
    {
        if (ker.wVirtualKeyCode == VK_TAB)
        {
            printf("key tab is pressed\n");
        }
        else
        {
            WriteConsoleInput(hStdin,irInBuf,128, &cNumRead);
        }
    }
    void RzConsole::MouseEventProc(MOUSE_EVENT_RECORD mer)
    {
#ifndef MOUSE_HWHEELED
#define MOUSE_HWHEELED 0x0008
#endif
        printf("Mouse event: ");

        switch (mer.dwEventFlags)
        {
        case 0:

            if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
            {
                printf("left button press \n");
            }
            else if (mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
            {
                printf("right button press \n");
            }
            else
            {
                printf("button press\n");
            }
            break;
        case DOUBLE_CLICK:
            printf("double click\n");
            break;
        case MOUSE_HWHEELED:
            printf("horizontal mouse wheel\n");
            break;
        case MOUSE_MOVED:
            printf("mouse moved\n");
            break;
        case MOUSE_WHEELED:
            printf("vertical mouse wheel\n");
            break;
        default:
            printf("unknown\n");
            break;
        }
    }
    void RzConsole::ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD wbsr)
    {
        printf("Resize event\n");
        printf("Console screen buffer is %d columns by %d rows.\n", wbsr.dwSize.X, wbsr.dwSize.Y);
    }

    void RzConsole::BeginEventLoop()
    {
        // Get the standard input handle.
        hStdin = GetStdHandle(STD_INPUT_HANDLE);
        if (hStdin == INVALID_HANDLE_VALUE)
        {
            ErrorExit(const_cast<char*>(static_cast<const char*>("GetStdHandle")));
        }

        // Save the current input mode, to be restored on exit.

        if (!GetConsoleMode(hStdin, &fdwSaveOldMode))
        {

            ErrorExit(const_cast<char*>(static_cast<const char*>("GetConsoleMode")));
        }

        // Enable the window and mouse input events.

        fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
        if (!SetConsoleMode(hStdin, fdwMode))
            ErrorExit(const_cast<char*>(static_cast<const char*>("SetConsoleMode")));

        // Loop to read and handle the next 100 input events.
    }
    void RzConsole::ProcessEvent()
    {
        // Wait for the events.
        if (!ReadConsoleInput(
            hStdin,      // input buffer handle
            irInBuf,     // buffer to read into
            128,         // size of read buffer
            &cNumRead)) // number of records read
            ErrorExit(const_cast<char*>(static_cast<const char*>("ReadConsoleInput")));

        // Dispatch the events to the appropriate handler.
        for (DWORD i = 0; i < cNumRead; i++)
        {
            switch (irInBuf[i].EventType)
            {
                case KEY_EVENT: // keyboard input
                    KeyEventProc(irInBuf[i].Event.KeyEvent);
                    break;

                case MOUSE_EVENT: // mouse input
                    MouseEventProc(irInBuf[i].Event.MouseEvent);
                    break;

                case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing
                    ResizeEventProc(irInBuf[i].Event.WindowBufferSizeEvent);
                    break;

                case FOCUS_EVENT:  // disregard focus events

                case MENU_EVENT:   // disregard menu events
                    break;

                default:
                {
                    ErrorExit(const_cast<char*>(static_cast<const char*>("Unknown event type")));
                    break;
                }
            }
        }
    }

    void RzConsole::EndEventLoop()
    {
        SetConsoleMode(hStdin, fdwSaveOldMode);
    }
}
