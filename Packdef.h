#pragma once
#include <string>
#include <iostream>
#include <QString>
#define PRINT(a) std::cout<<a<<"\n";

#define _DEF_PORT 1234
#define _SERVER_IP "127.0.0.1"
#define _DEF_CLIENTPORT 2000
#define _DEF_SERVERPORT 10086
#define _DEF_SIZE 64
#define _DEF_ROOMBUFFERSIZE 550*1000
#define _DEF_NUM 10
#define _DEF_SQLLEN 100
#define USER_LENGTH 20
using std::wstring;
enum class MS_TYPE :unsigned char
{
    REGISTER_RQ,
    REGISTER_RE_T,
    REGISTER_RE_F,
    LOGIN_RQ,
    LOGIN_RE_T,
    LOGIN_RE_F,
    ADD_ROOM,
    GET_ROOM_LIST,
    CREATE_ROOM,
    ENTER_ROOM,
    LEAVE_ROOM,
    UPDATE_ROOM,
    ADD_PLAYER,
    GAME_START,
    GET_POKER,
    HEARTBEAT,//心跳包
};
using std::string;
struct USER_BUF
{
    wchar_t buf[USER_LENGTH];
    USER_BUF()
    {
        for(int i=0;i<USER_LENGTH;i++)
        {
            buf[i]=0;
        }
    }
    USER_BUF(wstring str)
    {
        for(int i=0;i<USER_LENGTH;i++)
        {
            buf[i]=0;
        }
        str.copy(buf, str.size(), 0);
    }
    void operator=(const USER_BUF & u )
    {
        memcpy(buf,u.buf,USER_LENGTH);
    }
    void operator=(const wstring & str)
    {
        str.copy(buf, str.size(), 0);
    }
    wstring GetStr() const
    {
        wstring str=wstring(buf);
        return str;
    }
};

enum class CardType : unsigned char
{
    Heart		,
    Spade		,
    Diamond		,
    Club		,
    Joker	    ,
};
enum class PokerPoints : unsigned char
{
    Three=3,Four,Five,Six,Seven,Eight,Nine,Ten,jack,Queen,King,Ace,Two,Black_Joker,Red_Joker
};
struct Poker
{
    CardType c_type;
    PokerPoints point;
    Poker(){}
    Poker(CardType t,PokerPoints p)
    {
        c_type=t;
        point=p;
    }
    void operator=(const Poker & u )
    {
        c_type=u.c_type;point=u.point;
    }
};

struct PokerGroup
{
    char poker[3][16]; // 16 张/person
    char last[3];      // 地主牌
    char num;
    char & operator[](char i)
    {
        return *(poker[0]+i);
    }
};

struct USER_INFO
{
    USER_BUF name;
    USER_BUF password;
    USER_INFO()
    {
        name=L"";
        password=L"";
    }
    USER_INFO(USER_BUF n,USER_BUF p)
    {
        name=n;password=p;
    }
    void operator=(const USER_INFO & u )
    {
        name=u.name;password=u.password;
    }
};
struct CLIENT_INFO
{
    wstring username;
    string ip;
    unsigned short port;
};

struct ROOM_LIST_INFO
{
    USER_BUF master,name;
    unsigned char num;
};
class QTcpSocket;
struct ROOM_INFO
{
    wstring mate_arr[3],name;
    unsigned char num;
    QTcpSocket * socket_arr[3];
    ROOM_INFO()
    {
        for(int i=0;i<3;i++)
        {
            socket_arr[i]=nullptr;
        }
    }
    bool AddPlayer( QTcpSocket * _socket,const ROOM_LIST_INFO & info)
    {
        for(int i=0;i<3;i++)
        {
            if(socket_arr[i]==nullptr)
            {
                if(i==0)
                {
                    name=info.name.GetStr();
                    num=1;
                }
                else
                {
                    num++;
                }
                mate_arr[i]=info.master.GetStr();
                socket_arr[i]=_socket;
                return true;
            }
        }
        return false;
    }
};
const unsigned int MAX_BUF_SIZE=sizeof(ROOM_LIST_INFO);
struct DATA_BUF
{
    char buf[MAX_BUF_SIZE];
    DATA_BUF()
    {
        memset(buf,0,MAX_BUF_SIZE);
    }
    template  <class T>
    DATA_BUF(const T & u)
    {
        int i=0;
        for(i=0;i<sizeof(u);i++)
        {
            buf[i]=(*((char *)&u+i));
        }
        memset(buf+i,0,MAX_BUF_SIZE-i);
    }
};
struct DATA_PACKAGE
{
    MS_TYPE ms_type;
    DATA_BUF buf;
    template  <class T>
    DATA_PACKAGE(MS_TYPE type,T & u)
    {
        ms_type=type;
        buf=u;
    }
    DATA_PACKAGE()
    {
        ms_type=MS_TYPE::HEARTBEAT;
        buf="";
    }
};

