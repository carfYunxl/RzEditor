#pragma once

#include <QTextEdit>

namespace RzLib
{
    class RzServer;

    class RzTextEdit : public QTextEdit
    {
    public:
        explicit RzTextEdit(RzServer* parent = nullptr)
            : m_pServer(parent)
        {}
        ~RzTextEdit() {}
    protected:
        virtual void keyPressEvent(QKeyEvent* event) override;

    private:
        RzServer* m_pServer;
    };
}

