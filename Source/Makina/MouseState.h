
#ifndef MOUSESTATE_H
#define MOUSESTATE_H

namespace Makina
{
	struct MouseState
	{
		bool IsLMBDown;
		bool IsMMBDown;
		bool IsRMBDown;

		// Coordinates
		int x;
		int y;

		// Wheel
		int wheelValue;

		// This constructor will be called automatically
		MouseState()
		{
			IsLMBDown = IsMMBDown = IsRMBDown = false;
			wheelValue = x = y = 0;
		}
	};
}

#endif