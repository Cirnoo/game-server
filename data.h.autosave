#ifndef DATA_H
#define DATA_H
#include <QtSql>
#include <QVector>
#include <functional>
#include <QList>
#include "tcpnet.h"
#include "Packdef.h"
class Data
{
public:
    Data(QSqlQuery _query);
    //bool DealMS(MS_TYPE _type,USER_INFO data);
    bool DealMS(QTcpSocket *socket, DATA_PACKAGE pack);
    void SendMS(QTcpSocket *socket, DATA_PACKAGE pack);

private:
    QSqlQuery  query;
    void AddMS(MS_TYPE _type,std::function<bool (USER_INFO)> _cmd);
    void SocketInit();
    bool Login(QString name, QString password);
    bool ChangePassword(QString name, QString password);
    bool Register(QString name, QString password);
    bool AddRoom(wstring );
    TCPNet tcp;
    QTcpSocket * socket;
    QList<ROOM_INFO> room_list;
};

#endif // DATA_H
