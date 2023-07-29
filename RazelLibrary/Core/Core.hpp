#pragma once

#ifdef RZ_PALTFORM_WINDOWS
    #ifdef RZ_DYNAMIC_LINK
        #ifdef RZ_BUILD_DLL
        #define RazelAPI __declspec(dllexport)
        #else
        #define RazelAPI __declspec(dllimport)
        #endif
    #else
        #define RazelAPI
    #endif
#else
    #error Razel only support Windows!
#endif