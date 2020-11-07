
#ifndef LUA_SCRIPT_H
#define LUA_SCRIPT_H

#include "C\lua.hpp"
#include <string>
#include <vector>

typedef int (*FunctionPointer) (lua_State *);

namespace Makina
{
	enum LuaScriptState { Loaded, Running, Stopped };

	class LuaScript
	{
	public:
		__declspec(dllexport) LuaScript(const char *path, void *ownerObjPt);
		__declspec(dllexport) ~LuaScript();

		__declspec(dllexport) void RegisterFunction(char *name, FunctionPointer pt);
		__declspec(dllexport) void RegisterGlobalLib(const char *name, const luaL_Reg *bitmapLib);
		__declspec(dllexport) static LuaScript *GetCallerScript(lua_State *luaState);

		__declspec(dllexport) void Start(const std::vector<std::string> &parameters);
		__declspec(dllexport) void Update(float dt);
		__declspec(dllexport) void Stop(const std::vector<std::string> &parameters);

		LuaScriptState GetState() { return mScriptState; }
		void *GetOwnerObjectPt() { return mOwnerObjPt; }
		std::string GetPath() { return mPath; }

	private:
		__declspec(dllexport) static int PanicHandler(lua_State *luaState);
		__declspec(dllexport) static int OutputDebug(lua_State *luaState);

		void *mOwnerObjPt;
		std::string mPath;
		LuaScriptState mScriptState;

		// The Lua interpreter.
		lua_State *mLuaState;
	};

	namespace Lua
	{
		__declspec(dllexport) int GetTop(lua_State *l);
		__declspec(dllexport) lua_Number ToNumber(lua_State *l, int i);
		__declspec(dllexport) int ToInteger(lua_State *l, int i);
		__declspec(dllexport) std::string ToString(lua_State *l, int i);
		__declspec(dllexport) std::wstring ToWString(lua_State *l, int i);
		__declspec(dllexport) const void *ToLightUserData(lua_State *l, int i);
		__declspec(dllexport) bool ToBoolean(lua_State *l, int i);
		__declspec(dllexport) std::vector<float> ToFloatVector(lua_State *l, int i);

		__declspec(dllexport) void PushLightUserData(lua_State *l, void *p);
		__declspec(dllexport) void PushNumber(lua_State *l, lua_Number n);
		__declspec(dllexport) void PushString(lua_State *l, const char *s);
	}

}

#endif
