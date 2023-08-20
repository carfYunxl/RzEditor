#include "Communication.h"

namespace RzLib
{
	Communication::Communication(const char* bufCmd, int rtLen)
		: m_Cmd(0)
		, m_msg("")
    {
		Parser(bufCmd, rtLen);
	}

	void Communication::Parser(const char* cmd, int rtLen)
	{
		if (strlen(cmd) == 0)
		{
			return;
		}

		// byte[0] «cmd¿‡–Õ
		m_Cmd = cmd[0];

		unsigned short length = cmd[2] << 8 | cmd[1];

		if (length == 3)
		{
			return;
		}

		m_msg.resize(length);
		if (length > 0)
		{
			memcpy(&m_msg[0], &cmd[3], length);
		}

		return;
	}
}
