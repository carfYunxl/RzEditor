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

		unsigned short size = cmd[2] << 8 | cmd[1];

		if (size == 0)
		{
			return;
		}

		m_msg.resize(size);
		std::copy(&cmd[3], &cmd[3+size], m_msg.begin());

		return;
	}
}
