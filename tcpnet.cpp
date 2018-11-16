#include "tcpnet.h"
#include "Packdef.h"
#include <QDebug>
#include "data.h"
#include "mythread.h"
#include "clientmanager.h"
#include "roominfo.h"
TCPNet::TCPNet(Data & d):data(d)
{
    server = new QTcpServer(this);
    server->listen(QHostAddress::Any,_DEF_SERVERPORT);//监听的端口号
    PRINT("等待连接...")

    //my_thread_reply=new MyThread;
    //qthread_reply=new QThread;
    //my_thread_reply->moveToThread(qthread_reply);
    //qthread_reply->start();
    //connect(my_thread_reply, &MyThread::ReplySignal,my_thread_reply,&MyThread::Reply);
    connect(server,&QTcpServer::newConnection, this,&TCPNet::newConnection);
}

TCPNet::~TCPNet()
{

}

void TCPNet::SendData(QTcpSocket *socket,const DATA_PACKAGE & pack) const
{
    socket->write((char *)&pack,sizeof (pack));
}

void TCPNet::SendDataToRoom(const RoomInfo & room, const DATA_PACKAGE & pack) const
{
    for(const auto i:room.socket_arr)
    {
        SendData(i,pack);
    }
}

void TCPNet::SendMS(QTcpSocket * socket, const MS_TYPE ms_tp) const
{
    DATA_PACKAGE pack;
    pack.ms_type=ms_tp;
    SendData(socket,pack);
}



void TCPNet::newConnection() const
{
    auto client_socket=server->nextPendingConnection();
    new ClientManager(client_socket,data);
    data.NewConnection(client_socket);
    //connect(socket,&QTcpSocket::readyRead,this,&TCPNet::readMessage);
}


void TCPNet::PrintInfo(QTcpSocket *s)
{
    qDebug()<<s->peerAddress().toString()<<"::"<<s->peerPort();
}
