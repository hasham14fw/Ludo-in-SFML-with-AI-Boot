#include "Pawn.h"
#include "Tile.h"
#include "Team.h"

Pawn::Pawn(int id, Team* team, Tile* currentTile)
{
	this->id = id;
	this->team = team;
	this->currentTile = currentTile;
	this->isAtBase = true;
	this->isAtTarget = false;
	initSprite();
}

void Pawn::draw(Tile* tile, sf::RenderWindow* window)
{
	this->currentTile->setCurrentPawn(nullptr);
	this->sprite.setPosition(tile->getPositionX(), tile->getPositionY());
	this->currentTile = tile;
	this->currentTile->setCurrentPawn(this);
	window->draw(this->sprite);
	if (this->isTargetVisible) {
		window->draw(this->target);
	}
	if (this->isPossibleVisible) {
		window->draw(this->possible);
	}
}


bool Pawn::handleClick(int dice, sf::RenderWindow* window, Board* board)
{
	if (this->isAtBase) { //deploy desired
		return deploy(dice, window, board);
	}
	if (this->canMoveFurther(dice, board)) { //pawn move not exceeding its route
		Tile* desired = this->getDesiredTile(dice, board);
		if (this->move(desired, window, board)) { //move possible
			return true;
		}
	}
	return false;
}

void Pawn::handleMouseOver(int dice, sf::RenderWindow* window, Board* board)
{
	if (!this->isAtBase || dice == 1 || dice == 6) {
		Tile* destination = this->getDesiredTile(dice, board);
		if (destination != nullptr) {
			this->targetTexture.loadFromFile(string(TEXTURE_PATH) + "target.png");
			this->target.setTexture(targetTexture);
			this->target.setOrigin(target.getGlobalBounds().width / 2, target.getGlobalBounds().height / 2);
			this->target.setPosition(destination->getPositionX(), destination->getPositionY());
			this->isTargetVisible = true;
		}
	}
}


//return current tile to base
void Pawn::setAtBase(sf::RenderWindow* window, Board* board)
{
	int tileId = this->team->getStartingTile()->getId() + Board::BASE_FIRST_ID; //first base tile id
	Tile* tile = board->getTileById(tileId);
	for (int i = 0; i < Board::BASE_SIZE; i++) {
		if (this->move(tile, window, board)) { //desired base tile is free
			break;
		}
		tileId++; //get next base tile
		tile = board->getTileById(tileId);
	}
	this->isAtBase = true;
}



void Pawn::setIsTargetVisible(bool isTargetVisible)
{
	this->isTargetVisible = isTargetVisible;
}

bool Pawn::setIsPossibleVisible(int currentTeamId, int dice, sf::RenderWindow* window, Board* board)
{
	if (this->canMove(dice, board) && currentTeamId == this->team->getId()) {
		this->possibleTexture.loadFromFile(string(TEXTURE_PATH) + "possible.png");
		this->possible.setTexture(possibleTexture);
		this->possible.setOrigin(this->sprite.getOrigin());
		this->possible.setPosition(this->sprite.getPosition());
		this->isPossibleVisible = true;
		return true;
	}
	this->isPossibleVisible = false;
	return false;
}

bool Pawn::canMove(int dice, Board* board)
{
	return ((dice == 1 || dice == 6) && this->isAtBase && this->canMoveFurther(1, board)) || (!this->isAtBase && this->canMoveFurther(dice, board));
}

int Pawn::getId()  const
{
	return this->id;
}

Team* Pawn::getTeam()
{
	return this->team;
}

Tile* Pawn::getCurrentTile()
{
	return this->currentTile;
}

Tile* Pawn::getDesiredTile(int dice, Board* board)
{
	int nextId = this->getCurrentTile()->getId();
	for (int i = 0; i < dice; i++) {
		nextId = this->getNextTileId(nextId);
	}
	return board->getTileById(nextId);
}

int Pawn::getDistanceFromStart(Board* board)
{
	if (this->isAtBase) {
		return Board::LAST_TILE + 1;
	}
	int distance = 0;
	int tileId = this->team->getStartingTile()->getId();
	while (this->currentTile->getId() != tileId) {
		tileId = this->getNextTileId(tileId);
		distance++;
	}
	return distance;
}

bool Pawn::getIsAtBase() const
{
	return this->isAtBase;
}

bool Pawn::getIsAtTarget() const
{
	return this->isAtTarget;
}

bool Pawn::canMoveFurther(int tiles, Board* board)
{
	int nextId = this->getCurrentTile()->getId();
	for (int i = 0; i < tiles; i++) {
		nextId = this->getNextTileId(nextId);
	}
	if (nextId <= this->team->getStartingTile()->getId() + Board::TARGET_LAST_ID)
	{
		Tile* tile = board->getTileById(nextId);
		if (tile != nullptr && tile->getCurrentPawn() != nullptr && tile->getCurrentPawn()->team == this->team) { //pawn tries to move into its team mate
			return false;
		}
		return true;
	}
	return false;
}

void Pawn::initSprite()
{
	this->texture.loadFromFile(this->team->getTexturePath());
	this->sprite.setTexture(this->texture);
	this->sprite.setOrigin(this->sprite.getGlobalBounds().width / 2, this->sprite.getGlobalBounds().height / 2);
}

//move with checking possibility on desired tile
bool Pawn::move(Tile* tile, sf::RenderWindow* window, Board* board)
{
	if (tile->getCurrentPawn() != nullptr) {
		if (tile->getCurrentPawn()->team == this->team) {
			return false;
		}
		tile->getCurrentPawn()->setAtBase(window, board); //strike
	}
	this->draw(tile, window);
	this->checkIsAtTarget();
	return true;
}

bool Pawn::deploy(int dice, sf::RenderWindow* window, Board* board)
{
	if (dice == 1 || dice == 6) { //deploy condition 
		if (this->move(this->team->getStartingTile(), window, board)) { //deploy possible
			this->isAtBase = false;

			return true;
		}
	}
	return false;
}

int Pawn::getNextTileId(int currentId) {
	int nextId = currentId;
	if (this->isAtBase) {
		nextId = this->team->getStartingTile()->getId();
	}
	else if (currentId == this->team->getStartingTile()->getId() - 1 || (this->team->getStartingTile()->getId() == 1 && currentId == Board::LAST_TILE)) { //pawn at target-turning tile
		nextId = this->team->getStartingTile()->getId() + Board::TARGET_FIRST_ID;
	}
	else if (currentId == Board::LAST_TILE) {
		nextId = 1;
	}
	else {
		nextId++;
	}
	return nextId;
}

void Pawn::checkIsAtTarget()
{
	this->isAtTarget = this->currentTile->getId() > this->team->getStartingTile()->getId() + Board::TARGET_FIRST_ID - 1
		&& this->currentTile->getId() < this->team->getStartingTile()->getId() + Board::TARGET_LAST_ID + 1;
	if (this->isAtTarget == true) {
	}
}