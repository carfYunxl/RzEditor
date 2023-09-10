#include "RzSlim.h"

#include "RzServer/RzServer.hpp"
#include "RzUtility/Utility.hpp"

namespace RzLib {

    RzSlim::RzSlim(QWidget* parent)
        : QWidget(parent)
    {
        ui.setupUi(this);

        setWindowTitle("RzSlim_Server : 8080");
        resize(600,800);

        server = new RzServer(this, "127.0.0.1", 8080);

        m_pTextEdit = new RzTextEdit(server);
        m_pTextEdit->setParent(this);
        m_pTextEdit->resize(this->width(), this->height());
        m_pTextEdit->setFont(QFont("Consolas", 11));
        m_pTextEdit->setStyleSheet("background-color:rgb(0,0,0)");
        //m_pTextEdit->setReadOnly(true);

        server->Start();
    }

    RzSlim::~RzSlim()
    {
        if (server)
            delete server;
        server = nullptr;
    }

    void RzSlim::SetTextColor(LogLevel level)
    {
        switch (level)
        {
            case LogLevel::INFO:
            {
                this->m_pTextEdit->setTextColor(Qt::green);
                break;
            }
            case LogLevel::WARN:
            {
                this->m_pTextEdit->setTextColor(Qt::yellow);
                break;
            }
            case LogLevel::ERR:
            {
                this->m_pTextEdit->setTextColor(Qt::red);
                break;
            }
            case LogLevel::CONSOLE:
            {
                this->m_pTextEdit->setTextColor(Qt::gray);
                break;
            }
            case LogLevel::NORMAL:
            {
                this->m_pTextEdit->setTextColor(Qt::white);
                break;
            }
        }
    }

    void RzSlim::Log_NextLine(LogLevel level, const QString& list)
    {      
        SetTextColor(level);
        this->m_pTextEdit->append(list);
    }

    void RzSlim::Log_ThisLine(LogLevel level, const QString& list)
    {
        SetTextColor(level);
        this->m_pTextEdit->insertPlainText(list);
    }

    void RzSlim::AppendText(const QString& sText)
    {
        this->m_pTextEdit->append(sText);
    }

    void RzSlim::InsertText(const QString& sText)
    {
        this->m_pTextEdit->insertPlainText(sText);
    }
}