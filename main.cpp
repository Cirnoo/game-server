#include <QCoreApplication>
#include "data.h"
#include "tcpnet.h"
#include "Packdef.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Data data;
    return a.exec();
}
