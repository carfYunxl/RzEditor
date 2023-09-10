#pragma once

#include <QTextEdit>

namespace RzLib
{
    class RzSlim;

    class RzTextEdit : public QTextEdit
    {
    public:
        explicit RzTextEdit(RzSlim* parent = nullptr)
            : pParent(parent)
        {}
        ~RzTextEdit() {}
    protected:
        virtual void keyPressEvent(QKeyEvent* event) override;

    private:
        RzSlim* pParent;
    };
}

