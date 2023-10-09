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
#include <QPainter>

namespace RzLib
{
	RzTextEdit::RzTextEdit(QWidget* parent, RzServer* server)
		: QPlainTextEdit(parent)
		, m_pServer(server)
	{
		lineNumberArea = new LineNumberArea(this);

		connect(this, &RzTextEdit::blockCountChanged, this, &RzTextEdit::updateLineNumberAreaWidth);
		connect(this, &RzTextEdit::updateRequest, this, &RzTextEdit::updateLineNumberArea);
		connect(this, &RzTextEdit::cursorPositionChanged, this, &RzTextEdit::highlightCurrentLine);

		updateLineNumberAreaWidth(0);
		highlightCurrentLine();
	}

	RzTextEdit::~RzTextEdit()
	{

	}

	int RzTextEdit::lineNumberAreaWidth()
	{
		int digits = 1;
		int max = qMax(1, blockCount());
		while (max >= 10) {
			max /= 10;
			++digits;
		}

		int space = 30 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

		return space;
	}

	void RzTextEdit::updateLineNumberAreaWidth(int /* newBlockCount */)
	{
		setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
	}

	void RzTextEdit::updateLineNumberArea(const QRect& rect, int dy)
	{
		if (dy)
			lineNumberArea->scroll(0, dy);
		else
			lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

		if (rect.contains(viewport()->rect()))
			updateLineNumberAreaWidth(0);
	}

	void RzTextEdit::resizeEvent(QResizeEvent* e)
	{
		QPlainTextEdit::resizeEvent(e);

		QRect cr = contentsRect();
		lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
	}

	void RzTextEdit::highlightCurrentLine()
	{
		QList<QTextEdit::ExtraSelection> extraSelections;

		if (!isReadOnly()) {
			QTextEdit::ExtraSelection selection;

			QColor lineColor = QColor(Qt::lightGray).darker(180);

			selection.format.setBackground(lineColor);
			selection.format.setProperty(QTextFormat::FullWidthSelection, true);
			selection.cursor = textCursor();
			selection.cursor.clearSelection();
			extraSelections.append(selection);

		}

		setExtraSelections(extraSelections);
	}

	void RzTextEdit::lineNumberAreaPaintEvent(QPaintEvent* event)
	{
		QPainter painter(lineNumberArea);
		painter.fillRect(event->rect(), Qt::black);

		//![extraAreaPaintEvent_0]

		//![extraAreaPaintEvent_1]
		QTextBlock block = firstVisibleBlock();
		int blockNumber = block.blockNumber();
		int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
		int bottom = top + qRound(blockBoundingRect(block).height());
		//![extraAreaPaintEvent_1]

		//![extraAreaPaintEvent_2]
		while (block.isValid() && top <= event->rect().bottom()) {
			if (block.isVisible() && bottom >= event->rect().top()) {
				QString number = QString::number(blockNumber + 1);
				painter.setPen(Qt::gray);
				painter.drawText(0, top, lineNumberArea->width()-10, fontMetrics().height(),
					Qt::AlignRight, number);
			}

			block = block.next();
			top = bottom;
			bottom = top + qRound(blockBoundingRect(block).height());
			++blockNumber;
		}
	}

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
			case Qt::Key_Escape:
			{
				m_pServer->SetInputMode(InputMode::CONSOLE);
				m_pServer->GetUI()->GetModeLabel()->setText("Console");

				QString sEdit = toPlainText();

				clear();
				setPlainText(m_pServer->GetUIText());

				QTextCursor cursor = textCursor();

				cursor.movePosition(QTextCursor::End);

				setTextCursor(cursor);

				QTextCharFormat fmt;
				fmt.setForeground(QBrush(Qt::gray));
				fmt.setBackground(QBrush(Qt::black));
				mergeCurrentCharFormat(fmt);

				m_pServer->PrintConsoleHeader(m_pServer->GetCurrentDir().string());


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
				parser.SetCMD( sInput.toStdString() );
				parser.RunCmd();
				break;
			}
			case InputMode::SELECT:
			{
				// �����óɸ�ģʽ����ζ�Ž�������ȡ�������붼Ӧ�����͸�client
				// ������һ���˳�ָ��
				if (sInput.toStdString() == QUIT)
				{
					m_pServer->SetInputMode(InputMode::CONSOLE);
					m_pServer->GetUI()->GetModeLabel()->setText("Console");
				}
				else
				{
					// ���͸�client
					m_pServer->SendInfo(TCP_CMD::NORMAL, sInput.toStdString());
				}
				appendPlainText("");
				m_pServer->PrintConsoleHeader(m_pServer->GetCurrentDir().string());
				break;
			}
			case InputMode::SEND:
			{
				// �����óɸ�ģʽ����ζ��Ҫ�����ļ��������Ϣ
				if (sInput.toStdString() == QUIT)
				{
					m_pServer->SetInputMode(InputMode::CONSOLE);
					m_pServer->GetUI()->GetModeLabel()->setText("Console");
					return;
				}
				break;
			}
			case InputMode::EDITOR:
			{
				break;
			}
		}
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
		QPlainTextEdit::mousePressEvent(event);

		//����������¼�
		if (event->buttons() == Qt::LeftButton)
		{
			return;
		}

	}
}
