#include "data.h"
#include <QString>
#include <QtAlgorithms>
#define MSMAP(x,y) AddMS(MS_TYPE::x,[this,x](USER_INFO)->bool{ this->y(x)});
Data::Data():tcp(this)
{
    ROOM_INFO info;
    info.master=L"1234";
    info.name=L"123";
    info.num=2;
    room_list.push_back(info);
}



bool Data::DealMS(QTcpSocket * socket,DATA_PACKAGE pack)
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
    qSort(room_list.begin(),room_list.end(),[](ROOM_INFO & a,ROOM_INFO & b)->bool {return a.num>b.num;});
    for(auto i:room_list)
    {
        ROOM_INFO info;
        info.master=i.master;
        info.name=i.name;
        info.num=i.num;
        pack.buf=info;
        tcp.SendData(socket,pack);
    }
}

bool Data::CreatRoom(DATA_PACKAGE pack)
{
    ROOM_INFO info=*(ROOM_INFO *)&pack.buf;
    room_list.push_back(info);
}




