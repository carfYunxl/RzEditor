#include "CMDType.hpp"
#include "Core/Core.hpp"
#include "Core/Log.hpp"

namespace RzLib
{
    void CMDSingle::Run()
    {
		if (m_Cmd == "update")
		{
			// �յ�������������update����ζ������Ҫ��ʼ���¿ͻ�����
			m_Client->Update(true);
		}
		else
		{
			Log(LogLevel::INFO, "server say : ", m_Cmd);
		}
    }

    void CMDDouble::Run()
    {
		if (m_Cmd == "ver")
		{
			if (m_SecInfo != m_Client->GetVersion())
			{
				// ���¿ͻ���
				m_Client->UpdateClient();
			}
		}
		else
		{
			Log(LogLevel::INFO, "server say : ", m_Cmd);
		}
    }

    void CMDTriple::Run()
    {
		if (m_Client->IsUpdating())
		{
			int fileSize = 0;
			if (!m_SecInfo.empty())
				fileSize = stoi(m_SecInfo);

			m_Client->RecvExe(fileSize, m_message);
		}
		else
		{
			if (m_Cmd == "file") // file size path
			{
				int fileSize = 0;
				if (!m_SecInfo.empty())
					fileSize = stoi(m_SecInfo);

				std::string filepath = m_message;
				size_t idx = filepath.rfind("\\");
				if (idx != std::string::npos)
				{
					filepath = filepath.substr(idx + 2, filepath.size() - idx - 2);
				}

				m_Client->RecvFile(fileSize, filepath);
			}
			else
			{
				Log(LogLevel::INFO, "server say : ", m_Cmd);
			}
		}
    }
}
