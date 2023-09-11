#include "RzContrls/RzTextEdit.hpp"
#include "RzSlim.h"
#include "RzCore/Log.hpp"
#include "RzCMD/ConsoleCMDParser.hpp"
#include "RzServer/RzServer.hpp"
#include <QKeyEvent>
#include <QDebug>
#include <QTextDocument>
#include <QTextBlock>
#include <QMouseEvent>

namespace RzLib
{
    void RzTextEdit::keyPressEvent(QKeyEvent* event)
    {
		switch (event->key())
		{
			case Qt::Key_Tab:
			{
				ProcessKeyTab();
				break;
			}
			case Qt::Key_Return:
			{
				ProcessKeyReturn();
				break;
			}
			case Qt::Key_Backspace:
			{
				ProcessKeyBackspace();
				break;
			}
			default:
			{	
				insertPlainText(event->text());
				break;
			}
		}

		if (event->key() != Qt::Key_Tab)
		{
			//若按下除tab键以外的其他按键，就从新获取输入的字段
			QTextDocument* doc = document();
			QTextBlock tb = doc->findBlockByLineNumber(doc->lineCount() - 1);
			m_sInput = tb.text();
			
			if (m_sInput.left(2) == "$ ")
			{
				m_sInput = m_sInput.right(m_sInput.size() - 2);
			}
		}
    }

	void RzTextEdit::ProcessKeyTab()
	{
		if (m_sInput.isEmpty())
			return;

		QVector<QString> tabVec;

		// 删除当前行
		QTextCursor cursor = textCursor();
		cursor.movePosition(QTextCursor::End);
		cursor.select(QTextCursor::LineUnderCursor);
		cursor.removeSelectedText();
		setTextCursor(cursor);

		//找到当前要补全的字段
		int rIndex = m_sInput.lastIndexOf(" ");
		QString sFind;
		QString sPre;
		if (rIndex != std::string::npos)
		{
			sFind = m_sInput.right(m_sInput.size() - rIndex - 1);
		}
		sPre = m_sInput.left(rIndex + 1);

		// 自动补全功能
		for (const auto& dir_entry : std::filesystem::recursive_directory_iterator{ m_pServer->GetCurrentDir() })
		{
			if (dir_entry.path().filename().string().starts_with(sFind.toStdString()))
			{
				tabVec.push_back(dir_entry.path().filename().string().c_str());
			}
		}
		QString sTab;
		if (tabVec.empty())
		{
			sTab = m_sInput;
		}
		else
		{
			m_Cnt %= tabVec.size();
			sTab = tabVec[m_Cnt];
		}

		//再打印一次相关信息
		m_pServer->GetUI()->Log_ThisLine(LogLevel::INFO, "$ ");
		m_pServer->GetUI()->Log_ThisLine(LogLevel::NORMAL, sPre);
		m_pServer->GetUI()->Log_ThisLine(LogLevel::NORMAL, sTab);

		m_Cnt++;
	}

	void RzTextEdit::ProcessKeyReturn()
	{
		// 读取当前行的输入内容
		QTextDocument* doc = document();
		QTextBlock tb = doc->findBlockByLineNumber(doc->lineCount() - 1);

		QString sInput = tb.text();

		if (sInput.left(2) == "$ ")
		{
			sInput = sInput.right(sInput.size() - 2);
		}

		ConsoleCMDParser parser(m_pServer);
		parser.SetCMD(sInput.toStdString());

		switch (m_pServer->GetInputMode())
		{
			case InputMode::CONSOLE:
			{
				parser.SetCMD(sInput.toStdString());
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
				if (sInput.toStdString() == QUIT)
				{
					m_pServer->SetInputMode(InputMode::CONSOLE);
				}

				// 发送给client
				m_pServer->SendInfo(TCP_CMD::NORMAL, sInput.toStdString());
				break;
			}
		}

		append("");
		m_pServer->PrintConsoleHeader(m_pServer->GetCurrentDir().string());
	}

	void RzTextEdit::ProcessKeyBackspace()
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
	}

	void RzTextEdit::mousePressEvent(QMouseEvent* event)
	{
		//屏蔽鼠标点击事件
		if (event->buttons() == Qt::LeftButton)
		{
			return;
		}
	}
}
