#include "Game.h"
#include "Board.h"
#include "Team.h"
#include "Pawn.h"
#include "TossButton.h"
#include "Dial.h"
#include "Ai.h"

Game::Game(bool& restart) : restart(restart)
{
    this->initWindow();
    this->board = new Board(window);
    this->initGame();
}

Game::~Game()
{
    delete this->board;
    for (int i = 0; i < this->playersAmount; i++)
        delete this->teams[i];
    for (int i = 0; i < this->playersAmount * this->PAWNS_TEAM; i++)
        delete this->pawns[i];
    delete this->window;
}

void Game::update()
{
    this->pollEvents();
    if (this->teams[this->currentTeamId]->getIsAi()) {
        this->handleAiMove();
    }
}

void Game::render()
{
    this->window->clear();
    this->board->drawBoard(this->window);
    this->renderPawns();
    if (this->board->getTossButton()->canToss) {
        this->board->getTossButton()->draw(this->window);
    }
    this->window->display();
}

void Game::initWindow()
{
    this->window = new sf::RenderWindow(sf::VideoMode(900, 900), "Team NeuraLink", sf::Style::Titlebar | sf::Style::Close);
    this->window->setFramerateLimit(60);
    sf::Image icon;
    icon.loadFromFile(string(TEXTURE_PATH) + "logo.png");
    this->window->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
}



void Game::renderPawns()
{
    for (int i = 0; i < this->playersAmount * this->PAWNS_TEAM; i++) {
        this->pawns[i]->draw(this->pawns[i]->getCurrentTile(), this->window);
    }
}

void Game::initGame()
{
    this->playersAmount = 2; 
    this->createPlayers();
    for (int i = 0; i < this->playersAmount; i++) {
        Pawn* subpawns[] = { this->pawns[i * this->PAWNS_TEAM],
                             this->pawns[i * this->PAWNS_TEAM + 1],
                             this->pawns[i * this->PAWNS_TEAM + 2],
                             this->pawns[i * this->PAWNS_TEAM + 3] };
        this->teams[i]->setPawns(subpawns);
    }
    this->currentTeamId = random(0, playersAmount - 1);
    this->setNextTeamId(this->dice);
    this->delay(this->delayTime, "");
}

void Game::createPlayers()
{
    const string names[] = { "Red", "Blue" };
    const int startTiles[] = { 1, 11 };
    const int baseTiles[] = { 101, 111 };
    const string humanImages[] = { string(TEXTURE_PATH) + "Rpawn.png" };
    const string aiImages[] = { string(TEXTURE_PATH) + "BpawnAI.png" };

    // Create human player (Red)
    this->teams[0] = new Team(0, false, names[0], this->board->getTileById(startTiles[0]), humanImages[0], this->board);
    for (int k = 0; k < this->PAWNS_TEAM; k++) {
        int index = k;
        int tileId = baseTiles[0] + k;
        Tile* tile = this->board->getTileById(tileId);
        Pawn* pawn = new Pawn(index, this->teams[0], tile);
        this->pawns[index] = pawn;
    }

    // Create AI player (Blue)
    this->teams[1] = new Team(1, true, names[1], this->board->getTileById(startTiles[1]), aiImages[0], this->board);
    for (int k = 0; k < this->PAWNS_TEAM; k++) {
        int index = this->PAWNS_TEAM + k;
        int tileId = baseTiles[1] + k;
        Tile* tile = this->board->getTileById(tileId);
        Pawn* pawn = new Pawn(index, this->teams[1], tile);
        this->pawns[index] = pawn;
    }
}

void Game::handleAiMove()
{
    this->board->getTossButton()->handleClick(this->dice, this->board);
    this->delay(this->delayTime, "");
    if (!this->teams[this->currentTeamId]->getAi()->move(this->dice, this->window)) {
        this->board->getDial()->setText("Player blocked");
        this->delay(this->delayTime / 2, "");
    }
    else {
    }
    if (this->teams[this->currentTeamId]->isWin()) {
        this->handleSingleWin();
    }
    int diceT = this->dice;
    this->dice = 0;
    this->setNextTeamId(diceT);
    this->delay(this->delayTime / 2, "");
}

void Game::handlePlayerTossClick()
{
    this->board->getTossButton()->handleClick(this->dice, this->board);
    int autoMove = this->teams[this->currentTeamId]->getIsPossibleMovesOne(this->dice, this->board);
    if (autoMove != -1) {
        this->handlePawnClick(autoMove);
        return;
    }
    this->board->getDial()->setText("Player's turn: " + this->teams[this->currentTeamId]->getName());
    this->board->getTossButton()->canToss = false;
    if (this->teams[currentTeamId]->areAllObstructed(this->dice, this->board)) {
        this->handleAllObstructed();
        return;
    }
}

void Game::handlePawnClick(int pawnId)
{
    if (this->pawns[pawnId]->getTeam()->getId() == this->currentTeamId) {
        if (this->pawns[pawnId]->handleClick(this->dice, this->window, this->board)) {
            this->board->getTossButton()->canToss = false;
            if (this->teams[this->currentTeamId]->isWin()) {
                this->handleSingleWin();
            }
            int diceT = this->dice;
            this->dice = 0;
            this->setNextTeamId(diceT);
        }
        else {

        }
    }
    else {
        this->board->getDial()->setText("Error! Now it's player " + this->teams[this->currentTeamId]->getName() + "'s turn");
    }
}

void Game::handleMouseOverPawn(int pawnId)
{
    if (this->pawns[pawnId]->getTeam()->getId() == this->currentTeamId && this->dice != 0) {
        this->pawns[pawnId]->handleMouseOver(this->dice, this->window, this->board);
    }
}

void Game::setNextTeamId(int diceT)
{
    int id = this->currentTeamId;
    int attempts = 0;
    int won = 0;
    for (int i = 0; i < this->playersAmount; i++) {
        if (this->teams[i]->isWin()) {
            won++;
        }
    }
    if (won == this->playersAmount - 1) {
        this->handleGameEnd();
        return;
    }
    if (diceT == 6) {
        this->selectPlayer();
        this->board->getDial()->setText("Another roll for player " + this->teams[this->currentTeamId]->getName());
        this->delay(this->delayTime, "");
        return;
    }
    do {
        id = (id + 1) % this->playersAmount;
        attempts++;
    } while (this->teams[id]->isWin() && attempts < this->playersAmount);
    this->currentTeamId = id;
    this->selectPlayer();
    this->board->getDial()->setText("Player's roll: " + this->teams[this->currentTeamId]->getName());
    this->delay(this->delayTime, "");
}

void Game::selectPlayer()
{
    if (!this->teams[this->currentTeamId]->getIsAi()) {
        this->board->getTossButton()->canToss = true;
    }
    else {
        this->board->getTossButton()->canToss = false;
    }
}

void Game::handleAllObstructed()
{
    this->board->getDial()->setText("Player blocked");
    this->delay(this->delayTime, "");
    this->setNextTeamId(this->dice);
}

void Game::handleSingleWin()
{
    this->teams[this->currentTeamId]->setStanding(this->currentFreePodiumPlace);
    this->currentFreePodiumPlace++;
    this->delay(this->delayTime * 2, "Player " + this->teams[this->currentTeamId]->getName() + " wins!");
}

void Game::handleGameEnd()
{
    this->board->getTossButton()->canToss = false;
    this->delay(this->delayTime * 2, "Game over! " + this->teams[this->currentTeamId]->getName() + " wins!");
    this->restart = true;
    this->window->close();
}

void Game::delay(int time, string dial)
{
    this->delayClock.restart();
    while (this->delayClock.getElapsedTime().asMilliseconds() < time) {
        if (dial != "") {
            this->board->getDial()->setText(dial);
        }
        this->render();
        this->pollEvents();
    }
}

void Game::pollEvents()
{
    sf::Event event;
    while (this->window->pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            this->restart = false;
            this->window->close();
        }
        else if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Enter && this->board->getTossButton()->canToss)
            {
                this->handlePlayerTossClick();
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {

            for (int i = 0; i < this->playersAmount * this->PAWNS_TEAM; i++)
            {
                if (pawns[i]->isClicked(event) && !this->pawns[i]->getTeam()->getIsAi()) {
                    this->handlePawnClick(i);
                }
            }


        }
        for (int i = 0; i < this->playersAmount * this->PAWNS_TEAM; i++)
        {
            if (pawns[i]->isMouseOver(event) && !this->pawns[i]->getTeam()->getIsAi())
            {
                this->handleMouseOverPawn(i);
            }
            else
            {
                this->pawns[i]->setIsTargetVisible(false);
            }
        }
        for (int i = 0; i < this->playersAmount * this->PAWNS_TEAM; i++)
        {
            if (!this->pawns[i]->getTeam()->getIsAi())
                this->pawns[i]->setIsPossibleVisible(this->currentTeamId, this->dice, this->window, this->board);
        }
    }
}