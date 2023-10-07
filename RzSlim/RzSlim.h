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

        void        Log_NextLine(LogLevel level, const QString& list);
        void        Log_ThisLine(LogLevel level, const QString& list);

        RzTextEdit* GetMainEdit() { return m_pTextEdit; }
        QLabel*     GetModeLabel() { return m_ModeLabel; }
        QLabel*     GetStateLabel() { return m_statuslabel; }

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
