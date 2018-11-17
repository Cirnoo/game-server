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
#include "roominfo.h"
class Game;
class Data
{
public:
    Data();
    //bool DealMS(MS_TYPE _type,USER_INFO data);
    bool DealMS(QTcpSocket *m_socket, DATA_PACKAGE &pack);
    void NewConnection(QTcpSocket * socket);
private:

    MySQL sql;
    void AddMS(MS_TYPE _type,std::function<bool (USER_INFO)> _cmd);
    bool Login(QTcpSocket * socket,  DATA_PACKAGE & pack);
    bool ChangePassword(QTcpSocket * socket);
    bool Register(QTcpSocket *socket, DATA_PACKAGE & pack);
    bool AddRoom(wstring );
    bool SendRoomList(QTcpSocket *socket);
    bool CreatRoom(QTcpSocket *socket, DATA_PACKAGE & pack);
    bool LeaveRoom(QTcpSocket *socket);
    bool EnterRoom(QTcpSocket *socket, DATA_PACKAGE & pack);
    bool OffLine(QTcpSocket *socket);
    bool GameStart(QTcpSocket * socket);
    bool SelectLandlord(QTcpSocket * socket,bool is_want);
    void UpdateRoomList(const RoomInfo &info) const;
    void Broadcast(const DATA_PACKAGE &pack, const ClientState state) const;
    void RefreshRoomMateInfo(QTcpSocket * const socket);
    void SendToRoomMate(QTcpSocket * socket,const DATA_PACKAGE & pack) const;
    TCPNet tcp;
    Game & game;
    std::unordered_map<wstring,RoomInfo> room_map;
    std::unordered_map<QTcpSocket *,CLIENT_INFO> player_map;
};

#endif // DATA_H
