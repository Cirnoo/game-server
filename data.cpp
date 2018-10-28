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
    for(auto i:room_map)
    {
        ROOM_LIST_INFO info;
        info.master=i.second.mate_arr[0];
        info.name=i.second.name;
        info.num=i.second.num;
        pack.buf=info;
        tcp.SendData(socket,pack);
    }
}

bool Data::CreatRoom(QTcpSocket *socket,DATA_PACKAGE pack)
{
    ROOM_LIST_INFO* info=(ROOM_LIST_INFO *)&pack.buf;
    ROOM_INFO room;
    room.AddPlayer(socket,*info);
    room_map[info->master.GetStr()]=room;
    PRINT("创建房间成功")
    UpdateRoomList();
}

bool Data::LeaveRoom(QTcpSocket *socket,DATA_PACKAGE pack)
{
    ROOM_LIST_INFO* info=(ROOM_LIST_INFO *)&pack.buf;
    for(auto i=room_map.begin();i!=room_map.end();i++)
    {
        for(int j=0;j<3;j++)
        {
            if(i->second.mate_arr[j]==info->master.GetStr())
            {
                //do something
                i->second.mate_arr[j]=L"";
                if(--(i->second.num)==0)
                {
                    room_map.erase(i);
                    UpdateRoomList();
                    break;
                }
            }
        }

    }
    return true;
}

bool Data::EnterRoom(QTcpSocket *socket, DATA_PACKAGE pack)
{
    ROOM_LIST_INFO* info=(ROOM_LIST_INFO *)&pack.buf;
    auto  & cur_room=room_map[info->master.GetStr()];
    if(cur_room.AddPlayer(socket,*info))
    {
        //do somethin
        UpdateRoomList();
        DATA_PACKAGE pack;
        pack.ms_type=MS_TYPE::ADD_PLAYER;
        pack.buf=info->master;
        for(int i=0;i<2;i++)
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
    auto group=game->GetPokerGroup();
    auto & room_socker=room_map[user_map[socket].username].socket_arr;
    DATA_PACKAGE pack;
    pack.ms_type=MS_TYPE::GET_POKER;
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




