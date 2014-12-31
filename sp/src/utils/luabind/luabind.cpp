
#define lua_c
#include "LuaBind.h"

#include <fstream>
#include <string>

//g_pFullFileSystem
#include "filesystem.h"
//#include "game/client/cbase.h"
#include "tier0/dbg.h"

int LuaDevMsg(lua_State *pState)
{
	if (lua_gettop(pState)>=1)
	{
		if (lua_isstring(pState,1))
		{			
			DevMsg(lua_tolstring(pState,1,NULL));
		}
	}
	return 0;
}

LuaBind::LuaBind()
{
	m_pState = lua_open();	
	if (m_pState)
	{
		m_pState->userData = this;
		//lua_pushlightuserdata()
		luaopen_table(m_pState);
		luaopen_string(m_pState);
		luaopen_math(m_pState);
	}

	RegisterFunc("DevMsg",LuaDevMsg);
}

LuaBind::~LuaBind()
{
	if (m_pState)
	{
		lua_close(m_pState);
	}
}

void LuaBind::Run()
{
	if (m_pState)
	{
		int status;
		int base = lua_gettop(m_pState); 
		status = lua_pcall(m_pState, 0, 0, base);
		lua_remove(m_pState, base);
		if (status != 0) 
			lua_gc(m_pState, LUA_GCCOLLECT, 0);
		if (status != 0)
		{
			DevMsg("LUA run error: %s\n",lua_tostring(m_pState,-1));
		}
	}
}

bool LuaBind::RunCommand(const char *command, bool bReportError)
{
	if (m_pState && command)
	{
		int status = luaL_dostring(m_pState,command);
		if (status != 0)
		{
			if (bReportError)
				DevWarning("LUA run command error : %s\n", lua_tostring(m_pState,-1));
			return false;
		}
		return true;
	}
}

bool LuaBind::LoadFile( const char *filename )
{
	std::string script;

	FileHandle_t file = g_pFullFileSystem->Open(filename, "rb", NULL);
	if ( !file )
	{
		DevWarning("Unable to load LUA file : %s\n", filename);
		return false;
	}

	int fileSize = g_pFullFileSystem->Size( file );
	script.resize(fileSize, ' ');

	bool bRetOK = ( ((IFileSystem *)g_pFullFileSystem)->ReadEx( (void*)script.data(), fileSize, fileSize, file ) != 0 );

	g_pFullFileSystem->Close( file );	// close file after reading
	
	//DevMsg("Script : \n%s\n",script.c_str());
	m_sFilename = filename;
	bool status = LoadScript(script.c_str(), filename);
	if (!status)
	{

	}
	return status;
}

bool LuaBind::LoadScript( const char *script, const char *name )
{
	if (m_pState && script)
	{
		int status = luaL_loadbuffer(m_pState, script, strlen(script), name ? name : "vgui_screen");
		
		if (status != 0)
		{
			const char *msg = lua_tostring(m_pState, -1);
			DevWarning("LuaBind error : \n%s\n", msg);
		}
		return (status == 0);
	}
	return false;
}

void LuaBind::RegisterFunc(const char *name, lua_CFunction func)
{
	if (m_pState)
	{
		lua_register(m_pState, name, func);
	}
}

void LuaBind::SetVarString(const char *varname, const char *value)
{
	if (m_pState && varname && value)
	{
		//lua_getglobal(m_pState,"");
		lua_pushstring(m_pState, value);
		lua_setglobal(m_pState, varname);
	}
}

void LuaBind::SetVarInteger(const char *varname, int value)
{
	if (m_pState && varname && value)
	{
		lua_pushinteger(m_pState, value);
		lua_setglobal(m_pState, varname);
	}
}

void LuaBind::SetVarFloat(const char *varname, float value)
{
	if (m_pState && varname && value)
	{
		lua_pushnumber(m_pState, value);
		lua_setglobal(m_pState, varname);
	}
}
