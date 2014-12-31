
#ifndef LUABIND_H
#define LUABIND_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lstate.h"

#include <string>

class LuaBind
{
public:
	LuaBind();
	~LuaBind();

	bool LoadFile( const char *filename );
	bool LoadScript( const char *script, const char *name = NULL );

	void Run();
	bool RunCommand(const char *command, bool bReportError = true);

	void RegisterFunc(const char *name, lua_CFunction func);

	void SetVarString(const char *varname, const char *value);
	void SetVarInteger(const char *varname, int value);
	void SetVarFloat(const char *varname, float value);

	std::string m_sFilename;
protected:
	lua_State *m_pState;
};

#endif
