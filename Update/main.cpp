#include "Classes/OlpIO.hpp"

int main()
{
    RzLib::OverLappedIO io;
    if (!io.Init())
    {
        return -1;
    }

    if (!io.Run())
    {
        return -1;
    }

    return 0;
}
