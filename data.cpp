#include "data.h"
#include <QString>
#include <QtAlgorithms>
#include "game.h"
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
        return true;
    case MS_TYPE::GAME_OFFLINE:
        return OffLine(socket);
    default:
    {
        DATA_PACKAGE pack;
        tcp.SendData(socket,pack);
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
        player_map[socket]=(info);
        PRINT(info.ip<<"::"<<info.port)
    }
}


bool Data::Login(QTcpSocket * socket, DATA_PACKAGE &pack)
{
    USER_INFO * user=(USER_INFO*)&pack.buf;
    auto name=QString::fromStdWString(user->name.GetStr());
    auto password=QString::fromStdWString(user->password.GetStr());
    bool flag=sql.Login(name,password);
    pack.buf="";
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
    pack.buf="";
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
    ROOM_INFO room;             //get room name from client
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
    cur_player.room_name=room.name;
    cur_player.room_pos=player_pos;
    cur_player.state=ClientState::Gaming;       //change client state
    PRINT("创建房间成功")



    pack.ms_type=MS_TYPE::CREATE_ROOM_RE_T;
    tcp.SendData(socket,pack);        //回复客户端创建房间成功
    ROOM_LIST_INFO new_room_info;
    new_room_info.name=room.name;
    new_room_info.num=room.num;
    UpdateRoomList(new_room_info);
    return true;
}

bool Data::LeaveRoom(QTcpSocket *socket)
{
    auto & cur_player=player_map[socket];
    auto & cur_room=room_map[cur_player.room_name];
    cur_room.DelPlayer(cur_player.room_pos);
    char player_num=cur_room.num;
    if(cur_room.IsEmpty())
    {
        room_map.erase(room_map.find(cur_player.room_name));
    }
    cur_player.room_name.clear();
    cur_player.room_pos=-1;
    ROOM_LIST_INFO info;
    info.name=cur_player.room_name;
    info.num=player_num;
    UpdateRoomList(info);
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
        cur_player.room_pos=player_pos;
        cur_player.room_name=cur_room.name;
        cur_player.state=ClientState::Gaming;
        //Enter the room successfully  有问题
        typedef  USER_BUF SIMPLE_ROOM_INFO[3];
        SIMPLE_ROOM_INFO temp;
        for(int i:{0,1,2})
        {
            if(i==player_pos)
                continue;
            temp[i]=cur_room.mate_arr[i];
        }
        pack.buf=temp;
        for(int i=0;i<3;i++)
        {
            if(i==player_pos)
            {
                pack.ms_type=MS_TYPE::ENTER_ROOM_RE_T;
            }
            else
            {
                pack.ms_type=MS_TYPE::MATE_INFO_RE;
            }
            if(cur_room.socket_arr[i])
            {
                tcp.SendData(cur_room.socket_arr[i],pack);
            }
        }
        ROOM_LIST_INFO new_room_info;
        new_room_info.name=cur_room.name;
        new_room_info.num=cur_room.num;
        UpdateRoomList(new_room_info);
        return true;
    }

    return false;

}

bool Data::OffLine(QTcpSocket *socket)
{
    auto & user=player_map.find(socket)->second;
    if(!user.room_name.empty())
    {
        const auto & room=player_map[socket];
        auto room_ite=room_map.find(room.room_name);
        room_ite->second.DelPlayer(room.room_pos);  //从房间中删除
        if(room_ite->second.IsEmpty())
        {
            room_map.erase(room_ite);
        }
    }
    PRINT(user.ip<<"::"<<user.port<<" offline");
    player_map.erase(player_map.find(socket));      //从玩家列表删除
    socket->disconnect();
    socket->disconnectFromHost();
    socket->deleteLater();
    socket=nullptr;
    return true;
}

bool Data::GameStart(QTcpSocket * socket)
{
    /************send poker***********/
    auto group=game.GetPokerGroup();
    auto & room_socker=room_map[player_map[socket].username].socket_arr;
    DATA_PACKAGE pack;
    pack.ms_type=MS_TYPE::ALLOC_POKER;
    for(char i=0;i<3;i++)
    {
        group.num=i;
        pack.buf=group;
        tcp.SendData(room_socker[i],pack);
    }

}
void Data::UpdateRoomList(const ROOM_LIST_INFO & info)
{
    DATA_PACKAGE data(MS_TYPE::UPDATE_ROOM,info);
    Broadcast(data,ClientState::GameRoom);
}

void Data::Broadcast(const DATA_PACKAGE &pack,const ClientState  state) //只给此状态的玩家发送
{
  for(auto i=player_map.begin();i!=player_map.end();i++)
  {
      if(i->second.state==state)
      {
          tcp.SendData(i->first,pack);
      }
  }
}




