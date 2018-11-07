#include <algorithm>
#include "game.h"

Game * Game::game= new Game();
Game &Game::GetInstance()
{
    return *Game::game;
}

PokerGroup Game::GetPokerGroup()
{
    //QVector<QVector<Poker>> ans(4);
    PokerGroup group;
    ShufflePoker();
    for(char i=0;i<54;i++)
    {
        group[i]=all_poker[i];
    }
    return group;
}

void Game::InitPoker()
{
    all_poker.resize(54);
    for(char i=0;i<54;i++)
    {
        all_poker[i]=i;
    }
}

void Game::ShufflePoker()
{
    std::random_shuffle(all_poker.begin(),all_poker.end());
}
Game::Game()
{
    InitPoker();
}
