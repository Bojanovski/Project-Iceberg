
#ifndef DIRECTX11HEADERS_H
#define DIRECTX11HEADERS_H

#define ReleaseCOM(com) if (com) { (com)->Release();  (com) = NULL; }

// Stupid-ass microsoft did not included d3dx library in vs2012, more: http://msdn.microsoft.com/en-us/library/windows/desktop/ee663275.aspx
#pragma warning( disable : 4005 )

#include <Windows.h>
#include <xnamath.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx11effect.h>

#pragma warning( default : 4005 )

#endif
