#pragma once
#pragma warning(disable:6385)
#include <iostream>
#include "Random.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

constexpr auto TEXTURE_PATH = "images/";
constexpr auto FONTS_PATH = "fonts/";

using namespace std;

class Board;
class Team;
class Pawn;
class TossButton;
class Dial;
class Ai;

class Game
{

    static const int BASE_DELAY = 1000;

    int dice{};
    int playersAmount = 2; // Fixed to 2 for 1v1 (1 human + 1 AI)
    int currentTeamId{};
    int currentFreePodiumPlace = 1;
    int delayTime = BASE_DELAY; // Fixed delay

    bool& restart;
    Board* board;
    Team* teams[2]; // Only 2 teams: Red (human), Blue (AI)
    Pawn* pawns[8]; // 2 teams * 4 pawns
    sf::RenderWindow* window;
    sf::Clock delayClock;
public:
    static const int PAWNS_TEAM = 4;
    static const int MAX_TEAMS = 2; // Fixed to 2 for 1v1

    Game() = delete;
    Game(bool& restart);
    ~Game();
    void update();
    void render();
    void initGame();
    bool inline isRunning() const { return this->window->isOpen(); };
private:
    void initWindow();
    void createPlayers();
    void renderPawns();
    void handleAiMove();
    void handlePlayerTossClick();
    void handlePawnClick(int pawnId);
    void handleMouseOverPawn(int pawnId);
    void handleAllObstructed();
    void handleSingleWin();
    void handleGameEnd();
    void delay(int time, string dial);
    void setNextTeamId(int diceT);
    void selectPlayer();
    void pollEvents();
};