#pragma once

#include <QTextEdit>

namespace RzLib
{
    class RzServer;

    class RzTextEdit : public QTextEdit
    {
    public:
        explicit RzTextEdit(RzServer* parent = nullptr);
        ~RzTextEdit();
    protected:
        virtual void keyPressEvent(QKeyEvent* event) override;
        virtual void mousePressEvent(QMouseEvent* event) override;
        virtual void paintEvent(QPaintEvent* event) override;

    private:
        void ProcessKeyTab();
        void ProcessKeyReturn();
        void ProcessKeyBackspace();
    private:
        RzServer*       m_pServer;
        size_t          m_Cnt{0};
        QString         m_sInput;
    };
}

