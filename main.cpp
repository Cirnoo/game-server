#include <QCoreApplication>
#include "data.h"
#include "tcpnet.h"
#include "Packdef.h"
#include <QString>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Data data;
    return a.exec();
}
