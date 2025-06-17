#pragma once
#include <SFML/Graphics.hpp>
#include "Random.h"
#include "Board.h"
#include <vector>

using namespace std;

class Team;
class Board;

class Ai
{
    int dice{};
    int pawnToMoveId{};
    Team* team;
    Board* board;
public:
    Ai() = delete;
    Ai(Team* team, Board* board);
    bool move(int dice, sf::RenderWindow* window);
private:
    int decide();
    vector<int> getPossibleMoves();
    int getRandom(vector<int> possible);
};