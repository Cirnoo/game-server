#include "data.h"
#include <QString>
#include <QtAlgorithms>
#include "game.h"
#define MSMAP(x,y) AddMS(MS_TYPE::x,[this,x](USER_INFO)->bool{ this->y(x)});
Data::Data():
    tcp(this),game(Game::GetInstance())
{

}



bool Data::DealMS(QTcpSocket * socket,DATA_PACKAGE & pack)
{
    m_socket=socket;
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
    default:
    {
        DATA_PACKAGE pack;
        tcp.SendData(m_socket,pack);
        return true;
    }
    }
    return true;
}

void Data::NewConnection(QTcpSocket *socket)
{
    if(!user_map.count(socket))
    {
        PRINT("新的连接...")
        CLIENT_INFO info;
        info.ip=socket->peerAddress().toString().toStdString();
        info.port=socket->peerPort();
        user_map[socket]=(info);
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
    tcp.SendData(m_socket,pack);
    user_map[socket].username=user->name.GetStr();
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
    tcp.SendData(m_socket,pack);
}

bool Data::SendRoomList(QTcpSocket *socket)
{
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
    ROOM_INFO room;
    int player_pos=room.AddPlayer(socket,*info);
    DATA_PACKAGE pack_to_client;
    if(player_pos==-1)
    {
        pack_to_client.ms_type=MS_TYPE::CREATE_ROOM_RE_F;
        return false;
    }

    room_map[info->room_name.GetStr()]=room;
    user_map[socket].room_name=room.name;
    user_map[socket].room_pos=player_pos;
    PRINT("创建房间成功")
    pack_to_client.ms_type=MS_TYPE::CREATE_ROOM_RE_T;
    tcp.SendData(socket,pack_to_client);
    UpdateRoomList();
    return true;
}

bool Data::LeaveRoom(QTcpSocket *socket)
{
    const auto & room=user_map[socket];
    room_map[room.room_name].DelPlayer(room.room_pos);
    user_map[socket].room_name.clear();
    user_map[socket].room_pos=-1;
    UpdateRoomList();
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
        UpdateRoomList();

        user_map[socket].room_pos=player_pos;
        user_map[socket].room_name=cur_room.name;
        //Enter the room successfully  有问题
        typedef  USER_BUF SIMPLE_ROOM_INFO[3];
        SIMPLE_ROOM_INFO temp;
        DATA_PACKAGE pack;
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
        return true;
    }

    return false;

}

bool Data::OffLine(QTcpSocket *socket)
{
    auto & user=user_map.find(socket)->second;
    if(!user.room_name.empty())
    {
        const auto & room=user_map[socket];
        room_map[room.room_name].DelPlayer(room.room_pos);  //从房间中删除
    }
    PRINT(user.ip<<"::"<<user.port<<" offline");
    user_map.erase(user_map.find(socket));      //从房间列表删除

    return true;
}

bool Data::GameStart(QTcpSocket * socket)
{
    /************send poker***********/
    auto group=game.GetPokerGroup();
    auto & room_socker=room_map[user_map[socket].username].socket_arr;
    DATA_PACKAGE pack;
    pack.ms_type=MS_TYPE::ALLOC_POKER;
    for(char i=0;i<3;i++)
    {
        group.num=i;
        pack.buf=group;
        tcp.SendData(room_socker[i],pack);
    }

}
void Data::UpdateRoomList()
{
    DATA_PACKAGE data;
    data.ms_type=MS_TYPE::UPDATE_ROOM;
    Broadcast(data);
}

void Data::Broadcast(const DATA_PACKAGE &pack)
{
  for(auto i=user_map.begin();i!=user_map.end();i++)
  {
      tcp.SendData(i->first,pack);
  }
}




