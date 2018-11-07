#ifndef GAME_H
#define GAME_H

#include <QVector>
#include "Packdef.h"
#include <vector>
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
    QVector<char> all_poker;
    static Game* game; //hungry

};

#endif // GAME_H
