#include "tcpnet.h"
#include "Packdef.h"
#include <QDebug>
#include "data.h"
#include "mythread.h"
TCPNet::TCPNet(Data *d)
{
    server = new QTcpServer(this);
    //socket = new QTcpSocket(this);
    server->listen(QHostAddress::Any,_DEF_SERVERPORT);//监听的端口号
    PRINT("等待连接...")

    my_thread_reply=new MyThread;
    qthread_reply=new QThread;
    my_thread_reply->moveToThread(qthread_reply);
    qthread_reply->start();
    connect(my_thread_reply, &MyThread::ReplySignal,my_thread_reply,&MyThread::Reply);
    connect(server,&QTcpServer::newConnection, this,&TCPNet::newConnection);
    data=d;
}

TCPNet::~TCPNet()
{

}

void TCPNet::SendData(QTcpSocket *socket,const DATA_PACKAGE & pack)
{
    socket->write((char *)&pack,sizeof (pack));
}

void TCPNet::Broadcast(const DATA_PACKAGE &pack)
{
    for(auto i=user_map.begin();i!=user_map.end();i++)
    {
        i.key()->write((char *)&pack,sizeof (pack));
    }
}

void TCPNet::newConnection()
{
    socket=server->nextPendingConnection();
    if(!user_map.count(socket))
    {
        PRINT("新的连接...")
        CLIENT_INFO info;
        info.ip=socket->peerAddress().toString().toStdString();
        info.port=socket->peerPort();
        user_map[socket]=(info);
        PRINT(info.ip<<"::"<<info.port)
    }
    DATA_PACKAGE pack;
    connect(socket,&QTcpSocket::readyRead,this,&TCPNet::readMessage);
}

void TCPNet::readMessage()
{
    DATA_PACKAGE pack;
    socket->read((char*)&pack,sizeof (DATA_PACKAGE));
    data->DealMS(socket,pack);
}

void TCPNet::PrintInfo(QTcpSocket *s)
{
    qDebug()<<s->peerAddress().toString()<<"::"<<s->peerPort();
}
