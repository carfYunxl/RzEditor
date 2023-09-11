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
			//�����³�tab������������������ʹ��»�ȡ������ֶ�
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

		// ɾ����ǰ��
		QTextCursor cursor = textCursor();
		cursor.movePosition(QTextCursor::End);
		cursor.select(QTextCursor::LineUnderCursor);
		cursor.removeSelectedText();
		setTextCursor(cursor);

		//�ҵ���ǰҪ��ȫ���ֶ�
		int rIndex = m_sInput.lastIndexOf(" ");
		QString sFind;
		QString sPre;
		if (rIndex != std::string::npos)
		{
			sFind = m_sInput.right(m_sInput.size() - rIndex - 1);
		}
		sPre = m_sInput.left(rIndex + 1);

		// �Զ���ȫ����
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

		//�ٴ�ӡһ�������Ϣ
		m_pServer->GetUI()->Log_ThisLine(LogLevel::INFO, "$ ");
		m_pServer->GetUI()->Log_ThisLine(LogLevel::NORMAL, sPre);
		m_pServer->GetUI()->Log_ThisLine(LogLevel::NORMAL, sTab);

		m_Cnt++;
	}

	void RzTextEdit::ProcessKeyReturn()
	{
		// ��ȡ��ǰ�е���������
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
				// �����óɸ�ģʽ����ζ�Ž�������ȡ�������붼Ӧ�����͸�client
				// ������һ���˳�ָ��
				if (sInput.toStdString() == QUIT)
				{
					m_pServer->SetInputMode(InputMode::CONSOLE);
				}

				// ���͸�client
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
		//����������¼�
		if (event->buttons() == Qt::LeftButton)
		{
			return;
		}
	}
}
