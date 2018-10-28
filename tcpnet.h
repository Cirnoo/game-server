#ifndef TCPNET_H
#define TCPNET_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QObject>
#include <Packdef.h>
class MyThread;
class Data;
class TCPNet:public QObject
{
    Q_OBJECT
public:
    TCPNet( Data * d);
    ~TCPNet();
    void SendData(QTcpSocket *socket, const DATA_PACKAGE &pack);
private :
    void newConnection();
private:
    QTcpServer* server=nullptr;
    QTcpSocket* socket=nullptr;
    void readMessage();
    void PrintInfo(QTcpSocket * s);
    Data * data;
    MyThread * my_thread_reply;
    QThread * qthread_reply;
};

#endif // TCPNET_H
