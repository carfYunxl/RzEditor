/*****************************************************************//**
 * \file   ConsoleCMDParser.cpp
 * \brief  ConsoleCMDParser class have two mainly functions:
 * 
 *	1 - tells the cmd type : trans cmd or func cmd?
 * 
 *		what is trans cmd ? 
 *			> just like : socket: msg...
 *			> 234: hello client ...
 *			> 234: file path or others ...
 *			> ...etc
 * 
 *		what is func cmd ? just like:
 *			> exit
 *			> client
 *			> ls
 *			> ...etc
 * 
 *		how to recognize this two type cmd?
 *			> our cmd parser will find whether this input cmd line have space, no means only one word input, this should be a func cmd.
 *				then we will check whether its valid
 * 
 *			> otherwise, we think its should be a trans cmd, so check valid again
 * 
 *	2 - collect important information
 *		
 *		for func cmd:
 *			> cmd type: exit / client / ls / ...
 * 
 *		for trans cmd:
 *			> socket
 *			> message or filepath or link ...etc
 * 
 * \author yun
 * \date   September 2023
 *********************************************************************/

#include "pch.h"
#include "RzCMD/CMD.hpp"
#include "ConsoleCMDParser.hpp"
#include "RzUtility/Utility.hpp"
#include "RzServer/RzServer.hpp"

namespace RzLib
{
	//------------------------------------------------------------
	// CMDParser implementation									//
	//------------------------------------------------------------

	ConsoleCMDParser::ConsoleCMDParser(const std::string& CMD, char SPLIT)
		: m_CMD(CONSOLE_CMD::UNKNOWN)
	{
		Parser(CMD, SPLIT);
	}

	void ConsoleCMDParser::SetCMD(const std::string& CMD, char SPLIT)
	{
		this->Parser(CMD, SPLIT);
	}

	void ConsoleCMDParser::Parser(const std::string& CMD, char SPLIT)
	{	
		size_t index1 = CMD.find(SPLIT, 0);

		std::string strCmd = CMD.substr(0, index1);

		m_CMD = CMD_Cast(strCmd);

		if (m_CMD == CONSOLE_CMD::UNKNOWN)
		{
			return;
		}

		// may be here will be more options
		if (m_CMD == CONSOLE_CMD::SELECT)
		{
			std::string strSocket = strCmd.substr(index1 + 1, strCmd.size() - index1 - 1);
			if (!Utility::IsAllDigits(strSocket))
			{
				return;
			}

			size_t socket = stoi(strSocket);
			if (m_Server->IsClientSocket(socket))
			{
				m_socket = socket;
			}
		}
	}


	// New CMD add here
	CONSOLE_CMD ConsoleCMDParser::CMD_Cast(const std::string& cmd)
	{
		auto it = std::find_if(g_sMapCmd.begin(), g_sMapCmd.end(), [=](const std::pair<std::string, CONSOLE_CMD>& pair) {
			return pair.first == cmd;
		});

		if (it != g_sMapCmd.end())
		{
			return (*it).second;
		}

		return CONSOLE_CMD::UNKNOWN;
	}

	void ConsoleCMDParser::RunCmd()
	{
		std::unique_ptr<CMD> pCmd;
		switch (m_CMD)
		{
		case CONSOLE_CMD::SELECT:
			pCmd = std::make_unique<SelectCMD>(m_CMD, m_Server, m_socket, m_message);
			break;
		case CONSOLE_CMD::EXIT:
			pCmd = std::make_unique<ExitCMD>(m_CMD, m_Server);
			break;
		case CONSOLE_CMD::CLIENT:
			pCmd = std::make_unique<ClientCMD>(m_CMD, m_Server);
			break;
		case CONSOLE_CMD::VERSION:
			pCmd = std::make_unique<VersionCMD>(m_CMD, m_Server);
			break;
		case CONSOLE_CMD::LS:
			pCmd = std::make_unique<LsCMD>(m_CMD, m_Server);
			break;
		case CONSOLE_CMD::CD:
			pCmd = std::make_unique<CdCMD>(m_CMD, m_Server);
			break;
		case CONSOLE_CMD::UNKNOWN:
			pCmd = nullptr;
			break;
		}
		
		if (pCmd)
			pCmd->Run();
		else
			Log(LogLevel::ERR, "unknown command!\n");
	}
}
