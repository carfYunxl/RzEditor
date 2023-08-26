/*****************************************************************//**
 * \file   Core.hpp
 * \brief  core definition header
 * 
 * \author yun
 * \date   August 2023
 *********************************************************************/

#pragma once

#ifdef RZ_PALTFORM_WINDOWS
    #ifdef RZ_DYNAMIC_LINK
        #ifdef RZ_BUILD_DLL
        #define RzAPI __declspec(dllexport)
        #else
        #define RzAPI __declspec(dllimport)
        #endif
    #else
        #define RzAPI
    #endif
#else
    #error Razel only support Windows!
#endif

// CMD
enum class CONSOLE_CMD
{
    UNKNOWN = 0,    // δ֪CMD
    EXIT,           // �˳�
    CLIENT,         // �г����пͻ��˵�SOCKET
    VERSION,        // �ͻ��˰汾��Ϣ
    SEND
};

enum class TCP_CMD
{
    NORMAL = 0xF1,
    UPDATE = 0xF3,
    FILE_HEADER = 0xF5,
    FILE_PACKET = 0xF6,
    File_TAIL = 0xF7
};

enum class CMDType
{
    NONE = 0,
    FUNC,
    TRANSFER,
};

