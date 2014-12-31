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
			  
#include <game/client/iviewport.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// ==================================================================
// Ammo crate which will supply infinite ammo of the specified type
// ==================================================================

// Ammo types
enum
{
	AMMOCRATE,
	NUM_AMMO_CRATE_TYPES,
};

// Ammo crate

class CJL_Item_Case : public CBaseAnimating
{
public:
	DECLARE_CLASS( CJL_Item_Case, CBaseAnimating );

	void	Spawn( void );
	void	Precache( void );
	bool	CreateVPhysics( void );

	virtual void HandleAnimEvent( animevent_t *pEvent );

	void	SetupCrate( void );
	void	OnRestore( void );

	//FIXME: May not want to have this used in a radius
	int		ObjectCaps( void ) { return (BaseClass::ObjectCaps() | (FCAP_IMPULSE_USE|FCAP_USE_IN_RADIUS)); };
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void	InputKill( inputdata_t &data );
	void	CrateThink( void );

	void	Close( void );
	void	Open( void );
	
	virtual int OnTakeDamage( const CTakeDamageInfo &info );

protected:

	int		m_nAmmoType;
	int		m_nAmmoIndex;

	static const char *m_lpzModelNames;


	float	m_flCloseTime;
	COutputEvent	m_OnUsed;
	CHandle< CBasePlayer > m_hActivator;

	bool open;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS( jl_item_case, CJL_Item_Case );

BEGIN_DATADESC( CJL_Item_Case )

	//DEFINE_KEYFIELD( m_nAmmoType,	FIELD_INTEGER, "AmmoType" ),	

	//DEFINE_FIELD( m_flCloseTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_hActivator, FIELD_EHANDLE ),

	// These can be recreated
	//DEFINE_FIELD( m_nAmmoIndex,		FIELD_INTEGER ),
	//DEFINE_FIELD( m_lpzModelNames,	FIELD_ ),
	//DEFINE_FIELD( m_lpzAmmoNames,	FIELD_ ),
	//DEFINE_FIELD( m_nAmmoAmounts,	FIELD_INTEGER ),
	//DEFINE_AUTO_ARRAY( m_nItems,FIELD_INTEGER),

	DEFINE_OUTPUT( m_OnUsed, "OnUsed" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Kill", InputKill ),

	DEFINE_THINKFUNC( CrateThink ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Animation events.
//-----------------------------------------------------------------------------

// Models names
const char *CJL_Item_Case::m_lpzModelNames = "models/items/ammocrate.mdl";

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CJL_Item_Case::Spawn( void )
{
	Precache();

	BaseClass::Spawn();

	SetModel( STRING( GetModelName() ) );
	SetMoveType( MOVETYPE_NONE );
	SetSolid( SOLID_VPHYSICS );
	CreateVPhysics();

	ResetSequence( LookupSequence( "Idle" ) );
	SetBodygroup( 1, true );

	m_flCloseTime = gpGlobals->curtime;
	m_flAnimTime = gpGlobals->curtime;
	m_flPlaybackRate = 0.0;
	SetCycle( 0 );

	m_takedamage = DAMAGE_EVENTS_ONLY;

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );

	open=true;
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
bool CJL_Item_Case::CreateVPhysics( void )
{
	return ( VPhysicsInitStatic() != NULL );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CJL_Item_Case::Precache( void )
{
	SetupCrate();
	PrecacheModel( STRING( GetModelName() ) );

	PrecacheScriptSound( "AmmoCrate.Open" );
	PrecacheScriptSound( "AmmoCrate.Close" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CJL_Item_Case::SetupCrate( void )
{
	SetModelName( AllocPooledString( m_lpzModelNames ) );
	
	//m_nAmmoIndex = GetAmmoDef()->Index( m_lpzAmmoNames[m_nAmmoType] );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CJL_Item_Case::OnRestore( void )
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
void CJL_Item_Case::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = ToBasePlayer( pActivator );

	if ( pPlayer == NULL )
		return;

	m_OnUsed.FireOutput( pActivator, this );

	int iSequence = LookupSequence( "Open" );

	// See if we're not opening already
	if ( GetSequence() != iSequence )
	{
		Vector mins, maxs;
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
			
		m_hActivator = pPlayer;

		// Animate!
		ResetSequence( iSequence );

		// Make sound
		CPASAttenuationFilter sndFilter( this, "AmmoCrate.Open" );
		EmitSound( sndFilter, entindex(), "AmmoCrate.Open" );

		// Start thinking to make it return
		SetThink( &CJL_Item_Case::CrateThink );
		SetNextThink( gpGlobals->curtime + 0.1f );
	}
	
	ConVar *pOpen  = cvar->FindVar( "jl_showcasepanel" );
	pOpen->SetValue( 1 );
	open = true;
}

//-----------------------------------------------------------------------------
// Purpose: allows the crate to open up when hit by a crowbar
//-----------------------------------------------------------------------------
int CJL_Item_Case::OnTakeDamage( const CTakeDamageInfo &info )
{
	// if it's the player hitting us with a crowbar, open up
	CBasePlayer *player = ToBasePlayer(info.GetAttacker());
	if (player)
	{
		CBaseCombatWeapon *weapon = player->GetActiveWeapon();

		if (weapon && !stricmp(weapon->GetName(), "weapon_crowbar"))
		{
			// play the normal use sound
			player->EmitSound( "HL2Player.Use" );
			// open the crate
			//Use(info.GetAttacker(), info.GetAttacker(), USE_TOGGLE, 0.0f);
		}
	}

	// don't actually take any damage
	return 0;
}


//-----------------------------------------------------------------------------
// Purpose: Catches the monster-specific messages that occur when tagged
//			animation frames are played.
// Input  : *pEvent - 
//-----------------------------------------------------------------------------
void CJL_Item_Case::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_AMMOCRATE_PICKUP_AMMO )
	{
		if ( m_hActivator )
		{
			//if ( m_spawnflags & SF_CASE_SHOTGUN )
				//m_hActivator->Weapon_Create("Weapon_Pistol");

			m_hActivator = NULL;
		}
		return;
	}

	BaseClass::HandleAnimEvent( pEvent );
}

	
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CJL_Item_Case::CrateThink( void )
{
	StudioFrameAdvance();
	DispatchAnimEvents( this );

	SetNextThink( gpGlobals->curtime + 0.1f );

	if ( open == true)
	{
		if ( GetSequence() == LookupSequence( "Open" ) )
			if ( IsSequenceFinished() )
			{
				ConVar *pOpen  = cvar->FindVar( "jl_showcasepanel" );
				//DevMsg("Var %d %d \n",pOpen->GetInt(),open);
				if (!pOpen->GetInt()) 
				{
					open = false;
					ResetSequence( LookupSequence( "Close" ) );
					CPASAttenuationFilter sndFilter( this, "AmmoCrate.Close" );
					EmitSound( sndFilter, entindex(), "AmmoCrate.Close" );
				}
			}

	}else{
		if ( GetSequence() == LookupSequence( "Close" ) && IsSequenceFinished() )
		{
			ResetSequence( LookupSequence( "Idle" ) );
			//SetBodygroup( 1, true );
			//SetThink( NULL );			
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CJL_Item_Case::InputKill( inputdata_t &data )
{
	UTIL_Remove( this );
}

void CJL_Item_Case::Close( void )
{
	open = false;
}

void CJL_Item_Case::Open( void )
{
	open = true;
}
