#include "cbase.h"
#include "jl/jl_wind_controller.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define JL_PROP_FOLIAGE_ANIM_PREFIX "anim_"
#define JL_PROP_FOLIAGE_ANIM_INTENSITY_PREFIX "_int_"

class CJuraLifePropFoliage : public CBaseAnimating
{
public:

	DECLARE_CLASS( CJuraLifePropFoliage, CBaseAnimating);
	DECLARE_DATADESC();

	CJuraLifePropFoliage(){
		m_strModelName = "";
	}

	void Spawn();
	void Precache();
	void Activate();
	void Touch( CBaseEntity *pOther );

private:

	char* m_strModelName; // model string name
   

};

BEGIN_DATADESC( CJuraLifePropFoliage )

	DEFINE_KEYFIELD( m_strModelName, FIELD_STRING, "model"),

END_DATADESC()

LINK_ENTITY_TO_CLASS(jl_prop_tree, CJuraLifePropFoliage);

//////////////////////////////

// FUNCTIONS

//////////////////////////////


// Precache
void CJuraLifePropFoliage::Precache( void )
{

	PrecacheModel( m_strModelName );

	BaseClass::Precache();

}

// SPAWN
void CJuraLifePropFoliage::Spawn( void )
{
	Precache();

	SetModel( m_strModelName );
	SetSolid( SOLID_VPHYSICS );
	//SetMoveType( MOVETYPE_PUSH );
	VPhysicsInitShadow( false, false );

	SetSequence( LookupSequence( "idle" ) ); // COMMENT OUT IF NO IDLE ANIMATION
	SetPlaybackRate( 1.0f );
	UseClientSideAnimation();

}

// ACTIVATE
void CJuraLifePropFoliage::Activate( void ){
	Precache();

	GP_JL_GetWindController()->AddToWindList( this );
}

void CJuraLifePropFoliage::Touch( CBaseEntity *pOther ){

	CJuraLifeWindController *control = (CJuraLifeWindController *)pOther;
	
	if( !control ) // no controlling entity?
		return;

	// don't bother animating with zero intensity
	if( control->GetIntensity() == 0 && control->GetShouldAnimate() )
	{
		SetSequence( LookupSequence( "idle" ) ); // CHANGE TO DEFAULT IF NO IDLE ANIMATION
		SetPlaybackRate( 1.0f );
		UseClientSideAnimation();
		return;
	}

	char *animation = JL_PROP_FOLIAGE_ANIM_PREFIX;
	char *direction = "";
	char *intensity = "";
	int closest = 360;
	int angle = GetAbsAngles().z;
	if( angle == 360 )
		angle = 0;
	int dirs[8] = {0, 45, 90, 135, 180, 225, 270, 315}; // compass directions
	for( int i = 0; i < 8; i++ ){
		if( abs(dirs[i] - angle) < closest )
			closest = dirs[i];
	}

	int mod_direction = control->GetDirection() - closest;
	if( mod_direction < 0 )
		mod_direction = 360 + mod_direction;

	switch( mod_direction )
	{
	case 0:
		direction = "N";
		break;
	case 45:
		direction = "NE";
		break;
	case 90:
		direction = "E";
		break;
	case 135:
		direction = "SE";
		break;
	case 180:
		direction = "S";
		break;
	case 225:
		direction = "SW";
		break;
	case 270:
		direction = "W";
		break;
	case 315:
		direction = "NW";
		break;
	default:
		Warning( "**FOLIAGE ERROR, UNKNOWN DIRECTION**\n");
		return;
		break;
	}
	switch( control->GetIntensity() )
	{
	case 1:
		intensity = "1";
		break;
	case 2:
		intensity = "2";
		break;
	case 3:
		intensity = "3";
		break;
	default:
		Warning( "**FOLIAGE ERROR, UNKNOWN INTENSITY**\n");
		return;
		break;
	}

	strcat( animation, direction ); // compile our string
	strcat( animation, JL_PROP_FOLIAGE_ANIM_INTENSITY_PREFIX );
		strcat( animation, intensity );

   SetSequence( LookupSequence( animation ) );
   SetPlaybackRate( 1.0f );
   UseClientSideAnimation();
   return;

}