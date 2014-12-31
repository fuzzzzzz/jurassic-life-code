//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Crate of items for Jurassic-Life
//
//=============================================================================//

#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "items.h"
#include "ammodef.h"
#include "eventlist.h"
#include "npcevent.h"

//#include <cl_dll/iviewport.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"



// ==================================================================
// Ammo crate which will supply infinite ammo of the specified type
// ==================================================================

// First aid crate

class CJL_Item_First_Aid : public CBaseAnimating
{
public:
	DECLARE_CLASS( CJL_Item_First_Aid, CBaseAnimating );

	void	Spawn( void );
	void	Precache( void );
	bool	CreateVPhysics( void );

	//virtual void HandleAnimEvent( animevent_t *pEvent );

	void	SetupCrate( void );
	void	OnRestore( void );

	//FIXME: May not want to have this used in a radius
	int		ObjectCaps( void ) { return (BaseClass::ObjectCaps() | (FCAP_IMPULSE_USE|FCAP_USE_IN_RADIUS)); };
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void	InputKill( inputdata_t &data );
	void	CrateThink( void );
	

protected:
	static const char *m_lpzModelNames;

	COutputEvent	m_OnUsed;
	CHandle< CBasePlayer > m_hActivator;

	bool	m_bUse;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS( jl_item_firstaid, CJL_Item_First_Aid );

BEGIN_DATADESC( CJL_Item_First_Aid )

	//DEFINE_KEYFIELD( m_nAmmoType,	FIELD_INTEGER, "AmmoType" ),	

	//DEFINE_FIELD( m_flCloseTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_hActivator, FIELD_EHANDLE ),

	// These can be recreated
	//DEFINE_FIELD( m_nAmmoIndex,		FIELD_INTEGER ),
	//DEFINE_FIELD( m_lpzModelNames,	FIELD_ ),
	//DEFINE_FIELD( m_lpzAmmoNames,	FIELD_ ),
	//DEFINE_FIELD( m_nAmmoAmounts,	FIELD_INTEGER ),
	//DEFINE_AUTO_ARRAY( m_nItems,FIELD_INTEGER),
	DEFINE_FIELD( m_bUse, FIELD_BOOLEAN ),

	DEFINE_OUTPUT( m_OnUsed, "OnUsed" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Kill", InputKill ),

	DEFINE_THINKFUNC( CrateThink ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Animation events.
//-----------------------------------------------------------------------------

// Models names
const char *CJL_Item_First_Aid::m_lpzModelNames = "models/props_c17/powerbox.mdl";

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CJL_Item_First_Aid::Spawn( void )
{
	Precache();

	BaseClass::Spawn();

	SetModel( STRING( GetModelName() ) );
	SetMoveType( MOVETYPE_NONE );
	SetSolid( SOLID_VPHYSICS );
	CreateVPhysics();

	ResetSequence( LookupSequence( "Idle" ) );
	SetBodygroup( 1, true );

//	m_flCloseTime = gpGlobals->curtime;
	m_flAnimTime = gpGlobals->curtime;
	m_flPlaybackRate = 0.0;
	SetCycle( 0 );

	m_takedamage = DAMAGE_EVENTS_ONLY;

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );

	m_bUse=false;
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
bool CJL_Item_First_Aid::CreateVPhysics( void )
{
	return ( VPhysicsInitStatic() != NULL );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CJL_Item_First_Aid::Precache( void )
{
	SetupCrate();
	PrecacheModel( STRING( GetModelName() ) );

	//PrecacheScriptSound( "AmmoCrate.Open" );
	//PrecacheScriptSound( "AmmoCrate.Close" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CJL_Item_First_Aid::SetupCrate( void )
{
	SetModelName( AllocPooledString( m_lpzModelNames ) );
	
	//m_nAmmoIndex = GetAmmoDef()->Index( m_lpzAmmoNames[m_nAmmoType] );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CJL_Item_First_Aid::OnRestore( void )
{
	BaseClass::OnRestore();

	// Restore our internal state
	SetupCrate();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pActivator - 
//			*pCaller - 
//			useType - 
//			value - 
//-----------------------------------------------------------------------------
void CJL_Item_First_Aid::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = ToBasePlayer( pActivator );


	if ( pPlayer == NULL )
		return;

	if ( !(pPlayer->GetHealth()<100) || m_bUse==true)
		return;

	DevMsg("First Aid\n");

	m_OnUsed.FireOutput( pActivator, this );

	int iSequence = LookupSequence( "openPowerBox" );

	// See if we're not opening already
	if ( GetSequence() != iSequence )
	{
		/*Vector mins, maxs;
		trace_t tr;

		CollisionProp()->WorldSpaceAABB( &mins, &maxs );

		Vector vOrigin = GetAbsOrigin();
		vOrigin.z += ( maxs.z - mins.z );
		mins = (mins - GetAbsOrigin()) * 0.2f;
		maxs = (maxs - GetAbsOrigin()) * 0.2f;
		mins.z = ( GetAbsOrigin().z - vOrigin.z );  
		
		UTIL_TraceHull( vOrigin, vOrigin, mins, maxs, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

		if ( tr.startsolid || tr.allsolid )
			 return;
			
		m_hActivator = pPlayer;*/

		// Animate!
		ResetSequence( iSequence );

		// Make sound
		CPASAttenuationFilter sndFilter( this, "AmmoCrate.Open" );
		EmitSound( sndFilter, entindex(), "AmmoCrate.Open" );

		// Start thinking to make it return
		SetThink( &CJL_Item_First_Aid::CrateThink );
		SetNextThink( gpGlobals->curtime + 0.1f );
	}

	//ResetSequence( LookupSequence( "openPowerBox" ) );
	m_bUse = true;

	if (pPlayer->GetHealth()+75>100)
		pPlayer->SetHealth(100);
	else
		pPlayer->SetHealth(pPlayer->GetHealth()+75);

}

//-----------------------------------------------------------------------------
// Purpose: Catches the monster-specific messages that occur when tagged
//			animation frames are played.
// Input  : *pEvent - 
//-----------------------------------------------------------------------------
	
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CJL_Item_First_Aid::CrateThink( void )
{
	StudioFrameAdvance();
	DispatchAnimEvents( this );

	SetNextThink( gpGlobals->curtime + 0.1f );

	if ( m_bUse == true)
	{
		if ( GetSequence() == LookupSequence( "openPowerBox" ) )
		{
			if ( IsSequenceFinished() )
			{
				ResetSequence( LookupSequence( "hackPowerBox" ) );
			}
		}else
			ResetSequence( LookupSequence( "hackPowerBox" ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CJL_Item_First_Aid::InputKill( inputdata_t &data )
{
	UTIL_Remove( this );
}