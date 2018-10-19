#ifndef MYTHREAD_H
#define MYTHREAD_H
#include <QThread>
#include "Packdef.h"
class QTcpSocket;
class MyThread:public QThread
{
    Q_OBJECT
public:
    MyThread();
    void Reply(QTcpSocket *socket,const DATA_PACKAGE & pack);
    void test();
signals:
    void tests();
};

#endif // MYTHREAD_H
