#include "data.h"
#include <QString>
#include <QtAlgorithms>
#include "game.h"
#define MSMAP(x,y) AddMS(MS_TYPE::x,[this,x](USER_INFO)->bool{ this->y(x)});
Data::Data():
    tcp(this),game(Game::GetInstance())
{

}



bool Data::DealMS(QTcpSocket * socket,const DATA_PACKAGE & pack)
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
        return LeaveRoom(socket,pack);
    case MS_TYPE::ENTER_ROOM:
        return EnterRoom(socket,pack);
    case MS_TYPE::GAME_START:

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


bool Data::Login(QTcpSocket * socket,DATA_PACKAGE pack)
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

bool Data::Register(QTcpSocket *socket,DATA_PACKAGE pack)
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

bool Data::CreatRoom(QTcpSocket *socket,DATA_PACKAGE pack)
{
    PLAYER_INFO* info=(PLAYER_INFO *)&pack.buf;
    ROOM_INFO room;
    if(room.AddPlayer(socket,*info)==false)
    {
        return false;
    }
    room_map[info->room_name.GetStr()]=room;
    user_map[socket].room_name=room.name;
    PRINT("创建房间成功")
    UpdateRoomList();
    return true;
}

bool Data::LeaveRoom(QTcpSocket *socket,DATA_PACKAGE pack)
{
    PLAYER_INFO* info=(PLAYER_INFO *)&pack.buf;
    const auto & room_name=user_map[socket].room_name;
    room_map[room_name].DelPlayer(info->pos);
    user_map[socket].room_name.clear();
    UpdateRoomList();
    return true;
}

bool Data::EnterRoom(QTcpSocket *socket, DATA_PACKAGE pack)
{
    PLAYER_INFO* info=(PLAYER_INFO *)&pack.buf;
    auto  & cur_room=room_map[info->room_name.GetStr()];
    if(cur_room.AddPlayer(socket,*info))
    {
        //do somethin
        UpdateRoomList();
        DATA_PACKAGE pack;
        pack.ms_type=MS_TYPE::MATE_INFO_RE;

        //Enter the room successfully  发送另外2个玩家的信息 有问题
        typedef  PLAYER_INFO SIMPLE_ROOM_INFO[2];
        SIMPLE_ROOM_INFO temp;
        int cnt_flag=0;
        for(int i:{0,1,2})
        {
            temp[cnt_flag].name=cur_room.mate_arr[cnt_flag];
            temp[cnt_flag].pos=i;
            cnt_flag++;
        }
        pack.buf=temp;
        for(int i=0;i<cur_room.num-1;i++)
        {
            if(cur_room.socket_arr[i])
            {
                tcp.SendData(cur_room.socket_arr[i],pack);
            }
        }
        return true;
    }

    return false;

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




