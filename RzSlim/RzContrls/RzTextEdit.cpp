#include "RzContrls/RzTextEdit.hpp"
#include "RzSlim.h"
#include <QKeyEvent>
#include "RzCore/Log.hpp"
#include <QDebug>
#include <QTextDocument>
#include <QTextBlock>
#include "RzCMD/ConsoleCMDParser.hpp"
#include "RzServer/RzServer.hpp"

namespace RzLib
{
	static int i = 0;
	std::string sInput;

    void RzTextEdit::keyPressEvent(QKeyEvent* event)
    {
		QVector<QString> tabVec;

		switch (event->key())
		{
			case Qt::Key_Tab:
			{
				if (sInput.empty())
					break;
				// 删除当前行
				QTextCursor cursor = textCursor();
				cursor.movePosition(QTextCursor::End);
				cursor.select(QTextCursor::LineUnderCursor);
				cursor.removeSelectedText();
				setTextCursor(cursor);

				// 自动补全功能
				for (const auto& dir_entry : std::filesystem::recursive_directory_iterator{m_pServer->GetCurrentDir()})
				{
					if (dir_entry.path().filename().string().starts_with(sInput))
					{	
						tabVec.push_back(dir_entry.path().filename().string().c_str());
					}
				}
				QString sTab;
				if (tabVec.empty())
				{
					sTab = QString(sInput.c_str());
				}
				else
				{
					i %= tabVec.size();
					sTab = tabVec[i];
				}

				//再打印一次相关信息
				m_pServer->GetUI()->Log_ThisLine(LogLevel::INFO, "$ ");
				m_pServer->GetUI()->Log_ThisLine(LogLevel::NORMAL, sTab);

				i++;
				break;
			}
			case Qt::Key_Return:
			{
				// 读取当前行的输入内容
				QTextDocument* doc = document();
				QTextBlock tb = doc->findBlockByLineNumber(doc->lineCount() - 1);

				std::string sInput = tb.text().toStdString();

				if (sInput.find("$ ") != std::string::npos)
				{
					sInput = sInput.substr(2, sInput.size() - 2);
				}
				
				ConsoleCMDParser parser(m_pServer);
				parser.SetCMD(sInput);

				switch (m_pServer->GetInputMode())
				{
					case InputMode::CONSOLE:
					{
						parser.SetCMD(sInput);
						parser.RunCmd();
						break;
					}
					case InputMode::SELECT:
					{
						//TO DO
						break;
					}
					case InputMode::SEND:
					{
						// 被设置成该模式，意味着接下来获取到的输入都应当发送给client
						// 或者是一条退出指令
						if (sInput == QUIT)
						{
							m_pServer->SetInputMode(InputMode::CONSOLE);
						}

						// 发送给client
						m_pServer->SendInfo(TCP_CMD::NORMAL, sInput);
						break;
					}
				}

				append("");
				m_pServer->PrintConsoleHeader(m_pServer->GetCurrentDir().string());
				break;
			}
			case Qt::Key_Backspace:
			{
				QTextCursor cursor = textCursor();
				cursor.movePosition(QTextCursor::End);
				cursor.select(QTextCursor::LineUnderCursor);
				QString strSelect = cursor.selectedText();
				if (strSelect != "$ ")
				{
					cursor.clearSelection();
					cursor.deletePreviousChar();
					setTextCursor(cursor);
				}
				break;
			}
			default:
			{		
				break;
			}
		}

		if ( event->key() >= 0x30 && event->key() <= 0x7F )
		{
			insertPlainText(event->text());
		}

		if (event->key() != Qt::Key_Tab)
		{
			//若按下除tab键以外的其他按键，就从新获取输入的字段
			QTextDocument* doc = document();
			QTextBlock tb = doc->findBlockByLineNumber(doc->lineCount() - 1);
			sInput = tb.text().toStdString();

			if (sInput.find("$ ") != std::string::npos)
			{
				sInput = sInput.substr(2, sInput.size() - 2);
			}
		}
    }
}
