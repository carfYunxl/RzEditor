#pragma once

#include <QPlainTextEdit>

namespace RzLib
{
    class RzServer;

    class LineNumberArea;

    class RzTextEdit : public QPlainTextEdit
    {
    public:
        RzTextEdit(QWidget* parent = nullptr, RzServer* server = nullptr);
        ~RzTextEdit();

        void lineNumberAreaPaintEvent(QPaintEvent* event);
        int lineNumberAreaWidth();

    protected:
        virtual void keyPressEvent(QKeyEvent* event) override;
        virtual void mousePressEvent(QMouseEvent* event) override;
        virtual void resizeEvent(QResizeEvent* event) override;

    private slots:
        void updateLineNumberAreaWidth(int newBlockCount);
        void highlightCurrentLine();
        void updateLineNumberArea(const QRect& rect, int dy);

    private:
        void ProcessKeyTab();
        void ProcessKeyReturn();
        void ProcessKeyBackspace();
    private:
        RzServer*       m_pServer;
        size_t          m_Cnt{0};
        QString         m_sInput;
        size_t          m_lineWidth;
        QWidget*        lineNumberArea;
    };

    class LineNumberArea : public QWidget
    {
    public:
        LineNumberArea(RzTextEdit* editor) : QWidget(editor), codeEditor(editor)
        {}

        QSize sizeHint() const override
        {
            return QSize(codeEditor->lineNumberAreaWidth(), 0);
        }

    protected:
        void paintEvent(QPaintEvent* event) override
        {
            codeEditor->lineNumberAreaPaintEvent(event);
        }

    private:
        RzTextEdit* codeEditor;
    };

}

