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
enum class ServerCMD
{
    EXIT = 0,   // 退出
    CLIENT,     // 列出所有客户端的SOCKET
    FILE,       // 向客户端发送文件
    SEND,       // 向客户端发送信息
    UNKNOWN,    // 未知CMD
    VERSION,    // 客户端版本信息
    PATH        // 服务端当前运行的路径
};

enum class ClientCMD
{
    NORMAL = 0,
    IP,
    PORT,
    UPDATE
};
