#include "roominfo.h"
#include "Packdef.h"
#include <QMutex>
using namespace std;

QMutex g_mutex;
RoomInfo::RoomInfo()
{
    for(int i=0;i<3;i++)
    {
        socket_arr[i]=nullptr;
    }
    num=0;
    landlord_flag.fill(0);
}

RoomInfo::~RoomInfo()
{
}

int RoomInfo::AddPlayer(QTcpSocket *_socket, const PLAYER_INFO &info)
{
    QMutexLocker locker(&g_mutex);
    for(int i=0;i<3;i++)
    {
        if(socket_arr[i]==nullptr)
        {
            if(i==0)
            {
                name=info.room_name.GetStr();
            }
            ++num;
            mate_arr[i]=info.name.GetStr();
            socket_arr[i]=_socket;
            return i;
        }
    }

    return -1;
}

void RoomInfo::DelPlayer(const char pos)      //true
{
    Q_ASSERT(pos>=0&&pos<3);
    QMutexLocker locker(&g_mutex);
    if(socket_arr[pos]==nullptr)
    {
        return;
    }
    socket_arr[pos]=nullptr;
    mate_arr[pos].clear();
    --num;

}

char RoomInfo::NextPlayerTurn()
{
    Q_ASSERT(game_state==GameState::SelectLandLord&&game_state==GameState::Gaming);
    if(++turn_flag==3)
        turn_flag=0;
    return turn_flag;
}

char RoomInfo::WantLandlord(const uchar num)
{
    Q_ASSERT(game_state==GameState::SelectLandLord);
    if(++landlord_flag[num]==2)
    {
        landlord_pos=num;
        return num;     //当前为地主
    }
    if(count(landlord_flag.begin(),landlord_flag.end(),-1)==2)
    {
        landlord_pos=num;
        return num;
    }
    return WAIT_OTHERS;          //等待其他玩家继续抢地主
}

char RoomInfo::NotWantLandlord(const uchar num)
{
    Q_ASSERT(game_state==GameState::SelectLandLord);
    landlord_flag[num]=-1;
    array<char,3> temp={0};       //-1 0 1 三种可能
    for(auto i:landlord_flag)
    {
        temp[i+1]++;
    }
    if(temp[0]==3)      //三人都不叫地主
    {
        return NONE_LANDLORD;
    }
    if(temp[0]==2 && temp[2]==1)
    {
        landlord_pos=*find(temp.begin(),temp.end(),1);
        return landlord_pos;
    }
    else
    {
        return WAIT_OTHERS;
    }
}

bool RoomInfo::IsEmpty()
{
    return num<=0;
}

char RoomInfo::GetCurActPlayer()
{

    return turn_flag;
}

