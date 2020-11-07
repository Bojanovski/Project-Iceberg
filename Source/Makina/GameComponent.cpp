#include "GameComponent.h"

using namespace Makina;

long unsigned int GameComponent::counter = 0;

GameComponent::GameComponent(D3DAppValues *values) : mD3DAppValues(values), mDrawType(GAME_COMPONENT_DRAW_TYPE_NORMAL)
{
	Enabled = Visible = true;
}

GameComponent::~GameComponent()
{

}