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

    connect(this, &TCPNet::ReplySignal,my_thread_reply,&MyThread::Reply);
    connect(my_thread_reply, &MyThread::tests,my_thread_reply,&MyThread::test);
    connect(server,&QTcpServer::newConnection, this,&TCPNet::newConnection);
    data=d;
    emit my_thread_reply->tests();
}

TCPNet::~TCPNet()
{

}

void TCPNet::SendMessage(QTcpSocket *socket,const DATA_PACKAGE & pack)
{
    emit ReplySignal(socket,pack);

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
