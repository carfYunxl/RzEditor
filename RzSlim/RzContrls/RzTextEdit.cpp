#include "RzContrls/RzTextEdit.hpp"
#include "RzSlim.h"
#include <QKeyEvent>
#include "RzCore/Log.hpp"
#include <QDebug>
#include <QTextDocument>
#include <QTextBlock>
#include "RzCMD/ConsoleCMDParser.hpp"

namespace RzLib
{
    void RzTextEdit::keyPressEvent(QKeyEvent* event)
    {
        if (event->key() == Qt::Key_Tab)
        {
            // 自动补全功能
        }
        else if(event->nativeVirtualKey() == 13)
        {
            // 读取当前行的输入内容
            QTextDocument* doc = document();
            QTextBlock tb = doc->findBlockByLineNumber(doc->lineCount()-1);
            std::string sInput = tb.text().toStdString();

            ConsoleCMDParser parser();
            append("");
        }
        else
        {
            insertPlainText(event->text());
        }
    }
}
