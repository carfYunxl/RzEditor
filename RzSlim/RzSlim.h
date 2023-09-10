#pragma once

#include <QtWidgets/QWidget>
#include "ui_RzSlim.h"
#include "RzCore/Core.hpp"
#include "RzCore/Log.hpp"
#include "RzContrls/RzTextEdit.hpp"

namespace RzLib 
{
    class RzServer;
    class RzTextEdit;

    class RzSlim : public QWidget
    {
        Q_OBJECT

    public:
        RzSlim(QWidget* parent = nullptr);
        ~RzSlim();

        void AppendText(const QString& sText);
        void InsertText(const QString& sText);

        void Log_NextLine(LogLevel level, const QString& list);
        void Log_ThisLine(LogLevel level, const QString& list);
    private:
        void SetTextColor(LogLevel level);

    private:
        Ui::RzSlimClass ui;
        RzServer* server;
        RzTextEdit* m_pTextEdit;
    };
}
