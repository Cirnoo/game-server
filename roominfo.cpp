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
    landlord_flag.fill(WAIT_SEL);
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
    if(landlord_flag[num]==WANT)
    {
        landlord_pos=num;
        return landlord_pos;     //当前为地主
    }
    if(count(landlord_flag.begin(),landlord_flag.end(),NOT_WANT)==2)        //其他两人都不抢
    {
        landlord_pos=num;
        return landlord_pos;
    }
    return WAIT_OTHERS_ROB;          //等待其他玩家继续抢地主
}

char RoomInfo::NotWantLandlord(const uchar num)
{
    Q_ASSERT(game_state==GameState::SelectLandLord);
    landlord_flag[num]=NOT_WANT;
    array<int,3> want_cnt;       //三种可能
    want_cnt.fill(WAIT_SEL);
    for(auto i:landlord_flag)
    {
        want_cnt[i]++;
    }
    if(want_cnt[NOT_WANT]==3)      //三人都不叫地主
    {
        return NONE_LANDLORD;
    }
    if(want_cnt[NOT_WANT]==2 && want_cnt[WANT]==1)      //有一个人叫地主
    {
        landlord_pos=*find(want_cnt.begin(),want_cnt.end(),WANT);
        return landlord_pos;
    }
    else if(want_cnt[WANT]==0)      //还没有人叫地主
    {

        return WAIT_OTHERS_CALL;
    }
    else
    {
        return WAIT_OTHERS_ROB;
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

