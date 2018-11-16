#include "data.h"
#include <QString>
#include <QtAlgorithms>
#include "game.h"
#include <QMutex>
#define MSMAP(x,y) AddMS(MS_TYPE::x,[this,x](USER_INFO)->bool{ this->y(x)});

Data::Data():
    tcp(*this),game(Game::GetInstance())
{

}


bool Data::DealMS(QTcpSocket * socket,DATA_PACKAGE & pack)
{
    switch (pack.ms_type)
    {
    case MS_TYPE::REGISTER_RQ:
        return Register(socket,pack);
    case MS_TYPE::LOGIN_RQ:
        return Login(socket,pack);
    case MS_TYPE::GET_ROOM_LIST:
        return SendRoomList(socket);
    case MS_TYPE::CREATE_ROOM:
        return CreatRoom(socket,pack);
    case MS_TYPE::LEAVE_ROOM:
        return LeaveRoom(socket);
    case MS_TYPE::ENTER_ROOM:
        return EnterRoom(socket,pack);
    case MS_TYPE::GAME_START:
        return GameStart(socket);
    case MS_TYPE::GAME_OFFLINE:
        return OffLine(socket);
    default:
    {
        tcp.SendMS(socket,MS_TYPE::HEARTBEAT);
        return true;
    }
    }
    return true;
}

void Data::NewConnection(QTcpSocket *socket)
{
    if(!player_map.count(socket))
    {
        PRINT("新的连接...")
        CLIENT_INFO info;
        info.ip=socket->peerAddress().toString().toStdString();
        info.port=socket->peerPort();
        player_map[socket]=info;
        PRINT(info.ip<<"::"<<info.port)
    }
}


bool Data::Login(QTcpSocket * socket, DATA_PACKAGE &pack)
{
    USER_INFO * user=(USER_INFO*)&pack.buf;
    auto name=QString::fromStdWString(user->name.GetStr());
    auto password=QString::fromStdWString(user->password.GetStr());
    bool flag=sql.Login(name,password);
    pack.buf.Clear();
    pack.ms_type=flag?MS_TYPE::LOGIN_RE_T:MS_TYPE::LOGIN_RE_F;
    tcp.SendData(socket,pack);
    player_map[socket].username=user->name.GetStr();
    return flag;
}

bool Data::ChangePassword(QTcpSocket *socket)
{
    //bool flag=sql.ChangePassword(name,password);
    return true;
}

bool Data::Register(QTcpSocket *socket, DATA_PACKAGE &pack)
{
    USER_INFO * user=(USER_INFO*)&pack.buf;
    auto name=QString::fromStdWString(user->name.GetStr());
    auto password=QString::fromStdWString(user->password.GetStr());
    bool flag=sql.Register(name,password);
    pack.buf.Clear();
    pack.ms_type=flag?MS_TYPE::REGISTER_RE_T:MS_TYPE::REGISTER_RE_F;
    tcp.SendData(socket,pack);
}

bool Data::SendRoomList(QTcpSocket *socket)
{
    player_map[socket].state=ClientState::GameRoom;     //玩家处于游戏大厅
    DATA_PACKAGE pack;
    pack.ms_type=MS_TYPE::ADD_ROOM;
    for(const auto & i:room_map)
    {
        ROOM_LIST_INFO info;
        info.name=i.second.name;
        info.num=i.second.num;
        pack.buf=info;
        tcp.SendData(socket,pack);
    }
    return true;
}

bool Data::CreatRoom(QTcpSocket *socket, DATA_PACKAGE &pack)
{
    PLAYER_INFO* info=(PLAYER_INFO *)&pack.buf;

    if(room_map.count(info->room_name.GetStr())!=0)
    {
        pack.ms_type=MS_TYPE::CREATE_ROOM_RE_F;     //room name already exist
        pack.buf.Clear();
        tcp.SendData(socket,pack);
        return false;
    }
    RoomInfo room;             //get room name from client
    int player_pos=room.AddPlayer(socket,*info);
    room.name=info->room_name.GetStr();
    pack.buf.Clear();
    info=nullptr;
    if(player_pos==-1)
    {
        pack.ms_type=MS_TYPE::CREATE_ROOM_RE_F;
        tcp.SendData(socket,pack);
        return false;
    }
    auto & cur_player=player_map[socket];
    room_map[room.name]=room;
    cur_player.UpdateClientInfo(room.name,room.num);
    cur_player.state=ClientState::Gaming;       //change client state
    PRINT("创建房间成功")

    pack.ms_type=MS_TYPE::CREATE_ROOM_RE_T;
    tcp.SendData(socket,pack);        //回复客户端创建房间成功
    UpdateRoomList(room);
    return true;
}

bool Data::LeaveRoom(QTcpSocket *socket)
{
    auto & cur_player=player_map[socket];
    auto & cur_room=room_map[cur_player.room_name];
    cur_room.DelPlayer(cur_player.room_pos);
    if(cur_room.IsEmpty())
    {
        room_map.erase(room_map.find(cur_player.room_name));
    }
    cur_player.room_name.clear();
    cur_player.room_pos=-1;
    RefreshRoomMateInfo(socket);
    UpdateRoomList(cur_room);
    return true;
}

bool Data::EnterRoom(QTcpSocket *socket, DATA_PACKAGE &pack)
{
    PLAYER_INFO* info=(PLAYER_INFO *)&pack.buf;
    auto  & cur_room=room_map[info->room_name.GetStr()];
    int player_pos=cur_room.AddPlayer(socket,*info);
    if(player_pos!=-1)
    {
        //do somethin

        auto & cur_player=player_map[socket];
        cur_player.UpdateClientInfo(cur_room.name,cur_room.num);
        cur_player.state=ClientState::Gaming;
        //Enter the room successfully  有问题

        RefreshRoomMateInfo(socket);
        UpdateRoomList(cur_room);
        return true;
    }

    return false;

}

bool Data::OffLine(QTcpSocket *socket)
{
    auto player_ite=player_map.find(socket);
    auto & user=player_ite->second;
    if(!user.room_name.empty())
    {
        const auto room_name=user.room_name;
        auto & cur_room=room_map[room_name];
        cur_room.DelPlayer(user.room_pos);  //从房间中删除
        if(cur_room.IsEmpty())
        {
            room_map.erase(room_name);
        }
    }
    PRINT(user.ip<<"::"<<user.port<<" offline");
    player_map.erase(player_ite);      //从玩家列表删除
    socket->disconnect();
    socket->disconnectFromHost();
    socket->deleteLater();
    socket=nullptr;
    return true;
}

bool Data::GameStart(QTcpSocket * socket)
{
    /************selcet landlord***********/
    auto & cur_room=room_map[player_map[socket].room_name];
    Q_ASSERT(!cur_room.IsEmpty());
    DATA_PACKAGE pack;
    pack.ms_type=MS_TYPE::ALLOC_POKER;
    pack.buf=game.GetPokerGroup();
    cur_room.game_state=GameState::SelectLandLord;
    tcp.SendDataToRoom(cur_room,pack);

}

bool Data::SelectLandlord(QTcpSocket *socket, bool is_want)
{
    auto & cur_player=player_map[socket];
    auto & cur_room=room_map[cur_player.room_name];
    char flag=is_want       //返回状态或地主序列号
            ?cur_room.WantLandlord(cur_player.room_pos)
            :cur_room.NotWantLandlord(cur_player.room_pos);
    DATA_PACKAGE pack;

    if(flag==NONE_LANDLORD)
    {
        //没人叫地主 游戏结束
        pack.ms_type=MS_TYPE::GAME_RESTRT;
    }
    else if (flag==WAIT_OTHERS)
    {
        //等待别人叫地主
        pack.ms_type=MS_TYPE::SELECT_LANDLORD;
        pack.buf=cur_room.NextPlayerTurn();      //下一个玩家
    }
    else
    {
        //游戏开始
        pack.ms_type=MS_TYPE::SET_LANDLORD;
        pack.buf=flag;
        cur_room.turn_flag=flag;        //地主出牌
        cur_room.game_state=GameState::Gaming;
    }
    tcp.SendDataToRoom(cur_room,pack);
}
void Data::UpdateRoomList(const RoomInfo & info) const
{
    ROOM_LIST_INFO new_room_info;
    new_room_info.name=info.name;
    new_room_info.num=info.num;
    DATA_PACKAGE data(MS_TYPE::UPDATE_ROOM,new_room_info);
    Broadcast(data,ClientState::GameRoom);
}

void Data::Broadcast(const DATA_PACKAGE &pack,const ClientState  state) const//只给此状态的玩家发送
{
  for(auto i=player_map.begin();i!=player_map.end();i++)
  {
      if(i->second.state==state)
      {
          tcp.SendData(i->first,pack);
      }
  }
}

void Data::RefreshRoomMateInfo(QTcpSocket * const socket)
{
    ENTER_ROOM_RE enter_room_re;
    auto & mate_info=enter_room_re.mate_name;
    const auto & cur_player=player_map[socket];
    const char player_pos=cur_player.room_pos;
    const auto & cur_room=room_map[cur_player.room_name];
    for(int i:{0,1,2})
    {
        mate_info[i]=cur_room.mate_arr[i];
    }
    enter_room_re.player_pos=player_pos;
    DATA_PACKAGE pack;
    pack.buf=enter_room_re;
    for(int i=0;i<3;i++)
    {
        if(i==player_pos)       //回复进入房间请求
        {
            pack.ms_type=MS_TYPE::ENTER_ROOM_RE_T;
        }
        else        //其他玩家更新信息
        {
            pack.ms_type=MS_TYPE::MATE_INFO_UPDATE;
        }
        if(cur_room.socket_arr[i])
        {
            tcp.SendData(cur_room.socket_arr[i],pack);
        }
    }
}




