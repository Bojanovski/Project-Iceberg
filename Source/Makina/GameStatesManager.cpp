
#include "GameStatesManager.h"

using namespace std;
using namespace Makina;

GameState::GameState()
: mD3DAppValues(0)
{

}

GameState::~GameState()
{

}

GameStatesManager::GameStatesManager(D3DAppValues *values)
: GameComponent(values)
{

}

GameStatesManager::~GameStatesManager()
{
	for (auto &gameStates : mGameStates)
		delete gameStates;
}

void GameStatesManager::Update(float dt)
{
	for (auto &gameStates : mGameStates)
		gameStates->Update(dt);
}

void GameStatesManager::Draw(float dt)
{
	for (auto &gameStates : mGameStates)
		gameStates->Draw(dt);
}

void GameStatesManager::OnResize()
{
	for (auto &gameStates : mGameStates)
		gameStates->OnResize();
}

void GameStatesManager::PushState(GameState *gameState)
{
	gameState->mD3DAppValues = mD3DAppValues;
	gameState->Initialize();
	gameState->OnResize();
	mGameStates.push_back(gameState);
}

void GameStatesManager::PopState()
{
	delete mGameStates[mGameStates.size() - 1];
	mGameStates.pop_back();
}