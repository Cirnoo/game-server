#include "clientmanager.h"
#include <QTcpSocket>
#include "data.h"
#include <QDebug>
ClientManager::ClientManager(QTcpSocket *parent,Data & _data) :
    QObject(parent),socket(parent),data(_data)
{
    connect(parent,&QTcpSocket::readyRead,this,&ClientManager::readMessage);
}


void ClientManager::readMessage()
{
    DATA_PACKAGE pack;
    socket->read((char*)&pack,sizeof (DATA_PACKAGE));
    data.DealMS(socket,pack);
}
