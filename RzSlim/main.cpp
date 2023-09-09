#include "RzSlim.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RzLib::RzSlim w;
    w.show();
    return a.exec();
}
