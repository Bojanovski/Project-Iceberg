
#ifndef GAME_STATES_MANAGER_H
#define GAME_STATES_MANAGER_H

#include "GameComponent.h"
#include <vector>

namespace Makina
{
	class GameState
	{
		friend class GameStatesManager;

	public:
		__declspec(dllexport) GameState();
		__declspec(dllexport) virtual ~GameState();

	protected:
		D3DAppValues *mD3DAppValues;
		virtual void Initialize() = 0;

	private:
		virtual void Update(float dt) = 0;
		virtual void Draw(float dt) = 0;
		virtual void OnResize() = 0;
	};

	class GameStatesManager : public GameComponent
	{
	public:
		__declspec(dllexport) GameStatesManager(D3DAppValues *values);
		__declspec(dllexport) ~GameStatesManager();

		__declspec(dllexport) void Update(float dt);
		__declspec(dllexport) void Draw(float dt);
		__declspec(dllexport) void OnResize();

		__declspec(dllexport) void PushState(GameState *gameState);
		__declspec(dllexport) void PopState();

	private:
		std::vector<GameState *> mGameStates;
	};
}
#endif