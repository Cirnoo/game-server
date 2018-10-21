#include "data.h"
#include <QString>
#include <QtAlgorithms>
#define MSMAP(x,y) AddMS(MS_TYPE::x,[this,x](USER_INFO)->bool{ this->y(x)});
Data::Data():tcp(this)
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
    default:
    {
        DATA_PACKAGE pack;
        tcp.SendData(m_socket,pack);
        return true;
    }
    }
    return true;
}





//USER_INFO Data::RecvUserInfo( QTcpSocket * socket)
//{
//    USER_INFO info;
//    socket->read((char *)&info,sizeof (info));
//    return info;
//}



bool Data::Login(QTcpSocket * socket,DATA_PACKAGE pack)
{
    USER_INFO * user=(USER_INFO*)&pack.buf;
    auto name=QString::fromStdWString(user->name.GetStr());
    auto password=QString::fromStdWString(user->password.GetStr());
    bool flag=sql.Login(name,password);
    pack.buf="";
    pack.ms_type=flag?MS_TYPE::LOGIN_RE_T:MS_TYPE::LOGIN_RE_F;
    tcp.SendData(m_socket,pack);
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
        info.master=i.second.mate[0].GetStr();
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
    room.AddPlayer(*info);
    room_map[info->master.GetStr()]=room;
    PRINT("创建房间成功")
    UpdateRoom();
}

bool Data::LeaveRoom(QTcpSocket *socket,DATA_PACKAGE pack)
{
    ROOM_LIST_INFO* info=(ROOM_LIST_INFO *)&pack.buf;
    for(auto i=room_map.begin();i!=room_map.end();i++)
    {
        for(int j=0;j<3;j++)
        {
            if(i->second.mate[j].GetStr()==info->master.GetStr())
            {
                //do something
                i->second.mate[j]=L"";
                if(--(i->second.num)==0)
                {
                    room_map.erase(i);
                    UpdateRoom();
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
    if(room_map[info->master.GetStr()].AddPlayer(*info))
    {
        //do something
        UpdateRoom();
        return true;
    }
    else
    {
        return false;
    }

}

void Data::UpdateRoom()
{
    DATA_PACKAGE data;
    data.ms_type=MS_TYPE::UPDATE_ROOM;
    tcp.Broadcast(data);
}




