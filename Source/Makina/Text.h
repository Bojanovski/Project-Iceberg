
#ifndef TEXT_H
#define TEXT_H
#define TEXT_MAX_LEN 150

#include "Object2D.h"
#include "Font.h"

namespace Makina
{
	class Text : public Object2D
	{
	public:
		__declspec(dllexport) Text(D3DAppValues *values, Font *font, const wchar_t *text, float posX, float posY, float size, FXMVECTOR color);
		__declspec(dllexport) ~Text();
		__declspec(dllexport) void Draw();
		__declspec(dllexport) void ChangeProp(float posX, float posY, float size, FXMVECTOR color);
		__declspec(dllexport) void ChangeText(const wchar_t *newText);
		__declspec(dllexport) void OnResize();

	private:
		__declspec(dllexport) void BuildGeometryBuffers();
		__declspec(dllexport) void CreateInputLayout();
		__declspec(dllexport) void SetValues(int charId, void *vert, float *xOffset, float *yOffset);

	private:
		std::wstring mText;
		Font *mFont;
	};
}

#endif
