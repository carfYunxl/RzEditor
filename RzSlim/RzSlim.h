#pragma once

#include <QtWidgets/QWidget>
#include <QStatusBar>
#include "ui_RzSlim.h"
#include "RzCore/Core.hpp"
#include "RzCore/Log.hpp"
#include "RzContrls/RzTextEdit.hpp"
#include <QLabel>

namespace RzLib 
{
    class RzServer;
    class RzTextEdit;
    enum class InputMode;

    constexpr size_t HEIGHT = 30;

    class RzSlim : public QWidget
    {
        Q_OBJECT

    public:
        RzSlim(QWidget* parent = nullptr);
        ~RzSlim();

        void AppendText(const QString& sText);
        void InsertText(const QString& sText);

        void ChangeMode(InputMode mode);

        void Log_NextLine(LogLevel level, const QString& list);
        void Log_ThisLine(LogLevel level, const QString& list);

        void InsertStatus(const QString& sStatus);

        void Clear() { m_pTextEdit->clear(); }

    protected:
        virtual void resizeEvent(QResizeEvent* event) override;
    private:
        void SetTextColor(LogLevel level);

    private:
        Ui::RzSlimClass ui;
        RzServer*       m_server;
        RzTextEdit*     m_pTextEdit;
        QStatusBar*     m_statusbar;
        QLabel*         m_statuslabel;
        QLabel*         m_ModeLabel;
    };
}
