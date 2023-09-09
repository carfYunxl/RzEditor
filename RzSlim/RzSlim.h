#pragma once

#include <QtWidgets/QWidget>
#include "ui_RzSlim.h"

namespace RzLib 
{
    enum class LogLevel
    {
        INFO = 0,
        WARN,
        ERR,
        CONSOLE,
        NORMAL
    };

    class RzSlim : public QWidget
    {
        Q_OBJECT

    public:
        RzSlim(QWidget* parent = nullptr);
        ~RzSlim();

        template<typename...Args>
        void Log(LogLevel level, Args...args);

        void Print(LogLevel level, const std::string& title);

        void AppendText(const QString& sText);
        void InsertText(const QString& sText);

    private:
        Ui::RzSlimClass ui;
    };
}
