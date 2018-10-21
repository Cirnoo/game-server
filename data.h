#ifndef DATA_H
#define DATA_H
#include <QVector>
#include <functional>
#include <QList>
#include <string>
#include <unordered_map>
#include "tcpnet.h"
#include "Packdef.h"
#include "mysql.h"
class Data
{
public:
    Data();
    //bool DealMS(MS_TYPE _type,USER_INFO data);
    bool DealMS(QTcpSocket *m_socket, const DATA_PACKAGE &pack);
    unsigned char GetBufSize(MS_TYPE type);
private:
    MySQL sql;
    void AddMS(MS_TYPE _type,std::function<bool (USER_INFO)> _cmd);
    void SocketInit();
    bool Login(QTcpSocket * socket, DATA_PACKAGE pack);
    bool ChangePassword(QTcpSocket * socket);
    bool Register(QTcpSocket *socket,DATA_PACKAGE pack);
    bool AddRoom(wstring );
    bool SendRoomList(QTcpSocket *socket);
    bool CreatRoom(QTcpSocket *socket,DATA_PACKAGE pack);
    bool LeaveRoom(QTcpSocket *socket,DATA_PACKAGE pack);
    bool EnterRoom(QTcpSocket *socket,DATA_PACKAGE pack);
    void UpdateRoom();
    TCPNet tcp;
    QTcpSocket * m_socket;
    //QList<ROOM_LIST_INFO> room_list;
    std::unordered_map<wstring,ROOM_INFO> room_map;
};

#endif // DATA_H
