#include "mythread.h"
#include <QTcpSocket>
#include <QMetaType>
MyThread::MyThread()
{
    qRegisterMetaType<DATA_PACKAGE>("DATA_PACKAGE");
}

void MyThread::Reply(QTcpSocket * socket, const DATA_PACKAGE &pack)
{
    socket->write((char *)&pack,sizeof (pack));
}

void MyThread::test()
{
    PRINT("test");
}
