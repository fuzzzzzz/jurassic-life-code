#include "cbase.h"
#include "igamesystem.h"
#include "steam/steam_api.h"



//static ConVar jl_product_version("jl_product_version", 0, FCVAR_ARCHIVE, "display the production version into the console.");

//#define JL_BIN_REV "0001"

class C_LogGameStart : public CAutoGameSystem
{
public: 
	//C_LogGameStart() {};

	C_LogGameStart() : CAutoGameSystem( "JL_Binaries_Revision" )
	{
	}

	//~C_LogGameStart() //{};
	//{
	//	GetLogGameStartGamesystem();
	//}

	// Static singleton accessor
	//static C_LogGameStart		*GetLogGameStartGamesystem();

	virtual void Post_Init()
	//virtual void GetLogGameStartGameSystem()
	{
		Color color(255, 210, 18, 255);
		ConColorMsg( color, "====================================================\n");
		ConColorMsg( color ," Jurassic Life binaries revision build number\n");
		ConColorMsg( color, " Last Build date %s, at %s \n", __TIME__, __DATE__ );
		ConColorMsg( color, "====================================================\n");
	
		//if (jl_product_version.GetBool() == true)
		//{
		//	//jl_product_version.SetValue(true);
		//	ConColorMsg( color, "====================================================\n");
		//	ConColorMsg( color ," Jurassic Life binaries revision build number", JL_BIN_REV "\n");
		//	ConColorMsg( color, " Last Build date %s, at %s \n", __TIME__, __DATE__ );
		//	ConColorMsg( color, "====================================================\n");
		//	DevMsg( "product_version (test)\n");
		//}
	}
	virtual void Shutdown() {};
};
static C_LogGameStart g_LogGameStart;
/*
C_LogGameStart *GetLogGameStartGamesystem()
{
	return &g_LogGameStart;
}
*/