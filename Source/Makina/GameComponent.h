
#ifndef GAMECOMPONENT_H
#define GAMECOMPONENT_H

#define GAME_COMPONENT_DRAW_TYPE_NORMAL                   0x01
#define GAME_COMPONENT_DRAW_TYPE_DEPTH_BUFFER_READ_ONLY   0x02
#define GAME_COMPONENT_DRAW_TYPE_EFFECT                   0x04
#define GAME_COMPONENT_DRAW_TYPE_POSTPROCESS_EFFECT       0x08
#define GAME_COMPONENT_DRAW_TYPE_NDC_QUADS                0x10

#include "DirectX11Headers.h"
#include "Exceptions.h"
#include "D3DAppValues.h"

namespace Makina
{
	class GameComponent
	{
		friend class D3DApp;

	private:
		static long unsigned int counter;

	protected:
		D3DAppValues *mD3DAppValues;

	public:
		__declspec(dllexport) GameComponent(D3DAppValues *values);
		__declspec(dllexport) virtual ~GameComponent();

		virtual void Update(float dt) = 0;
		void Draw(float dt, UCHAR mDrawType) { if(this->mDrawType & mDrawType) Draw(dt); }
		virtual void Draw(float dt) = 0;
		virtual void OnResize() {};

	public:
		bool Enabled;
		bool Visible;	
		UCHAR mDrawType;
	};
}

#endif
