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
#include "RzCore/Log.hpp"
#include <fstream>

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

		m_sCMD = CMD.substr(0, index1);

		m_CMD = CMD_Cast(m_sCMD);

		// may be here will be more options
		if (m_CMD == CONSOLE_CMD::SELECT)
		{
			std::string strSocket = CMD.substr(index1 + 1, CMD.size() - index1 - 1);
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
		else if (
			m_CMD == CONSOLE_CMD::CD ||
			m_CMD == CONSOLE_CMD::MKDIR ||
			m_CMD == CONSOLE_CMD::TOUCH ||
			m_CMD == CONSOLE_CMD::REMOVE
			)
		{
			m_message = CMD.substr(index1 + 1, CMD.size() - index1 - 1);
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

	FILE_EXTENSION ConsoleCMDParser::EXT_Cast(const std::string& ext)
	{
		auto it = std::find_if(g_sMapEXT.begin(), g_sMapEXT.end(), [=](const std::pair<std::string, FILE_EXTENSION>& pair) {
			return pair.first == ext;
			});

		if (it != g_sMapEXT.end())
		{
			return (*it).second;
		}

		return FILE_EXTENSION::UNKNOWN;
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
				pCmd = std::make_unique<CdCMD>(m_CMD, m_Server, m_message);
				break;
			case CONSOLE_CMD::MKDIR:
				pCmd = std::make_unique<MkdirCMD>(m_CMD, m_Server, m_message);
				break;
			case CONSOLE_CMD::TOUCH:
				pCmd = std::make_unique<TouchCMD>(m_CMD, m_Server, m_message);
				break;
			case CONSOLE_CMD::REMOVE:
				pCmd = std::make_unique<RmCMD>(m_CMD, m_Server, m_message);
				break;
			case CONSOLE_CMD::CLEAR:
				pCmd = std::make_unique<ClearCMD>(m_CMD, m_Server);
				break;
			case CONSOLE_CMD::UNKNOWN:
				pCmd = nullptr;
				break;
		}
		
		if (pCmd)
		{
			pCmd->Run();
			//append("");
			m_Server->PrintConsoleHeader(m_Server->GetCurrentDir().string());
		}
		else
		{
			m_Server->GetUI()->Log_NextLine(LogLevel::ERR, "unknown command!");

			//不是CMD，再检查看看是不是文件，如果是的话，尝试打开文件
			std::filesystem::path path = m_Server->GetCurrentDir() / m_sCMD;
			if (std::filesystem::exists(path) && path.has_extension())
			{
				//如果路径确实存在，且是一个具有扩展名的文件，这时候就尝试打开该文件
				FILE_EXTENSION eExt = EXT_Cast(path.extension().string());
				switch (eExt)
				{
					case FILE_EXTENSION::EXE:
					{
						//TO DO
						break;
					}
					case FILE_EXTENSION::TXT:
					{
						RzTextEdit* edit = m_Server->GetUI()->GetMainEdit();
						//首先保存当前UI的内容
						m_Server->SetUIText(std::move(edit->toPlainText()));

						//读取这个txt，将其显示到UI上
						std::ifstream inf(path, std::ios::in);
						if (inf.is_open())
						{
							inf.seekg(0, std::ios::end);
							size_t size = inf.tellg();

							if (size != -1)
							{
								std::string sRead;
								sRead.resize(size);
								inf.seekg(0, std::ios::beg);
								inf.read(&sRead[0], size);
								inf.close();

								m_Server->GetUI()->GetModeLabel()->setText("Editor");
								m_Server->SetInputMode(InputMode::EDITOR);

								edit->setStyleSheet("background-color:rgb(255,255,255);"
									"color:rgb(0,0,0)");
								edit->setText( QString::fromStdString(std::move(sRead)));

								QTextCursor cursor = edit->textCursor();

								cursor.movePosition(QTextCursor::End);

								edit->setTextCursor(cursor);
							}
						}

						break;
					}
				}
			}
			else
			{
				m_Server->PrintConsoleHeader(m_Server->GetCurrentDir().string());
			}
			return;
		}

		m_Server->GetUI()->Log_ThisLine(LogLevel::NORMAL,"");
	}
}
