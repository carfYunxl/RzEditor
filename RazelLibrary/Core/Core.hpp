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
    EXIT = 0,   // �˳�
    CLIENT,     // �г����пͻ��˵�SOCKET
    FILE,       // ��ͻ��˷����ļ�
    SEND,       // ��ͻ��˷�����Ϣ
    UNKNOWN
};

enum class ClientCMD
{
    NORMAL = 0,
    IP,
    PORT
};
