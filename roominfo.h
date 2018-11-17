#ifndef ROOMINFO_H
#define ROOMINFO_H

#include <string>
#include <array>
#include <QtGlobal>
#include <mutex>

enum SelectRetrun:char
{
    WAIT_OTHERS_CALL=-3,WAIT_OTHERS_ROB=-2,NONE_LANDLORD=-1,
};
enum IsWantLandlord:char
{
     WAIT_SEL=0,NOT_WANT=1,WANT=2
};
enum class GameState
{
    Wait,GetCards,SelectLandLord,Gaming,Over
};

class QTcpSocket;
class PLAYER_INFO;
class RoomInfo
{

public:
    std::wstring mate_arr[3],name;
    char num=0;       //房间有几人
    std::array<IsWantLandlord,3> landlord_flag;  //三人叫地主标记
    char turn_flag=0;  //当前行动玩家 0代表第一个
    char landlord_pos=0;
    std::array<QTcpSocket *,3> socket_arr;
    GameState game_state=GameState::Wait;
    int rate=5;
    RoomInfo();
    ~RoomInfo();
    int AddPlayer( QTcpSocket * _socket,const PLAYER_INFO & info);
    void DelPlayer(const char pos);
    char WantLandlord(const uchar num);
    char NotWantLandlord(const uchar num);
    char NextPlayerTurn();
    bool IsEmpty();
    char GetCurActPlayer();
private:
};

#endif // ROOMINFO_H
