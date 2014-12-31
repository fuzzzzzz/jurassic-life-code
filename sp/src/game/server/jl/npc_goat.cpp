//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//
#include "cbase.h"
#include "ai_default.h"
#include "ai_task.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "soundent.h"
#include "jl/npc_goat.h"
#include "game.h"
#include "npcevent.h"
#include "entitylist.h"
#include "activitylist.h"
#include "ai_basenpc.h"
#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar	sk_goat_health( "sk_goat_health", "0" );

#define	GOAT_MODEL	"models/jl/animals/goat.mdl"

//=========================================================
// Private animevents
//=========================================================

//int GOAT_AE_FOOTSTEPWALK;
//int GOAT_AE_FOOTSTEPRUN;

//=========================================================
// Private activities
//=========================================================

//Activity ACT_GOAT_ACTIVITY;
//Activity ACT_GOAT_ACTIVITY2;

//=========================================================
// Shared interaction
//=========================================================
//int g_interactionExample = 0; // REMEMBER TO ADD THIS TO AI_Interactions.h
//int g_interactionExample2 = 0; // REMEMBER TO ADD THIS TO AI_Interactions.h

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CGoat::Precache( void )
{
	PrecacheModel( GOAT_MODEL );

	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CGoat::Spawn( void )
{
	Precache();

	SetModel( GOAT_MODEL );
	SetHullType(HULL_MEDIUM);
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );
	SetBloodColor( BLOOD_COLOR_RED );
	m_iHealth				= sk_goat_health.GetFloat();
//	m_iMaxHealth			= m_iHealth;
	m_flFieldOfView	= 0.5; // indicates the width of this NPC's forward view cone ( as a dotproduct result )
	m_NPCState	= NPC_STATE_NONE;

	CapabilitiesClear();

	CapabilitiesAdd( bits_CAP_TURN_HEAD | bits_CAP_MOVE_GROUND );
	// innate Range attack ( howling / shaking)	
	CapabilitiesAdd( bits_CAP_INNATE_RANGE_ATTACK1 | bits_CAP_INNATE_RANGE_ATTACK2 );
	
	// innate Melee attack ( charge+head attack / bite attack (from fish bite or byte missed with event )	
	CapabilitiesAdd( bits_CAP_INNATE_MELEE_ATTACK1 | bits_CAP_INNATE_MELEE_ATTACK2 );
	
	BaseClass::Spawn();
}


//-----------------------------------------------------------------------------
// Purpose: 
//
//
// Output : 
//-----------------------------------------------------------------------------
Class_T	CGoat::Classify( void )
{
	return	CLASS_EARTH_FAUNA;
}
