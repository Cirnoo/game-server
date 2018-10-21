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
signals:
    void ReplySignal(QTcpSocket *socket,const DATA_PACKAGE & pack);
};

#endif // MYTHREAD_H
