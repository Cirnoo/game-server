#include <algorithm>
#include "game.h"

Game * Game::game= new Game();
Game &Game::GetInstance()
{
    return *Game::game;
}

PokerGroup Game::GetPokerGroup()
{
    ShufflePoker();
    return all_poker;
}

void Game::InitPoker()
{
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
