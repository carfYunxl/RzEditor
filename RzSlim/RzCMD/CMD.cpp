#include "pch.h"
#include "CMD.hpp"
#include "RzUtility/utility.hpp"

namespace RzLib
{
	CMD::CMD(CONSOLE_CMD cmd, RzServer* server)
		: m_Cmd(cmd), m_Server(server)
	{
		//...
	}
}
