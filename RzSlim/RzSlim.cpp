#include "RzSlim.h"

#include "RzServer/RzServer.hpp"
#include "RzUtility/Utility.hpp"
#include <QResizeEvent>

namespace RzLib {

    RzSlim::RzSlim(QWidget* parent)
        : QWidget(parent)
    {
        ui.setupUi(this);

        setWindowTitle("RzSlim_Server : 8080");
        resize(600,800);

        m_server = new RzServer(this, "127.0.0.1", 8080);

        m_pTextEdit = new RzTextEdit(this, m_server);
        m_pTextEdit->setParent(this);
        m_pTextEdit->resize(this->width(), this->height() - HEIGHT);
        m_pTextEdit->setFont(QFont("Consolas", 11));
        m_pTextEdit->setStyleSheet("background-color:rgb(0,0,0)");

        m_statusbar = new QStatusBar(this);
        m_statusbar->move(0, this->height() - HEIGHT);
        m_statusbar->resize(this->width(), HEIGHT);
        m_statusbar->setStyleSheet("background-color:rgb(0,0,0)");
        m_statusbar->show();

        m_statuslabel = new QLabel("Hello", this);
        m_statuslabel->show();
        m_statuslabel->setText("World! -------------------------------");
        m_statuslabel->setFont(QFont("Consolas", 10));
        m_statuslabel->setStyleSheet("background-color:rgb(0,0,0);"
                             "color:rgb(255,255,255)");

        m_ModeLabel = new QLabel("Console", this);
        m_ModeLabel->show();
        m_ModeLabel->setFont(QFont("Consolas", 10));
        m_ModeLabel->setStyleSheet("background-color:rgb(0,0,0);"
            "color:rgb(255,255,255)");

        m_statusbar->addWidget(m_statuslabel,1);
        m_statusbar->addWidget(m_ModeLabel,0);

        m_server->Start();
    }

    RzSlim::~RzSlim()
    {
        if (m_server)
            delete m_server;
        m_server = nullptr;
    }

    void RzSlim::SetTextColor(LogLevel level)
    {
        QTextCharFormat fmt;
        switch (level)
        {
            case LogLevel::INFO:
            {
                fmt.setForeground(Qt::green);
                break;
            }
            case LogLevel::WARN:
            {
                fmt.setForeground(Qt::yellow);
                break;
            }
            case LogLevel::ERR:
            {
                fmt.setForeground(Qt::red);
                break;
            }
            case LogLevel::CONSOLE:
            {
                fmt.setForeground(Qt::gray);
                break;
            }
            case LogLevel::NORMAL:
            {
                fmt.setForeground(Qt::white);
                break;
            }
        }
        this->m_pTextEdit->mergeCurrentCharFormat(fmt);
    }

    void RzSlim::Log_NextLine(LogLevel level, const QString& list)
    {      
        SetTextColor(level);
        this->m_pTextEdit->appendPlainText(list);
    }

    void RzSlim::Log_ThisLine(LogLevel level, const QString& list)
    {
        SetTextColor(level);
        this->m_pTextEdit->insertPlainText(list);
    }

    void RzSlim::resizeEvent(QResizeEvent* event)
    {
        m_pTextEdit->resize(event->size().width(), event->size().height() - HEIGHT);

        m_statusbar->move(0, event->size().height() - HEIGHT);
        m_statusbar->resize(event->size().width(), HEIGHT);
    }
}