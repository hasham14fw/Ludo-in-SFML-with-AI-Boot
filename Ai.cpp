#include "Ai.h"
#include "Game.h"
#include "Team.h"
#include "Board.h"

Ai::Ai(Team* team, Board* board)
{
    this->team = team;
    this->board = board;
}

bool Ai::move(int dice, sf::RenderWindow* window)
{
    this->dice = dice;
    this->pawnToMoveId = this->decide();
    if (this->pawnToMoveId != -1 && this->team->getPawns()[this->pawnToMoveId]->handleClick(dice, window, this->board)) {
        return true;
    }
    return false;
}

int Ai::decide() {
    auto possible = this->getPossibleMoves();
    if (possible.empty()) {
        return 0;
    }
    // Check for strike
    vector<int> strikeMoves;
    for (int i : possible) {
        Tile* desired = this->team->getPawns()[i]->getDesiredTile(this->dice, this->board);
        if (desired->getCurrentPawn() != nullptr && desired->getCurrentPawn()->getTeam() != this->team) {
            strikeMoves.push_back(i);
        }
    }
    if (!strikeMoves.empty()) {
        return this->getRandom(strikeMoves);
    }
    // Check for deploy
    if (this->dice == 1 || this->dice == 6) {
        vector<int> atBase;
        for (int i : possible) {
            if (this->team->getPawns()[i]->getIsAtBase()) {
                atBase.push_back(i);
            }
        }
        if (!atBase.empty()) {
            return this->getRandom(atBase);
        }
    }
    // Move furthest pawn
    int maxDistance = -1;
    int bestPawn = possible[0];
    for (int i : possible) {
        int distance = this->team->getPawns()[i]->getDistanceFromStart(this->board);
        if (distance > maxDistance) {
            maxDistance = distance;
            bestPawn = i;
        }
    }
    return bestPawn;
}

vector<int> Ai::getPossibleMoves()
{
    vector<int> possible;
    for (int i = 0; i < Game::PAWNS_TEAM; i++) {
        if (this->team->getPawns()[i]->canMove(this->dice, this->board)) {
            possible.push_back(i);
        }
    }
    return possible;
}


int Ai::getRandom(vector<int> possible)
{
    return possible.at(random(0, possible.size() - 1));
}