#include "LuaScript.h"
#include <sstream>
#include <Windows.h>
#include "Exceptions.h"

using namespace std;
using namespace Makina;

LuaScript *LuaScript::GetCallerScript(lua_State *luaState)
{
	lua_getglobal(luaState, "luaScriptPt"); // push to stack
	const LuaScript *pt = static_cast<const LuaScript *>(lua_topointer(luaState, -1)); // read
	lua_pop(luaState, 1); // pop from stack
	return const_cast<LuaScript *>(pt);
}

LuaScript::LuaScript(const char *path, void *ownerObjPt)
: mPath(path),
mOwnerObjPt(ownerObjPt),
mScriptState(LuaScriptState::Loaded)
{
	// initialize Lua
	mLuaState = luaL_newstate();
	luaL_openlibs(mLuaState);

	// register our functions
	lua_register(mLuaState, "OutputDebug", OutputDebug);

	// panic handler
	lua_atpanic(mLuaState, PanicHandler);

	// run the script
	luaL_dofile(mLuaState, path);

	// save pointer to this object
	lua_pushlightuserdata(mLuaState, this); // pushes the value on the stack
	lua_setglobal(mLuaState, "luaScriptPt"); // pops the value from the stack
}

LuaScript::~LuaScript()
{
	// cleanup Lua
	lua_close(mLuaState);
}

void LuaScript::RegisterFunction(char *name, FunctionPointer pt)
{
	lua_register(mLuaState, name, pt);
}

void LuaScript::RegisterGlobalLib(const char *name, const luaL_Reg *bitmapLib)
{
	luaL_newlib(mLuaState, bitmapLib);
	lua_setglobal(mLuaState, name);
}

int LuaScript::PanicHandler(lua_State *luaState)
{
	LuaScript *callerScript = GetCallerScript(luaState);

	wstringstream wss;
	wss << "Panic in '" << &callerScript->mPath << "': ";
	wss << lua_tostring(luaState, -1);
	throw ScriptError(wss.str());
	return 0;
}

void LuaScript::Start(const vector<string> &parameters)
{
	if (mScriptState == LuaScriptState::Running)
	{
		throw ScriptError(L"Trying to start a script that cannot be started.");
		return;
	}

	// the function name
	lua_getglobal(mLuaState, "Start");

	// push all args
	int argN = parameters.size();
	for (int i = 0; i < argN; ++i)
		lua_pushstring(mLuaState, &parameters[i][0]);

	// call the function with argN arguments, return 0 result
	if (lua_pcall(mLuaState, argN, 0, 0) != 0)
	{
		wstringstream wss;
		wss << "Panic in '" << &mPath << "': ";
		wss << lua_tostring(mLuaState, -1);
		throw ScriptError(wss.str());
	}

	// update state
	mScriptState = LuaScriptState::Running;
}

void LuaScript::Update(float dt)
{
	if (mScriptState != LuaScriptState::Running)
	{
		throw ScriptError(L"Trying to update a script that isn't running.");
		return;
	}

	// push function to the stack
	lua_getglobal(mLuaState, "Update");

	// then delta time
	lua_pushnumber(mLuaState, dt);

	// Call the function with 1 arguments, return 0 result. It removes 2 values we added from the stack.
	if (lua_pcall(mLuaState, 1, 0, 0) != 0)
	{
		wstringstream wss;
		wss << "Panic in '" << &mPath << "': ";
		wss << lua_tostring(mLuaState, -1);
		throw ScriptError(wss.str());
	}
}

void LuaScript::Stop(const vector<string> &parameters)
{
	if (mScriptState != LuaScriptState::Running)
	{
		throw ScriptError(L"Trying to stop a script that cannot be stopped.");
		return;
	}

	// the function name
	lua_getglobal(mLuaState, "Stop");

	// push all args
	int argN = parameters.size();
	for (int i = 0; i < argN; ++i)
		lua_pushstring(mLuaState, &parameters[i][0]);

	// call the function with argN arguments, return 0 result
	if (lua_pcall(mLuaState, argN, 0, 0) != 0)
	{
		wstringstream wss;
		wss << "Panic in '" << &mPath << "': ";
		wss << lua_tostring(mLuaState, -1);
		throw ScriptError(wss.str());
	}

	// update state
	mScriptState = LuaScriptState::Stopped;
}

int LuaScript::OutputDebug(lua_State *luaState)
{
	// get pointer
	LuaScript *callerScript = GetCallerScript(luaState);

	// get number of arguments
	int n = lua_gettop(luaState);
	wstringstream wss;
	int i;

	wss << "Debug from '" << &callerScript->mPath[0] << "': ";

	// loop through each argument
	for (i = 1; i <= n; ++i)
	{
		// add to string
		wss << lua_tostring(luaState, i);
	}

	wss << endl;
	wstring ws = wss.str();
	OutputDebugString(&ws[0]);

	// return the number of results
	return 0;
}

int Lua::GetTop(lua_State *l) { return lua_gettop(l); }
lua_Number Lua::ToNumber(lua_State *l, int i) { return lua_tonumber(l, i); }
int Lua::ToInteger(lua_State *l, int i) { return lua_tointeger(l, i); }
std::string Lua::ToString(lua_State *l, int i) { return lua_tostring(l, i); }
std::wstring Lua::ToWString(lua_State *l, int i) {
	string s = lua_tostring(l, i);
	wstring ws;
	ws.assign(s.begin(), s.end());
	return wstring(ws);
}
const void *Lua::ToLightUserData(lua_State *l, int i) { return lua_topointer(l, i); }
bool Lua::ToBoolean(lua_State *l, int i) { return (lua_toboolean(l, i) == 0) ? false : true; }

std::vector<float> Lua::ToFloatVector(lua_State *l, int i)
{
	luaL_checktype(l, i, LUA_TTABLE);

	lua_len(l, i);
	int len = lua_tointeger(l, -1);
	lua_pop(l, 1);
	vector<float> retVec;
	retVec.reserve(len);

	for (int j = 0; j < len; j++) 
	{
		lua_pushinteger(l, j + 1);
		lua_gettable(l, -2);
		if (lua_isnumber(l, -1)) 
		{
			retVec.push_back((float)lua_tonumber(l, -1));
		}
		else 
		{
			lua_pushfstring(l, strcat(strcat("invalid entry #%d in array argument #%d (expected number, got ", luaL_typename(l, -1)), ")"), j, i);
			lua_error(l);
		}
		lua_pop(l, 1);
	}

	return retVec;
}

void Lua::PushLightUserData(lua_State *l, void *p) { lua_pushlightuserdata(l, p); }
void Lua::PushNumber(lua_State *l, lua_Number n) { lua_pushnumber(l, n); }
void Lua::PushString(lua_State *l, const char *s) { lua_pushfstring(l, s); }