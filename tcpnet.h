#ifndef TCPNET_H
#define TCPNET_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QObject>
#include <unordered_map>
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
    std::unordered_map<QTcpSocket *,CLIENT_INFO> user_map;
    void readMessage();
    void PrintInfo(QTcpSocket * s);
    Data * data;
    MyThread * my_thread_reply;
    QThread * qthread_reply;
};

#endif // TCPNET_H
