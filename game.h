#ifndef GAME_H
#define GAME_H

#include <QVector>
#include "Packdef.h"
#include <array>
typedef  std::array<char,53> PokerGroup;
class Game
{
public:
    static Game & GetInstance();
    PokerGroup GetPokerGroup();
private:
    void InitPoker();
    void ShufflePoker();
private:
    Game();
    PokerGroup all_poker;
    static Game* game; //hungry

};

#endif // GAME_H
