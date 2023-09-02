#include "CMDType.hpp"
#include "RzCore/Core.hpp"
#include "RzCore/Log.hpp"

namespace RzLib
{
    void CMDSingle::Run()
    {
		if (m_Cmd == "update")
		{
			
		}
		else
		{
			Log(LogLevel::INFO, "server say : ", m_Cmd);
		}
    }

    void CMDDouble::Run()
    {
		if (m_Cmd == "ver")
		{
			
		}
		else
		{
			Log(LogLevel::INFO, "server say : ", m_Cmd);
		}
    }

    void CMDTriple::Run()
    {
		
    }
}
