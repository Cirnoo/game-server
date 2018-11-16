#ifndef TCPNET_H
#define TCPNET_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QObject>
#include <Packdef.h>
class MyThread;
class Data;
class RoomInfo;
class TCPNet:public QObject
{
    Q_OBJECT
public:
    TCPNet( Data & d);
    ~TCPNet();
    void SendData(QTcpSocket *socket, const DATA_PACKAGE &pack) const;
    void SendDataToRoom(const RoomInfo & room, const DATA_PACKAGE &pack) const;
    void SendMS(QTcpSocket *socket, const MS_TYPE ms_tp) const;
private :
    void newConnection() const;
private:
    QTcpServer* server=nullptr;
    void readMessage();
    void PrintInfo(QTcpSocket * s);
    Data & data;
    MyThread * my_thread_reply;
    QThread * qthread_reply;
};

#endif // TCPNET_H
