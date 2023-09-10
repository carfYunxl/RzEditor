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
				// ɾ����ǰ��
				QTextCursor cursor = textCursor();
				cursor.movePosition(QTextCursor::End);
				cursor.select(QTextCursor::LineUnderCursor);
				cursor.removeSelectedText();
				setTextCursor(cursor);

				// �Զ���ȫ����
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

				//�ٴ�ӡһ�������Ϣ
				m_pServer->GetUI()->Log_ThisLine(LogLevel::INFO, "$ ");
				m_pServer->GetUI()->Log_ThisLine(LogLevel::NORMAL, sTab);

				i++;
				break;
			}
			case Qt::Key_Return:
			{
				// ��ȡ��ǰ�е���������
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
						// �����óɸ�ģʽ����ζ�Ž�������ȡ�������붼Ӧ�����͸�client
						// ������һ���˳�ָ��
						if (sInput == QUIT)
						{
							m_pServer->SetInputMode(InputMode::CONSOLE);
						}

						// ���͸�client
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
			//�����³�tab������������������ʹ��»�ȡ������ֶ�
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
