#pragma once

namespace RzLib
{
    struct CMDBase
    {
    protected:
        size_t m_CmdId;
    };

    struct CMDSingle : public CMDBase
    {

    };
}
