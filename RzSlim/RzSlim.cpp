#include "RzSlim.h"

#include "RzServer/RzServer.hpp"
#include "RzUtility/Utility.hpp"

namespace RzLib {

    RzSlim::RzSlim(QWidget* parent)
        : QWidget(parent)
    {
        ui.setupUi(this);

        this->setWindowTitle("RzSlim_Server : 8080");

        RzLib::RzServer server(this,"127.0.0.1", 8080);

        server.Start();
    }

    RzSlim::~RzSlim()
    {

    }

    template<typename...Args>
    void RzSlim::Log(LogLevel level, Args...args)
    {
        switch (level)
        {
        case LogLevel::INFO:
        {
            this->ui.m_pTextEdit->setTextColor(Qt::white);
            break;
        }
        case LogLevel::WARN:
            this->ui.m_pTextEdit->setTextColor(Qt::yellow);
            break;
        case LogLevel::ERR:
            this->ui.m_pTextEdit->setTextColor(Qt::red);
            break;
        case LogLevel::CONSOLE:
            this->ui.m_pTextEdit->setTextColor(Qt::gray);
            break;
        case LogLevel::NORMAL:
            this->ui.m_pTextEdit->setTextColor(Qt::green);
            break;
        }

        this->ui.m_pTextEdit->insertPlainText(QString(title.c_str()));
    }

    void RzSlim::Print(LogLevel level, const std::string& title)
    {
        switch(level)
        {
            case LogLevel::INFO:
            {
                this->ui.m_pTextEdit->setTextColor(Qt::white);
                break;
            }
            case LogLevel::WARN:
                this->ui.m_pTextEdit->setTextColor(Qt::yellow);
                break;
            case LogLevel::ERR:
                this->ui.m_pTextEdit->setTextColor(Qt::red);
                break;
            case LogLevel::CONSOLE:
                this->ui.m_pTextEdit->setTextColor(Qt::gray);
                break;
            case LogLevel::NORMAL:
                this->ui.m_pTextEdit->setTextColor(Qt::green);
                break;
        }

        this->ui.m_pTextEdit->insertPlainText(QString(title.c_str()));
    }

    void RzSlim::AppendText(const QString& sText)
    {
        this->ui.m_pTextEdit->append(sText);
    }

    void RzSlim::InsertText(const QString& sText)
    {
        this->ui.m_pTextEdit->insertPlainText(sText);
    }
}