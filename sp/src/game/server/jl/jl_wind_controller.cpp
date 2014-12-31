//=================================================================================================================================

// JURASSIC LIFE WIND CONTROLLER

#include "cbase.h"
#include "jl/jl_wind_controller.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CJuraLifeWindController *g_juraLifeWindController;

CJuraLifeWindController *GP_JL_GetWindController(){ return g_juraLifeWindController; }

BEGIN_DATADESC( CJuraLifeWindController )

	DEFINE_KEYFIELD( m_bWind, FIELD_BOOLEAN, "wind"),
	DEFINE_KEYFIELD( m_iDirection, FIELD_INTEGER, "direction"),
	DEFINE_KEYFIELD( m_iIntensity, FIELD_INTEGER, "intensity"),

	DEFINE_INPUTFUNC( FIELD_VOID, "ToggleWind", InputToggleWind),
	DEFINE_INPUTFUNC( FIELD_VOID, "StartWind", InputStartWind),
	DEFINE_INPUTFUNC( FIELD_VOID, "EndWind", InputEndWind),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "ChangeDirection", InputChangeDirection),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "ChangeIntensity", InputChangeIntensity),

END_DATADESC()

LINK_ENTITY_TO_CLASS(jl_wind_controller, CJuraLifeWindController);

void CJuraLifeWindController::Spawn( void ){
	g_juraLifeWindController = this;
	BaseClass::Spawn();
    }

void CJuraLifeWindController::UpdateFoliage(){

	if( m_iIntensity < 0 || m_iIntensity > 3 ){

		Warning( "** WIND CONTROLLER ERROR ** wind outside of acceptable threshold!\n" );
		m_iIntensity = 0;

	}

	if( m_iDirection < 0 || m_iDirection > 360 ){
		Warning( "** WIND CONTROLLER ERROR ** direction outside of acceptable threshold!\n" );
		m_iDirection = 0;
	}
	
switch( m_iDirection )
	{

	case 0:
	case 45:
	case 90:
	case 135:
	case 180:
	case 225:
	case 270:
	case 315:
	case 360:

		for( int i = 0; i < m_vecFoliage.Count(); i++ )
		{
			m_vecFoliage.Element( i )->Touch( this );
		}
		break;

	default:
		Warning( "** WIND DIRECTION NOT A VALID COMPASS DIRECTION! **\n" );
		m_iDirection = 0;
		break;

	}
}