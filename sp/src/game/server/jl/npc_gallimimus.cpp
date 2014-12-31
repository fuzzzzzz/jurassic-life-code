//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Gallimimus - nasty bug
//
//=============================================================================//

#include "cbase.h"
#include "ai_hint.h"
//#include "ai_squad.h"
#include "ai_moveprobe.h"
#include "ai_default.h"
#include "ai_route.h"
#include "ai_hull.h"
#include "npcevent.h"
#include "gib.h"
#include "entitylist.h"
#include "ndebugoverlay.h"
#include "antlion_dust.h"
#include "engine/IEngineSound.h"
#include "globalstate.h"
#include "movevars_shared.h"
#include "te_effect_dispatch.h"
#include "vehicle_base.h"
#include "mapentities.h"
//#include "antlion_maker.h"
#include "jl/npc_gallimimus.h"
#include "decals.h"
#include "hl2_shareddefs.h"
#include "explode.h"
//#include "weapon_physcannon.h"
#include "baseparticleentity.h"
#include "props.h"
#include "particle_parse.h"
#include "ai_tacticalservices.h"

//#ifdef HL2_EPISODIC
//#include "grenade_spit.h"
//#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//Debug visualization
ConVar	g_debug_gallimimus( "g_debug_gallimimus", "0" );

// base Gallimimus stuff
ConVar	sk_gallimimus_health( "sk_gallimimus_health", "0" );
//ConVar	sk_gallimimus_swipe_damage( "sk_gallimimus_swipe_damage", "0" ); //fix me
//ConVar	sk_gallimimus_jump_damage( "sk_gallimimus_jump_damage", "0" ); //fix me
//ConVar  sk_gallimimus_air_attack_dmg( "sk_gallimimus_air_attack_dmg", "0" ); // fixme

//ConVar  g_test_new_gallimimus_jump( "g_test_new_gallimimus_jump", "1", FCVAR_ARCHIVE );
//ConVar	gallimimus_easycrush( "gallimimus_easycrush", "1" ); // fix me

int AE_GALLIMIMUS_WALK_FOOTSTEP;
int AE_GALLIMIMUS_FOOTSTEP_SOFT;
int AE_GALLIMIMUS_FOOTSTEP_HEAVY;
//int AE_GALLIMIMUS_START_JUMP;

//#define	GALLIMIMUS_MODEL			"models/jl/dinosaurs/gallimimus.mdl"

//Jump range definitions

//#define	GALLIMIMUS_JUMP_MIN			128.0f
//#define	GALLIMIMUS_JUMP_MAX_RISE		512.0f
//#define	GALLIMIMUS_JUMP_MAX			1024.0f


//==================================================
// GallimimusSquadSlots
//==================================================
/*
enum
{	
	SQUAD_SLOT_GALLIMIMUS_JUMP = LAST_SHARED_SQUADSLOT,

};
*/

//==================================================
// Gallimimus Activities
//==================================================

//int ACT_GALLIMIMUS_JUMP_START;
int	ACT_GALLIMIMUS_DISTRACT;
int ACT_GALLIMIMUS_DISTRACT_ARRIVED;
int	ACT_GALLIMIMUS_RUN_AGITATED;
int ACT_GALLIMIMUS_FLIP;
int ACT_GALLIMIMUS_ZAP_FLIP;
int ACT_GALLIMIMUS_DROWN;
//int ACT_GALLIMIMUS_LAND;

//==================================================
// CNPC_Gallimimus
//==================================================

CNPC_Gallimimus::CNPC_Gallimimus( void )
{
	m_flIdleDelay	= 0.0f;
//	m_flJumpTime	= 0.0f;
	m_flAlertRadius	= 256.0f;
	m_flFieldOfView	= -0.5f;
	m_bAgitatedSound	= false;
	m_flIgnoreSoundTime	= 0.0f;
	m_bHasHeardSound	= false;
//	m_flNextJumpPushTime = 0.0f;
//	m_vecLastJumpAttempt.Init();
//	m_vecSavedJump.Init();
	m_bLoopingStarted = false;
//	m_bForcedStuckJump = false;
	m_nBodyBone = -1;
}

LINK_ENTITY_TO_CLASS( npc_Gallimimus, CNPC_Gallimimus );

//==================================================
// CNPC_Gallimimus::m_DataDesc
//==================================================

BEGIN_DATADESC( CNPC_Gallimimus )

	DEFINE_KEYFIELD( m_flAlertRadius,		FIELD_FLOAT,	"radius" ),


	DEFINE_FIELD( m_flIdleDelay,			FIELD_TIME ),
//	DEFINE_FIELD( m_flJumpTime,				FIELD_TIME ),
//	DEFINE_FIELD( m_vecSavedJump,			FIELD_VECTOR ),
//	DEFINE_FIELD( m_vecLastJumpAttempt,		FIELD_VECTOR ),

	DEFINE_FIELD( m_flIgnoreSoundTime,		FIELD_TIME ),
	DEFINE_FIELD( m_vecHeardSound,			FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_bHasHeardSound,			FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bAgitatedSound,			FIELD_BOOLEAN ),
	//DEFINE_FIELD( m_strParentSpawner,		FIELD_STRING ),

	//DEFINE_FIELD( m_MoveState,				FIELD_INTEGER ),
//	DEFINE_FIELD( m_bDisableJump,			FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flTimeDrown,			FIELD_TIME ),
	DEFINE_FIELD( m_flTimeDrownSplash,		FIELD_TIME ),
//	DEFINE_FIELD( m_flNextJumpPushTime,		FIELD_TIME ),
//	DEFINE_FIELD( m_bForcedStuckJump,		FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flZapDuration,			FIELD_TIME ),
//#if HL2_EPISODIC
//	DEFINE_FIELD( m_bHasDoneAirAttack,		FIELD_BOOLEAN ),
//#endif	
	// DEFINE_FIELD( m_bLoopingStarted, FIELD_BOOLEAN ),
	//			  m_FollowBehavior
	//			  m_AssaultBehavior
	
//	DEFINE_INPUTFUNC( FIELD_VOID,	"EnableJump", InputEnableJump ),
//	DEFINE_INPUTFUNC( FIELD_VOID,	"DisableJump", InputDisableJump ),
//	DEFINE_INPUTFUNC( FIELD_STRING,	"JumpAtTarget", InputJumpAtTarget ),

	
	// Function Pointers
	DEFINE_ENTITYFUNC( Touch ),
	DEFINE_THINKFUNC( ZapThink ),

	// DEFINE_FIELD( FIELD_SHORT, m_hFootstep ),  // test that (disable by default)
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::Spawn( void )
{
	Precache();

#ifdef _XBOX
	// Always fade the corpse
	AddSpawnFlags( SF_NPC_FADE_CORPSE );
#endif // _XBOX

	SetModel( "models/jl/dinosaurs/gallimimus.mdl" );
	SetBloodColor( BLOOD_COLOR_RED );
	SetHullType (HULL_WIDE_SHORT);
	SetHullSizeNormal();
	SetDefaultEyeOffset();
	
	SetNavType( NAV_GROUND );

	m_NPCState	= NPC_STATE_NONE;

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );

	SetMoveType( MOVETYPE_STEP );

	SetCollisionGroup( JLCOLLISION_GROUP_GALLI );

	CapabilitiesAdd( bits_CAP_MOVE_GROUND ); //| bits_CAP_MOVE_JUMP );
	

	// JAY: Optimize these out for now
//	if ( HasSpawnFlags( SF_GALLIMIMUS_USE_GROUNDCHECKS ) == false )
//		 CapabilitiesAdd( bits_CAP_SKIP_NAV_GROUND_CHECK );

	NPCInit();

	m_iHealth	= sk_gallimimus_health.GetFloat();
	// Gallimimuss will always pursue
	m_flDistTooFar = FLT_MAX;

//	m_bDisableJump = false;
		
	BaseClass::Spawn();

	//m_nSkin = random->RandomInt( 0, GALLIMIMUS_SKIN_COUNT-1 ); // fixme
}

#define	NUM_GALLIMIMUS_GIBS_UNIQUE	3
#define	NUM_GALLIMIMUS_GIBS_MEDIUM	3
#define	NUM_GALLIMIMUS_GIBS_SMALL	3
//-----------------------------------------------------------------------------
// Purpose: override this to simplify the physics shadow of the GALLIMIMUSs
//-----------------------------------------------------------------------------
bool CNPC_Gallimimus::CreateVPhysics()
{
	bool bRet = BaseClass::CreateVPhysics();
	return bRet;
}

// Use all the gibs
//#define	NUM_GALLIMIMUS_GIBS_UNIQUE	3
const char *pszGallimimusGibs_Unique[NUM_GALLIMIMUS_GIBS_UNIQUE] = {
	"models/gibs/antlion_gib_large_1.mdl",
	"models/gibs/antlion_gib_large_2.mdl",
	"models/gibs/antlion_gib_large_3.mdl"
};

//#define	NUM_GALLIMIMUS_GIBS_MEDIUM	3
const char *pszGallimimusGibs_Medium[NUM_GALLIMIMUS_GIBS_MEDIUM] = {
	"models/gibs/antlion_gib_medium_1.mdl",
	"models/gibs/antlion_gib_medium_2.mdl",
	"models/gibs/antlion_gib_medium_3.mdl"
};

// XBox doesn't use the smaller gibs, so don't cache them
//#define	NUM_GALLIMIMUS_GIBS_SMALL	3
const char *pszGallimimusGibs_Small[NUM_GALLIMIMUS_GIBS_SMALL] = {
	"models/gibs/antlion_gib_small_1.mdl",
	"models/gibs/antlion_gib_small_2.mdl",
	"models/gibs/antlion_gib_small_3.mdl"

};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::Precache( void )
{
	{
		PrecacheModel( "models/jl/dinosaurs/gallimimus.mdl" );
		PropBreakablePrecacheAll( MAKE_STRING( "models/jl/dinosaurs/gallimimus.mdl" ) );
		PrecacheParticleSystem( "blood_impact_antlion_01" );
		PrecacheParticleSystem( "AntlionGib" );
		
	}

	for ( int i = 0; i < NUM_GALLIMIMUS_GIBS_UNIQUE; ++i )
	{
		PrecacheModel( pszGallimimusGibs_Unique[ i ] );
	}
	for ( int i = 0; i < NUM_GALLIMIMUS_GIBS_MEDIUM; ++i )
	{
		PrecacheModel( pszGallimimusGibs_Medium[ i ] );
	}
	for ( int i = 0; i < NUM_GALLIMIMUS_GIBS_SMALL; ++i )
	{
		PrecacheModel( pszGallimimusGibs_Small[ i ] );
	}


	PrecacheScriptSound( "NPC_Gallimimus.RunOverByVehicle" );
	//PrecacheScriptSound( "NPC_Gallimimus.MeleeAttack" );
	m_hFootstep = PrecacheScriptSound( "NPC_Gallimimus.Footstep" );
	PrecacheScriptSound( "NPC_Gallimimus.FootstepSoft" );
	PrecacheScriptSound( "NPC_Gallimimus.FootstepHeavy" );
	//PrecacheScriptSound( "NPC_Gallimimus.MeleeAttackSingle" );
	//PrecacheScriptSound( "NPC_Gallimimus.MeleeAttackDouble" );
	PrecacheScriptSound( "NPC_Gallimimus.Distracted" );
	PrecacheScriptSound( "NPC_Gallimimus.Idle" );
	PrecacheScriptSound( "NPC_Gallimimus.Pain" );
	//PrecacheScriptSound( "NPC_Gallimimus.Land" );
	PrecacheScriptSound( "NPC_Gallimimus.LoopingAgitated" );
	PrecacheScriptSound( "NPC_Gallimimus.Distracted" );

/*
#ifdef HL2_EPISODIC
	PrecacheScriptSound( "NPC_Gallimimus.PoisonBurstScream" );
	PrecacheScriptSound( "NPC_Gallimimus.PoisonBurstScreamSubmerged" );
	PrecacheScriptSound( "NPC_Gallimimus.PoisonBurstExplode" );
	PrecacheScriptSound( "NPC_Gallimimus.MeleeAttack_Muffled" );
	PrecacheScriptSound( "NPC_Gallimimus.TrappedMetal" );
	PrecacheScriptSound( "NPC_Gallimimus.ZappedFlip" );
	PrecacheScriptSound( "NPC_Gallimimus.PoisonShoot" );
	PrecacheScriptSound( "NPC_Gallimimus.PoisonBall" );
#endif
*/
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CNPC_Gallimimus::MaxYawSpeed( void )
{
	switch ( GetActivity() )
	{
	case ACT_IDLE:		
		return 32.0f;
		break;
	
	case ACT_WALK:
		return 250.0f; //24;Of;
		break;
	
	default:
	case ACT_RUN:
		return 400.0f; // 32.0f;
		break;
	}

	return 250.0f;
}

#define	GALLIMIMUS_VIEW_FIELD_NARROW	0.85f

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEntity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Gallimimus::FInViewCone( CBaseEntity *pEntity )
{
	m_flFieldOfView = ( GetEnemy() != NULL ) ? GALLIMIMUS_VIEW_FIELD_NARROW : VIEW_FIELD_WIDE;

	return BaseClass::FInViewCone( pEntity );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &vecSpot - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Gallimimus::FInViewCone( const Vector &vecSpot )
{
	m_flFieldOfView = ( GetEnemy() != NULL ) ? GALLIMIMUS_VIEW_FIELD_NARROW : VIEW_FIELD_WIDE;

	return BaseClass::FInViewCone( vecSpot );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::HandleAnimEvent( animevent_t *pEvent )
{
		
	if ( pEvent->event == AE_GALLIMIMUS_WALK_FOOTSTEP )
	{
		MakeAIFootstepSound( 240.0f );
		EmitSound( "NPC_Gallimimus.Footstep", m_hFootstep, pEvent->eventtime );
		return;
	}
/*
	if ( pEvent->event == AE_GALLIMIMUS_MELEE_HIT1 )
	{
		MeleeAttack( GALLIMIMUS_MELEE1_RANGE, sk_gallimimus_swipe_damage.GetFloat(), QAngle( 20.0f, 0.0f, -12.0f ), Vector( -250.0f, 1.0f, 1.0f ) );
		return;
	}

	if ( pEvent->event == AE_GALLIMIMUS_MELEE_HIT2 )
	{
		MeleeAttack( GALLIMIMUS_MELEE1_RANGE, sk_gallimimus_swipe_damage.GetFloat(), QAngle( 20.0f, 0.0f, 0.0f ), Vector( -350.0f, 1.0f, 1.0f ) );
		return;
	}

	if ( pEvent->event == AE_GALLIMIMUS_MELEE_POUNCE )
	{
		MeleeAttack( GALLIMIMUS_MELEE2_RANGE, sk_gallimimus_swipe_damage.GetFloat(), QAngle( 4.0f, 0.0f, 0.0f ), Vector( -250.0f, 1.0f, 1.0f ) );
		return;
	}
*/

/*
	if ( pEvent->event == AE_GALLIMIMUS_VANISH )
	{
		AddSolidFlags( FSOLID_NOT_SOLID );
		m_takedamage	= DAMAGE_NO;
		AddEffects( EF_NODRAW );
		//SetWings( false );

		return;
	}
*/
	
	if ( pEvent->event == AE_GALLIMIMUS_FOOTSTEP_SOFT )
	{
		EmitSound( "NPC_Gallimimus.FootstepSoft", pEvent->eventtime );
		return;
	}

	if ( pEvent->event == AE_GALLIMIMUS_FOOTSTEP_HEAVY )
	{
		EmitSound( "NPC_Gallimimus.FootstepHeavy", pEvent->eventtime );
		return;
	}
	
/*	
	if ( pEvent->event == AE_GALLIMIMUS_MELEE1_SOUND )
	{
		EmitSound( "NPC_Gallimimus.MeleeAttackSingle" );
		return;
	}
	
	if ( pEvent->event == AE_GALLIMIMUS_MELEE2_SOUND )
	{
		EmitSound( "NPC_Gallimimus.MeleeAttackDouble" );
		return;
	}
*/

/*
	if ( pEvent->event == AE_GALLIMIMUS_START_JUMP )
	{
		StartJump();
		return;
	}
*/	
	BaseClass::HandleAnimEvent( pEvent );
}

bool NPC_Gallimimus_IsGallimimus( CBaseEntity *pEntity )
{
	CNPC_Gallimimus *pGallimimus = dynamic_cast<CNPC_Gallimimus *>(pEntity);

	return pGallimimus ? true : false;
}

class CTraceFilterGallimimus : public CTraceFilterEntitiesOnly
{
public:
	CTraceFilterGallimimus( const CBaseEntity *pEntity ) { m_pIgnore = pEntity; }

	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );

		if ( m_pIgnore == pEntity )
			 return false;
		
		if ( pEntity->IsNPC() == false )
			 return false;
		
		if ( NPC_Gallimimus_IsGallimimus( pEntity ) )
			 return true;
		
		return false;
	}
private:
	
	const CBaseEntity		*m_pIgnore;
};

//-----------------------------------------------------------------------------
// Purpose:  
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask ) 
	{
	case TASK_GALLIMIMUS_FIND_COVER_FROM_SAVEPOSITION:
		{
			Vector coverPos;

			if ( GetTacticalServices()->FindCoverPos( m_vSavePosition, EyePosition(), 0, CoverRadius(), &coverPos ) ) 
			{
				AI_NavGoal_t goal(coverPos, ACT_RUN, AIN_HULL_TOLERANCE);
				GetNavigator()->SetGoal( goal );

				m_flMoveWaitFinished = gpGlobals->curtime + pTask->flTaskData;
			}
			else
			{
				// no coverwhatsoever.
				TaskFail(FAIL_NO_COVER);
			}
		}
		break;

/*	case TASK_ANNOUNCE_ATTACK:
		{
			EmitSound( "NPC_Gallimimus.MeleeAttackSingle" );
			TaskComplete();
			break;
		}
*/
	//case TASK_GALLIMIMUS_FACE_JUMP:
	//	break;

	case TASK_GALLIMIMUS_DROWN:
	{
		// Set the gravity really low here! Sink slowly
		SetGravity( 0 );
		SetAbsVelocity( vec3_origin );
		m_flTimeDrownSplash = gpGlobals->curtime + random->RandomFloat( 0, 0.5 );
		m_flTimeDrown = gpGlobals->curtime + 4;
		break;
	}
/*
	case TASK_GALLIMIMUS_REACH_FIGHT_GOAL:

		m_OnReachFightGoal.FireOutput( this, this );
		TaskComplete();
		break;
*/

	case TASK_GALLIMIMUS_WAIT_FOR_TRIGGER:
		m_flIdleDelay = gpGlobals->curtime + 1.0f;

		break;
/*
	case TASK_GALLIMIMUS_JUMP:
		
		if ( CheckLanding() )
		{
			TaskComplete();
		}

		break;
*/	
	case TASK_GALLIMIMUS_GET_DANGER_ESCAPE_PATH:
		{
			if ( GetPathToSoundFleePoint( SOUND_DANGER ) )			
			{
				TaskComplete();
			}
			else
			{
				TaskFail( FAIL_NO_REACHABLE_NODE );
			}
		}
		
		break;

	case TASK_GALLIMIMUS_GET_PHYSICS_DANGER_ESCAPE_PATH:
		{
			if ( GetPathToSoundFleePoint( SOUND_PHYSICS_DANGER ) )
			{
				TaskComplete();
			}
			else
			{
				TaskFail( FAIL_NO_REACHABLE_NODE );
			}
		}
		
		break;


	default:
		BaseClass::StartTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::RunTask( const Task_t *pTask )
{
	// some state that needs be set each frame
/*
#if HL2_EPISODIC
	if ( GetFlags() & FL_ONGROUND )
	{
		m_bHasDoneAirAttack = false;
	}
#endif
*/

	switch ( pTask->iTask )
/*	{
	case TASK_GALLIMIMUS_FACE_JUMP:
		{
			Vector	jumpDir = m_vecSavedJump;
			VectorNormalize( jumpDir );
			
			QAngle	jumpAngles;
			VectorAngles( jumpDir, jumpAngles );

			GetMotor()->SetIdealYawAndUpdate( jumpAngles[YAW], AI_KEEP_YAW_SPEED );
			SetTurnActivity();
			
			if ( GetMotor()->DeltaIdealYaw() < 2 )
			{
				TaskComplete();
			}
		}

		break;

*/

	{
	case TASK_GALLIMIMUS_DROWN:
	{
		if ( gpGlobals->curtime > m_flTimeDrownSplash )
		{
			float flWaterZ = UTIL_FindWaterSurface( GetAbsOrigin(), GetAbsOrigin().z, GetAbsOrigin().z + NAI_Hull::Maxs( GetHullType() ).z );

			CEffectData	data;
			data.m_fFlags = 0;
			data.m_vOrigin = GetAbsOrigin();
			data.m_vOrigin.z = flWaterZ;
			data.m_vNormal = Vector( 0, 0, 1 );
			data.m_flScale = random->RandomFloat( 12.0, 16.0 );

			DispatchEffect( "watersplash", data );
			
			m_flTimeDrownSplash = gpGlobals->curtime + random->RandomFloat( 0.5, 2.5 );
		}
	
		if ( gpGlobals->curtime > m_flTimeDrown )
		{
			OnTakeDamage( CTakeDamageInfo( this, this, m_iHealth+1, DMG_DROWN ) );
			TaskComplete();
		}
		break;
	}

	case TASK_GALLIMIMUS_WAIT_FOR_TRIGGER:
	{
		if ( ( m_flIdleDelay > gpGlobals->curtime ) || GetEntityName() != NULL_STRING )
			return;
	
		TaskComplete();

		break;
	}
	
/*
	case TASK_GALLIMIMUS_JUMP:

		if ( CheckLanding() )
		{
			TaskComplete();
		}

		break;
*/
	default:
		BaseClass::RunTask( pTask );
		break;
	}
}


/*
//-----------------------------------------------------------------------------
// Purpose: Returns true if a reasonable jumping distance
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CNPC_Gallimimus::IsJumpLegal( const Vector &startPos, const Vector &apex, const Vector &endPos ) const
{
	const float MAX_JUMP_RISE		= 512;
	const float MAX_JUMP_DROP		= 512;
	const float MAX_JUMP_DISTANCE	= 1024;
	const float MIN_JUMP_DISTANCE   = 128;

	if ( CGallimimusRepellant::IsPositionRepellantFree( endPos ) == false )
		 return false;
	
	//Adrian: Don't try to jump if my destination is right next to me.
	if ( ( endPos - GetAbsOrigin()).Length() < MIN_JUMP_DISTANCE ) 
		 return false;

	if ( HasSpawnFlags( SF_GALLIMIMUS_USE_GROUNDCHECKS ) && g_test_new_gallimimus_jump.GetBool() == true )
	{
		trace_t	tr;
		AI_TraceHull( endPos, endPos, GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );
		
		if ( tr.m_pEnt )
		{
			CAI_BaseNPC *pBlocker = tr.m_pEnt->MyNPCPointer();

			if ( pBlocker && pBlocker->Classify() == CLASS_GALLIMIMUS )
			{
				// HACKHACK
				CNPC_Gallimimus *pGallimimus = dynamic_cast< CNPC_Gallimimus * > ( pBlocker );

				if ( pGallimimus )
				{
					if ( pGallimimus->AllowedToBePushed() == true )
					{
					//	NDebugOverlay::Line( GetAbsOrigin(), endPos, 255, 0, 0, 0, 2 );
					//	NDebugOverlay::Box( pGallimimus->GetAbsOrigin(), GetHullMins(), GetHullMaxs(), 0, 0, 255, 0, 2 );
						pGallimimus->GetMotor()->SetIdealYawToTarget( endPos );
						pGallimimus->SetSchedule( SCHED_MOVE_AWAY );
						pGallimimus->m_flNextJumpPushTime = gpGlobals->curtime + 2.0f;
					}
				}
			}
		}
	}

	return BaseClass::IsJumpLegal( startPos, apex, endPos, MAX_JUMP_RISE, MAX_JUMP_DROP, MAX_JUMP_DISTANCE );
}
*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::IdleSound( void )
{
	EmitSound( "NPC_Gallimimus.Idle" );
	m_flIdleDelay = gpGlobals->curtime + 4.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::PainSound( const CTakeDamageInfo &info )
{
	EmitSound( "NPC_Gallimimus.Pain" );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CNPC_Gallimimus::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	CTakeDamageInfo newInfo = info;
/*
	if( hl2_episodic.GetBool() && gallimimus_easycrush.GetBool() )
	{
		if( newInfo.GetDamageType() & DMG_CRUSH )
		{
			if( newInfo.GetInflictor() && newInfo.GetInflictor()->VPhysicsGetObject() )
			{
				float flMass = newInfo.GetInflictor()->VPhysicsGetObject()->GetMass();

				if( flMass > 250.0f && newInfo.GetDamage() < GetHealth() )
				{
					newInfo.SetDamage( GetHealth() );
				}
			}
		}
	}
*/
	// Find out how much damage we're about to take
	int nDamageTaken = BaseClass::OnTakeDamage_Alive( newInfo );
	if ( gpGlobals->curtime - m_flLastDamageTime < 0.5f )
	{
		// Accumulate it
		m_nSustainedDamage += nDamageTaken;
	}
	else
	{
		// Reset, it's been too long
		m_nSustainedDamage = nDamageTaken;
	}

	m_flLastDamageTime = gpGlobals->curtime;

	return nDamageTaken;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator )
{
	CTakeDamageInfo newInfo = info;

	Vector	vecShoveDir = vecDir;
	vecShoveDir.z = 0.0f;

	//Are we already flipped?
	if ( IsFlipped() )
	{
		//If we were hit by physics damage, move with it
		if ( newInfo.GetDamageType() & (DMG_CRUSH|DMG_PHYSGUN) )
		{
			PainSound( newInfo );
			Vector vecForce = ( vecShoveDir * random->RandomInt( 500.0f, 1000.0f ) ) + Vector(0,0,64.0f);
			//CascadePush( vecForce );
			ApplyAbsVelocityImpulse( vecForce );
			SetGroundEntity( NULL );
		}

		//More vulnerable when flipped
		newInfo.ScaleDamage( 4.0f );
	}
	else if ( newInfo.GetDamageType() & (DMG_PHYSGUN) || 
			( newInfo.GetDamageType() & (DMG_BLAST|DMG_CRUSH) && newInfo.GetDamage() >= 25.0f ) )
	{
		// Don't do this if we're in an interaction
		if ( !IsRunningDynamicInteraction() )
 		{
			//Grenades, physcannons, and physics impacts make us fuh-lip!
			
			if( hl2_episodic.GetBool() )
			{
				PainSound( newInfo );

				if( GetFlags() & FL_ONGROUND )
				{
					// Only flip if on the ground.
					SetCondition( COND_GALLIMIMUS_FLIPPED );
				}

				Vector vecForce = ( vecShoveDir * random->RandomInt( 500.0f, 1000.0f ) ) + Vector(0,0,64.0f);

				//CascadePush( vecForce );
				ApplyAbsVelocityImpulse( vecForce );
				SetGroundEntity( NULL );
			}
			else
			{
				//Don't flip off the deck
				if ( GetFlags() & FL_ONGROUND )
				{
					PainSound( newInfo );

					SetCondition( COND_GALLIMIMUS_FLIPPED );

					//Get tossed!
					ApplyAbsVelocityImpulse( ( vecShoveDir * random->RandomInt( 500.0f, 1000.0f ) ) + Vector(0,0,64.0f) );
					SetGroundEntity( NULL );
				}
			}
		}
	}

	BaseClass::TraceAttack( info, vecDir, ptr, pAccumulator );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------
float CNPC_Gallimimus::GetIdealAccel( void ) const
{
	return GetIdealSpeed() * 2.0;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::GatherConditions( void )
{
	BaseClass::GatherConditions();

	// See if I've landed on an NPC!
//	CBaseEntity *pGroundEnt = GetGroundEntity();
	
	if ( IsCurSchedule(SCHED_FALL_TO_GROUND ) == false &&
		 IsEffectActive( EF_NODRAW ) == false )
	{
		if( m_lifeState == LIFE_ALIVE && GetWaterLevel() > 1 )
		{
			// Start Drowning!
			SetCondition( COND_GALLIMIMUS_IN_WATER );
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::PrescheduleThink( void )
{
	UpdateHead();

	Activity eActivity = GetActivity();

	//See if we need to play their agitated sound
	if ( ( eActivity == ACT_GALLIMIMUS_RUN_AGITATED ) && ( m_bAgitatedSound == false ) )
	{
		//Start sound
		CPASAttenuationFilter filter( this, "NPC_Gallimimus.LoopingAgitated" );
		filter.MakeReliable();

		EmitSound( filter, entindex(), "NPC_Gallimimus.LoopingAgitated" );
		m_bAgitatedSound = true;
	}
	else if ( ( eActivity != ACT_GALLIMIMUS_RUN_AGITATED ) && ( m_bAgitatedSound == true ) )
	{
		//Stop sound
		StopSound( "NPC_Gallimimus.LoopingAgitated" );
		m_bAgitatedSound = false;
	}

	//New Enemy? Try to jump at him.
//	if ( HasCondition( COND_NEW_ENEMY ) )
//	{
//		m_flJumpTime = 0.0f;
//	}

	/*
	// See if we should jump because of desirables conditions, or a scripted request
	if ( ShouldJump() )
	{
		SetCondition( COND_GALLIMIMUS_CAN_JUMP );
	}
	else
	{
		ClearCondition( COND_GALLIMIMUS_CAN_JUMP );
	}
	*/
	BaseClass::PrescheduleThink();
}




//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Gallimimus::ShouldPlayIdleSound( void )
{
	//Only do idles in the right states
	if ( ( m_NPCState != NPC_STATE_IDLE && m_NPCState != NPC_STATE_ALERT ) )
		return false;

	//Gagged monsters don't talk
	if ( m_spawnflags & SF_NPC_GAG )
		return false;

	//Don't cut off another sound or play again too soon
	if ( m_flIdleDelay > gpGlobals->curtime )
		return false;

	//Randomize it a bit
	if ( random->RandomInt( 0, 20 ) )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: turn in the direction of movement
// Output :
//-----------------------------------------------------------------------------
bool CNPC_Gallimimus::OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval )
{
	
	//Adrian: Make Gallimimus face the danger (ai_sound) while they flee away.
	if ( IsCurSchedule( SCHED_GALLIMIMUS_FLEE_DANGER ) )
	{
		CSound *pSound = GetLoudestSoundOfType( SOUND_DANGER );

		if ( pSound )
		{
			AddFacingTarget( pSound->GetSoundOrigin(), 1.0, 0.5f );
		}
	}
	else if ( GetEnemy() && GetNavigator()->GetMovementActivity() == ACT_RUN )
  	{
		// FIXME: this will break scripted sequences that walk when they have an enemy
		Vector vecEnemyLKP = GetEnemyLKP();
		if ( UTIL_DistApprox( vecEnemyLKP, GetAbsOrigin() ) < 512 )
		{
			// Only start facing when we're close enough
			AddFacingTarget( GetEnemy(), vecEnemyLKP, 1.0, 0.2 );
		}
	}

	return BaseClass::OverrideMoveFacing( move, flInterval );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flDamage - 
//			bitsDamageType - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Gallimimus::IsLightDamage( const CTakeDamageInfo &info )
{
	if ( ( random->RandomInt( 0, 1 ) ) && ( info.GetDamage() > 3 ) )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int CNPC_Gallimimus::SelectSchedule( void )
{
	//Flipped?
	if ( HasCondition( COND_GALLIMIMUS_FLIPPED ) )
	{
		ClearCondition( COND_GALLIMIMUS_FLIPPED );
		
		// See if it's a forced, electrical flip
		if ( m_flZapDuration > gpGlobals->curtime )
		{
			SetContextThink( &CNPC_Gallimimus::ZapThink, gpGlobals->curtime, "ZapThink" );
			return SCHED_GALLIMIMUS_ZAP_FLIP;
		}

		// Regular flip
		return SCHED_GALLIMIMUS_FLIP;
	}

	if( HasCondition( COND_GALLIMIMUS_IN_WATER ) )
	{
		// No matter what, drown in water
		return SCHED_GALLIMIMUS_DROWN;
	}

	
	//Hear a Danger?
	if ( HasCondition( COND_HEAR_DANGER ) )
	{
		// Ignore thumpers that aren't visible
		CSound *pSound = GetLoudestSoundOfType( SOUND_DANGER );
		
		if ( pSound )
		{
			CTakeDamageInfo info;
			PainSound( info );
			ClearCondition( COND_HEAR_DANGER );

			return SCHED_GALLIMIMUS_FLEE_DANGER;
		}
	}

	//Hear a physics danger sound?
	if( HasCondition( COND_HEAR_PHYSICS_DANGER ) )
	{
		CTakeDamageInfo info;
		PainSound( info );
		return SCHED_GALLIMIMUS_FLEE_PHYSICS_DANGER;
	}

/*
	// If we're scripted to jump at a target, do so
	if ( HasCondition( COND_GALLIMIMUS_CAN_JUMP_AT_TARGET ) )
	{
		// NDebugOverlay::Cross3D( m_vecSavedJump, 32.0f, 255, 0, 0, true, 2.0f );
		ClearCondition( COND_GALLIMIMUS_CAN_JUMP_AT_TARGET );
		return SCHED_GALLIMIMUS_JUMP;
	}
*/

	//Otherwise do basic state schedule selection
/*	switch ( m_NPCState )
	{	


	case NPC_STATE_COMBAT:
		{
			// Worker-only AI
			if ( hl2_episodic.GetBool() && IsWorker() )
			{
				// Melee attack if we can
				if ( HasCondition( COND_CAN_MELEE_ATTACK1 ) )
					return SCHED_MELEE_ATTACK1;

				// Pounce if they're too near us
				if ( HasCondition( COND_CAN_MELEE_ATTACK2 ) )
				{
					m_flPounceTime = gpGlobals->curtime + 1.5f;

					if ( m_bLeapAttack == true )
						return SCHED_GALLIMIMUS_POUNCE_MOVING;

					return SCHED_GALLIMIMUS_POUNCE;
				}

				// A squadmate died, so run away!
				if ( HasCondition( COND_GALLIMIMUS_SQUADMATE_KILLED ) )
				{
					SetNextAttack( gpGlobals->curtime + random->RandomFloat( 2.0f, 4.0f ) );
					ClearCondition( COND_GALLIMIMUS_SQUADMATE_KILLED );
					return SCHED_GALLIMIMUS_TAKE_COVER_FROM_ENEMY;
				}

				// Flee on heavy damage
				if ( HasCondition( COND_HEAVY_DAMAGE ) )
				{
					SetNextAttack( gpGlobals->curtime + random->RandomFloat( 2.0f, 4.0f ) );
					return SCHED_GALLIMIMUS_TAKE_COVER_FROM_ENEMY;
				}

				// Range attack if we're able
				if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) )
				{
					if ( OccupyStrategySlot( SQUAD_SLOT_GALLIMIMUS_WORKER_FIRE ) )
					{
						EmitSound( "NPC_Gallimimus.PoisonBurstScream" );
						SetNextAttack( gpGlobals->curtime + random->RandomFloat( 0.5f, 2.5f ) );
						if ( GetEnemy() )
						{
							m_vSavePosition = GetEnemy()->BodyTarget( GetAbsOrigin() );
						}

						return SCHED_GALLIMIMUS_WORKER_RANGE_ATTACK1;
					}
				}
				
				// Back up, we're too near an enemy or can't see them
				if ( HasCondition( COND_TOO_CLOSE_TO_ATTACK ) || HasCondition( COND_ENEMY_OCCLUDED ) )
					return SCHED_ESTABLISH_LINE_OF_FIRE;

				// See if we need to destroy breakable cover
				if ( HasCondition( COND_WEAPON_SIGHT_OCCLUDED ) )
					return SCHED_SHOOT_ENEMY_COVER;

				// Run around randomly if our target is looking in our direction
				if ( HasCondition( COND_BEHIND_ENEMY ) == false )
					return SCHED_GALLIMIMUS_WORKER_FLANK_RANDOM;

				// Face our target and continue to fire
				return SCHED_COMBAT_FACE;
			}
			else
			{
				// Lunge at the enemy
				if ( HasCondition( COND_CAN_MELEE_ATTACK2 ) )
				{
					m_flPounceTime = gpGlobals->curtime + 1.5f;

					if ( m_bLeapAttack == true )
						return SCHED_GALLIMIMUS_POUNCE_MOVING;
					else
						return SCHED_GALLIMIMUS_POUNCE;
				}

				// Try to jump
				if ( HasCondition( COND_GALLIMIMUS_CAN_JUMP ) )
					return SCHED_GALLIMIMUS_JUMP;
			}
		}

	default:
		{
			int	moveSched = ();

			if ( moveSched != SCHED_NONE )
				return moveSched;

			if ( GetEnemy() == NULL && ( HasCondition( COND_LIGHT_DAMAGE ) || HasCondition( COND_HEAVY_DAMAGE ) ) )
			{
				Vector vecEnemyLKP;

				// Retrieve a memory for the damage taken
				// Fill in where we're trying to look
				if ( GetEnemies()->Find( AI_UNKNOWN_ENEMY ) )
				{
					vecEnemyLKP = GetEnemies()->LastKnownPosition( AI_UNKNOWN_ENEMY );
				}
				else
				{
					// Don't have an enemy, so face the direction the last attack came from (don't face north)
					vecEnemyLKP = WorldSpaceCenter() + ( g_vecAttackDir * 128 );
				}
				
				// If we're already facing the attack direction, then take cover from it
				if ( FInViewCone( vecEnemyLKP ) )
				{
					// Save this position for our cover search
					m_vSavePosition = vecEnemyLKP;
					return SCHED_GALLIMIMUS_TAKE_COVER_FROM_SAVEPOSITION;
				}
				
				// By default, we'll turn to face the attack
			}
		}
		break;
	}
*/
	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::Touch( CBaseEntity *pOther )
{
	//See if the touching entity is a vehicle
	CBasePlayer *pPlayer = ToBasePlayer( AI_GetSinglePlayer() );
	
	// FIXME: Technically we'll want to check to see if a vehicle has touched us with the player OR NPC driver

	if ( pPlayer && pPlayer->IsInAVehicle() )
	{
		IServerVehicle	*pVehicle = pPlayer->GetVehicle();
		CBaseEntity *pVehicleEnt = pVehicle->GetVehicleEnt();

		if ( pVehicleEnt == pOther )
		{
			CPropVehicleDriveable	*pDrivableVehicle = dynamic_cast<CPropVehicleDriveable *>( pVehicleEnt );

			if ( pDrivableVehicle != NULL )
			{
				//Get tossed!
				Vector	vecShoveDir = pOther->GetAbsVelocity();
				Vector	vecTargetDir = GetAbsOrigin() - pOther->GetAbsOrigin();
				
				VectorNormalize( vecShoveDir );
				VectorNormalize( vecTargetDir );

				//bool bBurrowingOut = IsCurSchedule( SCHED_GALLIMIMUS_BURROW_OUT );

				if ( ( ( pDrivableVehicle->m_nRPM > 75 ) && DotProduct( vecShoveDir, vecTargetDir ) <= 0 ) ) //|| bBurrowingOut == true )
				{
					if ( IsFlipped() == true ) //|| bBurrowingOut == true )
					{
						float flDamage = m_iHealth;

						if ( random->RandomInt( 0, 10 ) > 4 )
							 flDamage += 25;
									
						CTakeDamageInfo	dmgInfo( pVehicleEnt, pPlayer, flDamage, DMG_VEHICLE );
					
						CalculateMeleeDamageForce( &dmgInfo, vecShoveDir, pOther->GetAbsOrigin() );
						TakeDamage( dmgInfo );
					}
					else
					{
						// We're being shoved
						CTakeDamageInfo	dmgInfo( pVehicleEnt, pPlayer, 0, DMG_VEHICLE );
						PainSound( dmgInfo );

						SetCondition( COND_GALLIMIMUS_FLIPPED );

						vecTargetDir[2] = 0.0f;

						ApplyAbsVelocityImpulse( ( vecTargetDir * 250.0f ) + Vector(0,0,64.0f) );
						SetGroundEntity( NULL );

						CSoundEnt::InsertSound( SOUND_PHYSICS_DANGER, GetAbsOrigin(), 256, 0.5f, this );
					}
				}
			}
		}
	}

	BaseClass::Touch( pOther );
}
	// in episodic, an Gallimimus colliding with the player in midair does him damage.
	// pursuant bugs 58590, 56960, this happens only once per glide.
/*
#ifdef HL2_EPISODIC 
	if ( GetActivity() == ACT_GLIDE && IsValidEnemy( pOther ) && !m_bHasDoneAirAttack )
	{
		CTakeDamageInfo	dmgInfo( this, this, sk_gallimimus_air_attack_dmg.GetInt(), DMG_SLASH );

		//CalculateMeleeDamageForce( &dmgInfo, Vector( 0, 0, 1 ), GetAbsOrigin() );
		//pOther->TakeDamage( dmgInfo );

		//Kick the player angles
		//bool bIsPlayer = pOther->IsPlayer();
		//if ( bIsPlayer && !(pOther->GetFlags() & FL_GODMODE ) && pOther->GetMoveType() != MOVETYPE_NOCLIP )
		//{
		//	pOther->ViewPunch( QAngle( 4.0f, 0.0f, 0.0f ) );
		//}

		// set my "I have already attacked someone" flag
		if ( bIsPlayer || pOther->IsNPC())
		{
			m_bHasDoneAirAttack = true;
		}
	}
#endif
*/
	// Did the player touch me?
	//if ( pOther->IsPlayer() )
	//{
		// Don't test for this if the pusher isn't friendly
		//if ( IsValidEnemy( pOther ) )
		//	return;

		// Ignore if pissed at player
		//if ( m_afMemory & bits_MEMORY_PROVOKED )
		//	return;
	
		//if ( !IsCurSchedule( SCHED_MOVE_AWAY ) && !IsCurSchedule( SCHED_GALLIMIMUS_BURROW_OUT ) )
		//	 TestPlayerPushing( pOther );
	//}

	//Adrian: Explode if hit by gunship!
	//Maybe only do this if hit by the propellers?
/*	if ( pOther->IsNPC() )
	{
		if ( pOther->Classify() == CLASS_COMBINE_GUNSHIP )
		{
			float flDamage = m_iHealth + 25;
						
			CTakeDamageInfo	dmgInfo( pOther, pOther, flDamage, DMG_GENERIC );
			GuessDamageForce( &dmgInfo, (pOther->GetAbsOrigin() - GetAbsOrigin()), pOther->GetAbsOrigin() );
			TakeDamage( dmgInfo );
		}
*///}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CNPC_Gallimimus::IsHeavyDamage( const CTakeDamageInfo &info )
{
	if ( hl2_episodic.GetBool() ) // && IsWorker() )
	{
		if ( m_nSustainedDamage + info.GetDamage() > 6 )
			return true;
	}
	
	return BaseClass::IsHeavyDamage( info );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pVictim - 
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::Event_Killed( const CTakeDamageInfo &info )
{
	//Turn off wings
	//SetWings( false );
	VacateStrategySlot();

	if ( info.GetDamageType() & DMG_CRUSH )
	{
		CSoundEnt::InsertSound( SOUND_PHYSICS_DANGER, GetAbsOrigin(), 256, 0.5f, this );
	}

	BaseClass::Event_Killed( info );
/*
	CBaseEntity *pAttacker = info.GetInflictor();

	if ( pAttacker && pAttacker->GetServerVehicle() && ShouldGib( info ) == true )
	{
		trace_t tr;
		UTIL_TraceLine( GetAbsOrigin() + Vector( 0, 0, 64 ), pAttacker->GetAbsOrigin(), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );
		UTIL_DecalTrace( &tr, "Antlion.Splat" );

		SpawnBlood( GetAbsOrigin(), g_vecAttackDir, BloodColor(), info.GetDamage() );

		CPASAttenuationFilter filter( this );
		EmitSound( filter, entindex(), "NPC_Gallimimus.RunOverByVehicle" );
	}

	// Stop our zap effect!
	SetContextThink( NULL, gpGlobals->curtime, "ZapThink" );
*/

}

/*
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEnemy - 
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::GatherEnemyConditions( CBaseEntity *pEnemy )
{
	// Do the base class
	BaseClass::GatherEnemyConditions( pEnemy );

	// If we're not already too far away, check again
	//TODO: Check to make sure we don't already have a condition set that removes the need for this
	if ( HasCondition( COND_ENEMY_UNREACHABLE ) == false )
	{
		Vector	predPosition;
		UTIL_PredictedPosition( GetEnemy(), 1.0f, &predPosition );

		Vector	predDir = ( predPosition - GetAbsOrigin() );
		float	predLength = VectorNormalize( predDir );

		// See if we'll be outside our effective target range
		if ( predLength > m_flEludeDistance )
		{
			Vector	predVelDir = ( predPosition - GetEnemy()->GetAbsOrigin() );
			float	predSpeed  = VectorNormalize( predVelDir );

			// See if the enemy is moving mostly away from us
			if ( ( predSpeed > 512.0f ) && ( DotProduct( predVelDir, predDir ) > 0.0f ) )
			{
				// Mark the enemy as eluded and burrow away
				ClearEnemyMemory();
				SetEnemy( NULL );
				SetIdealState( NPC_STATE_ALERT );
				SetCondition( COND_ENEMY_UNREACHABLE );
			}
		}
	}
}

*/



//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Gallimimus::ShouldGib( const CTakeDamageInfo &info )
{
	if ( info.GetDamageType() & (DMG_NEVERGIB|DMG_DISSOLVE) )
		return false;

	if ( info.GetDamageType() & (DMG_ALWAYSGIB|DMG_BLAST) )
		return true;

	if ( m_iHealth < -20 )
		return true;
	
	return false;
}



//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Gallimimus::CorpseGib( const CTakeDamageInfo &info )
{
	{
		// Use the bone position to handle being moved by an animation (like a dynamic scripted sequence)
		static int s_nBodyBone = -1;
		if ( s_nBodyBone == -1 )
		{
			s_nBodyBone = LookupBone( "spine" );
		}

		Vector vecOrigin;
		QAngle angBone;
		GetBonePosition( s_nBodyBone, vecOrigin, angBone );

		DispatchParticleEffect( "AntlionGib", vecOrigin, QAngle( 0, 0, 0 ) );
	}

	Vector velocity = vec3_origin;
	AngularImpulse	angVelocity = RandomAngularImpulse( -150, 150 );
	breakablepropparams_t params( EyePosition(), GetAbsAngles(), velocity, angVelocity );
	params.impactEnergyScale = 1.0f;
	params.defBurstScale = 150.0f;
	params.defCollisionGroup = COLLISION_GROUP_DEBRIS;
	PropBreakableCreateAll( GetModelIndex(), NULL, params, this, -1, true, true );

	return true;
}



/*
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::InputJumpAtTarget( inputdata_t &inputdata )
{
	CBaseEntity *pJumpTarget = gEntList.FindEntityByName( NULL, inputdata.value.String(), this, inputdata.pActivator, inputdata.pCaller );
	if ( pJumpTarget == NULL )
	{
		Msg("Unable to find jump target named (%s)\n", inputdata.value.String() );
		return;
	}

#if HL2_EPISODIC

	// Try the jump
	AIMoveTrace_t moveTrace;
	Vector targetPos = pJumpTarget->GetAbsOrigin();

	// initialize jump state
	float minJumpHeight = 0.0;
	float maxHorzVel = 800.0f;

	// initial jump, sets baseline for minJumpHeight
	Vector vecApex;
	Vector rawJumpVel = GetMoveProbe()->CalcJumpLaunchVelocity(GetAbsOrigin(), targetPos, sv_gravity.GetFloat() * GetJumpGravity(), &minJumpHeight, maxHorzVel, &vecApex );

	if ( g_debug_gallimimus.GetInt() == 2 )
	{
		NDebugOverlay::Box( targetPos, GetHullMins(), GetHullMaxs(), 0, 255, 0, 0, 5 );
		NDebugOverlay::Line( GetAbsOrigin(), targetPos, 0, 255, 0, 0, 5 );
		NDebugOverlay::Line( GetAbsOrigin(), rawJumpVel, 255, 255, 0, 0, 5 );
	}

	m_vecSavedJump = rawJumpVel;

#else	

	// Get the direction and speed to our target
	Vector vecJumpDir = ( pJumpTarget->GetAbsOrigin() - GetAbsOrigin() );
	VectorNormalize( vecJumpDir );
	vecJumpDir *= 800.0f;	// FIXME: We'd like to pass this in as a parameter, but comma delimited lists are bad
	m_vecSavedJump = vecJumpDir;

#endif

	SetCondition( COND_GALLIMIMUS_CAN_JUMP_AT_TARGET );
}
*/


void CNPC_Gallimimus::StopLoopingSounds( void )
{
	//if ( m_bLoopingStarted )
	//{
	//	StopSound( "NPC_Gallimimus.WingsOpen" );
	//	m_bLoopingStarted = false;
	//}
	if ( m_bAgitatedSound )
	{
		StopSound( "NPC_Gallimimus.LoopingAgitated" );
		m_bAgitatedSound = false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Special version helps other NPCs hit overturned Gallimimus
//-----------------------------------------------------------------------------
Vector CNPC_Gallimimus::BodyTarget( const Vector &posSrc, bool bNoisy /*= true*/ )
{ 
	// Cache the bone away to avoid future lookups
	if ( m_nBodyBone == -1 )
	{
		CBaseAnimating *pAnimating = GetBaseAnimating();
		m_nBodyBone = pAnimating->LookupBone( "spine" );
	}

	// Get the exact position in our center of mass (thorax)
	Vector vecResult;
	QAngle vecAngle;
	GetBonePosition( m_nBodyBone, vecResult, vecAngle );
	
	if ( bNoisy )
		return vecResult + RandomVector( -8, 8 );

	return vecResult;
}

//-----------------------------------------------------------------------------
// Purpose: Flip the Gallimimus over
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::Flip( bool bZapped /*= false*/ )
{
	// We can't flip an already flipped Gallimimus
	if ( IsFlipped() )
		return;

	// Must be on the ground
	if ( ( GetFlags() & FL_ONGROUND ) == false ) 
		return;

	// Can't be in a dynamic interation
	if ( IsRunningDynamicInteraction() )
		return;

	SetCondition( COND_GALLIMIMUS_FLIPPED ); 

	if ( bZapped )
	{
		m_flZapDuration = gpGlobals->curtime + SequenceDuration( SelectWeightedSequence( (Activity) ACT_GALLIMIMUS_ZAP_FLIP) ) + 0.1f;

		EmitSound( "NPC_Gallimimus.ZappedFlip"  );
	}
}

/*
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CNPC_Gallimimus::CanBecomeRagdoll()
{
	if ( IsCurSchedule( SCHED_DIE ))
	return true;
	
	return BaseClass::CanBecomeRagdoll();
}
*/

/*
//-----------------------------------------------------------------------------
// Determines the best type of death anim to play based on how we died.
//-----------------------------------------------------------------------------
Activity CNPC_Gallimimus::GetDeathActivity()
{
	return ACT_DIESIMPLE;
}

*/

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::ClampRagdollForce( const Vector &vecForceIn, Vector *vecForceOut )
{
	// Assumes the compy mass is 18kg (100 feet per second)
	float MAX_GALLI_RAGDOLL_SPEED = 200.0f * 24.0f * 50.0f; // = 100.0f * 12.0f * 30.0f; ( * 18.0F; works good)

	Vector vecClampedForce; 
	BaseClass::ClampRagdollForce( vecForceIn, &vecClampedForce );

	// Copy the force to vecForceOut, in case we don't change it.
	*vecForceOut = vecClampedForce;

	float speed = VectorNormalize( vecClampedForce );
	if( speed > MAX_GALLI_RAGDOLL_SPEED )
	{
		// Don't let the ragdoll go as fast as it was going to.
		vecClampedForce *= MAX_GALLI_RAGDOLL_SPEED;
		*vecForceOut = vecClampedForce;
	}

}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
inline CBaseEntity *CNPC_Gallimimus::EntityToWatch( void )
{
	return ( m_hFollowTarget != NULL ) ? m_hFollowTarget.Get() : GetEnemy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::UpdateHead( void )
{
	float yaw = GetPoseParameter( m_poseHead_Yaw );
	float pitch = GetPoseParameter( m_poseHead_Pitch );

	CBaseEntity *pTarget = EntityToWatch();

	if ( pTarget != NULL )
	{
		Vector	enemyDir = pTarget->WorldSpaceCenter() - WorldSpaceCenter();
		VectorNormalize( enemyDir );
		
		if ( DotProduct( enemyDir, BodyDirection3D() ) < 0.0f )
		{
			SetPoseParameter( m_poseHead_Yaw,	UTIL_Approach( 0, yaw, 10 ) );
			SetPoseParameter( m_poseHead_Pitch, UTIL_Approach( 0, pitch, 10 ) );
			
			return;
		}

		float facingYaw = VecToYaw( BodyDirection3D() );
		float yawDiff = VecToYaw( enemyDir );
		yawDiff = UTIL_AngleDiff( yawDiff, facingYaw + yaw );

		float facingPitch = UTIL_VecToPitch( BodyDirection3D() );
		float pitchDiff = UTIL_VecToPitch( enemyDir );
		pitchDiff = UTIL_AngleDiff( pitchDiff, facingPitch + pitch );

		SetPoseParameter( m_poseHead_Yaw, UTIL_Approach( yaw + yawDiff, yaw, 50 ) );
		SetPoseParameter( m_poseHead_Pitch, UTIL_Approach( pitch + pitchDiff, pitch, 50 ) );
	}
	else
	{
		SetPoseParameter( m_poseHead_Yaw,	UTIL_Approach( 0, yaw, 10 ) );
		SetPoseParameter( m_poseHead_Pitch, UTIL_Approach( 0, pitch, 10 ) );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Cache whatever pose parameters we intend to use
//-----------------------------------------------------------------------------
void	CNPC_Gallimimus::PopulatePoseParameters( void )
{
	m_poseHead_Pitch = LookupPoseParameter("head_pitch");
	m_poseHead_Yaw   = LookupPoseParameter("head_yaw" );

	BaseClass::PopulatePoseParameters();
}

/*
// Number of times the Gallimimus will attempt to generate a random chase position
#define NUM_CHASE_POSITION_ATTEMPTS		3

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &targetPos - 
//			&result - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Gallimimus::FindChasePosition( const Vector &targetPos, Vector &result )
{
	if ( HasSpawnFlags( SF_GALLIMIMUS_USE_GROUNDCHECKS ) == true )
	{
		 result = targetPos;
		 return true;
	}

	Vector runDir = ( targetPos - GetAbsOrigin() );
	VectorNormalize( runDir );
	
	Vector	vRight, vUp;
	VectorVectors( runDir, vRight, vUp );

	for ( int i = 0; i < NUM_CHASE_POSITION_ATTEMPTS; i++ )
	{
		result	= targetPos;
		result += -runDir * random->RandomInt( 64, 128 );
		result += vRight * random->RandomInt( -128, 128 );
		
		//FIXME: We need to do a more robust search here
		// Find a ground position and try to get there
		if ( GetGroundPosition( result, result ) )
			return true;
	}
	
	//TODO: If we're making multiple inquiries to this, make sure it's evenly spread

	if ( g_debug_gallimimus.GetInt() == 1 )
	{
		NDebugOverlay::Cross3D( result, -Vector(32,32,32), Vector(32,32,32), 255, 255, 0, true, 2.0f );
	}

	return false;
}
*/

/*
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &testPos - 
//-----------------------------------------------------------------------------
bool CNPC_Gallimimus::GetGroundPosition( const Vector &testPos, Vector &result )
{
	// Trace up to clear the ground
	trace_t	tr;
	AI_TraceHull( testPos, testPos + Vector( 0, 0, 64 ), NAI_Hull::Mins( GetHullType() ), NAI_Hull::Maxs( GetHullType() ), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );

	// If we're stuck in solid, this can't be valid
	if ( tr.allsolid )
	{
		if ( g_debug_gallimimus.GetInt() == 3 )
		{
			NDebugOverlay::BoxDirection( testPos, NAI_Hull::Mins( GetHullType() ), NAI_Hull::Maxs( GetHullType() ) + Vector( 0, 0, 128 ), Vector( 0, 0, 1 ), 255, 0, 0, true, 2.0f );
		}

		return false;
	}

	if ( g_debug_gallimimus.GetInt() == 3 )
	{
		NDebugOverlay::BoxDirection( testPos, NAI_Hull::Mins( GetHullType() ), NAI_Hull::Maxs( GetHullType() ) + Vector( 0, 0, 128 ), Vector( 0, 0, 1 ), 0, 255, 0, true, 2.0f );
	}

	// Trace down to find the ground
	AI_TraceHull( tr.endpos, tr.endpos - Vector( 0, 0, 128 ), NAI_Hull::Mins( GetHullType() ), NAI_Hull::Maxs( GetHullType() ), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );

	if ( g_debug_gallimimus.GetInt() == 3 )
	{
		NDebugOverlay::BoxDirection( tr.endpos, NAI_Hull::Mins( GetHullType() ) - Vector( 0, 0, 256 ), NAI_Hull::Maxs( GetHullType() ), Vector( 0, 0, 1 ), 255, 255, 0, true, 2.0f );
	}

	// We must end up on the floor with this trace
	if ( tr.fraction < 1.0f )
	{
		if ( g_debug_gallimimus.GetInt() == 3 )
		{
			NDebugOverlay::Cross3D( tr.endpos, NAI_Hull::Mins( GetHullType() ), NAI_Hull::Maxs( GetHullType() ), 255, 0, 0, true, 2.0f );
		}

		result = tr.endpos;
		return true;
	}

	// Ended up in open space
	return false;
}
*/
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : soundType - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Gallimimus::GetPathToSoundFleePoint( int soundType )
{
	CSound *pSound = GetLoudestSoundOfType( soundType );

	if ( pSound == NULL  )
	{
		//NOTENOTE: If you're here, there's a disparity between Listen() and GetLoudestSoundOfType() - jdw
		TaskFail( "Unable to find danger sound!" );
		return false;
	}

	ManageFleeCapabilities( false );

	//Try and find a hint-node first
	CHintCriteria	hintCriteria;

	hintCriteria.SetHintType( HINT_GALLIMIMUS_DANGER_FLEE_POINT );
	hintCriteria.SetFlag( bits_HINT_NODE_NEAREST );
	hintCriteria.AddIncludePosition( WorldSpaceCenter(), 2500 );

	CAI_Hint *pHint = CAI_HintManager::FindHint( WorldSpaceCenter(), hintCriteria );

	Vector vecFleeGoal;
	Vector vecSoundPos = pSound->GetSoundOrigin();

	// Put the sound location on the same plane as the Gallimimus.
	vecSoundPos.z = GetAbsOrigin().z;

	Vector vecFleeDir = GetAbsOrigin() - vecSoundPos;
	VectorNormalize( vecFleeDir );

	if ( pHint != NULL )
	{
		// Get our goal position
		pHint->GetPosition( this, &vecFleeGoal );

		// Find a route to that position
		AI_NavGoal_t goal( vecFleeGoal, (Activity) ACT_GALLIMIMUS_RUN_AGITATED, 128, AIN_DEF_FLAGS );

		if ( GetNavigator()->SetGoal( goal ) )
		{
			pHint->Lock( this );
			pHint->Unlock( 2.0f );

			GetNavigator()->SetArrivalActivity( (Activity) ACT_GALLIMIMUS_DISTRACT_ARRIVED );
			GetNavigator()->SetArrivalDirection( -vecFleeDir );

			ManageFleeCapabilities( true );
			return true;
		}
	}

	//Make us offset this a little at least
	float flFleeYaw = VecToYaw( vecFleeDir ) + random->RandomInt( -20, 20 );

	vecFleeDir = UTIL_YawToVector( flFleeYaw );

	// Move us to the outer radius of the noise (with some randomness)
	vecFleeGoal = vecSoundPos + vecFleeDir * ( pSound->Volume() + random->RandomInt( 32, 64 ) );

	// Find a route to that position
	AI_NavGoal_t goal( vecFleeGoal + Vector( 0, 0, 8 ), (Activity) ACT_GALLIMIMUS_RUN_AGITATED, 512, AIN_DEF_FLAGS );

	if ( GetNavigator()->SetGoal( goal ) )
	{
		GetNavigator()->SetArrivalActivity( (Activity) ACT_GALLIMIMUS_DISTRACT_ARRIVED );
		GetNavigator()->SetArrivalDirection( -vecFleeDir );

		ManageFleeCapabilities( true );
		return true;
	}

	ManageFleeCapabilities( true );
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
inline bool CNPC_Gallimimus::IsFlipped( void ) 
{
	return ( GetActivity() == ACT_GALLIMIMUS_FLIP || GetActivity() == ACT_GALLIMIMUS_ZAP_FLIP );
}

/*
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::InputDisableJump( inputdata_t &inputdata )
{
	m_bDisableJump = true;
	CapabilitiesRemove( bits_CAP_MOVE_JUMP );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::InputEnableJump( inputdata_t &inputdata )
{
	m_bDisableJump = false;
	CapabilitiesAdd( bits_CAP_MOVE_JUMP );
}
*/
/*
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::CreateDust( bool placeDecal )
{
	trace_t	tr;
	AI_TraceLine( GetAbsOrigin()+Vector(0,0,1), GetAbsOrigin()-Vector(0,0,64), MASK_SOLID_BRUSHONLY | CONTENTS_PLAYERCLIP | CONTENTS_MONSTERCLIP, this, COLLISION_GROUP_NONE, &tr );

//	if ( tr.fraction < 1.0f )
//	{
//		const surfacedata_t *pdata = physprops->GetSurfaceData( tr.surface.surfaceProps );

//		if ( hl2_episodic.GetBool() == true || ( pdata->game.material == CHAR_TEX_CONCRETE ) || 
//			 ( pdata->game.material == CHAR_TEX_DIRT ) ||
//			 ( pdata->game.material == CHAR_TEX_SAND ) ) 
		//{
		//
		//	if ( !m_bSuppressUnburrowEffects )
		//	{
		//		UTIL_CreateAntlionDust( tr.endpos + Vector(0,0,24), GetAbsAngles() );
		//		
		//		if ( placeDecal )
		//		{
		//			UTIL_DecalTrace( &tr, "Antlion.Unburrow" );
		//		}
		//	}
		//}
//	}
}
*/
/*
//-----------------------------------------------------------------------------
// Purpose: Monitor the Gallimimus's jump to play the proper landing sequence
//-----------------------------------------------------------------------------
bool CNPC_Gallimimus::CheckLanding( void )
{
	trace_t	tr;
	Vector	testPos;

	//Amount of time to predict forward
	const float	timeStep = 0.1f;

	//Roughly looks one second into the future
	testPos = GetAbsOrigin() + ( GetAbsVelocity() * timeStep );
	testPos[2] -= ( 0.5 * sv_gravity.GetFloat() * GetGravity() * timeStep * timeStep);

	if ( g_debug_gallimimus.GetInt() == 2 )
	{
		NDebugOverlay::Line( GetAbsOrigin(), testPos, 255, 0, 0, 0, 0.5f );
		NDebugOverlay::Cross3D( m_vecSavedJump, -Vector(2,2,2), Vector(2,2,2), 0, 255, 0, true, 0.5f );
	} 
	
	// Look below
	AI_TraceHull( GetAbsOrigin(), testPos, NAI_Hull::Mins( GetHullType() ), NAI_Hull::Maxs( GetHullType() ), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );

	//See if we're about to contact, or have already contacted the ground
	if ( ( tr.fraction != 1.0f ) || ( GetFlags() & FL_ONGROUND ) )
	{
		int	sequence = SelectWeightedSequence( (Activity)ACT_GALLIMIMUS_LAND );

		if ( GetSequence() != sequence )
		{
			//SetWings( false );
			VacateStrategySlot();
			SetIdealActivity( (Activity) ACT_GALLIMIMUS_LAND );

			CreateDust( false );
			EmitSound( "NPC_Gallimimus.Land" );

			if ( GetEnemy() && GetEnemy()->IsPlayer()  )
			{
				CBasePlayer *pPlayer = ToBasePlayer( GetEnemy() );

				if ( pPlayer && pPlayer->IsInAVehicle() == false )
					 MeleeAttack( GALLIMIMUS_MELEE1_RANGE, sk_gallimimus_swipe_damage.GetFloat(), QAngle( 4.0f, 0.0f, 0.0f ), Vector( -250.0f, 1.0f, 1.0f ) );
			}

			SetAbsVelocity( GetAbsVelocity() * 0.33f );
			return false;
		}

		return IsActivityFinished();
	}

	return false;
}

*/


/*
//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Gallimimus::Alone( void )
{
	if ( m_pSquad == NULL )
		return true;

	if ( m_pSquad->NumMembers() <= 1 )
		return true;

	return false;
}
*/
/*
//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Gallimimus::ShouldJump( void )
{
	if ( GetEnemy() == NULL )
		return false;

	//Too soon to try to jump
	if ( m_flJumpTime > gpGlobals->curtime )
		return false;

	// only jump if you're on the ground
  	if (!(GetFlags() & FL_ONGROUND) || GetNavType() == NAV_JUMP )
		return false;

	// Don't jump if I'm not allowed
	if ( ( CapabilitiesGet() & bits_CAP_MOVE_JUMP ) == false )
		return false;

	Vector vEnemyForward, vForward;

	GetEnemy()->GetVectors( &vEnemyForward, NULL, NULL );
	GetVectors( &vForward, NULL, NULL );

	float flDot = DotProduct( vForward, vEnemyForward );

	if ( flDot < 0.5f )
		 flDot = 0.5f;

	Vector vecPredictedPos;

	//Get our likely position in two seconds
	UTIL_PredictedPosition( GetEnemy(), flDot * 2.5f, &vecPredictedPos );

	// Don't jump if we're already near the target
	if ( ( GetAbsOrigin() - vecPredictedPos ).LengthSqr() < (512*512) )
		return false;

	//Don't retest if the target hasn't moved enough
	//FIXME: Check your own distance from last attempt as well
	if ( ( ( m_vecLastJumpAttempt - vecPredictedPos ).LengthSqr() ) < (128*128) )
	{
		m_flJumpTime = gpGlobals->curtime + random->RandomFloat( 1.0f, 2.0f );		
		return false;
	}

	Vector	targetDir = ( vecPredictedPos - GetAbsOrigin() );

	float flDist = VectorNormalize( targetDir );

	// don't jump at target it it's very close
	if (flDist < GALLIMIMUS_JUMP_MIN)
		return false;

	Vector	targetPos = vecPredictedPos + ( targetDir * (GetHullWidth()*4.0f) );

	if ( CGallimimusRepellant::IsPositionRepellantFree( targetPos ) == false )
		 return false;

	// Try the jump
	AIMoveTrace_t moveTrace;
	GetMoveProbe()->MoveLimit( NAV_JUMP, GetAbsOrigin(), targetPos, MASK_NPCSOLID, GetNavTargetEntity(), &moveTrace );

	//See if it succeeded
	if ( IsMoveBlocked( moveTrace.fStatus ) )
	{
		if ( g_debug_gallimimus.GetInt() == 2 )
		{
			NDebugOverlay::Box( targetPos, GetHullMins(), GetHullMaxs(), 255, 0, 0, 0, 5 );
			NDebugOverlay::Line( GetAbsOrigin(), targetPos, 255, 0, 0, 0, 5 );
		}

		m_flJumpTime = gpGlobals->curtime + random->RandomFloat( 1.0f, 2.0f );
		return false;
	}

	if ( g_debug_gallimimus.GetInt() == 2 )
	{
		NDebugOverlay::Box( targetPos, GetHullMins(), GetHullMaxs(), 0, 255, 0, 0, 5 );
		NDebugOverlay::Line( GetAbsOrigin(), targetPos, 0, 255, 0, 0, 5 );
	}

	//Save this jump in case the next time fails
	m_vecSavedJump = moveTrace.vJumpVelocity;
	m_vecLastJumpAttempt = targetPos;

	return true;
}
*/
/*
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::StartJump( void )
{
	if ( m_bForcedStuckJump == false )
	{
		// FIXME: Why must this be true?
		// Must be jumping at an enemy
		// if ( GetEnemy() == NULL )
		//	return;

		//Don't jump if we're not on the ground
		if ( ( GetFlags() & FL_ONGROUND ) == false )
			return;
	}

	//Take us off the ground
	SetGroundEntity( NULL );
	SetAbsVelocity( m_vecSavedJump );

	m_bForcedStuckJump = false;
#if HL2_EPISODIC
	m_bHasDoneAirAttack = false;
#endif

	//Setup our jump time so that we don't try it again too soon
	m_flJumpTime = gpGlobals->curtime + random->RandomInt( 2, 6 );
}

void CNPC_Gallimimus::LockJumpNode( void )
{
	if ( HasSpawnFlags( SF_GALLIMIMUS_USE_GROUNDCHECKS ) == false )
		 return;
	
	if ( GetNavigator()->GetPath() == NULL )
		 return;

	if ( g_test_new_gallimimus_jump.GetBool() == false )
		 return;

	AI_Waypoint_t *pWaypoint = GetNavigator()->GetPath()->GetCurWaypoint();

	while ( pWaypoint )
	{
		AI_Waypoint_t *pNextWaypoint = pWaypoint->GetNext();
		if ( pNextWaypoint && pNextWaypoint->NavType() == NAV_JUMP && pWaypoint->iNodeID != NO_NODE )
		{
			CAI_Node *pNode = GetNavigator()->GetNetwork()->GetNode( pWaypoint->iNodeID );

			if ( pNode )
			{
				//NDebugOverlay::Box( pNode->GetOrigin(), Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 255, 0, 0, 0, 2 );
				pNode->Lock( 0.5f );
				break;
			}
		}
		else
		{
			pWaypoint = pWaypoint->GetNext();
		}
	}
}

bool CNPC_Gallimimus::IsUnusableNode(int iNodeID, CAI_Hint *pHint)
{
	bool iBaseReturn = BaseClass::IsUnusableNode( iNodeID, pHint );

	if ( g_test_new_gallimimus_jump.GetBool() == 0 )
		 return iBaseReturn;

	CAI_Node *pNode = GetNavigator()->GetNetwork()->GetNode( iNodeID );

	if ( pNode )
	{
		if ( pNode->IsLocked() )
			 return true;
	}

	return iBaseReturn;
}
*/

bool NPC_CheckBrushExclude( CBaseEntity *pEntity, CBaseEntity *pBrush );
//-----------------------------------------------------------------------------
// traceline methods
//-----------------------------------------------------------------------------
class CTraceFilterSimpleNPCExclude : public CTraceFilterSimple
{
public:
	DECLARE_CLASS( CTraceFilterSimpleNPCExclude, CTraceFilterSimple );

	CTraceFilterSimpleNPCExclude( const IHandleEntity *passentity, int collisionGroup )
		: CTraceFilterSimple( passentity, collisionGroup )
	{
	}

	bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		Assert( dynamic_cast<CBaseEntity*>(pHandleEntity) );
		CBaseEntity *pTestEntity = static_cast<CBaseEntity*>(pHandleEntity);

		if ( GetPassEntity() )
		{
			CBaseEntity *pEnt = gEntList.GetBaseEntity( GetPassEntity()->GetRefEHandle() );

			if ( pEnt->IsNPC() )
			{
				if ( NPC_CheckBrushExclude( pEnt, pTestEntity ) == true )
					return false;
			}
		}
		return BaseClass::ShouldHitEntity( pHandleEntity, contentsMask );
	}
};

/*
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CNPC_Gallimimus::OnObstructionPreSteer( AILocalMoveGoal_t *pMoveGoal, float distClear, AIMoveResult_t *pResult )
{
	bool iBaseReturn = BaseClass::OnObstructionPreSteer( pMoveGoal, distClear, pResult );

//	if ( g_test_new_gallimimus_jump.GetBool() == false )
//		 return iBaseReturn;

	if ( HasSpawnFlags( SF_GALLIMIMUS_USE_GROUNDCHECKS ) == false )
		 return iBaseReturn;

	//CAI_BaseNPC *pBlocker = pMoveGoal->directTrace.pObstruction->MyNPCPointer();

	return iBaseReturn;
}
*/

void CNPC_Gallimimus::ManageFleeCapabilities( bool bEnable )
{
	if ( bEnable == false )
	{
		//Remove the jump capabilty when we build our route.
		//We'll enable it back again after the route has been built.
//		CapabilitiesRemove( bits_CAP_MOVE_JUMP );

		if ( HasSpawnFlags( SF_GALLIMIMUS_USE_GROUNDCHECKS ) == false  )
			 CapabilitiesRemove( bits_CAP_SKIP_NAV_GROUND_CHECK );
	}
	else
	{
//		if ( m_bDisableJump == false )
//			 CapabilitiesAdd( bits_CAP_MOVE_JUMP );

		if ( HasSpawnFlags( SF_GALLIMIMUS_USE_GROUNDCHECKS ) == false  )
			 CapabilitiesAdd( bits_CAP_SKIP_NAV_GROUND_CHECK );
	}
}


/*
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CNPC_Gallimimus::SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode )
{

	if ( m_FollowBehavior.GetNumFailedFollowAttempts() >= 2 )
	{
		if( IsFirmlyOnGround() == false )
		{
			Vector vecJumpDir; 
				
			vecJumpDir.z = 0;
			vecJumpDir.x = 0;
			vecJumpDir.y = 0;
			
			while( vecJumpDir.x == 0 && vecJumpDir.y == 0 )
			{
				vecJumpDir.x = random->RandomInt( -1, 1 ); 
				vecJumpDir.y = random->RandomInt( -1, 1 );
			}

			vecJumpDir.NormalizeInPlace();

			SetGroundEntity( NULL );
	
			m_vecSavedJump = vecJumpDir * 512 + Vector( 0, 0, 256 );
			m_bForcedStuckJump = true;
	
			return SCHED_GALLIMIMUS_JUMP;
		}
	}

	// Catch the LOF failure and choose another route to take
	if ( failedSchedule == SCHED_ESTABLISH_LINE_OF_FIRE )
		return SCHED_GALLIMIMUS_WORKER_FLANK_RANDOM;

	return BaseClass::SelectFailSchedule( failedSchedule, failedTask, taskFailCode );
}
*/

/*
bool CNPC_Gallimimus::IsFirmlyOnGround( void )
{
	if( !( GetFlags()&FL_ONGROUND ) )
		return false;

	trace_t tr;

	float flHeight =  fabs( GetHullMaxs().z - GetHullMins().z );
	
	Vector vOrigin = GetAbsOrigin() + Vector( GetHullMins().x, GetHullMins().y, 0 );
//	NDebugOverlay::Line( vOrigin, vOrigin - Vector( 0, 0, flHeight * 0.5  ), 255, 0, 0, true, 5 );
	UTIL_TraceLine( vOrigin, vOrigin - Vector( 0, 0, flHeight * 0.5  ), MASK_NPCSOLID, this, GetCollisionGroup(), &tr );

	if ( tr.fraction != 1.0f )
		 return true;
	
	vOrigin = GetAbsOrigin() - Vector( GetHullMins().x, GetHullMins().y, 0 );
//	NDebugOverlay::Line( vOrigin, vOrigin - Vector( 0, 0, flHeight * 0.5  ), 255, 0, 0, true, 5 );
	UTIL_TraceLine( vOrigin, vOrigin - Vector( 0, 0, flHeight * 0.5  ), MASK_NPCSOLID, this, GetCollisionGroup(), &tr );

	if ( tr.fraction != 1.0f )
		 return true;

	vOrigin = GetAbsOrigin() + Vector( GetHullMins().x, -GetHullMins().y, 0 );
//	NDebugOverlay::Line( vOrigin, vOrigin - Vector( 0, 0, flHeight * 0.5  ), 255, 0, 0, true, 5 );
	UTIL_TraceLine( vOrigin, vOrigin - Vector( 0, 0, flHeight * 0.5  ), MASK_NPCSOLID, this, GetCollisionGroup(), &tr );

	if ( tr.fraction != 1.0f )
		 return true;

	vOrigin = GetAbsOrigin() + Vector( -GetHullMins().x, GetHullMins().y, 0 );
//	NDebugOverlay::Line( vOrigin, vOrigin - Vector( 0, 0, flHeight * 0.5  ), 255, 0, 0, true, 5 );
	UTIL_TraceLine( vOrigin, vOrigin - Vector( 0, 0, flHeight * 0.5  ), MASK_NPCSOLID, this, GetCollisionGroup(), &tr );

	if ( tr.fraction != 1.0f )
		 return true;
	
	return false;
}
*/

//-----------------------------------------------------------------------------
// Purpose: Allows for modification of the interrupt mask for the current schedule.
//			In the most cases the base implementation should be called first.
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::BuildScheduleTestBits( void )
{
	//Don't allow any modifications when scripted
	if ( m_NPCState == NPC_STATE_SCRIPT )
		return;

	
	if ( !IsCurSchedule( SCHED_GALLIMIMUS_DROWN ) )
	{
		// Interrupt any schedule unless already drowning.
		SetCustomInterruptCondition( COND_GALLIMIMUS_IN_WATER );
	}
	else
	{
		// Don't stop drowning just because you're in water!
		ClearCustomInterruptCondition( COND_GALLIMIMUS_IN_WATER );
	}
	
	
	// Make sure we don't stop in midair
	/* FIX ME ??
	if ( GetActivity() == ACT_JUMP || GetActivity() == ACT_GLIDE || GetActivity() == ACT_LAND )
	{
		ClearCustomInterruptCondition( COND_NEW_ENEMY );
	}
	*/
	
	//Interrupt any schedule unless already fleeing, burrowing, burrowed, or unburrowing.
	if( !IsCurSchedule(SCHED_GALLIMIMUS_FLEE_DANGER)			&& 	
		!IsCurSchedule(SCHED_GALLIMIMUS_FLEE_PHYSICS_DANGER)	&& 		
//		!IsCurSchedule(SCHED_GALLIMIMUS_JUMP)					&&
		!IsCurSchedule(SCHED_GALLIMIMUS_FLIP)					&&
		( GetFlags() & FL_ONGROUND ) )
	{
		// Only do these if not jumping as well
/*		if (!IsCurSchedule(SCHED_GALLIMIMUS_JUMP))
		{
			if ( GetEnemy() == NULL )
			{
				SetCustomInterruptCondition( COND_HEAR_PHYSICS_DANGER );
			}
			
			SetCustomInterruptCondition( COND_HEAR_DANGER );
			SetCustomInterruptCondition( COND_HEAR_BUGBAIT );
			SetCustomInterruptCondition( COND_GALLIMIMUS_FLIPPED );
			SetCustomInterruptCondition( COND_GALLIMIMUS_CAN_JUMP_AT_TARGET );

		}
*/
	
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Gallimimus::ZapThink( void )
{
	CEffectData	data;
	data.m_nEntIndex = entindex();
	data.m_flMagnitude = 4;
	data.m_flScale = random->RandomFloat( 0.25f, 1.0f );

	DispatchEffect( "TeslaHitboxes", data );
	
	if ( m_flZapDuration > gpGlobals->curtime )
	{
		SetContextThink( &CNPC_Gallimimus::ZapThink, gpGlobals->curtime + random->RandomFloat( 0.05f, 0.25f ), "ZapThink" );
	}
	else
	{
		SetContextThink( NULL, gpGlobals->curtime, "ZapThink" );
	}
}

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CGallimimusRepellant )
	DEFINE_KEYFIELD( m_flRepelRadius,	FIELD_FLOAT,	"repelradius" ),
	DEFINE_FIELD( m_bEnabled, FIELD_BOOLEAN ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"Disable", InputDisable ),
END_DATADESC()

static CUtlVector< CHandle< CGallimimusRepellant > >m_hRepellantList;


CGallimimusRepellant::~CGallimimusRepellant()
{
	m_hRepellantList.FindAndRemove( this );
}

void CGallimimusRepellant::Spawn( void )
{
	BaseClass::Spawn();
	m_bEnabled = true;

	m_hRepellantList.AddToTail( this );
}

void CGallimimusRepellant::InputEnable( inputdata_t &inputdata )
{
	m_bEnabled = true;

	if ( m_hRepellantList.HasElement( this ) == false )
		 m_hRepellantList.AddToTail( this );
}

void CGallimimusRepellant::InputDisable( inputdata_t &inputdata )
{
	m_bEnabled = false;
	m_hRepellantList.FindAndRemove( this );
}

float CGallimimusRepellant::GetRadius( void )
{
	if ( m_bEnabled == false )
		 return 0.0f;

	return m_flRepelRadius;
}

void CGallimimusRepellant::OnRestore( void )
{
	BaseClass::OnRestore();

	if ( m_bEnabled == true )
	{
		if ( m_hRepellantList.HasElement( this ) == false )
			 m_hRepellantList.AddToTail( this );
	}
}

bool CGallimimusRepellant::IsPositionRepellantFree( Vector vDesiredPos )
{
	for ( int i = 0; i < m_hRepellantList.Count(); i++ )
	{
		if ( m_hRepellantList[i] )
		{
			CGallimimusRepellant *pRep = m_hRepellantList[i].Get();

			if ( pRep )
			{
				float flDist = (vDesiredPos - pRep->GetAbsOrigin()).Length();

				if ( flDist <= pRep->GetRadius() )
					 return false;
			}
		}
	}

	return true;
}

LINK_ENTITY_TO_CLASS( point_gallimimus_repellant, CGallimimusRepellant);


//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_gallimimus, CNPC_Gallimimus )

	//Register our interactions
	//DECLARE_INTERACTION( g_interactionGallimimusFoundTarget )
	//DECLARE_INTERACTION( g_interactionGallimimusFiredAtTarget )

	//Conditions
	DECLARE_CONDITION( COND_GALLIMIMUS_FLIPPED )
	//DECLARE_CONDITION( COND_GALLIMIMUS_CAN_JUMP )
	//DECLARE_CONDITION( COND_GALLIMIMUS_FOLLOW_TARGET_TOO_FAR )
	DECLARE_CONDITION( COND_GALLIMIMUS_IN_WATER )
	//DECLARE_CONDITION( COND_GALLIMIMUS_CAN_JUMP_AT_TARGET )
	//DECLARE_CONDITION( COND_GALLIMIMUS_SQUADMATE_KILLED )
		
	//Squad slots
	//DECLARE_SQUADSLOT( SQUAD_SLOT_GALLIMIMUS_JUMP )
	//DECLARE_SQUADSLOT( SQUAD_SLOT_GALLIMIMUS_WORKER_FIRE )

	//Tasks
	DECLARE_TASK( TASK_GALLIMIMUS_SET_CHARGE_GOAL )
	//DECLARE_TASK( TASK_GALLIMIMUS_JUMP )
	DECLARE_TASK( TASK_GALLIMIMUS_WAIT_FOR_TRIGGER )
	DECLARE_TASK( TASK_GALLIMIMUS_GET_DANGER_ESCAPE_PATH )
	DECLARE_TASK( TASK_GALLIMIMUS_GET_PHYSICS_DANGER_ESCAPE_PATH )
	//DECLARE_TASK( TASK_GALLIMIMUS_FACE_JUMP )
	DECLARE_TASK( TASK_GALLIMIMUS_DROWN )
	DECLARE_TASK( TASK_GALLIMIMUS_GET_PATH_TO_RANDOM_NODE )
	DECLARE_TASK( TASK_GALLIMIMUS_FIND_COVER_FROM_SAVEPOSITION )

	//Activities
	DECLARE_ACTIVITY( ACT_GALLIMIMUS_DISTRACT )
	DECLARE_ACTIVITY( ACT_GALLIMIMUS_DISTRACT_ARRIVED )
	//DECLARE_ACTIVITY( ACT_GALLIMIMUS_JUMP_START )
	DECLARE_ACTIVITY( ACT_GALLIMIMUS_RUN_AGITATED )
	DECLARE_ACTIVITY( ACT_GALLIMIMUS_FLIP )
	//DECLARE_ACTIVITY( ACT_GALLIMIMUS_POUNCE )
	//DECLARE_ACTIVITY( ACT_GALLIMIMUS_POUNCE_MOVING )
	DECLARE_ACTIVITY( ACT_GALLIMIMUS_DROWN )
	//DECLARE_ACTIVITY( ACT_GALLIMIMUS_LAND )
	//DECLARE_ACTIVITY( ACT_GALLIMIMUS_WORKER_EXPLODE )
	DECLARE_ACTIVITY( ACT_GALLIMIMUS_ZAP_FLIP )

	//Events
	DECLARE_ANIMEVENT( AE_GALLIMIMUS_WALK_FOOTSTEP )
	//DECLARE_ANIMEVENT( AE_GALLIMIMUS_MELEE_HIT1 )
	//DECLARE_ANIMEVENT( AE_GALLIMIMUS_MELEE_HIT2 )
	//DECLARE_ANIMEVENT( AE_GALLIMIMUS_MELEE_POUNCE )
	DECLARE_ANIMEVENT( AE_GALLIMIMUS_FOOTSTEP_SOFT )
	DECLARE_ANIMEVENT( AE_GALLIMIMUS_FOOTSTEP_HEAVY )
	//DECLARE_ANIMEVENT( AE_GALLIMIMUS_START_JUMP )
	
	//Schedules

	//==================================================
	// Jump
	//==================================================
/*
	DEFINE_SCHEDULE
	(
		SCHED_GALLIMIMUS_JUMP,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_GALLIMIMUS_FACE_JUMP			0"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_GALLIMIMUS_JUMP_START"
		"		TASK_GALLIMIMUS_JUMP				0"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
	)
*/

	//==================================================
	// Run from the sound of a fire source !
	//==================================================
	DEFINE_SCHEDULE
	(
		SCHED_GALLIMIMUS_FLEE_DANGER,
		
		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE					SCHEDULE:SCHED_COWER" //SCHED_IDLE_STAND"

		//"		TASK_GALLIMIMUS_GET_DANGER_SCAPE_PATH	0"
		"		TASK_STOP_MOVING						0"
		"		TASK_MOVE_AWAY_PATH						600"
		"		TASK_RUN_PATH_FLEE						0"
		"		TASK_WAIT_FOR_MOVEMENT					0"
		"		TASK_STOP_MOVING						0"
		"		TASK_PLAY_SEQUENCE						ACTIVITY:ACT_GALLIMIMUS_DISTRACT_ARRIVED"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_GALLIMIMUS_FLIPPED"
	)

	//==================================================
	// SCHED_GALLIMIMUS_ZAP_FLIP 
	//==================================================
	DEFINE_SCHEDULE
	(
		SCHED_GALLIMIMUS_ZAP_FLIP,

		"	Tasks"
		"		TASK_STOP_MOVING	0"
		"		TASK_RESET_ACTIVITY		0"
		"		TASK_PLAY_SEQUENCE		ACTIVITY:ACT_GALLIMIMUS_ZAP_FLIP"

		"	Interrupts"
		"		COND_TASK_FAILED"
	)
	
	//==================================================
	// SCHED_GALLIMIMUS_FLIP
	//==================================================
	DEFINE_SCHEDULE
	(
	SCHED_GALLIMIMUS_FLIP,

	"	Tasks"
	"		TASK_STOP_MOVING	0"
	"		TASK_RESET_ACTIVITY		0"
	"		TASK_PLAY_SEQUENCE		ACTIVITY:ACT_GALLIMIMUS_FLIP"

	"	Interrupts"
	"		COND_TASK_FAILED"
	)


	//==================================================
	// Run from the sound of a physics crash
	//==================================================
	DEFINE_SCHEDULE
	(
		SCHED_GALLIMIMUS_FLEE_PHYSICS_DANGER,
		
		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE						SCHEDULE:SCHED_ALERT_STAND" //SCHED_CHASE_ENEMY"
		"		TASK_GALLIMIMUS_GET_PHYSICS_DANGER_ESCAPE_PATH	1024"
		"		TASK_RUN_PATH								0"
		"		TASK_WAIT_FOR_MOVEMENT						0"
		"		TASK_STOP_MOVING							0"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
	)

	
	//=========================================================
	// The irreversible process of drowning
	//=========================================================
	
	DEFINE_SCHEDULE
	(
		SCHED_GALLIMIMUS_DROWN,

		"	Tasks"
		"		TASK_SET_ACTIVITY			ACTIVITY:ACT_GALLIMIMUS_DROWN"
		"		TASK_GALLIMIMUS_DROWN			0"
		""
		"	Interrupts"
	)
	

	
	DEFINE_SCHEDULE
	(
		SCHED_GALLIMIMUS_TAKE_COVER_FROM_ENEMY,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_FAIL_TAKE_COVER"
		"		TASK_FIND_COVER_FROM_ENEMY		0"
		"		TASK_RUN_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_STOP_MOVING				0"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
		//"		COND_NEW_ENEMY"
	)

	DEFINE_SCHEDULE
	(
		SCHED_GALLIMIMUS_TAKE_COVER_FROM_SAVEPOSITION,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE						SCHEDULE:SCHED_FAIL_TAKE_COVER"
		"		TASK_GALLIMIMUS_FIND_COVER_FROM_SAVEPOSITION	0"
		"		TASK_RUN_PATH								0"
		"		TASK_WAIT_FOR_MOVEMENT						0"
		"		TASK_STOP_MOVING							0"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
		//"		COND_NEW_ENEMY"
	)

AI_END_CUSTOM_NPC()


/*
//-----------------------------------------------------------------------------
// Purpose: Whether or not the target is a worker class of Gallimimus
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool IsGallimimusWorker( CBaseEntity *pEntity )
{
	// Must at least be valid and an Gallimimus
	return ( pEntity != NULL && 
			 pEntity->Classify() == CLASS_GALLIMIMUS && 
			 pEntity->HasSpawnFlags( SF_GALLIMIMUS_WORKER ) &&
			 dynamic_cast<CNPC_Gallimimus *>(pEntity) != NULL );	// Save this as the last step
}
*/

//-----------------------------------------------------------------------------
// Purpose: Whether or not the entity is a common Gallimimus
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool IsGallimimus( CBaseEntity *pEntity )
{
	// Must at least be valid and an Gallimimus
	return ( pEntity != NULL && 
			 pEntity->Classify() == CLASS_GALLIMIMUS && 
			 dynamic_cast<CNPC_Gallimimus *>(pEntity) != NULL );	// Save this as the last step
}

/*
#ifdef HL2_EPISODIC
//-----------------------------------------------------------------------------
// Purpose: Used by other entities to judge the Gallimimus worker's radius of damage
//-----------------------------------------------------------------------------
float GallimimusWorkerBurstRadius( void )
{
	return sk_gallimimus_worker_burst_radius.GetFloat();
}
#endif // HL2_EPISODIC
*/
