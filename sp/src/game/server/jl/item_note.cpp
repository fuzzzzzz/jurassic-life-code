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

#define SF_REMOVE_AFTER_USED				0x0001
#define SF_BLINK							0x0002

class CJL_Item_Note : public CBaseAnimating
{
public:
	DECLARE_CLASS( CJL_Item_Note, CBaseAnimating );

	void	Spawn( void );
	void	Precache( void );
	bool	CreateVPhysics1( void );

	//FIXME: May not want to have this used in a radius
	int		ObjectCaps( void ) { return (BaseClass::ObjectCaps() | (FCAP_IMPULSE_USE|FCAP_USE_IN_RADIUS)); };
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void	InputKill( inputdata_t &data );
	
	virtual int OnTakeDamage( const CTakeDamageInfo &info );

protected:
	int m_iNoteId;
	bool m_bUsed;

	COutputEvent	m_OnUsed;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS( jl_item_note, CJL_Item_Note );

BEGIN_DATADESC( CJL_Item_Note )
	DEFINE_KEYFIELD( m_iNoteId,	FIELD_INTEGER, "NoteId" ),
	DEFINE_FIELD( m_bUsed, FIELD_BOOLEAN ),
	
	DEFINE_OUTPUT( m_OnUsed, "OnUsed" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Kill", InputKill ),
END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CJL_Item_Note::Spawn( void )
{
	Precache();

	BaseClass::Spawn();

	SetModel( STRING( GetModelName() ) );
	SetMoveType( MOVETYPE_VPHYSICS );
	SetSolid( SOLID_VPHYSICS );
	CreateVPhysics();

	m_takedamage = DAMAGE_EVENTS_ONLY;
	m_bUsed = false;

	SetSolid( SOLID_VPHYSICS );
	AddSolidFlags( FSOLID_NOT_STANDABLE );

	if (HasSpawnFlags(SF_BLINK))
	{
		SetEffects(EF_ITEM_BLINK);
	}
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
bool CJL_Item_Note::CreateVPhysics1( void )
{
	return ( VPhysicsInitStatic() != NULL );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CJL_Item_Note::Precache( void )
{
	PrecacheModel( STRING( GetModelName() ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pActivator - 
//			*pCaller - 
//			useType - 
//			value - 
//-----------------------------------------------------------------------------
void CJL_Item_Note::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if (!m_bUsed)
	{
		m_bUsed=true;
		CBasePlayer *pPlayer = ToBasePlayer( pActivator );

		if ( pPlayer == NULL )
			return;

		pPlayer->AddNote(m_iNoteId);
		m_OnUsed.FireOutput( pActivator, this );

		if (HasSpawnFlags(SF_REMOVE_AFTER_USED))
		{
			UTIL_Remove( this );
		}
		if (HasSpawnFlags(SF_BLINK))
		{
			SetEffects(0);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: allows the crate to open up when hit by a crowbar
//-----------------------------------------------------------------------------
int CJL_Item_Note::OnTakeDamage( const CTakeDamageInfo &info )
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
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CJL_Item_Note::InputKill( inputdata_t &data )
{
	UTIL_Remove( this );
}

