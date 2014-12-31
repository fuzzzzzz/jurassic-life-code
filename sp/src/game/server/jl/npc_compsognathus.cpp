//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements the compy, a tiny, jumpy dinosaur.
//
// TODO: make poison compy hop in response to nearby bullet impacts?
//
//=============================================================================//

#include "cbase.h" // OK CbaseEntity ( Touch, TouchDamage, LeapTouch, ClimbTouch)
#include "world.h" // Ok link to CBaseEntity
#include "game.h" // must be set ! link with ?? base classes)
#include "bitstring.h" // must be set ! link with CBaseEntity)
#include "decals.h" // OK CreateDust
#include "props.h" // OK CPhysicProp (ClimbTouch) 

#include "hl2_shareddefs.h" // OK collision Group
#include "hl2_gamerules.h" //OK for colission ( ShouldCollide)

#include "ai_basenpc.h" // OK (parent class)
#include "ai_default.h" // OK  CAIBase_npc schedules
//#include "ai_behavior.h"
//#include "ai_behavior_follow.h"
#include "ai_schedule.h" // OK (default schedule)
#include "ai_hint.h" //OK (burrow/unburrow)
#include "ai_hull.h" //OK 

#include "ai_task.h"

#include "ai_interactions.h" // ok interaction with Vortigaut
#include "ai_navigator.h" // cAI_navigator for the ClimbTouch() Class
#include "ai_moveprobe.h" // CAI_Moveprobe is used with task hop_ASIDE Task in the StartTask() Class and RunTask() Class but only in CFastCompsognathus class !
#include "ai_motor.h"
#include "ai_memory.h"
#include "ai_pathfinder.h"
#include "ai_route.h"
#include "ai_tacticalservices.h"

// my header
#include "jl/npc_compsognathus.h"

//#include "vcollide.h"
#include "npcevent.h" // OK CBaseAnimating
#include "soundent.h" // OK
#include "physics_prop_ragdoll.h"

// not sure if this are need,
// because i don't any link to CGIB class or the CRagGIB class definied in gib.cpp
// just a link to the EventGibbed() class from the CAIBase_NPC class)
#include "gib.h" 

#include "vehicle_base.h" // OK use for vehicle_collision 

#include "antlion_dust.h" // OK UTIL_CreateAntlionDust
#include "ndebugoverlay.h" // OK use with Vectors ?
#include "vstdlib/random.h" // Ok Randomfloat()
#include "engine/IEngineSound.h" // OK need for precaching sound !
#include "movevars_shared.h" // OK it's is a must to define for sv_commands in the console

#include "npc_bullseye.h"  // ok (CBaseCompsognathus:RangeAttack1Conditions)
#include "physics_npc_solver.h"  // OK TASK_COMPY_HOP_OFF_NPC, and ClimbTouch() class 

#include "particle_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define COMPY_ATTN_IDLE				(float)0.5 // 1.5
#define COMPY_GUTS_GIB_COUNT		1
#define COMPY_LEGS_GIB_COUNT		3
#define COMPY_ALL_GIB_COUNT		5

#define COMPY_RUNMODE_ACCELERATE		1
#define COMPY_RUNMODE_IDLE			2
#define COMPY_RUNMODE_DECELERATE		3
#define COMPY_RUNMODE_FULLSPEED		4
#define COMPY_RUNMODE_PAUSE			5

#define COMPY_RUN_MINSPEED	1.0 // 0.5
#define COMPY_RUN_MAXSPEED	1.1 // 1.0

const float COMPY_BURROWED_FOV = -1.0f;
const float COMPY_UNBURROWED_FOV = 0.5f;

#define COMPY_IGNORE_WORLD_COLLISION_TIME 1.0//0.5

const int COMPY_MIN_JUMP_DIST = 32; //48; //32; //12
const int COMPY_MAX_JUMP_DIST = 96; //72; //64; // 256

#define COMPY_BURROW_POINT_SEARCH_RADIUS 256.0


// compy attack 
#define	COMPY_MELEE1_RANGE		48 //31.0f //64.0f

// Debugging
#define	COMPY_DEBUG_HIDING		1

#define COMPY_BURN_SOUND_FREQUENCY 10

//CLIMB and JUMP
/*
#define COMPY_IDLE_PITCH			35
#define COMPY_MIN_PITCH			70
#define COMPY_MAX_PITCH			130
#define COMPY_SOUND_UPDATE_FREQ	0.5
*/
ConVar g_debug_compsognathus( "g_debug_compsognathus", "0", FCVAR_CHEAT );

// NPC damage adjusters
ConVar	sk_compy_head( "sk_compy_head","2" );
ConVar	sk_compy_chest( "sk_compy_chest","1" );
ConVar	sk_compy_stomach( "sk_compy_stomach","1" );
ConVar	sk_compy_arm( "sk_compy_arm","1" );
ConVar	sk_compy_leg( "sk_compy_leg","1" );

//------------------------------------
// Spawnflags
//------------------------------------
#define SF_COMPY_START_HIDDEN		(1 << 16)
#define SF_COMPY_START_HANGING		(1 << 17)
#define	SF_COMPY_SERVERSIDE_RAGDOLL	( 1 << 18 )

//-----------------------------------------------------------------------------
// Think contexts.
//-----------------------------------------------------------------------------
static const char *s_pPitchContext = "PitchContext";


//-----------------------------------------------------------------------------
// Animation events.
//-----------------------------------------------------------------------------
int AE_COMPY_JUMPATTACK;
int AE_COMPY_JUMP_TELEGRAPH;
int AE_POISONCOMPY_FLINCH_HOP;
int AE_POISONCOMPY_FOOTSTEP;
int AE_POISONCOMPY_THREAT_SOUND;
int AE_COMPY_BURROW_IN;
int AE_COMPY_BURROW_IN_FINISH;
int AE_COMPY_BURROW_OUT;
int AE_COMPY_CEILING_DETACH;

int AE_COMPY_FOOTSTEP;

int AE_COMPY_MELEE1_HIT;
int AE_COMPY_MELEE1_ANNOUNCE;

// climb and jump capability
//int AE_COMPY_CLIMB_LEFT;
//int AE_COMPY_CLIMB_RIGHT;

//-----------------------------------------------------------------------------
// Custom schedules.
//-----------------------------------------------------------------------------
enum
{
	SCHED_COMPY_RANGE_ATTACK1 = LAST_SHARED_SCHEDULE,
	SCHED_COMPY_WAKE_ANGRY,
	SCHED_COMPY_WAKE_ANGRY_NO_DISPLAY,
	//SCHED_COMPY_SWIM,
	//SCHED_COMPY_FAIL_SWIM,
	SCHED_COMPY_AMBUSH,
	SCHED_COMPY_HOP_RANDOMLY, // get off something you're not supposed to be on.
	//SCHED_COMPY_BARNACLED,
	SCHED_COMPY_UNHIDE,
	SCHED_COMPY_HARASS_ENEMY,
	SCHED_COMPY_FALL_TO_GROUND,
	SCHED_COMPY_RUN_TO_BURROW_IN,
	SCHED_COMPY_RUN_TO_SPECIFIC_BURROW,
	SCHED_COMPY_BURROW_IN,
	SCHED_COMPY_BURROW_WAIT,
	SCHED_COMPY_BURROW_OUT,
	SCHED_COMPY_WAIT_FOR_CLEAR_UNBURROW,
	SCHED_COMPY_CRAWL_FROM_CANISTER,
	
	SCHED_FAST_COMPY_RANGE_ATTACK1,

	SCHED_COMPY_CEILING_WAIT,
	SCHED_COMPY_CEILING_DROP,

	// climb, jump
	//SCHED_COMPY_UNSTICK_JUMP,
	//SCHED_COMPY_CLIMBING_UNSTICK_JUMP,
	//SCHED_COMPY_TAKE_SAFEPOSITION,
	
};


//=========================================================
// tasks
//=========================================================
enum 
{
	TASK_COMPY_HOP_ASIDE = LAST_SHARED_TASK,
	TASK_COMPY_HOP_OFF_NPC,
	//TASK_COMPY_SWIM,
	//TASK_COMPY_WAIT_FOR_BARNACLE_KILL,
	TASK_COMPY_UNHIDE,
	TASK_COMPY_HARASS_HOP,
	TASK_COMPY_FIND_BURROW_IN_POINT,
	TASK_COMPY_BURROW,
	TASK_COMPY_UNBURROW,
	TASK_COMPY_BURROW_WAIT,
	TASK_COMPY_CHECK_FOR_UNBURROW,
	TASK_COMPY_JUMP_FROM_CANISTER,
	TASK_COMPY_CLIMB_FROM_CANISTER,
	
	TASK_COMPY_CEILING_WAIT,
	TASK_COMPY_CEILING_POSITION,
	TASK_COMPY_CEILING_DETACH,
	TASK_COMPY_CEILING_FALL,
	TASK_COMPY_CEILING_LAND,

	//climb and jump

	TASK_COMPY_LAND_RECOVER,
	//TASK_COMPY_UNSTICK_JUMP,
	TASK_COMPY_JUMP_BACK,
	//TASK_COMPY_TAKE_SAFEPOSITION,
	//TASK_COMPY_FALL_TO_GROUND,
	//LAST_BASE_COMPY_TASK, //is buggy ?
};


//=========================================================
// conditions 
//=========================================================
enum
{
	COND_COMPY_IN_WATER = LAST_SHARED_CONDITION,
	COND_COMPY_ILLEGAL_GROUNDENT = LAST_SHARED_CONDITION,
	//COND_COMPY_BARNACLED,
	COND_COMPY_UNHIDE,
	//LAST_BASE_COMPY_CONDITION,
	//COND_COMPY_CLIMB_TOUCH	= LAST_BASE_COMPY_CONDITION,
};

//=========================================================
// private activities
//=========================================================
int ACT_COMPY_THREAT_DISPLAY;
int ACT_COMPY_HOP_LEFT;
int ACT_COMPY_HOP_RIGHT;
//int ACT_COMPY_DROWN;
int ACT_COMPY_BURROW_IN;
int ACT_COMPY_BURROW_OUT;
int ACT_COMPY_BURROW_IDLE;
int ACT_COMPY_CRAWL_FROM_CANISTER_LEFT;
int ACT_COMPY_CRAWL_FROM_CANISTER_CENTER;
int ACT_COMPY_CRAWL_FROM_CANISTER_RIGHT;
int ACT_COMPY_CEILING_IDLE;
int ACT_COMPY_CEILING_DETACH;
int ACT_COMPY_CEILING_FALL;
int ACT_COMPY_CEILING_LAND;

//climb
int ACT_COMPY_LAND_RIGHT;
int ACT_COMPY_LAND_LEFT;

int ACT_FALL_TO_GROUND;
//-----------------------------------------------------------------------------
// Skill settings.
//-----------------------------------------------------------------------------
ConVar	sk_compsognathus_health( "sk_compsognathus_health","0");
ConVar	sk_compsognathus_fast_health( "sk_compsognathus_fast_health","0");
ConVar	sk_compsognathus_poison_health( "sk_compsognathus_poison_health","0");
ConVar	sk_compsognathus_melee_dmg( "sk_compsognathus_melee_dmg","0");
ConVar	sk_compsognathus_poison_npc_damage( "sk_compsognathus_poison_npc_damage", "0" );

BEGIN_DATADESC( CBaseCompsognathus )

	// m_nGibCount - don't save
	DEFINE_FIELD( m_bHidden, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flTimeDrown, FIELD_TIME ),
	DEFINE_FIELD( m_bCommittedToJump, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_vecCommittedJumpPos, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_flNextNPCThink, FIELD_TIME ),
	DEFINE_FIELD( m_flIgnoreWorldCollisionTime, FIELD_TIME ),
	
	DEFINE_KEYFIELD( m_bStartBurrowed, FIELD_BOOLEAN, "startburrowed" ),
	DEFINE_FIELD( m_bBurrowed, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flBurrowTime, FIELD_TIME ),
	DEFINE_FIELD( m_nContext, FIELD_INTEGER ),
	DEFINE_FIELD( m_bCrawlFromCanister, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bMidJump, FIELD_BOOLEAN ), // original Headcrab jump pointer
	DEFINE_FIELD( m_nJumpFromCanisterDir, FIELD_INTEGER ),

	DEFINE_FIELD( m_bHangingFromCeiling, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flIlluminatedTime, FIELD_TIME ),


// climb
	//DEFINE_FIELD( m_iClimbCount, FIELD_CHARACTER ),
//jump

	DEFINE_FIELD( m_fIsNavJumping, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fHitApex, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flJumpDist, FIELD_FLOAT ),
	DEFINE_FIELD( m_fJustJumped, FIELD_BOOLEAN ),

	DEFINE_INPUTFUNC( FIELD_VOID,	"Burrow", InputBurrow ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"BurrowImmediate", InputBurrowImmediate ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"Unburrow", InputUnburrow ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"StartHangingFromCeiling", InputStartHangingFromCeiling ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"DropFromCeiling", InputDropFromCeiling ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"Ragdoll", InputRagdoll ),

	// Function Pointers
	DEFINE_THINKFUNC( EliminateRollAndPitch ),
	DEFINE_THINKFUNC( ThrowThink ),
	DEFINE_ENTITYFUNC( LeapTouch ),

// CLIMB CAPABILITY

//	DEFINE_ENTITYFUNC( ClimbTouch ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::Spawn( void )
{
	//Precache();
	//SetModel( "models/headcrab.mdl" );
	//m_iHealth			= sk_headcrab_health.GetFloat();
	
#ifdef _XBOX
	// Always fade the corpse
	AddSpawnFlags( SF_NPC_FADE_CORPSE );
#endif // _XBOX

	SetHullType( HULL_TINY );
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetNavType( NAV_GROUND );

	SetMoveType( MOVETYPE_STEP );

	// ?? Try with HL2COLLISION_GROUP_NPC_ACTOR ( not collide with us)or custom collision group ?
	// Take a look in hl2_gamerules.cpp and HL2_sharedef.cpp for the declared NPC groups.
	// original value : HL2COLLISION_GROUP_HEADCRAB
	SetCollisionGroup( JLCOLLISION_GROUP_COMPY ); 

	SetViewOffset( Vector(6, 0, 11) ) ;		// Position of the eyes relative to NPC's origin.

	SetBloodColor( BLOOD_COLOR_RED );
	m_flFieldOfView		= 0.5;
	m_NPCState			= NPC_STATE_NONE;
	m_nGibCount			= COMPY_ALL_GIB_COUNT;
	m_hFollowTarget = NULL;

	m_nBodyBone = -1;

	// Are we starting hidden?
	if ( m_spawnflags & SF_COMPY_START_HIDDEN )
	{
		m_bHidden = true;
		AddSolidFlags( FSOLID_NOT_SOLID );
		SetRenderColorA( 0 );
		m_nRenderMode = kRenderTransTexture;
		AddEffects( EF_NODRAW );
	}
	else
	{
		m_bHidden = false;
	}

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_INNATE_MELEE_ATTACK1 | bits_CAP_INNATE_RANGE_ATTACK1 );
	CapabilitiesAdd( bits_CAP_MOVE_JUMP  | bits_CAP_TURN_HEAD ); //| bits_CAP_MOVE_FLY ); // bits_CAP_MOVE_CLIMB

	//Only do this if a squadname appears in the entity
	if ( m_SquadName != NULL_STRING )
	{
		CapabilitiesAdd( bits_CAP_SQUAD );
	}

//Climb

	//m_iClimbCount = 0;

//JUMP
	EndNavJump();

	// headcrabs get to cheat for 5 seconds (sjb)
	GetEnemies()->SetFreeKnowledgeDuration( 5.0 );

	m_bHangingFromCeiling = false;
	m_flIlluminatedTime = -1;
}

//-----------------------------------------------------------------------------
// Purpose: Stuff that must happen after NPCInit is called.
//-----------------------------------------------------------------------------
void CBaseCompsognathus::CompsognathusInit()
{
	// See if we're supposed to start burrowed
	if ( m_bStartBurrowed )
	{
		SetBurrowed( true );
		SetSchedule( SCHED_COMPY_BURROW_WAIT );
	}

	if ( GetSpawnFlags() & SF_COMPY_START_HANGING )
	{
		SetSchedule( SCHED_COMPY_CEILING_WAIT );
		m_flIlluminatedTime = -1;
	}
}	


//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CBaseCompsognathus::Precache( void )
{
	BaseClass::Precache();
}	


//-----------------------------------------------------------------------------
// The headcrab will crawl from the cannister, then jump to a burrow point
//-----------------------------------------------------------------------------
void CBaseCompsognathus::CrawlFromCanister()
{
	// This is necessary to prevent ground computations, etc. from happening
	// while the crawling animation is occuring
	AddFlag( FL_FLY );
	m_bCrawlFromCanister = true;
	SetNextThink( gpGlobals->curtime );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : NewActivity - 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::OnChangeActivity( Activity NewActivity )
{
	bool fRandomize = false;
	float flRandomRange = 0.0;

	// If this crab is starting to walk or idle, pick a random point within
	// the animation to begin. This prevents lots of crabs being in lockstep.
	if ( NewActivity == ACT_IDLE )
	{
		flRandomRange = 0.75;
		fRandomize = true;
	}
	else if ( NewActivity == ACT_RUN )
	{
		flRandomRange = 0.25;
		fRandomize = true;
	}

	if( fRandomize )
	{
		SetCycle( random->RandomFloat( 0.0, flRandomRange ) );
	}
	
//__________________//
//					//
// JUMP CAPABILITY //
//__________________//


	if( NewActivity == ACT_JUMP )
	{
		BeginNavJump();
	}
	else if( GetActivity() == ACT_JUMP )
	{
		EndNavJump();
	}

	if ( NewActivity == ACT_LAND )
	{
		m_flNextAttack = gpGlobals->curtime + 1.0;
	}

	if ( NewActivity == ACT_GLIDE )
	{
		// Started a jump.
		BeginNavJump();
	}
	else if ( GetActivity() == ACT_GLIDE )
	{
		// Landed a jump
		EndNavJump();

	}
//__________________//
//					//
// CLIMB CAPABILITY //
//__________________//
/*
	if ( NewActivity == ACT_CLIMB_UP )
	{
		// Started a climb!
		SetTouch( &CBaseCompsognathus::ClimbTouch );
	}
	else if ( GetActivity() == ACT_CLIMB_DISMOUNT || ( GetActivity() == ACT_CLIMB_UP && NewActivity != ACT_CLIMB_DISMOUNT ) )
	{
		// Ended a climb
		SetTouch( NULL );
	}
*/
	BaseClass::OnChangeActivity( NewActivity );

	if( fRandomize )
	{
		SetCycle( random->RandomFloat( 0.0, flRandomRange ) );
	}

}
//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CBaseCompsognathus::Classify( void )
{
	if( m_bHidden )
	{
		// Effectively invisible to other AI's while hidden.
		return( CLASS_NONE ); 
	}
	else
	{
		return( CLASS_COMPY );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &posSrc - 
// Output : Vector
//-----------------------------------------------------------------------------
Vector CBaseCompsognathus::BodyTarget( const Vector &posSrc, bool bNoisy ) 
{ 
	/* OLD headcrab code 
	Vector vecResult;
	vecResult = GetAbsOrigin();
	vecResult.z += 6;
	return vecResult;
	*/
	// Cache the bone away to avoid future lookups
	if ( m_nBodyBone == -1 )
	{
		CBaseAnimating *pAnimating = GetBaseAnimating();
		CBaseEntity *pPlayer = GetEnemy();
		if ( pPlayer )
		{
			m_nBodyBone = pAnimating->LookupBone( "Bip01 Neck" ); //pelvis
		}
		CBaseEntity *pEnemy = GetEnemy();
		if ( pEnemy )
		{
			m_nBodyBone = pAnimating->LookupBone( "ValveBiped.Bip01_Head1" ); //pelvis
		}
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
//-----------------------------------------------------------------------------
float CBaseCompsognathus::GetAutoAimRadius()
{ 
	if( g_pGameRules->GetAutoAimMode() == AUTOAIM_ON_CONSOLE )
	{
		return 24.0f;
	}

	return 12.0f;
}


//-----------------------------------------------------------------------------
// Purpose: Allows each sequence to have a different turn rate associated with it.
// Output : float
//-----------------------------------------------------------------------------
float CBaseCompsognathus::MaxYawSpeed( void )
{
	return BaseClass::MaxYawSpeed();
}

//-----------------------------------------------------------------------------
// Because the AI code does a tracehull to find the ground under an NPC, headcrabs
// can often be seen standing with one edge of their box perched on a ledge and
// 80% or more of their body hanging out over nothing. This is often a case
// where a headcrab will be unable to pathfind out of its location. This heuristic
// very crudely tries to determine if this is the case by casting a simple ray 
// down from the center of the headcrab.
//-----------------------------------------------------------------------------
#define COMPY_MAX_LEDGE_HEIGHT	5.0f //32.0f // 24.0f // 12.0f
bool CBaseCompsognathus::IsFirmlyOnGround()
{
	if( !(GetFlags()&FL_ONGROUND) )
		return false;

	trace_t tr;
	UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() - Vector( 0, 0, COMPY_MAX_LEDGE_HEIGHT ), MASK_NPCSOLID, this, GetCollisionGroup(), &tr );
	return tr.fraction != 1.0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CBaseCompsognathus::MoveOrigin( const Vector &vecDelta )
{
	UTIL_SetOrigin( this, GetLocalOrigin() + vecDelta );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : vecPos - 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::ThrowAt( const Vector &vecPos )
{
	JumpAttack( false, vecPos, true );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : vecPos - 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::JumpToBurrowHint( CAI_Hint *pHint )
{
	Vector vecVel = VecCheckToss( this, GetAbsOrigin(), pHint->GetAbsOrigin(), 0.5f, 1.0f, false, NULL, NULL );

	// Undershoot by a little because it looks bad if we overshoot and turn around to burrow.
	vecVel *= 0.9f;
	Leap( vecVel );

	GrabHintNode( pHint );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : vecVel - 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::Leap( const Vector &vecVel )
{
	SetTouch( &CBaseCompsognathus::LeapTouch );

	SetCondition( COND_FLOATING_OFF_GROUND );
	SetGroundEntity( NULL );

	m_flIgnoreWorldCollisionTime = gpGlobals->curtime + COMPY_IGNORE_WORLD_COLLISION_TIME;

	if( HasHeadroom() )
	{
		// Take him off ground so engine doesn't instantly reset FL_ONGROUND.
		MoveOrigin( Vector( 0, 0, 1 ) );
	}

	SetAbsVelocity( vecVel );

	// Think every frame so the player sees the headcrab where he actually is...
	m_bMidJump = true;
	SetThink( &CBaseCompsognathus::ThrowThink );
	SetNextThink( gpGlobals->curtime );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::ThrowThink( void )
{
	if (gpGlobals->curtime > m_flNextNPCThink)
	{
		NPCThink();
		m_flNextNPCThink = gpGlobals->curtime + 0.1;
	}

	if( GetFlags() & FL_ONGROUND )
	{
		SetThink( &CBaseCompsognathus::CallNPCThink );
		SetNextThink( gpGlobals->curtime + 0.1 );
		return;
	}

	SetNextThink( gpGlobals->curtime );
}


//-----------------------------------------------------------------------------
// Purpose: Does a jump attack at the given position.
// Input  : bRandomJump - Just hop in a random direction.
//			vecPos - Position to jump at, ignored if bRandom is set to true.
//			bThrown - 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::JumpAttack( bool bRandomJump, const Vector &vecPos, bool bThrown )
{
	Vector vecJumpVel;
	if ( !bRandomJump )
	{
		float gravity = sv_gravity.GetFloat();
		if ( gravity <= 1 )
		{
			gravity = 1;
		}

		// How fast does the headcrab need to travel to reach the position given gravity?
		float flActualHeight = vecPos.z - GetAbsOrigin().z;
		float height = flActualHeight;
		if ( height < 8 ) //if ( height < 16)
		{
			height = 8;  //height = 16;
		}
		else
		{
			float flMaxHeight = bThrown ? 60 : 80; // 100 : 120; //200 : 120;  // 400 : 120;
			if ( height > flMaxHeight )
			{
				height = flMaxHeight;
			}
		}

		// overshoot the jump by an additional 8 inches
		// NOTE: This calculation jumps at a position INSIDE the box of the enemy (player)
		// so if you make the additional height too high, the crab can land on top of the
		// enemy's head.  If we want to jump high, we'll need to move vecPos to the surface/outside
		// of the enemy's box.
		
		float additionalHeight = 0;
		if ( height < 16 ) // 32
		{
			additionalHeight = 1; //4; // 8
		}

		height += additionalHeight;

		// NOTE: This equation here is from vf^2 = vi^2 + 2*a*d
		float speed = sqrt( 2 * gravity * height );
		float time = speed / gravity;

		// add in the time it takes to fall the additional height
		// So the impact takes place on the downward slope at the original height
		time += sqrt( (2 * additionalHeight) / gravity );

		// Scale the sideways velocity to get there at the right time
		VectorSubtract( vecPos, GetAbsOrigin(), vecJumpVel );
		vecJumpVel /= time;

		// Speed to offset gravity at the desired height.
		vecJumpVel.z = speed;

		// Don't jump too far/fast.
		float flJumpSpeed = vecJumpVel.Length();
		float flMaxSpeed = bThrown ? 1000.0f : 650.0f;
		if ( flJumpSpeed > flMaxSpeed )
		{
			vecJumpVel *= flMaxSpeed / flJumpSpeed;
		}
	}
	else
	{
		//
		// Jump hop, don't care where.
		//
		Vector forward, up;
		AngleVectors( GetLocalAngles(), &forward, NULL, &up );
		vecJumpVel = Vector( forward.x, forward.y, up.z ) * 350;
	}

	AttackSound();
	Leap( vecJumpVel );

}


//-----------------------------------------------------------------------------
// Purpose: Catches the monster-specific messages that occur when tagged
//			animation frames are played.
// Input  : *pEvent - 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_COMPY_JUMPATTACK )
	{
		// Ignore if we're in mid air
	//	if ( m_bMidJump )
	//		return;

		CBaseEntity *pEnemy = GetEnemy();
		
		// don't target the eyeposition of the dinosaurs
		if ( pEnemy )
		{
			if ( m_bCommittedToJump )
			{
				JumpAttack( false, m_vecCommittedJumpPos );
			}
			else
			{
				// Jump at my enemy's eyes.
				JumpAttack( false, pEnemy->EyePosition() );
				//JumpAttack( false, pEnemy->BodyTarget( m_nBodyBone ) );
				
			}

			m_bCommittedToJump = false;
		}
		
		// target eyeposition of the player
		CBaseEntity *pPlayer = GetEnemy();
		if ( pPlayer )
		{
			if ( m_bCommittedToJump )
			{
				JumpAttack( false, m_vecCommittedJumpPos );
			}
		else
			{
				// Jump at my enemy's eyes.
				JumpAttack( false, pEnemy->EyePosition() );
				//JumpAttack( false, pEnemy->BodyTarget( m_nBodyBone ) );
			}

			m_bCommittedToJump = false;
			
		}
		else
		{
			// Jump hop, don't care where.
			JumpAttack( true );
		}

		return;
	}
	
	if ( pEvent->event == AE_COMPY_MELEE1_ANNOUNCE )
	{
		MeleeAttack( COMPY_MELEE1_RANGE, sk_compsognathus_melee_dmg.GetFloat(), QAngle( 20.0f, 0.0f, -12.0f ), Vector( -250.0f, 1.0f, 1.0f ) );
		return;
	}

	if ( pEvent->event == AE_COMPY_MELEE1_HIT )
	{
		EmitSound( "NPC_Compy.MeleeAttack" );
		
		return;
	}

	if ( pEvent->event == AE_COMPY_CEILING_DETACH )
	{
		SetMoveType( MOVETYPE_STEP );
		RemoveFlag( FL_ONGROUND );
		RemoveFlag( FL_FLY );

		SetAbsVelocity( Vector ( 0, 0, -128 ) );
		return;
	}
	if ( pEvent->event == AE_COMPY_JUMP_TELEGRAPH )
	{
		TelegraphSound();

		CBaseEntity *pEnemy = GetEnemy();
		
		if ( pEnemy )
		{
			// Once we telegraph, we MUST jump. This is also when commit to what point
			// we jump at. Jump at our enemy's eyes.
			m_vecCommittedJumpPos = pEnemy->EyePosition();
			m_bCommittedToJump = true;
		}

		return;
	}
	if ( pEvent->event == AE_COMPY_FOOTSTEP )
	{
		EmitSound( "NPC_Compy.Footstep" );
		
		return;
	}

	if ( pEvent->event == AE_COMPY_BURROW_IN )
	{
		EmitSound( "NPC_Compy.BurrowIn" );
		CreateDust();

		return;
	}

	if ( pEvent->event == AE_COMPY_BURROW_IN_FINISH )
	{
		SetBurrowed( true );
		return;
	}

	if ( pEvent->event == AE_COMPY_BURROW_OUT )
	{
		Assert( m_bBurrowed );
		if ( m_bBurrowed )
		{
			EmitSound( "NPC_Compy.BurrowOut" );
			CreateDust();
			SetBurrowed( false );

			// We're done with this burrow hint node. It might be NULL here
			// because we may have started burrowed (no hint node in that case).
			GrabHintNode( NULL );
		}

		return;
	}

	CAI_BaseNPC::HandleAnimEvent( pEvent );
}


//-----------------------------------------------------------------------------
// Purpose: Does all the fixup for going to/from the burrowed state.
//-----------------------------------------------------------------------------
void CBaseCompsognathus::SetBurrowed( bool bBurrowed )
{
	if ( bBurrowed )
	{
		AddEffects( EF_NODRAW );
		AddFlag( FL_NOTARGET );
		m_spawnflags |= SF_NPC_GAG;
		AddSolidFlags( FSOLID_NOT_SOLID );
		m_takedamage = DAMAGE_NO;
		m_flFieldOfView = COMPY_BURROWED_FOV;

		SetState( NPC_STATE_IDLE );
		SetActivity( (Activity) ACT_COMPY_BURROW_IDLE );
	}
	else
	{
		RemoveEffects( EF_NODRAW );
		RemoveFlag( FL_NOTARGET );
		m_spawnflags &= ~SF_NPC_GAG;
		RemoveSolidFlags( FSOLID_NOT_SOLID );
		m_takedamage = DAMAGE_YES;
		m_flFieldOfView	= COMPY_UNBURROWED_FOV;
	}

	m_bBurrowed = bBurrowed;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::RunTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
		case TASK_COMPY_CLIMB_FROM_CANISTER:
			AutoMovement( );
			if ( IsActivityFinished() )
			{
				TaskComplete();
			}
			break;

		case TASK_COMPY_JUMP_FROM_CANISTER:
			GetMotor()->UpdateYaw();
			if ( FacingIdeal() )
			{
				TaskComplete();
			}
			break;
/*
		case TASK_COMPY_WAIT_FOR_BARNACLE_KILL:
			if ( m_flNextFlinchTime < gpGlobals->curtime )
			{
				m_flNextFlinchTime = gpGlobals->curtime + random->RandomFloat( 1.0f, 2.0f );
				CTakeDamageInfo info;
				PainSound( info );
			}
			break;
*/
		case TASK_COMPY_HOP_OFF_NPC:
			if( GetFlags() & FL_ONGROUND )
			{
				TaskComplete();
			}
			else
			{
				// Face the direction I've been forced to jump.
				GetMotor()->SetIdealYawToTargetAndUpdate( GetAbsOrigin() + GetAbsVelocity() );
			}
			//break;

/*		case TASK_COMPY_SWIM:
			if( gpGlobals->curtime > m_flTimeDrown )
			{
				OnTakeDamage( CTakeDamageInfo( this, this, m_iHealth * 2, DMG_DROWN ) );
			}
			//break;
*/
		case TASK_COMPY_JUMP_BACK:
/*		case TASK_COMPY_UNSTICK_JUMP:
			if( GetFlags() & FL_ONGROUND )
			{
			
				TaskComplete();
			}
			break;
*/
		case TASK_RANGE_ATTACK1:
		case TASK_RANGE_ATTACK2:
		case TASK_MELEE_ATTACK1: 
		case TASK_COMPY_HARASS_HOP:
		{		
			if ( IsActivityFinished() )
			{
				TaskComplete();
				m_bMidJump = false;
				SetTouch( NULL );
				SetThink( &CBaseCompsognathus::CallNPCThink );
				SetIdealActivity( ACT_IDLE );

				if ( m_bAttackFailed )
				{
					// our attack failed because we just ran into something solid.
					// delay attacking for a while so we don't just repeatedly leap
					// at the enemy from a bad location.
					m_bAttackFailed = false;
					m_flNextAttack = gpGlobals->curtime + 1.2f;
				}
			}
			break;
		}

		case TASK_COMPY_CHECK_FOR_UNBURROW:
		{
			// Must wait for our next check time
			if ( m_flBurrowTime > gpGlobals->curtime )
				return;

			// See if we can pop up
			if ( ValidBurrowPoint( GetAbsOrigin() ) )
			{
				m_spawnflags &= ~SF_NPC_GAG;
				RemoveSolidFlags( FSOLID_NOT_SOLID );

				TaskComplete();
				return;
			}

			// Try again in a couple of seconds
			m_flBurrowTime = gpGlobals->curtime + random->RandomFloat( 0.5f, 1.0f );
			break;
		}
		case TASK_COMPY_BURROW_WAIT:
		{	
			if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) || HasCondition( COND_CAN_RANGE_ATTACK2 ) )
			{
				TaskComplete();
			}
			break;
		}			
		case TASK_COMPY_CEILING_WAIT:
			{	
#ifdef HL2_EPISODIC
				if ( DarknessLightSourceWithinRadius( this, DARKNESS_LIGHTSOURCE_SIZE ) )
				{
					DropFromCeiling();
				}
#endif

				if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) || HasCondition( COND_CAN_RANGE_ATTACK2 ) )
				{
					TaskComplete();
				}
				break;
			}
			
		case TASK_COMPY_CEILING_DETACH:
			{
				if ( IsActivityFinished() )
				{
					ClearCondition( COND_CAN_RANGE_ATTACK1 );
					RemoveFlag(FL_FLY);
					TaskComplete();
				}
			}
			break;

		case TASK_COMPY_CEILING_FALL:
			{
				Vector vecPrPos;
				trace_t tr;

				//Figure out where the headcrab is going to be in quarter of a second.
				vecPrPos = GetAbsOrigin() + ( GetAbsVelocity() * 0.25f );
				UTIL_TraceHull( vecPrPos, vecPrPos, GetHullMins(), GetHullMaxs(), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );
				
				if ( tr.startsolid == true || GetFlags() & FL_ONGROUND )
				{
					RemoveSolidFlags( FSOLID_NOT_SOLID );
					TaskComplete();
				}
			}
			break;

		case TASK_COMPY_CEILING_LAND:
			{
				if ( IsActivityFinished() )
				{
					RemoveSolidFlags( FSOLID_NOT_SOLID ); //double-dog verify that we're solid.
					TaskComplete();
					m_bHangingFromCeiling = false;
				}
			}
			break;
//from Alyx fall to ground 's code
/*		case TASK_COMPY_FALL_TO_GROUND:
			if ( GetFlags() & FL_ONGROUND )
			{
				TaskComplete();
			}
			else if( IsWaitFinished() )
			{
				// Call back to the base class & see if it can find a ground for us
				// If it can't, we'll fall to our death
				ChainRunTask( TASK_FALL_TO_GROUND );
				if ( TaskIsRunning() )
				{
					CTakeDamageInfo info;
					info.SetDamage( m_iHealth );
					info.SetAttacker( this );
					info.SetInflictor( this );
					info.SetDamageType( DMG_GENERIC );
					TakeDamage( info );
				}
			}
		break;
*/
		default:
		{
			BaseClass::RunTask( pTask );
		}
	}
}

//-----------------------------------------------------------------------------
// Before jumping, headcrabs usually use SetOrigin() to lift themselves off the 
// ground. If the headcrab doesn't have the clearance to so, they'll be stuck
// in the world. So this function makes sure there's headroom first.
//-----------------------------------------------------------------------------
bool CBaseCompsognathus::HasHeadroom()
{
	trace_t tr;
	UTIL_TraceEntity( this, GetAbsOrigin(), GetAbsOrigin() + Vector( 0, 0, 1 ), MASK_NPCSOLID, this, GetCollisionGroup(), &tr );

#if 0
	if( tr.fraction == 1.0f )
	{
		Msg("Headroom\n");
	}
	else
	{
		Msg("NO Headroom\n");
	}
#endif

	return (tr.fraction == 1.0);
}


//-----------------------------------------------------------------------------
// Purpose: LeapTouch - this is the headcrab's touch function when it is in the air.
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::LeapTouch( CBaseEntity *pOther )
{
	m_bMidJump = false;

	if ( IRelationType( pOther ) == D_HT )
	{
		// Don't hit if back on ground
		if ( !( GetFlags() & FL_ONGROUND ) )
		{
	 		if ( pOther->m_takedamage != DAMAGE_NO )
			{
				BiteSound();
				TouchDamage( pOther );

				// attack succeeded, so don't delay our next attack if we previously thought we failed
				m_bAttackFailed = true; //false; red test
			}
			else
			{
				ImpactSound();
			}
		}
		else
		{
			ImpactSound();
		}
	}
	else if( !(GetFlags() & FL_ONGROUND) )
	{
		// Still in the air...
		if( !pOther->IsSolid() )
		{
			// Touching a trigger or something.
			return;
		}

		// just ran into something solid, so the attack probably failed.  make a note of it
		// so that when the attack is done, we'll delay attacking for a while so we don't
		// just repeatedly leap at the enemy from a bad location.
		m_bAttackFailed = true;

		if( gpGlobals->curtime < m_flIgnoreWorldCollisionTime )
		{
			// Headcrabs try to ignore the world, static props, and friends for a 
			// fraction of a second after they jump. This is because they often brush
			// doorframes or props as they leap, and touching those objects turns off
			// this touch function, which can cause them to hit the player and not bite.
			// A timer probably isn't the best way to fix this, but it's one of our 
			// safer options at this point (sjb).
			return;
		}
	}

	// Shut off the touch function.
	SetTouch( NULL );
	SetThink ( &CBaseCompsognathus::CallNPCThink );
}



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CBaseCompsognathus::CalcDamageInfo( CTakeDamageInfo *pInfo )
{
	pInfo->Set( this, this, sk_compsognathus_melee_dmg.GetFloat(), DMG_SLASH );
	CalculateMeleeDamageForce( pInfo, GetAbsVelocity(), GetAbsOrigin() );
	return pInfo->GetDamage();
}

//-----------------------------------------------------------------------------
// Purpose: Deal the damage from the headcrab's touch attack.
//-----------------------------------------------------------------------------
void CBaseCompsognathus::TouchDamage( CBaseEntity *pOther )
{
	CTakeDamageInfo info;
	CalcDamageInfo( &info );
	pOther->TakeDamage( info  );
}


//---------------------------------------------------------
//---------------------------------------------------------
void CBaseCompsognathus::GatherConditions( void )
{
		// If we're hidden, just check to see if we should unhide
	if ( m_bHidden )
	{
		// See if there's enough room for our hull to fit here. If so, unhide.
		trace_t tr;
		AI_TraceHull( GetAbsOrigin(), GetAbsOrigin(),GetHullMins(), GetHullMaxs(), MASK_SHOT, this, GetCollisionGroup(), &tr );
		if ( tr.fraction == 1.0 )
		{
			SetCondition( COND_PROVOKED );
			SetCondition( COND_COMPY_UNHIDE );

			if ( g_debug_compsognathus.GetInt() == COMPY_DEBUG_HIDING )
			{
				NDebugOverlay::Box( GetAbsOrigin(), GetHullMins(), GetHullMaxs(), 0,255,0, true, 1.0 );
			}
		}
		else if ( g_debug_compsognathus.GetInt() == COMPY_DEBUG_HIDING )
		{
			NDebugOverlay::Box( GetAbsOrigin(), GetHullMins(), GetHullMaxs(), 255,0,0, true, 0.1 );
		}

		// Prevent baseclass thinking, so we don't respond to enemy fire, etc.
		return;
	}
	//BaseClass::GatherConditions();

/* disabled
	if( m_lifeState == LIFE_ALIVE && GetWaterLevel() > 1 )
	{
		// Start Drowning!
		SetCondition( COND_COMPY_IN_WATER );
	}	
*/
	// See if I've landed on an NPC or player or something else illegal
	ClearCondition( COND_COMPY_ILLEGAL_GROUNDENT );
	CBaseEntity *ground = GetGroundEntity();
	if( (GetFlags() & FL_ONGROUND) && ground && !ground->IsWorld() )
	{
		if ( IsHangingFromCeiling() == false )
		{
			if( ( ground->IsNPC() || ground->IsPlayer() ) )
			{
				SetCondition( COND_COMPY_ILLEGAL_GROUNDENT );
			}
			else if( ground->VPhysicsGetObject() && (ground->VPhysicsGetObject()->GetGameFlags() & FVPHYSICS_PLAYER_HELD) )
			{
				SetCondition( COND_COMPY_ILLEGAL_GROUNDENT );
			}
		}
	}
	BaseClass::GatherConditions();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::PrescheduleThink( void )
{
	//BaseClass::PrescheduleThink();
	UpdateHead();
	
	// Are we fading in after being hidden?
	if ( !m_bHidden && (m_nRenderMode != kRenderNormal) )
	{
		int iNewAlpha = min( 255, GetRenderColor().a + 120 );
		if ( iNewAlpha >= 255 )
		{
			m_nRenderMode = kRenderNormal;
			SetRenderColorA( 0 );
		}
		else
		{
			SetRenderColorA( iNewAlpha );
		}
	}

	//
	// Make the crab coo a little bit in combat state.
	//
	if (( m_NPCState == NPC_STATE_COMBAT ) && ( random->RandomFloat( 0, 5 ) < 0.1 ))
	{
		IdleSound();
	}

	// Make sure we've turned off our burrow state if we're not in it
	Activity eActivity = GetActivity();
	if ( m_bBurrowed &&
		 ( eActivity != ACT_COMPY_BURROW_IDLE ) &&
		 ( eActivity != ACT_COMPY_BURROW_OUT ) &&
		 ( eActivity != ACT_COMPY_BURROW_IN) )
	{
		DevMsg( "Compy failed to unburrow properly!\n" );
		Assert( 0 );
		SetBurrowed( false );
	}
	BaseClass::PrescheduleThink();
}


//-----------------------------------------------------------------------------
// Eliminates roll + pitch from the headcrab
//-----------------------------------------------------------------------------
#define COMPY_ROLL_ELIMINATION_TIME 0.3f
#define COMPY_PITCH_ELIMINATION_TIME 0.3f

//-----------------------------------------------------------------------------
// Eliminates roll + pitch potentially in the headcrab at canister jump time
//-----------------------------------------------------------------------------
void CBaseCompsognathus::EliminateRollAndPitch()
{
	QAngle angles = GetAbsAngles();
	angles.x = AngleNormalize( angles.x );
	angles.z = AngleNormalize( angles.z );
	if ( ( angles.x == 0.0f ) && ( angles.z == 0.0f ) )
		return;

	float flPitchRate = 90.0f / COMPY_PITCH_ELIMINATION_TIME;
	float flPitchDelta = flPitchRate * TICK_INTERVAL;
	if ( fabs( angles.x ) <= flPitchDelta )
	{
		angles.x = 0.0f;
	}
	else
	{
		flPitchDelta *= (angles.x > 0.0f) ? -1.0f : 1.0f;
		angles.x += flPitchDelta;
	}

	float flRollRate = 180.0f / COMPY_ROLL_ELIMINATION_TIME;
	float flRollDelta = flRollRate * TICK_INTERVAL;
	if ( fabs( angles.z ) <= flRollDelta )
	{
		angles.z = 0.0f;
	}
	else
	{
		flRollDelta *= (angles.z > 0.0f) ? -1.0f : 1.0f;
		angles.z += flRollDelta;
	}

	SetAbsAngles( angles );

	SetContextThink( &CBaseCompsognathus::EliminateRollAndPitch, gpGlobals->curtime + TICK_INTERVAL, s_pPitchContext );
}


//-----------------------------------------------------------------------------
// Begins the climb from the canister
//-----------------------------------------------------------------------------
void CBaseCompsognathus::BeginClimbFromCanister()
{
	Assert( GetMoveParent() );
	// Compute a desired position or hint
	Vector vecForward, vecActualForward;
	AngleVectors( GetMoveParent()->GetAbsAngles(), &vecActualForward );
	vecForward = vecActualForward;
	vecForward.z = 0.0f;
	VectorNormalize( vecForward );

	Vector vecSearchCenter = GetAbsOrigin();
	CAI_Hint *pHint = CAI_HintManager::FindHint( this, HINT_COMPY_BURROW_POINT, 0, COMPY_BURROW_POINT_SEARCH_RADIUS, &vecSearchCenter );

	if( !pHint && hl2_episodic.GetBool() )
	{
		// Look for exit points within 10 feet.
		pHint = CAI_HintManager::FindHint( this, HINT_COMPY_EXIT_POD_POINT, 0, 120.0f, &vecSearchCenter );
	}

	if ( pHint && ( !pHint->IsLocked() ) )
	{
		// Claim the hint node so other headcrabs don't try to take it!
		GrabHintNode( pHint );

		// Compute relative yaw..
		Vector vecDelta;
		VectorSubtract( pHint->GetAbsOrigin(), vecSearchCenter, vecDelta );
		vecDelta.z = 0.0f;
		VectorNormalize( vecDelta );

		float flAngle = DotProduct( vecDelta, vecForward );
		if ( flAngle >= 0.707f )
		{
			m_nJumpFromCanisterDir = 1;
		}
		else
		{
			// Check the cross product to see if it's on the left or right.
			// All we care about is the sign of the z component. If it's +, the hint is on the left.
			// If it's -, then the hint is on the right.
			float flCrossZ = vecForward.x * vecDelta.y - vecDelta.x * vecForward.y;
			m_nJumpFromCanisterDir = ( flCrossZ > 0 ) ? 0 : 2;
		}
	}
	else
	{
		// Choose a random direction (forward, left, or right)
		m_nJumpFromCanisterDir = random->RandomInt( 0, 2 );
	}

	Activity act;
	switch( m_nJumpFromCanisterDir )
	{
	case 0:	
		act = (Activity)ACT_COMPY_CRAWL_FROM_CANISTER_LEFT; 
		break;

	default:
	case 1:
		act = (Activity)ACT_COMPY_CRAWL_FROM_CANISTER_CENTER; 
		break;

	case 2:	
		act = (Activity)ACT_COMPY_CRAWL_FROM_CANISTER_RIGHT; 
		break;
	}

	SetIdealActivity( act );
}


//-----------------------------------------------------------------------------
// Jumps from the canister
//-----------------------------------------------------------------------------
#define COMPY_ATTACK_PLAYER_FROM_CANISTER_DIST 250.0f
#define COMPY_ATTACK_PLAYER_FROM_CANISTER_COSANGLE 0.866f

void CBaseCompsognathus::JumpFromCanister()
{
	Assert( GetMoveParent() );

	Vector vecForward, vecActualForward, vecActualRight;
	AngleVectors( GetMoveParent()->GetAbsAngles(), &vecActualForward, &vecActualRight, NULL );

	switch( m_nJumpFromCanisterDir )
	{
	case 0:
		VectorMultiply( vecActualRight, -1.0f, vecForward );
		break;
	case 1:
		vecForward = vecActualForward;
		break;
	case 2:
		vecForward = vecActualRight;
		break;
	}

	vecForward.z = 0.0f;
	VectorNormalize( vecForward );
	QAngle compsognathusAngles;
	VectorAngles( vecForward, compsognathusAngles );

	SetActivity( ACT_RANGE_ATTACK1 );
	StudioFrameAdvanceManual( 0.0 );
	SetParent( NULL );
	RemoveFlag( FL_FLY );
	AddEffects( EF_NOINTERP );

	GetMotor()->SetIdealYaw( compsognathusAngles.y );
	
	// Check to see if the player is within jump range. If so, jump at him!
	bool bJumpedAtEnemy = false;

	// FIXME: Can't use GetEnemy() here because enemy only updates during
	// schedules which are interruptible by COND_NEW_ENEMY or COND_LOST_ENEMY
	CBaseEntity *pEnemy = BestEnemy();
	if ( pEnemy )
	{
		Vector vecDirToEnemy;
		VectorSubtract( pEnemy->GetAbsOrigin(), GetAbsOrigin(), vecDirToEnemy );
		vecDirToEnemy.z = 0.0f;
		float flDist = VectorNormalize( vecDirToEnemy );
		if ( ( flDist < COMPY_ATTACK_PLAYER_FROM_CANISTER_DIST ) && 
			( DotProduct( vecDirToEnemy, vecForward ) >= COMPY_ATTACK_PLAYER_FROM_CANISTER_COSANGLE ) )
		{
			GrabHintNode( NULL );
			JumpAttack( false, pEnemy->EyePosition(), false );
			bJumpedAtEnemy = true;
		}
	}

	if ( !bJumpedAtEnemy )
	{
		if ( GetHintNode() )
		{
			JumpToBurrowHint( GetHintNode() );
		}
		else
		{
			vecForward *= 100.0f;
			vecForward += GetAbsOrigin();
			JumpAttack( false, vecForward, false );
		}
	}

	EliminateRollAndPitch();
}

#define COMPY_ILLUMINATED_TIME 0.15f

void CBaseCompsognathus::DropFromCeiling( void )
{
	if ( HL2GameRules()->IsAlyxInDarknessMode() )
	{
		if ( IsHangingFromCeiling() )
		{
			if ( m_flIlluminatedTime == -1 )
			{
				m_flIlluminatedTime = gpGlobals->curtime + COMPY_ILLUMINATED_TIME;
				return;
			}

			if ( m_flIlluminatedTime <= gpGlobals->curtime )
			{
				if ( IsCurSchedule( SCHED_COMPY_CEILING_DROP ) == false )
				{
					SetSchedule( SCHED_COMPY_CEILING_DROP );

					CBaseEntity *pPlayer = AI_GetSinglePlayer();

					if ( pPlayer )
					{
						SetEnemy( pPlayer ); //Is this a bad thing to do?
						UpdateEnemyMemory( pPlayer, pPlayer->GetAbsOrigin());
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Player has illuminated this NPC with the flashlight
//-----------------------------------------------------------------------------
void CBaseCompsognathus::PlayerHasIlluminatedNPC( CBasePlayer *pPlayer, float flDot )
{
	if ( flDot < 0.97387f )
		return;

	DropFromCeiling();
}

bool CBaseCompsognathus::CanBeAnEnemyOf( CBaseEntity *pEnemy )
{
#ifdef HL2_EPISODIC
	if ( IsHangingFromCeiling() )
		return false;
#endif

	return BaseClass::CanBeAnEnemyOf( pEnemy );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pTask - 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
/*	case TASK_COMPY_WAIT_FOR_BARNACLE_KILL:
		break;
*/
	case TASK_COMPY_BURROW_WAIT:
		break;

	case TASK_COMPY_CLIMB_FROM_CANISTER:
		BeginClimbFromCanister();
		break;

	case TASK_COMPY_JUMP_FROM_CANISTER:
		JumpFromCanister();
		break;

	case TASK_COMPY_CEILING_POSITION:
		{
			trace_t tr;
			UTIL_TraceHull( GetAbsOrigin(), GetAbsOrigin() + Vector( 0, 0, 512 ), NAI_Hull::Mins( GetHullType() ), NAI_Hull::Maxs( GetHullType() ), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

			// SetMoveType( MOVETYPE_NONE );
			AddFlag(FL_FLY);
			m_bHangingFromCeiling = true;

			//Don't need this anymore
			RemoveSpawnFlags( SF_COMPY_START_HANGING );

			SetAbsOrigin( tr.endpos );

			TaskComplete();
		}
		break;

	case TASK_COMPY_CEILING_WAIT:
		break;

	case TASK_COMPY_CEILING_DETACH:
		{
			SetIdealActivity( (Activity)ACT_COMPY_CEILING_DETACH );
		}
		break;

	case TASK_COMPY_CEILING_FALL:
		{
			SetIdealActivity( (Activity)ACT_COMPY_CEILING_FALL );
		}
		break;

	case TASK_COMPY_CEILING_LAND:
		{
			SetIdealActivity( (Activity)ACT_COMPY_CEILING_LAND );
		}
		break;

	case TASK_COMPY_HARASS_HOP:
		{
			// Just pop up into the air like you're trying to get at the
			// enemy, even though it's known you can't reach them.
			if ( GetEnemy() )
			{
				Vector forward, up;

				GetVectors( &forward, NULL, &up );

				m_vecCommittedJumpPos = GetAbsOrigin();
				m_vecCommittedJumpPos += up * random->RandomFloat( 80, 150 );
				m_vecCommittedJumpPos += forward * random->RandomFloat( 32, 80 );

				m_bCommittedToJump = true;

				SetIdealActivity( ACT_RANGE_ATTACK1 );
			}
			else
			{
				TaskFail( "No enemy" );
			}
		}
		break;

	case TASK_COMPY_HOP_OFF_NPC:
		{
			//if ( m_bMidJump ) // doesn't works! -red
			//	return;

			CBaseEntity *ground = GetGroundEntity();
			if( ground )
			{
				// If jumping off of a physics object that the player is holding, create a 
				// solver to prevent the headcrab from colliding with that object for a 
				// short time.
				if( ground && ground->VPhysicsGetObject() )
				{
					if( ground->VPhysicsGetObject()->GetGameFlags() & FVPHYSICS_PLAYER_HELD )
					{
						NPCPhysics_CreateSolver( this, ground, true, 0.5 );
					}
				}


				Vector vecJumpDir; 

				// Jump in some random direction. This way if the person I'm standing on is
				// against a wall, I'll eventually get away.
				
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
				
				if( HasHeadroom() )
				{
					// Bump up
					MoveOrigin( Vector( 0, 0, 1 ) );
				}
				
				SetAbsVelocity( vecJumpDir * 2 + Vector( 0, 0, 2 ) );
			}
			else
			{
				// *shrug* I guess they're gone now. Or dead.
				TaskComplete();
			}
		}
		//break;
/*	
		case TASK_COMPY_SWIM:
		{
			// Set the gravity really low here! Sink slowly
			//SetGravity( UTIL_ScaleForGravity( 80 ) ); // ?? IMPORTANT ?? default 80
			m_flTimeDrown = gpGlobals->curtime + 14;
			//SetIdealActivity( ACT_COMPY_DROWN ) ;
			SetIdealActivity( (Activity)ACT_COMPY_DROWN );
			break;
		}
*/
		case TASK_RANGE_ATTACK1:
		{
#ifdef WEDGE_FIX_THIS
			CPASAttenuationFilter filter( this, ATTN_IDLE );
			EmitSound( filter, entindex(), CHAN_WEAPON, pAttackSounds[0], GetSoundVolume(), ATTN_IDLE, 0, GetVoicePitch() );
#endif
			SetIdealActivity( ACT_RANGE_ATTACK1 );
			break;
		}

		case TASK_MELEE_ATTACK1:
		{
			SetIdealActivity( ACT_MELEE_ATTACK1 ) ;
			break;
		}

		case TASK_COMPY_UNHIDE:
		{
			m_bHidden = false;
			RemoveSolidFlags( FSOLID_NOT_SOLID );
			RemoveEffects( EF_NODRAW );

			TaskComplete();
			break;
		}

		case TASK_COMPY_CHECK_FOR_UNBURROW:
		{
			if ( ValidBurrowPoint( GetAbsOrigin() ) )
			{
				m_spawnflags &= ~SF_NPC_GAG;
				RemoveSolidFlags( FSOLID_NOT_SOLID );
				TaskComplete();
			}
			break;
		}

		case TASK_COMPY_FIND_BURROW_IN_POINT:
		{	
			if ( FindBurrow( GetAbsOrigin(), pTask->flTaskData, true ) == false )
			{
				TaskFail( "TASK_COMPY_FIND_BURROW_IN_POINT: Unable to find burrow in position\n" );
			}
			else
			{
				TaskComplete();
			}
			break;
		}

		case TASK_COMPY_BURROW:
		{
			Burrow();
			TaskComplete();
			break;
		}

		case TASK_COMPY_UNBURROW:
		{
			Unburrow();
			TaskComplete();
			break;
		}

		case TASK_COMPY_JUMP_BACK:
		{
			SetActivity( ACT_IDLE );

			SetGroundEntity( NULL );

			BeginAttackJump();

			Vector forward;
			AngleVectors( GetLocalAngles(), &forward );

			//
			// Take him off ground so engine doesn't instantly reset FL_ONGROUND.
			//
			UTIL_SetOrigin( this, GetLocalOrigin() + Vector( 0 , 0 , 1 ));

			ApplyAbsVelocityImpulse( forward * -200 + Vector( 0, 0, 200 ) );
		}
		break;
/*
		case TASK_COMPY_UNSTICK_JUMP:
		{
			SetGroundEntity( NULL );

			// Call begin attack jump. A little bit later if we fail to pathfind, we check
			// this value to see if we just jumped. If so, we assume we've jumped 
			// to someplace that's not pathing friendly, and so must jump again to get out.
			BeginAttackJump();

			//
			// Take him off ground so engine doesn't instantly reset FL_ONGROUND.
			//
			UTIL_SetOrigin( this, GetLocalOrigin() + Vector( 0 , 0 , 1 ));

			CBaseEntity *pEnemy = GetEnemy();
			Vector vecJumpDir;
*/

/*
			if ( GetActivity() == ACT_CLIMB_UP || GetActivity() == ACT_CLIMB_DOWN )
			{
				// Jump off the pipe backwards!
				Vector forward;

				GetVectors( &forward, NULL, NULL );

				ApplyAbsVelocityImpulse( forward * -200 );
			}
*/

/*
			if( pEnemy )
			{
				vecJumpDir = pEnemy->GetLocalOrigin() - GetLocalOrigin();
				VectorNormalize( vecJumpDir );
				vecJumpDir.z = 0;

				ApplyAbsVelocityImpulse( vecJumpDir * 300 + Vector( 0, 0, 200 ) );
			}
			else
			{
				DevMsg("UNHANDLED CASE! Stuck Fast Compy with no enemy!\n");
			}
		}
		break;
*/		
				
		case TASK_COMPY_LAND_RECOVER:
		{
			// Set the ideal yaw
			Vector flEnemyLKP = GetEnemyLKP();
			GetMotor()->SetIdealYawToTarget( flEnemyLKP );

			// figure out which way to turn.
			float flDeltaYaw = GetMotor()->DeltaIdealYaw();

			if( flDeltaYaw < 0 )
			{
				SetIdealActivity( (Activity)ACT_COMPY_LAND_RIGHT );
			}
			else
			{
				SetIdealActivity( (Activity)ACT_COMPY_LAND_LEFT );
			}


			TaskComplete();
		}
		break;

// From tha Alyx's  Fall to ground  code
/*		case TASK_COMPY_FALL_TO_GROUND:
			// If we wait this long without landing, we'll fall to our death
			SetWait(2);
			break;
*/
		default:
		{
			BaseClass::StartTask( pTask );
			//break; buggy ?
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: For innate melee attack
// Input  :
// Output :
//-----------------------------------------------------------------------------
float CBaseCompsognathus::InnateRange1MinRange( void )
{
	return COMPY_MIN_JUMP_DIST;
}

float CBaseCompsognathus::InnateRange1MaxRange( void )
{
	return COMPY_MAX_JUMP_DIST;
}

int CBaseCompsognathus::RangeAttack1Conditions( float flDot, float flDist )
{
	if ( gpGlobals->curtime < m_flNextAttack )
		return 0;

	if ( ( GetFlags() & FL_ONGROUND ) == false )
		return 0;

	// When we're burrowed ignore facing, because when we unburrow we'll cheat and face our enemy.
	if ( !m_bBurrowed && ( flDot < 0.65 ) )
		return COND_NOT_FACING_ATTACK;

	// !! IMPORTANT !!
	// This code stops lots of headcrabs swarming you and blocking you
	// whilst jumping up and down in your face over and over. It forces
	// them to back up a bit. If this causes problems, consider using it
	// for the fast headcrabs only, rather than just removing it.(sjb)
	if ( flDist < COMPY_MIN_JUMP_DIST )
		//return COND_TOO_CLOSE_TO_ATTACK;
		return COND_CAN_MELEE_ATTACK1;

	if ( flDist > COMPY_MAX_JUMP_DIST )
		return COND_TOO_FAR_TO_ATTACK;

	// Make sure the way is clear!
	CBaseEntity *pEnemy = GetEnemy();
	if( pEnemy )
	{
		bool bEnemyIsBullseye = ( dynamic_cast<CNPC_Bullseye *>(pEnemy) != NULL );

		trace_t tr;
		AI_TraceLine( EyePosition(), pEnemy->EyePosition(), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );
		//AI_TraceLine( BodyTarget( m_nBodyBone ), pEnemy->BodyTarget( m_nBodyBone ), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr ); //red


		if ( tr.m_pEnt != GetEnemy() )
		{
			if ( !bEnemyIsBullseye || tr.m_pEnt != NULL )
				return COND_NONE;
		}

		if( GetEnemy()->EyePosition().z - 36.0f > GetAbsOrigin().z ) // default: 36 .0f //72
		//if( GetEnemy()->BodyTarget( m_nBodyBone ).z /* - 36.0f */ > GetAbsOrigin().z ) // red
		{
			// Only run this test if trying to jump at a player who is higher up than me, else this 
			// code will always prevent a headcrab from jumping down at an enemy, and sometimes prevent it
			// jumping just slightly up at an enemy.
			Vector vStartHullTrace = GetAbsOrigin();
			vStartHullTrace.z += 1.0;

			Vector vEndHullTrace = GetEnemy()->EyePosition() - GetAbsOrigin();
			//Vector vEndHullTrace = GetEnemy()->BodyTarget( m_nBodyBone ) - GetAbsOrigin(); // red
			vEndHullTrace.NormalizeInPlace();
			vEndHullTrace *= 8.0;
			vEndHullTrace += GetAbsOrigin();

			AI_TraceHull( vStartHullTrace, vEndHullTrace,GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, GetCollisionGroup(), &tr );

			if ( tr.m_pEnt != NULL && tr.m_pEnt != GetEnemy() )
			{
				return COND_TOO_CLOSE_TO_ATTACK;
				//return COND_CAN_MELEE_ATTACK1;
			}
		}
	}

	return COND_CAN_RANGE_ATTACK1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCompsognathus::MeleeAttack1Conditions( float flDot, float flDist )
{
#if 1 //NOTENOTE: Use predicted position melee attacks

	//Get our likely position in one half second
	Vector vecPrPos;
	UTIL_PredictedPosition( GetEnemy(), 0.3f, &vecPrPos ); //0.5f default

	float flPrDist = ( vecPrPos - GetAbsOrigin() ).LengthSqr();
	if ( flPrDist > Square( COMPY_MELEE1_RANGE ) )
		return COND_TOO_FAR_TO_ATTACK;

	// Compare our target direction to our body facing
	Vector2D vec2DPrDir	= ( vecPrPos - GetAbsOrigin() ).AsVector2D();
	Vector2D vec2DBodyDir = BodyDirection2D().AsVector2D();
	
	float flPrDot = DotProduct2D ( vec2DPrDir, vec2DBodyDir );
	if ( flPrDot < 0.5f )
		return COND_NOT_FACING_ATTACK;

	trace_t	tr;
	AI_TraceHull( WorldSpaceCenter(), GetEnemy()->WorldSpaceCenter(), -Vector(8,8,8), Vector(8,8,8), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );

	// If the hit entity isn't our target and we don't hate it, don't hit it
	if ( tr.m_pEnt != GetEnemy() && tr.fraction < 1.0f && IRelationType( tr.m_pEnt ) != D_HT )
		return 0;

	// finish the range attack ? -red
	if ( IsCurSchedule( SCHED_RANGE_ATTACK1 ))
		return 0;

#else

	if ( flDot < 0.5f )
		return COND_NOT_FACING_ATTACK;

	float flAdjustedDist = COMPY_MELEE1_RANGE;

	if ( GetEnemy() )
	{
		// Give us extra space if our enemy is in a vehicle
		CBaseCombatCharacter *pCCEnemy = GetEnemy()->MyCombatCharacterPointer();
		if ( pCCEnemy != NULL && pCCEnemy->IsInAVehicle() )
		{
			flAdjustedDist *= 2.0f;
		}
	}

	if ( flDist > flAdjustedDist )
		return COND_TOO_FAR_TO_ATTACK;

	trace_t	tr;
	AI_TraceHull( WorldSpaceCenter(), GetEnemy()->WorldSpaceCenter(), -Vector(8,8,8), Vector(8,8,8), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

	if ( tr.fraction < 1.0f )
		return 0;

#endif

	if ( IsCurSchedule( SCHED_RANGE_ATTACK1 ))
		return 0;

	return COND_CAN_MELEE_ATTACK1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CBaseCompsognathus::MeleeAttack( float distance, float damage, QAngle &viewPunch, Vector &shove )
{
	Vector vecForceDir;

	// Always hurt bullseyes for now
	if ( ( GetEnemy() != NULL ) && ( GetEnemy()->Classify() == CLASS_BULLSEYE ) )
	{
		vecForceDir = (GetEnemy()->GetAbsOrigin() - GetAbsOrigin());
		CTakeDamageInfo info( this, this, damage, DMG_SLASH );
		CalculateMeleeDamageForce( &info, vecForceDir, GetEnemy()->GetAbsOrigin() );
		GetEnemy()->TakeDamage( info );
		return;
	}

	CBaseEntity *pHurt = CheckTraceHullAttack( distance, -Vector(16,16,32), Vector(16,16,32), damage, DMG_SLASH, 5.0f );

	if ( pHurt )
	{
		vecForceDir = ( pHurt->WorldSpaceCenter() - WorldSpaceCenter() );

		//FIXME: Until the interaction is setup, kill combine soldiers in one hit -- jdw
		if ( FClassnameIs( pHurt, "npc_combine_s" ) )
		{
			CTakeDamageInfo	dmgInfo( this, this, pHurt->m_iHealth+25, DMG_SLASH );
			CalculateMeleeDamageForce( &dmgInfo, vecForceDir, pHurt->GetAbsOrigin() );
			pHurt->TakeDamage( dmgInfo );
			return;
		}

		CBasePlayer *pPlayer = ToBasePlayer( pHurt );

		if ( pPlayer != NULL )
		{
			//Kick the player angles
			if ( !(pPlayer->GetFlags() & FL_GODMODE ) && pPlayer->GetMoveType() != MOVETYPE_NOCLIP )
			{
				pPlayer->ViewPunch( viewPunch );

				Vector	dir = pHurt->GetAbsOrigin() - GetAbsOrigin();
				VectorNormalize(dir);

				color32 red = {2, 0, 0, 255}; //{64, 0, 0, 255}
				UTIL_ScreenFade( pPlayer, red, 0.2, 0, FFADE_IN  );
				QAngle angles;
				VectorAngles( dir, angles );
				Vector forward, right;
				AngleVectors( angles, &forward, &right, NULL );

				//Push the target back
				pHurt->ApplyAbsVelocityImpulse( - right * shove[1] - forward * shove[0] );
			}
		}

		// disabled, ramdoming is not what we need, ae_event are preferred to fit the sequence animation. -red
		// Play a random attack hit sound
		EmitSound( "NPC_Compy.MeleeAttack" ); 
	}
}

// UPDATE COMPY HEAD
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::UpdateHead( void )
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
			SetPoseParameter( m_poseHead_Yaw,	UTIL_Approach( 0, yaw, 15 ) );
			SetPoseParameter( m_poseHead_Pitch, UTIL_Approach( 0, pitch, 15 ) );
			
			return;
		}

		float facingYaw = VecToYaw( BodyDirection3D() );
		float yawDiff = VecToYaw( enemyDir );
		yawDiff = UTIL_AngleDiff( yawDiff, facingYaw + yaw );

		float facingPitch = UTIL_VecToPitch( BodyDirection3D() );
		float pitchDiff = UTIL_VecToPitch( enemyDir );
		pitchDiff = UTIL_AngleDiff( pitchDiff, facingPitch + pitch );

		SetPoseParameter( m_poseHead_Yaw, UTIL_Approach( yaw + yawDiff, yaw, 30 ) );
		SetPoseParameter( m_poseHead_Pitch, UTIL_Approach( pitch + pitchDiff, pitch, 70 ) );
	}
	else
	{
		SetPoseParameter( m_poseHead_Yaw,	UTIL_Approach( 0, yaw, 10 ) );
		SetPoseParameter( m_poseHead_Pitch, UTIL_Approach( 0, pitch, 10 ) );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
inline CBaseEntity *CBaseCompsognathus::EntityToWatch( void )
{
	return ( m_hFollowTarget != NULL ) ? m_hFollowTarget.Get() : GetEnemy();
}
/*
//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseCompsognathus::ShouldWatchEnemy( void )
{
	Activity nActivity = GetActivity();

	//if ( ( nActivity == ACT_ANTLIONGUARD_SEARCH ) || 
	//	 ( nActivity == ACT_ANTLIONGUARD_PEEK_ENTER ) || 
	//	 ( nActivity == ACT_ANTLIONGUARD_PEEK_EXIT ) )
	//{
	//	return false;
	//}

	return true;
}
*/

//-----------------------------------------------------------------------------
// Purpose: Cache whatever pose parameters we intend to use
//-----------------------------------------------------------------------------
void CBaseCompsognathus::PopulatePoseParameters( void )
{
	m_poseHead_Pitch = LookupPoseParameter("head_pitch");
	m_poseHead_Yaw   = LookupPoseParameter("head_yaw" );

	BaseClass::PopulatePoseParameters();
}


/*
//------------------------------------------------------------------------------
// Purpose: Override to do headcrab specific gibs
// Output :
//------------------------------------------------------------------------------
bool CBaseCompsognathus::CorpseGib( const CTakeDamageInfo &info )
{
	EmitSound( "NPC_Compy.Gib" );	

	return BaseClass::CorpseGib( info );
}

*/
//------------------------------------------------------------------------------
// Purpose:
// Input  :
//------------------------------------------------------------------------------
void CBaseCompsognathus::Touch( CBaseEntity *pOther )
{ 
	// If someone has smacked me into a wall then gib!
	if (m_NPCState == NPC_STATE_DEAD) 
	{
		if (GetAbsVelocity().Length() > 250)
		{
			trace_t tr;
			Vector vecDir = GetAbsVelocity();
			VectorNormalize(vecDir);
			AI_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vecDir * 100, 
				MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr); 
			float dotPr = DotProduct(vecDir,tr.plane.normal);
			if ((tr.fraction						!= 1.0) && 
				(dotPr <  -0.8) )
			{
				CTakeDamageInfo	info( GetWorldEntity(), GetWorldEntity(), 100.0f, DMG_CRUSH );

				info.SetDamagePosition( tr.endpos );

				Event_Gibbed( info );
			}
		
		}
	}

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

				if ( ( ( pDrivableVehicle->m_nRPM > 75 ) && DotProduct( vecShoveDir, vecTargetDir ) <= 0 ) )
				{
					float flDamage = m_iHealth;

					if ( random->RandomInt( 0, 10 ) > 4 )
						 flDamage += 25;
									
					CTakeDamageInfo	dmgInfo( pVehicleEnt, pPlayer, flDamage, DMG_VEHICLE );
				
					CalculateMeleeDamageForce( &dmgInfo, vecShoveDir, pOther->GetAbsOrigin() );
					TakeDamage( dmgInfo );
				}
/*				else
				{
					// We're being shoved
					CTakeDamageInfo	dmgInfo( pVehicleEnt, pPlayer, 0, DMG_VEHICLE );
					PainSound( dmgInfo );

					SetCondition( COND_ANTLION_FLIPPED );

						vecTargetDir[2] = 0.0f;

						ApplyAbsVelocityImpulse( ( vecTargetDir * 250.0f ) + Vector(0,0,64.0f) );
						SetGroundEntity( NULL );

						CSoundEnt::InsertSound( SOUND_PHYSICS_DANGER, GetAbsOrigin(), 256, 0.5f, this );
					}
				}*/
			}
		}
	}

	BaseClass::Touch(pOther);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pevInflictor - 
//			pevAttacker - 
//			flDamage - 
//			bitsDamageType - 
// Output : 
//-----------------------------------------------------------------------------
int CBaseCompsognathus::OnTakeDamage_Alive( const CTakeDamageInfo &inputInfo )
{
	CTakeDamageInfo info = inputInfo;

	// add pain to the conditions
	if ( IsLightDamage( info ) )
	{
		SetCondition( COND_LIGHT_DAMAGE );
	}
	if ( IsHeavyDamage( info ) )
	{
		SetCondition( COND_HEAVY_DAMAGE );
	}

	//
	// Take acid damage.
	//
	if ( info.GetDamageType() & DMG_ACID )
	{
		info.SetDamage( 5 );
	}

	//
	// Certain death from melee bludgeon weapons!
	//
	if ( info.GetDamageType() & DMG_CLUB )
	{
		info.SetDamage( 5 );  //( m_iHealth );
	}


	if( info.GetDamageType() & DMG_BLAST )
	{
		if( random->RandomInt( 0 , 1 ) == 0 )
		{
			// Catch on fire randomly if damaged in a blast.
			Ignite( 15 ); //default : 30
		}
	}

	if( info.GetDamageType() & DMG_BURN )
	{
		// Slow down burn damage so that headcrabs live longer while on fire.
		info.ScaleDamage( 0.25 );

#define COMPY_SCORCH_RATE	5
#define COMPY_SCORCH_FLOOR	15 // 30

		if( IsOnFire() )
		{
			Scorch( COMPY_SCORCH_RATE, COMPY_SCORCH_FLOOR );

			if( m_iHealth <= 1 && (entindex() % 2) )
			{
				// Some headcrabs leap at you with their dying breath
				if( !IsCurSchedule( SCHED_COMPY_RANGE_ATTACK1 ) && !IsRunningDynamicInteraction() )
				{
					SetSchedule( SCHED_COMPY_RANGE_ATTACK1 );
				}
			}
		}

		Ignite( 15 ); // 30
	}

	//
	// Don't take any acid damage.
	//
	if ( info.GetDamageType() & DMG_ACID )
	{
		info.SetDamage( 3 );
	}

	if ( info.GetDamageType() & DMG_DROWN )
	{
		info.SetDamage( 3 );
	}
	return CAI_BaseNPC::OnTakeDamage_Alive( info );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CBaseCompsognathus::ClampRagdollForce( const Vector &vecForceIn, Vector *vecForceOut )
{
	// Assumes the compy mass is 18kg (100 feet per second)
	float MAX_COMPY_RAGDOLL_SPEED = 100.0f * 12.0f * 30.0f; //200.0f * 24.0f * 50.0f; // = 100.0f * 12.0f * 30.0f; ( * 18.0F; works good)

	Vector vecClampedForce; 
	BaseClass::ClampRagdollForce( vecForceIn, &vecClampedForce );

	// Copy the force to vecForceOut, in case we don't change it.
	*vecForceOut = vecClampedForce;

	float speed = VectorNormalize( vecClampedForce );
	if( speed > MAX_COMPY_RAGDOLL_SPEED )
	{
		// Don't let the ragdoll go as fast as it was going to.
		vecClampedForce *= MAX_COMPY_RAGDOLL_SPEED;
		*vecForceOut = vecClampedForce;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &force - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseCompsognathus::BecomeRagdollOnClient( const Vector &force )
{
	if ( !CanBecomeRagdoll() ) 
		return false;

	//EmitSound( "NPC_AntlionGuard.Fallover" );

	// Become server-side ragdoll if we're flagged to do it
	if ( m_spawnflags & SF_COMPY_SERVERSIDE_RAGDOLL )
	{
		CTakeDamageInfo	info;

		// Fake the info
		info.SetDamageType( DMG_GENERIC );
		info.SetDamageForce( force );
		info.SetDamagePosition( WorldSpaceCenter() );

		CBaseEntity *pRagdoll = CreateServerRagdoll( this, 0, info, COLLISION_GROUP_NONE );

		// Transfer our name to the new ragdoll
		pRagdoll->SetName( GetEntityName() );
		pRagdoll->SetCollisionGroup( COLLISION_GROUP_DEBRIS );
		
		// Get rid of our old body
		UTIL_Remove(this);

		return true;
	}

	return BaseClass::BecomeRagdollOnClient( force );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::InputRagdoll( inputdata_t &inputdata )
{
	if ( IsAlive() == false )
		return;

	//Set us to nearly dead so the velocity from death is minimal
	SetHealth( 1 );

	CTakeDamageInfo info( this, this, GetHealth(), DMG_CRUSH );
	BaseClass::TakeDamage( info );
}

//-----------------------------------------------------------------------------
// Purpose: Don't become a ragdoll until we've finished our death anim
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseCompsognathus::CanBecomeRagdoll( void )
{
	if ( IsCurSchedule( SCHED_DIE ) )
		return true;

	return hl2_episodic.GetBool();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CBaseCompsognathus::Event_Killed( const CTakeDamageInfo &info )
{
	// Create a little decal underneath the compy
	// This type of damage combination happens from dynamic scripted sequences
	if ( info.GetDamageType() & (DMG_GENERIC | DMG_PREVENT_PHYSICS_FORCE) )
	{
		trace_t	tr;
		AI_TraceLine( GetAbsOrigin()+Vector(0,0,1), GetAbsOrigin()-Vector(0,0,64), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

		UTIL_DecalTrace( &tr, "RedBlood" );
	}

	BaseClass::Event_Killed( info );

	CBaseEntity *pAttacker = info.GetInflictor();

	if ( pAttacker && pAttacker->GetServerVehicle() && ShouldGib( info ) == true )
	{
		trace_t tr;
		UTIL_TraceLine( GetAbsOrigin() + Vector( 0, 0, 64 ), pAttacker->GetAbsOrigin(), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );
		UTIL_DecalTrace( &tr, "Antlion.Splat" );

		SpawnBlood( GetAbsOrigin(), g_vecAttackDir, BloodColor(), info.GetDamage() );

		CPASAttenuationFilter filter( this );
		EmitSound( filter, entindex(), "NPC_Antlion.RunOverByVehicle" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseCompsognathus::ShouldGib( const CTakeDamageInfo &info )
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
bool CBaseCompsognathus::CorpseGib( const CTakeDamageInfo &info )
{
	// Use the bone position to handle being moved by an animation (like a dynamic scripted sequence)
	static int s_nBodyBone = -1;
	if ( s_nBodyBone == -1 )
	{
		s_nBodyBone = LookupBone( "Pelvis" );
	}

	Vector vecOrigin;
	QAngle angBone;
	GetBonePosition( s_nBodyBone, vecOrigin, angBone );

	DispatchParticleEffect( "AntlionGib", vecOrigin, QAngle( 0, 0, 0 ) );
	
	Vector velocity = vec3_origin;
	AngularImpulse	angVelocity = RandomAngularImpulse( -150, 150 );
	breakablepropparams_t params( EyePosition(), GetAbsAngles(), velocity, angVelocity );
	params.impactEnergyScale = 1.0f;
	params.defBurstScale = 150.0f;
	params.defCollisionGroup = COLLISION_GROUP_DEBRIS;
	PropBreakableCreateAll( GetModelIndex(), NULL, params, this, -1, true, true );

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: override this to simplify the physics shadow of the antlions
//-----------------------------------------------------------------------------
bool CBaseCompsognathus::CreateVPhysics()
{
	bool bRet = BaseClass::CreateVPhysics();
	return bRet;
}

// Use all the gibs
#define	NUM_COMPY_GIBS_UNIQUE	3
const char *pszCompyGibs_Unique[NUM_COMPY_GIBS_UNIQUE] = {
	"models/jl/dinosaurs/compyfast_gib_tail.mdl",
	"models/jl/dinosaurs/compyfast_gib_tail.mdl",
	"models/jl/dinosaurs/compyfast_gib_tail.mdl"
};


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : Type - 
//-----------------------------------------------------------------------------
int CBaseCompsognathus::TranslateSchedule( int scheduleType )
{
	switch( scheduleType )
	{
		case SCHED_FALL_TO_GROUND:
			return SCHED_COMPY_FALL_TO_GROUND;
			
			
		case SCHED_WAKE_ANGRY:
		{
			if ( HaveSequenceForActivity((Activity)ACT_COMPY_THREAT_DISPLAY) )
				return SCHED_COMPY_WAKE_ANGRY;
			else
				return SCHED_COMPY_WAKE_ANGRY_NO_DISPLAY;
		}
		
		case SCHED_RANGE_ATTACK1:
			return SCHED_COMPY_RANGE_ATTACK1;

		case SCHED_FAIL_TAKE_COVER:
			return SCHED_ALERT_FACE;


		//case SCHED_BACK_AWAY_FROM_SAVE_POSITION:
		//	return SCHED_COMPY_TAKE_SAFEPOSITION;
			

		case SCHED_CHASE_ENEMY_FAILED:
			{
				if( !GetEnemy() )
					break;

				if( !HasCondition( COND_SEE_ENEMY ) )
					break;

				float flZDiff;
				flZDiff = GetEnemy()->GetAbsOrigin().z - GetAbsOrigin().z;

				// Make sure the enemy isn't so high above me that this would look silly.
				if( flZDiff < 128.0f || flZDiff > 512.0f )
					return SCHED_COMBAT_PATROL;

				float flDist;
				flDist = ( GetEnemy()->GetAbsOrigin() - GetAbsOrigin() ).Length2D();

				// Maybe a patrol will bring me closer.
				if( flDist > 384.0f )
				{
					return SCHED_COMBAT_PATROL;
				}

				return SCHED_COMPY_HARASS_ENEMY;
			}
			break;
	}

	return BaseClass::TranslateSchedule( scheduleType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCompsognathus::SelectSchedule( void )
{
	if ( m_bCrawlFromCanister )
	{
		m_bCrawlFromCanister = false;
		return SCHED_COMPY_CRAWL_FROM_CANISTER;
	}

	// If we're hidden or waiting until seen, don't do much at all
	if ( m_bHidden || HasSpawnFlags(SF_NPC_WAIT_TILL_SEEN) )
	{
		if( HasCondition( COND_COMPY_UNHIDE ) )
		{
			// We've decided to unhide 
			return SCHED_COMPY_UNHIDE;
		}

		return m_bBurrowed ? SCHED_COMPY_BURROW_WAIT : SCHED_IDLE_STAND;
	}

	if ( GetSpawnFlags() & SF_COMPY_START_HANGING && IsHangingFromCeiling() == false )
	{
		return SCHED_COMPY_CEILING_WAIT;
	}

	if ( IsHangingFromCeiling() )
	{
		if ( HL2GameRules()->IsAlyxInDarknessMode() == false && ( HasCondition( COND_CAN_RANGE_ATTACK1 ) || HasCondition( COND_NEW_ENEMY ) ) )
			return SCHED_COMPY_CEILING_DROP;

		if ( HasCondition( COND_LIGHT_DAMAGE ) || HasCondition( COND_HEAVY_DAMAGE ) )
			return SCHED_COMPY_CEILING_DROP;

		return SCHED_COMPY_CEILING_WAIT;
	}

	if ( m_bBurrowed )
	{
		if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) )
			return SCHED_COMPY_BURROW_OUT;

		return SCHED_COMPY_BURROW_WAIT;
	}

	if ( GetHintNode() && GetHintNode()->HintType() == HINT_COMPY_BURROW_POINT )
	{
		// Only burrow if we're not within leap attack distance of our enemy.
		if ( ( GetEnemy() == NULL ) || ( ( GetEnemy()->GetAbsOrigin() - GetAbsOrigin() ).Length() > COMPY_MAX_JUMP_DIST ) )
		{
			return SCHED_COMPY_RUN_TO_SPECIFIC_BURROW;
		}
		else
		{
			// Forget about burrowing, we've got folks to leap at!
			GrabHintNode( NULL );
		}
	}
/*
	if( HasCondition( COND_COMPY_IN_WATER ) )
	{
		//SetGravity( UTIL_ScaleForGravity( 16.0 ) ); // 1600
		SetGroundEntity( NULL );
		SetNavType( NAV_FLY );
		SetActivity( (Activity) ACT_COMPY_DROWN );
		// seek for an hint node for a safe position
		return SCHED_COMPY_SWIM; 
	}
*/
	if( HasCondition( COND_COMPY_ILLEGAL_GROUNDENT ) )
	{
		// You're on an NPC's head. Get off.
		return SCHED_COMPY_HOP_RANDOMLY;
	}

/*	if ( HasCondition( COND_COMPY_BARNACLED ) )
	{
		// Caught by a barnacle!
		return SCHED_COMPY_BARNACLED;
	}
*/

//__________________//
//					//
// CLIMB CAPABILITY //
//__________________//
/*
	if ( HasCondition( COND_COMPY_CLIMB_TOUCH ) )
	{
		return SCHED_COMPY_UNSTICK_JUMP;
	}
*/// end //

	switch ( m_NPCState )
	{
	case NPC_STATE_ALERT:
		if (HasCondition( COND_LIGHT_DAMAGE ) || HasCondition( COND_HEAVY_DAMAGE ))
		{
			if ( fabs( GetMotor()->DeltaIdealYaw() ) < ( 1.0 - m_flFieldOfView) * 60 ) // roughly in the correct direction
			{
				return SCHED_TAKE_COVER_FROM_ORIGIN;
			}
			else if ( SelectWeightedSequence( ACT_SMALL_FLINCH ) != -1 )
			{
				m_flNextFlinchTime = gpGlobals->curtime + random->RandomFloat( 1, 3 );
				return SCHED_SMALL_FLINCH;
			}
		}
		else if (HasCondition( COND_HEAR_DANGER ) ||
				 HasCondition( COND_HEAR_PLAYER ) ||
				 HasCondition( COND_HEAR_WORLD  ) ||
				 HasCondition( COND_HEAR_COMBAT ))
		{
			return SCHED_ALERT_FACE_BESTSOUND;
		}
		else
		{
			return SCHED_PATROL_WALK;
		}
		break; 
	
	case NPC_STATE_COMBAT:
		// Melee attack if we can
		if ( HasCondition( COND_CAN_MELEE_ATTACK1 ) )
		{		
			return SCHED_MELEE_ATTACK1;
			//break;
		}
		break;
	}
	
	if (HasCondition( COND_LIGHT_DAMAGE ))
	{
		if ( fabs( GetMotor()->DeltaIdealYaw() ) < ( 1.0 - m_flFieldOfView) * 60 ) // roughly in the correct direction
		{
			return SCHED_TAKE_COVER_FROM_ORIGIN;
		}
		else if ( SelectWeightedSequence( ACT_SMALL_FLINCH ) != -1 )
		{
			m_flNextFlinchTime = gpGlobals->curtime + random->RandomFloat( 1, 3 );
			return SCHED_SMALL_FLINCH;
		}
	}

	if (HasCondition( COND_HEAVY_DAMAGE ))
	{
		if ( fabs( GetMotor()->DeltaIdealYaw() ) < ( 1.0 - m_flFieldOfView) * 60 ) // roughly in the correct direction
		{
			return SCHED_TAKE_COVER_FROM_ORIGIN;
		}
		else if ( SelectWeightedSequence( ACT_BIG_FLINCH ) != -1 )
		{
			m_flNextFlinchTime = gpGlobals->curtime + random->RandomFloat( 1, 3 );
			return SCHED_BIG_FLINCH;
		}
	}
	if ( HasCondition( COND_FLOATING_OFF_GROUND ) )
	{
		//SetGravity( 1.0 );
		//SetGroundEntity( NULL );
		return SCHED_FALL_TO_GROUND;
	}

	//return BaseClass::SelectSchedule();

	int nSchedule = BaseClass::SelectSchedule();
	if ( nSchedule == SCHED_SMALL_FLINCH )
	{
		 m_flNextFlinchTime = gpGlobals->curtime + random->RandomFloat( 1, 3 );
	}

	return nSchedule;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CBaseCompsognathus::SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode )
{
	if ( failedSchedule == SCHED_BACK_AWAY_FROM_ENEMY && failedTask == TASK_FIND_BACKAWAY_FROM_SAVEPOSITION )
	{
		if ( HasCondition( COND_SEE_ENEMY ) )
		{
			return SCHED_RANGE_ATTACK1;
		}
	}

	if ( failedSchedule == SCHED_BACK_AWAY_FROM_ENEMY || failedSchedule == SCHED_PATROL_WALK || failedSchedule == SCHED_COMBAT_PATROL )
	{
		if( !IsFirmlyOnGround() )
		{
			return SCHED_COMPY_HOP_RANDOMLY;
		}
	}
/*
	if ( m_fJustJumped )
	{
		// Assume we failed cause we jumped to a bad place.
		m_fJustJumped = false;
		return SCHED_COMPY_UNSTICK_JUMP;
	}
*/
	/*
	if ( failedSchedule == SCHED_COMPY_SWIM )
	{
		return SCHED_TAKE_COVER_FROM_ENEMY;
	}
	*/
	return BaseClass::SelectFailSchedule( failedSchedule, failedTask, taskFailCode );
}


//CLIMB AND JUMP CODE BEGIN HERE
/*
//-----------------------------------------------------------------------------
// Purpose: Lets us know if we touch the player while we're climbing.
//-----------------------------------------------------------------------------
void CBaseCompsognathus::ClimbTouch( CBaseEntity *pOther )
{
	if ( pOther->IsPlayer() )
	{
		// If I hit the player, shove him aside.
		Vector vecDir = pOther->WorldSpaceCenter() - WorldSpaceCenter();
		vecDir.z = 0.0; // planar
		VectorNormalize( vecDir );


		pOther->VelocityPunch( vecDir );

		if ( GetActivity() != ACT_CLIMB_DISMOUNT || 
			 ( pOther->GetGroundEntity() == NULL &&
			   GetNavigator()->IsGoalActive() &&
			   pOther->GetAbsOrigin().z - GetNavigator()->GetCurWaypointPos().z < -1.0 ) )
		{
			SetCondition( COND_COMPY_CLIMB_TOUCH );
		}

		SetTouch( NULL );
	}
	else if ( dynamic_cast<CPhysicsProp *>(pOther) )
	{
		NPCPhysics_CreateSolver( this, pOther, true, 5.0 );
	}
}
*/

//=========================================================
// Purpose: Do some record keeping for jumps made for 
//			navigational purposes (i.e., not attack jumps)
//=========================================================
void CBaseCompsognathus::BeginNavJump( void )
{
	m_fIsNavJumping = true;
	m_fHitApex = false;
}

//=========================================================
// 
//=========================================================
void CBaseCompsognathus::EndNavJump( void )
{
	m_fIsNavJumping = false;
	m_fHitApex = false;
}

//=========================================================
// 
//=========================================================
void CBaseCompsognathus::BeginAttackJump( void )
{
	// Set this to true. A little bit later if we fail to pathfind, we check
	// this value to see if we just jumped. If so, we assume we've jumped 
	// to someplace that's not pathing friendly, and so must jump again to get out.
	m_fJustJumped = true;

	m_flJumpStartAltitude = GetLocalOrigin().z;
}

//=========================================================
// 
//=========================================================
void CBaseCompsognathus::EndAttackJump( void )
{
}


//-----------------------------------------------------------------------------
// Purpose: Returns true if a reasonable jumping distance
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CBaseCompsognathus::IsJumpLegal(const Vector &startPos, const Vector &apex, const Vector &endPos) const
{
	const float MAX_JUMP_RISE		= 100.0f; //= 45.0f; //220.0f;
	const float MAX_JUMP_DISTANCE	= 180.0f; //= 60.0f;//= 180.0f; //512.0f;
	const float MAX_JUMP_DROP		= 150.0f; //= 52.0f;//= 150.0f; //384.0f;

	if ( BaseClass::IsJumpLegal( startPos, apex, endPos, MAX_JUMP_RISE, MAX_JUMP_DROP, MAX_JUMP_DISTANCE ) )
	{
		// Hang onto the jump distance. The AI is going to want it.
		m_flJumpDist = (startPos - endPos).Length();

		return true;
	}
	return false;
}

bool CBaseCompsognathus::MovementCost( int moveType, const Vector &vecStart, const Vector &vecEnd, float *pCost )
{
	float delta = vecEnd.z - vecStart.z;

	float multiplier = 1;
	if ( moveType == bits_CAP_MOVE_JUMP )
	{
		multiplier = ( delta < 0 ) ? 0.5 : 1.5;
	}
/*	else if ( moveType == bits_CAP_MOVE_CLIMB )
	{
		multiplier = ( delta > 0 ) ? 0.5 : 4.0;
	}
*/
	*pCost *= multiplier;

	return ( multiplier != 1 );
}

//-----------------------------------------------------------------------------
// MOVE jump
//-----------------------------------------------------------------------------

bool CBaseCompsognathus::ShouldFailNav( bool bMovementFailed )
{
	if ( !BaseClass::ShouldFailNav( bMovementFailed ) )
	{
		DevMsg( 2, "Fast Compy in scripted sequence probably hit bad node configuration at %s\n", VecToString( GetAbsOrigin() ) );
		
		if ( GetNavigator()->GetPath()->CurWaypointNavType() == NAV_JUMP && GetNavigator()->RefindPathToGoal( false ) )
		{
			return false;
		}
		DevMsg( 2, "Fast Compy failed to get to scripted sequence\n" );
	}

	return true;
}

//---------------------------------------------------------
// Purpose: Notifier that lets us know when the fast
//			zombie has hit the apex of a navigational jump.
//---------------------------------------------------------
void CBaseCompsognathus::OnNavJumpHitApex( void )
{
	m_fHitApex = true;	// stop subsequent notifications
}

//CLIMB AND JUMP CODE FINISH HERE

//-----------------------------------------------------------------------------
// Purpose: Allows for modification of the interrupt mask for the current schedule.
//			In the most cases the base implementation should be called first.
//-----------------------------------------------------------------------------
void CBaseCompsognathus::BuildScheduleTestBits( void )
{
/*
	if ( !IsCurSchedule(SCHED_COMPY_SWIM) )
	{
		// Interrupt any schedule unless already swimming.
		SetCustomInterruptCondition( COND_COMPY_IN_WATER );
	}
	else
	{
		// Don't stop Swimming just because you're in water!
		ClearCustomInterruptCondition( COND_COMPY_IN_WATER );
	}
*/
	if( !IsCurSchedule(SCHED_COMPY_HOP_RANDOMLY) )
	{
		SetCustomInterruptCondition( COND_COMPY_ILLEGAL_GROUNDENT );
	}
	else
	{
		ClearCustomInterruptCondition( COND_COMPY_ILLEGAL_GROUNDENT );
	}

	// Any schedule that makes us climb should break if we touch player
/*	if ( GetActivity() == ACT_CLIMB_UP || GetActivity() == ACT_CLIMB_DOWN || GetActivity() == ACT_CLIMB_DISMOUNT)
	{
		SetCustomInterruptCondition( COND_COMPY_CLIMB_TOUCH );
	}
	else
	{
		ClearCustomInterruptCondition( COND_COMPY_CLIMB_TOUCH );
	}
*/
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
//			&vecDir - 
//			*ptr - 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator )
{
	CTakeDamageInfo	newInfo = info;

	// Ignore if we're in a dynamic scripted sequence
	// Flashlight
	if ( info.GetDamageType() & DMG_CLUB && HITGROUP_HEAD )
	{
		SetCustomInterruptCondition( COND_HEAVY_DAMAGE );
	}
	else
	{
		SetCustomInterruptCondition( COND_LIGHT_DAMAGE );
	}

	// SPAS 12
	if ( info.GetDamageType() & DMG_BUCKSHOT && HITGROUP_CHEST )
	{
		SetCustomInterruptCondition( COND_HEAVY_DAMAGE );
	}
	else
	{
		SetCustomInterruptCondition( COND_LIGHT_DAMAGE );
	}

	// Taser
	if ( info.GetDamageType() & DMG_SHOCK && HITGROUP_HEAD )
	{
		SetCustomInterruptCondition( COND_HEAVY_DAMAGE );
	}
	else
	{
		SetCustomInterruptCondition( COND_LIGHT_DAMAGE );
	}
	
	// Machete
	if ( info.GetDamageType() & DMG_SLASH && HITGROUP_HEAD )
	{
		SetCustomInterruptCondition( COND_HEAVY_DAMAGE );
	}
	else
	{
		SetCustomInterruptCondition( COND_LIGHT_DAMAGE );
	}

	// Medusa
	if ( info.GetDamageType() & DMG_BULLET && HITGROUP_HEAD | HITGROUP_CHEST )
	{
		SetCustomInterruptCondition( COND_HEAVY_DAMAGE );
	}
	else
	{
		SetCustomInterruptCondition( COND_LIGHT_DAMAGE );
	}


	// Ignore if we're in a dynamic scripted sequence
	if ( info.GetDamageType() & DMG_PHYSGUN && !IsRunningDynamicInteraction() )
	{
		Vector	puntDir = ( info.GetDamageForce() * 1000.0f );

		newInfo.SetDamage( m_iMaxHealth / 3.0f );

		if( info.GetDamage() >= GetHealth() )
		{
			// This blow will be fatal, so scale the damage force
			// (it's a unit vector) so that the ragdoll will be 
			// affected.
			newInfo.SetDamageForce( info.GetDamageForce() * 3000.0f );
		}

		PainSound( newInfo );
		SetGroundEntity( NULL );
		ApplyAbsVelocityImpulse( puntDir );
	}

	BaseClass::TraceAttack( newInfo, vecDir, ptr, pAccumulator );
}

//---------------------------------------------------------
// Return the number by which to multiply incoming damage
// based on the hitgroup it hits. This exists mainly so
// that individual NPC's can have more or less resistance
// to damage done to certain hitgroups.
//---------------------------------------------------------
float CBaseCompsognathus::GetHitgroupDamageMultiplier( int iHitGroup, const CTakeDamageInfo &info )
{
	switch( iHitGroup )
	{
	case HITGROUP_GENERIC:
		return 1.0f;

	case HITGROUP_HEAD:
		return sk_compy_head.GetFloat();

	case HITGROUP_CHEST:
		return sk_compy_chest.GetFloat();

	case HITGROUP_STOMACH:
		return sk_compy_stomach.GetFloat();

	case HITGROUP_LEFTARM:
	case HITGROUP_RIGHTARM:
		return sk_compy_arm.GetFloat();

	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		return sk_compy_leg.GetFloat();

	default:
		return 1.0f;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CBaseCompsognathus::Ignite( float flFlameLifetime, bool bNPCOnly, float flSize, bool bCalledByLevelDesigner )
{
	// Can't start on fire if we're burrowed
	if ( m_bBurrowed )
		return;

	bool bWasOnFire = IsOnFire();

#ifdef HL2_EPISODIC
	if( GetHealth() > flFlameLifetime )
	{
		// Add some burn time to very healthy headcrabs to fix a bug where
		// black headcrabs would sometimes spontaneously extinguish (and survive)
		flFlameLifetime += 10.0f;
	}
#endif// HL2_EPISODIC

 	BaseClass::Ignite( flFlameLifetime, bNPCOnly, flSize, bCalledByLevelDesigner );

	if( !bWasOnFire )
	{
#ifdef HL2_EPISODIC
		if ( HL2GameRules()->IsAlyxInDarknessMode() == true )
		{
			GetEffectEntity()->AddEffects( EF_DIMLIGHT );
		}
#endif // HL2_EPISODIC

		// For the poison headcrab, who runs around when ignited
		SetActivity( TranslateActivity(GetIdealActivity()) );
	}
}


//-----------------------------------------------------------------------------
// Purpose:  This is a generic function (to be implemented by sub-classes) to
//			 handle specific interactions between different types of characters
//			 (For example the barnacle grabbing an NPC)
// Input  :  Constant for the type of interaction
// Output :	 true  - if sub-class has a response for the interaction
//			 false - if sub-class has no response
//-----------------------------------------------------------------------------
bool CBaseCompsognathus::HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt)
{
	if (interactionType == g_interactionBarnacleVictimDangle)
	{
		// Die instantly
		return false;
	}
	else if (interactionType ==	g_interactionVortigauntStomp)
	{
		SetIdealState( NPC_STATE_PRONE );
		return true;
	}
	else if (interactionType ==	g_interactionVortigauntStompFail)
	{
		SetIdealState( NPC_STATE_COMBAT );
		return true;
	}
	else if (interactionType ==	g_interactionVortigauntStompHit)
	{
		// Gib the existing guy, but only with legs and guts
		m_nGibCount = COMPY_LEGS_GIB_COUNT;
		OnTakeDamage ( CTakeDamageInfo( sourceEnt, sourceEnt, m_iHealth, DMG_CRUSH|DMG_ALWAYSGIB ) );
		
		// Create dead headcrab in its place
		CBaseCompsognathus *pEntity = (CBaseCompsognathus*) CreateEntityByName( "npc_compsognathus" );
		pEntity->Spawn();
		pEntity->SetLocalOrigin( GetLocalOrigin() );
		pEntity->SetLocalAngles( GetLocalAngles() );
		pEntity->m_NPCState = NPC_STATE_DEAD;
		return true;
	}
	else if (	(interactionType ==	g_interactionVortigauntKick)
				/* || (interactionType ==	g_interactionBullsquidThrow) */
				)
	{
		SetIdealState( NPC_STATE_PRONE );
		
		if( HasHeadroom() )
		{
			MoveOrigin( Vector( 0, 0, 1 ) );
		}

		Vector vHitDir = GetLocalOrigin() - sourceEnt->GetLocalOrigin();
		VectorNormalize(vHitDir);

		CTakeDamageInfo info( sourceEnt, sourceEnt, m_iHealth+1, DMG_CLUB );
		CalculateMeleeDamageForce( &info, vHitDir, GetAbsOrigin() );

		TakeDamage( info );

		return true;
	}

	return BaseClass::HandleInteraction( interactionType, data, sourceEnt );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCompsognathus::FValidateHintType( CAI_Hint *pHint )
{
	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::ClearBurrowPoint( const Vector &origin )
{
	CBaseEntity *pEntity = NULL;
	float		flDist;
	Vector		vecSpot, vecCenter, vecForce;

	//Cause a ruckus
	UTIL_ScreenShake( origin, 1.0f, 80.0f, 1.0f, 256.0f, SHAKE_START );

	//Iterate on all entities in the vicinity.
	for ( CEntitySphereQuery sphere( origin, 128 ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		if ( pEntity->m_takedamage != DAMAGE_NO && pEntity->Classify() != CLASS_PLAYER && pEntity->VPhysicsGetObject() )
		{
			vecSpot	 = pEntity->BodyTarget( origin );
			vecForce = ( vecSpot - origin ) + Vector( 0, 0, 16 );

			// decrease damage for an ent that's farther from the bomb.
			flDist = VectorNormalize( vecForce );

			//float mass = pEntity->VPhysicsGetObject()->GetMass();
			CollisionProp()->RandomPointInBounds( vec3_origin, Vector( 1.0f, 1.0f, 1.0f ), &vecCenter );

			if ( flDist <= 128.0f )
			{
				pEntity->VPhysicsGetObject()->Wake();
				pEntity->VPhysicsGetObject()->ApplyForceOffset( vecForce * 250.0f, vecCenter );
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Determine whether a point is valid or not for burrowing up into
// Input  : &point - point to test for validity
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseCompsognathus::ValidBurrowPoint( const Vector &point )
{
	trace_t	tr;

	AI_TraceHull( point, point+Vector(0,0,1), GetHullMins(), GetHullMaxs(), 
		MASK_NPCSOLID, this, GetCollisionGroup(), &tr );

	// See if we were able to get there
	if ( ( tr.startsolid ) || ( tr.allsolid ) || ( tr.fraction < 1.0f ) )
	{
		CBaseEntity *pEntity = tr.m_pEnt;

		//If it's a physics object, attempt to knock is away, unless it's a car
		if ( ( pEntity ) && ( pEntity->VPhysicsGetObject() ) && ( pEntity->GetServerVehicle() == NULL ) )
		{
			ClearBurrowPoint( point );
		}

		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::GrabHintNode( CAI_Hint *pHint )
{
	// Free up the node for use
	ClearHintNode();

	if ( pHint )
	{
		SetHintNode( pHint );
		pHint->Lock( this );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Finds a point where the headcrab can burrow underground.
// Input  : distance - radius to search for burrow spot in
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseCompsognathus::FindBurrow( const Vector &origin, float distance, bool excludeNear )
{
	// Attempt to find a burrowing point
	CHintCriteria	hintCriteria;

	hintCriteria.SetHintType( HINT_COMPY_BURROW_POINT );
	hintCriteria.SetFlag( bits_HINT_NODE_NEAREST );

	hintCriteria.AddIncludePosition( origin, distance );
	
	if ( excludeNear )
	{
		hintCriteria.AddExcludePosition( origin, 128 );
	}

	CAI_Hint *pHint = CAI_HintManager::FindHint( this, hintCriteria );

	if ( pHint == NULL )
		return false;

	GrabHintNode( pHint );

	// Setup our path and attempt to run there
	Vector vHintPos;
	pHint->GetPosition( this, &vHintPos );

	AI_NavGoal_t goal( vHintPos, ACT_RUN );

	return GetNavigator()->SetGoal( goal );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::Burrow( void )
{
	// Stop us from taking damage and being solid
	m_spawnflags |= SF_NPC_GAG;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::Unburrow( void )
{
	// Become solid again and visible
	m_spawnflags &= ~SF_NPC_GAG;
	RemoveSolidFlags( FSOLID_NOT_SOLID );
	m_takedamage = DAMAGE_YES;

	SetGroundEntity( NULL );

	// If we have an enemy, come out facing them
	if ( GetEnemy() )
	{
		Vector dir = GetEnemy()->GetAbsOrigin() - GetAbsOrigin();
		VectorNormalize(dir);

		GetMotor()->SetIdealYaw( dir );

		QAngle angles = GetLocalAngles();
		angles[YAW] = UTIL_VecToYaw( dir );
		SetLocalAngles( angles );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Tells the headcrab to unburrow as soon the space is clear.
//-----------------------------------------------------------------------------
void CBaseCompsognathus::InputUnburrow( inputdata_t &inputdata )
{
	if ( IsAlive() == false )
		return;

	SetSchedule( SCHED_COMPY_WAIT_FOR_CLEAR_UNBURROW );
}


//-----------------------------------------------------------------------------
// Purpose: Tells the headcrab to run to a nearby burrow point and burrow.
//-----------------------------------------------------------------------------
void CBaseCompsognathus::InputBurrow( inputdata_t &inputdata )
{
	if ( IsAlive() == false )
		return;

	SetSchedule( SCHED_COMPY_RUN_TO_BURROW_IN );
}


//-----------------------------------------------------------------------------
// Purpose: Tells the headcrab to burrow right where he is.
//-----------------------------------------------------------------------------
void CBaseCompsognathus::InputBurrowImmediate( inputdata_t &inputdata )
{
	if ( IsAlive() == false )
		return;

	SetSchedule( SCHED_COMPY_BURROW_IN );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::InputStartHangingFromCeiling( inputdata_t &inputdata )
{
	if ( IsAlive() == false )
		return;

	SetSchedule( SCHED_COMPY_CEILING_WAIT );
	m_flIlluminatedTime = -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::InputDropFromCeiling( inputdata_t &inputdata )
{
	if ( IsAlive() == false )
		return;

	if ( IsHangingFromCeiling() == false )
		return;

	SetSchedule( SCHED_COMPY_CEILING_DROP );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCompsognathus::CreateDust( bool placeDecal )
{
	trace_t	tr;
	AI_TraceLine( GetAbsOrigin()+Vector(0,0,1), GetAbsOrigin()-Vector(0,0,64), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

	if ( tr.fraction < 1.0f )
	{
		const surfacedata_t *pdata = physprops->GetSurfaceData( tr.surface.surfaceProps );

		if ( ( (char) pdata->game.material == CHAR_TEX_CONCRETE ) || ( (char) pdata->game.material == CHAR_TEX_DIRT ) )
		{
			UTIL_CreateAntlionDust( tr.endpos + Vector(0, 0, 24), GetLocalAngles() );

			//CEffectData data;
			//data.m_vOrigin = GetAbsOrigin();
			//data.m_vNormal = tr.plane.normal;
			//DispatchEffect( "headcrabdust", data );
			
			if ( placeDecal )
			{
				UTIL_DecalTrace( &tr, "compy.Unburrow" );
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCompsognathus::Precache( void )
{
	PrecacheModel( "models/jl/dinosaurs/compyclassic.mdl" );

	PrecacheScriptSound( "NPC_Compy.Gib" );
	PrecacheScriptSound( "NPC_Compy.Idle" );
	PrecacheScriptSound( "NPC_Compy.Alert" );
	PrecacheScriptSound( "NPC_Compy.Pain" );
	PrecacheScriptSound( "NPC_Compy.Die" );
	PrecacheScriptSound( "NPC_Compy.Attack" );
	PrecacheScriptSound( "NPC_Compy.Bite" );
	PrecacheScriptSound( "NPC_Compy.BurrowIn" );
	PrecacheScriptSound( "NPC_Compy.BurrowOut" );

	PrecacheScriptSound( "NPC_Compy.MeleeAttack" );
	PrecacheScriptSound( "NPC_Compy.Footstep" );
	
	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCompsognathus::Spawn( void )
{
	Precache();
	SetModel( "models/jl/dinosaurs/compyclassic.mdl" );

	BaseClass::Spawn();

	m_iHealth = sk_compsognathus_health.GetFloat();
	m_flBurrowTime = 0.0f;
	m_bCrawlFromCanister = false;
	m_bMidJump = false;

	NPCInit();
	CompsognathusInit();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Activity CCompsognathus::NPC_TranslateActivity( Activity eNewActivity )
{
//	if ( eNewActivity == ACT_WALK )
//		return ACT_RUN;

	return BaseClass::NPC_TranslateActivity( eNewActivity );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCompsognathus::IdleSound( void )
{
	EmitSound( "NPC_Compy.Idle" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCompsognathus::AlertSound( void )
{
	EmitSound( "NPC_Compy.Alert" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCompsognathus::PainSound( const CTakeDamageInfo &info )
{
	if( IsOnFire() && random->RandomInt( 0, COMPY_BURN_SOUND_FREQUENCY ) > 0 )
	{
		// Don't squeak every think when burning.
		return;
	}

	EmitSound( "NPC_Compy.Pain" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCompsognathus::DeathSound( const CTakeDamageInfo &info )
{
	EmitSound( "NPC_Compy.Die" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCompsognathus::TelegraphSound( void )
{
	//FIXME: Need a real one
	EmitSound( "NPC_Compy.Alert" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCompsognathus::AttackSound( void )
{
	EmitSound( "NPC_compy.Attack" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCompsognathus::BiteSound( void )
{
	EmitSound( "NPC_compy.Bite" );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  :
// Output : 
//-----------------------------------------------------------------------------
float CCompsognathus::MaxYawSpeed ( void )
{
	switch ( GetActivity() )
	{
	case ACT_IDLE:
	{
		return( 120 );
	}

	case ACT_WALK:
	{
		return( 115 );
	}
	case ACT_RUN:
	{
		return( 150 );
	}

	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		return 15;

	case ACT_RANGE_ATTACK1:
		{
			const Task_t *pCurTask = GetTask();
			if ( pCurTask && pCurTask->iTask == TASK_COMPY_JUMP_FROM_CANISTER )
				return 15;
		}
		return 30;

	default:
		return 30;
	}

	return BaseClass::MaxYawSpeed();
}



//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CFastCompsognathus )

	DEFINE_FIELD( m_iRunMode,			FIELD_INTEGER ),
	DEFINE_FIELD( m_flRealGroundSpeed,	FIELD_FLOAT ),
	DEFINE_FIELD( m_flSlowRunTime,		FIELD_TIME ),
	DEFINE_FIELD( m_flPauseTime,			FIELD_TIME ),
	DEFINE_FIELD( m_vecJumpVel,			FIELD_VECTOR ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFastCompsognathus::Precache( void )
{
	PrecacheModel( "models/jl/dinosaurs/compyfast.mdl" );
	
	PrecacheModel( "models/jl/dinosaurs/compyfast_gib_tail.mdl" );

	PrecacheParticleSystem( "blood_impact_antlion_01" );
	PrecacheParticleSystem( "AntlionGib" );

	PrecacheScriptSound( "NPC_FastCompy.Idle" );
	PrecacheScriptSound( "NPC_FastCompy.Alert" );
	PrecacheScriptSound( "NPC_FastCompy.Pain" );
	PrecacheScriptSound( "NPC_FastCompy.Die" );
	PrecacheScriptSound( "NPC_FastCompy.Bite" );
	PrecacheScriptSound( "NPC_FastCompy.Attack" );
	PrecacheScriptSound( "NPC_FastCompy.MeleeAttack" );
	PrecacheScriptSound( "NPC_Compy.Footstep" );
	
	PrecacheScriptSound( "NPC_Antlion.RunOverByVehicle" );

	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFastCompsognathus::Spawn( void )
{
	Precache();
	SetModel( "models/jl/dinosaurs/compyfast.mdl" );

	BaseClass::Spawn();

	m_iHealth = sk_compsognathus_fast_health.GetFloat();

	m_iRunMode = COMPY_RUNMODE_IDLE;
	m_flPauseTime = 999999;

	NPCInit();
	CompsognathusInit();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFastCompsognathus::IdleSound( void )
{
	EmitSound( "NPC_FastCompy.Idle" );
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFastCompsognathus::AlertSound( void )
{
	EmitSound( "NPC_FastCompy.Alert" );
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFastCompsognathus::PainSound( const CTakeDamageInfo &info )
{
	if( IsOnFire() && random->RandomInt( 0, COMPY_BURN_SOUND_FREQUENCY ) > 0 )
	{
		// Don't squeak every think when burning.
		return;
	}

	EmitSound( "NPC_FastCompy.Pain" );
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFastCompsognathus::DeathSound( const CTakeDamageInfo &info )
{
	EmitSound( "NPC_FastCompy.Die" );
}


//-----------------------------------------------------------------------------
// Purpose: Make the sound of this compy chomping a target.
// Input  : 
//-----------------------------------------------------------------------------
void CFastCompsognathus::BiteSound( void )
{
	EmitSound( "NPC_FastCompy.Bite" );
}

void CFastCompsognathus::AttackSound( void )
{
	EmitSound( "NPC_FastCompy.Attack" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFastCompsognathus::PrescheduleThink( void )
{
/*
#if 0 // #IF 0 this to stop the accelrating/decelerating movement.
#define COMPY_ACCELERATION 1.5 // 0.0
	if( IsAlive() && GetNavigator()->IsGoalActive() )
	{
		switch( m_iRunMode )
		{
		case COMPY_RUNMODE_IDLE:
			if ( GetActivity() == ACT_RUN ) //ACT_RUN
			{
				m_flRealGroundSpeed = m_flGroundSpeed;
				m_iRunMode = COMPY_RUNMODE_ACCELERATE;
				m_flPlaybackRate = COMPY_RUN_MINSPEED;
			}
			break;

		case COMPY_RUNMODE_FULLSPEED:
			if( gpGlobals->curtime > m_flSlowRunTime )
			{
				m_iRunMode = COMPY_RUNMODE_DECELERATE;
			}
			break;

		case COMPY_RUNMODE_ACCELERATE:
			if( m_flPlaybackRate < COMPY_RUN_MAXSPEED )
			{
				m_flPlaybackRate += COMPY_ACCELERATION;
			}

			if( m_flPlaybackRate >= COMPY_RUN_MAXSPEED )
			{
				m_flPlaybackRate = COMPY_RUN_MAXSPEED;
				m_iRunMode = COMPY_RUNMODE_FULLSPEED;

				m_flSlowRunTime = gpGlobals->curtime + random->RandomFloat( 0.1, 1.0 );
			}
			break;

		case COMPY_RUNMODE_DECELERATE:
			m_flPlaybackRate -= COMPY_ACCELERATION;

			if( m_flPlaybackRate <= COMPY_RUN_MINSPEED )
			{
				m_flPlaybackRate = COMPY_RUN_MINSPEED;

				// Now stop the crab.
			//	m_iRunMode = COMPY_RUNMODE_PAUSE;
			//	SetActivity( ACT_IDLE );
			//	GetNavigator()->SetMovementActivity(ACT_IDLE);
			//	m_flPauseTime = gpGlobals->curtime + random->RandomFloat( 0.2, 0.5 );
			//	m_flRealGroundSpeed = 0.0;
			}
			break;

		case COMPY_RUNMODE_PAUSE:
			{
				if( gpGlobals->curtime > m_flPauseTime )
				{
					m_iRunMode = COMPY_RUNMODE_IDLE;
					SetActivity( ACT_WALK ); // ACT_RUN
					GetNavigator()->SetMovementActivity(ACT_WALK); //ACT_RUN
					m_flPauseTime = gpGlobals->curtime - 1;
					m_flRealGroundSpeed = m_flGroundSpeed;
				}
			}
			break;

		default:
			Warning( "BIG TIME COMPY ERROR\n" );
			break;
		}

		m_flGroundSpeed = m_flRealGroundSpeed * m_flPlaybackRate;
	}
	else
	{
		m_flPauseTime = gpGlobals->curtime - 1;
	}


#endif
*/
	// Crudely detect the apex of our jump
	if( IsNavJumping() && !m_fHitApex && GetAbsVelocity().z <= 0.0 )
	{
		OnNavJumpHitApex();
	}

	BaseClass::PrescheduleThink();
}

/*
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Activity CFastCompsognathus::NPC_TranslateActivity( Activity baseAct )
{

	if ( baseAct == ACT_CLIMB_DOWN )
		return ACT_CLIMB_UP;

	return BaseClass::NPC_TranslateActivity( baseAct );
}
*/

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : scheduleType - 
//-----------------------------------------------------------------------------
int	CFastCompsognathus::SelectSchedule( void )
{
	if ( HasSpawnFlags(SF_NPC_WAIT_TILL_SEEN) )
	{
		return SCHED_IDLE_STAND;
	}

	if ( HasCondition(COND_CAN_RANGE_ATTACK1) && IsHangingFromCeiling() == false )
	{
		if ( OccupyStrategySlotRange( SQUAD_SLOT_ATTACK1, SQUAD_SLOT_ATTACK2 ) )
			return SCHED_RANGE_ATTACK1;
		ClearCondition(COND_CAN_RANGE_ATTACK1);
	}

	if ( HasCondition( COND_HEAVY_DAMAGE ) )
	{
		// Death waits for no man. Or zombie. Or something.
		return SCHED_BIG_FLINCH;
	}
	
	if ( HasCondition( COND_LIGHT_DAMAGE ) )
	{
		// Death waits for no man. Or zombie. Or something.
		return SCHED_SMALL_FLINCH;
	}

	if ( HasCondition( COND_LIGHT_DAMAGE ) && ( GetDamageType() & DMG_CLUB ) )
	{
		// Death waits for no man. Or zombie. Or something.
		return SCHED_FLINCH_PHYSICS;
	}

	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : scheduleType - 
//-----------------------------------------------------------------------------
int CFastCompsognathus::TranslateSchedule( int scheduleType )
{
	switch( scheduleType )
	{
	case SCHED_IDLE_STAND:
		return SCHED_PATROL_WALK;
		break;

	// Need to translate this ?
	//case SCHED_MELEE_ATTACK1:
	//	return SCHED_FAST_COMPY_MELEE_ATTACK1;
	//	break;

	case SCHED_FALL_TO_GROUND:
		return SCHED_COMPY_FALL_TO_GROUND;
		break;

	case SCHED_RANGE_ATTACK1:
		return SCHED_FAST_COMPY_RANGE_ATTACK1;
		break;

	case SCHED_CHASE_ENEMY:
		if ( !OccupyStrategySlotRange( SQUAD_SLOT_ENGAGE1, SQUAD_SLOT_ENGAGE4 ) )
			return SCHED_PATROL_WALK;
		break;
	}
	return BaseClass::TranslateSchedule( scheduleType );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CFastCompsognathus::RunTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_COMPY_HOP_ASIDE:
		if ( GetEnemy() )
			GetMotor()->SetIdealYawAndUpdate( GetEnemy()->GetAbsOrigin() - GetAbsOrigin(), AI_KEEP_YAW_SPEED );

		if( GetFlags() & FL_ONGROUND )
		{
			SetGravity(1.0); //10.0
			SetMoveType( MOVETYPE_STEP );

			if( GetEnemy() && ( GetEnemy()->GetAbsOrigin() - GetAbsOrigin() ).Length() > COMPY_MAX_JUMP_DIST )
			{
				TaskFail( "");
			}
			TaskComplete(); //
		}
		//break;

	case TASK_MELEE_ATTACK1:
	case TASK_RANGE_ATTACK1:
	case TASK_RANGE_ATTACK2:
		
		if ( GetEnemy() )
			// Fast headcrab faces the target in flight.
			GetMotor()->SetIdealYawAndUpdate( GetEnemy()->GetAbsOrigin() - GetAbsOrigin(), AI_KEEP_YAW_SPEED );
		
		
		// Call back up into base headcrab for collision.
		BaseClass::RunTask( pTask ); //buggy ?
		break; //buggy ?

	default:
		BaseClass::RunTask( pTask );
		//break; //buggy ?
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pTask - 
//-----------------------------------------------------------------------------
void CFastCompsognathus::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_COMPY_HOP_ASIDE:
		{
			Vector vecDir, vecForward, vecRight;
			bool fJumpIsLeft;
			trace_t tr;

			GetVectors( &vecForward, &vecRight, NULL );

			fJumpIsLeft = false;
			if( random->RandomInt( 0, 100 ) < 50 )
			{
				fJumpIsLeft = true;
				vecRight.Negate();
			}

			vecDir = ( vecRight + ( vecForward * 2 ) );
			VectorNormalize( vecDir );
			vecDir *= 150.0;

			// This could be a problem. Since I'm adjusting the headcrab's gravity for flight, this check actually
			// checks farther ahead than the crab will actually jump. (sjb)
			AI_TraceHull( GetAbsOrigin(), GetAbsOrigin() + vecDir,GetHullMins(), GetHullMaxs(), MASK_SHOT, this, GetCollisionGroup(), &tr );

			//NDebugOverlay::Line( tr.startpos, tr.endpos, 0, 255, 0, false, 1.0 );

			if( tr.fraction == 1.0 )
			{
				AIMoveTrace_t moveTrace;
				GetMoveProbe()->MoveLimit( NAV_JUMP, GetAbsOrigin(), tr.endpos, MASK_NPCSOLID, GetEnemy(), &moveTrace );

				// FIXME: Where should this happen?
				m_vecJumpVel = moveTrace.vJumpVelocity;

				if( !IsMoveBlocked( moveTrace ) )
				{
					SetAbsVelocity( m_vecJumpVel );// + 0.5f * Vector(0,0,sv_gravity.GetFloat()) * flInterval;
					SetGravity( UTIL_ScaleForGravity( 16.0 ) ); // 1600
					SetGroundEntity( NULL );
					SetNavType( NAV_JUMP );

					if( fJumpIsLeft )
					{
						SetIdealActivity( (Activity)ACT_COMPY_HOP_LEFT );
						GetNavigator()->SetMovementActivity( (Activity) ACT_COMPY_HOP_LEFT );
					}
					else
					{
						SetIdealActivity( (Activity)ACT_COMPY_HOP_RIGHT );
						GetNavigator()->SetMovementActivity( (Activity) ACT_COMPY_HOP_RIGHT );
					}
				}
				else
				{
					// Can't jump, just fall through.
					TaskComplete();
				}
			}
			else
			{
				// Can't jump, just fall through.
				TaskComplete();
			}
		break;
		}

	default:
		BaseClass::StartTask( pTask );		
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CFastCompsognathus::MaxYawSpeed( void )
{
	switch ( GetActivity() )
	{
		case ACT_IDLE:
		{
			return( 90 ); // 120
		}

		case ACT_RUN:
		{
			return( 120 ); // default 40
		}

		case ACT_WALK:
		{
			return( 60 ); // 30
		}

		case ACT_TURN_LEFT:
		case ACT_TURN_RIGHT:
		{
			return( 18 ); // 120
		}
		
		case ACT_90_LEFT:
		case ACT_90_RIGHT:
		{
			return 45;
		}
		
		case ACT_RANGE_ATTACK1:
		{
			return( 120 );
		}

		case ACT_JUMP:
		{
			return( 20 );
		}

		case ACT_GLIDE:
		{
			return( 20 );
		}

		default:
		{
			return( 55 );
		}
	}
}


bool CFastCompsognathus::QuerySeeEntity(CBaseEntity *pSightEnt, bool bOnlyHateOrFearIfNPC )
{
	if ( IsHangingFromCeiling() == true )
		return BaseClass::QuerySeeEntity(pSightEnt, bOnlyHateOrFearIfNPC);

	if( m_NPCState != NPC_STATE_COMBAT )
	{
		if( fabs( pSightEnt->GetAbsOrigin().z - GetAbsOrigin().z ) >= 150 )
		{
			// Don't see things much higher or lower than me unless
			// I'm already pissed.
			return false;
		}
	}

	return BaseClass::QuerySeeEntity(pSightEnt, bOnlyHateOrFearIfNPC);
}

//-----------------------------------------------------------------------------
// Black headcrab stuff
//-----------------------------------------------------------------------------
int ACT_BLACKCOMPY_RUN_PANIC;

BEGIN_DATADESC( CBlackCompsognathus )

	DEFINE_FIELD( m_bPanicState, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flPanicStopTime, FIELD_TIME ),
	DEFINE_FIELD( m_flNextHopTime, FIELD_TIME ),

	DEFINE_ENTITYFUNC( EjectTouch ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Make the sound of this headcrab chomping a target.
//-----------------------------------------------------------------------------
void CBlackCompsognathus::BiteSound( void )
{
	EmitSound( "NPC_BlackCompy.Bite" );
}


//-----------------------------------------------------------------------------
// Purpose: The sound we make when leaping at our enemy.
//-----------------------------------------------------------------------------
void CBlackCompsognathus::AttackSound( void )
{
	EmitSound( "NPC_BlackCompy.Attack" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBlackCompsognathus::TelegraphSound( void )
{
	EmitSound( "NPC_BlackCompy.Telegraph" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBlackCompsognathus::Spawn( void )
{
	Precache();
	SetModel( "models/jl/dinosaurs/blackcompy.mdl" );

	BaseClass::Spawn();

	m_bPanicState = false;
	m_iHealth = sk_compsognathus_poison_health.GetFloat();

	NPCInit();
	CompsognathusInit();
}



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBlackCompsognathus::Precache( void )
{
	PrecacheModel( "models/jl/dinosaurs/blackcompy.mdl" );

	PrecacheScriptSound( "NPC_BlackCompy.Telegraph" );
	PrecacheScriptSound( "NPC_BlackCompy.Attack" );
	PrecacheScriptSound( "NPC_BlackCompy.Bite" );
	PrecacheScriptSound( "NPC_BlackCompy.Threat" );
	PrecacheScriptSound( "NPC_BlackCompy.Alert" );
	PrecacheScriptSound( "NPC_BlackCompy.Idle" );
	PrecacheScriptSound( "NPC_BlackCompy.Talk" );
	PrecacheScriptSound( "NPC_BlackCompy.AlertVoice" );
	PrecacheScriptSound( "NPC_BlackCompy.Pain" );
	PrecacheScriptSound( "NPC_BlackCompy.Die" );
	PrecacheScriptSound( "NPC_BlackCompy.Impact" );
	PrecacheScriptSound( "NPC_BlackCompy.ImpactAngry" );

	PrecacheScriptSound( "NPC_BlackCompy.FootstepWalk" );
	PrecacheScriptSound( "NPC_BlackCompy.Footstep" );

	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose: Returns the max yaw speed for the current activity.
//-----------------------------------------------------------------------------
float CBlackCompsognathus::MaxYawSpeed( void )
{
	// Not a constant, can't be in a switch statement.
	if ( GetActivity() == ACT_BLACKCOMPY_RUN_PANIC )
	{
		return 30;
	}

	switch ( GetActivity() )
	{
		case ACT_WALK:
		case ACT_RUN:
		{
			return 10;
		}

		case ACT_TURN_LEFT:
		case ACT_TURN_RIGHT:
		{
			return( 30 );
		}

		case ACT_RANGE_ATTACK1:
		{
			return( 30 );
		}

		default:
		{
			return( 30 );
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Activity CBlackCompsognathus::NPC_TranslateActivity( Activity eNewActivity )
{
	if ( eNewActivity == ACT_RUN || eNewActivity == ACT_WALK )
	{
		if( m_bPanicState || IsOnFire() )
		{
			return ( Activity )ACT_BLACKCOMPY_RUN_PANIC;
		}
	}

	return BaseClass::NPC_TranslateActivity( eNewActivity );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBlackCompsognathus::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBlackCompsognathus::TranslateSchedule( int scheduleType )
{
	switch ( scheduleType )
	{
		// Keep trying to take cover for at least a few seconds.
		case SCHED_FAIL_TAKE_COVER:
		{
			if ( ( m_bPanicState ) && ( gpGlobals->curtime > m_flPanicStopTime ) )
			{
				//DevMsg( "I'm sick of panicking\n" );
				m_bPanicState = false;
				return SCHED_CHASE_ENEMY;
			}

			break;
		}
	}

	return BaseClass::TranslateSchedule( scheduleType );
}


//-----------------------------------------------------------------------------
// Purpose: Allows for modification of the interrupt mask for the current schedule.
//			In the most cases the base implementation should be called first.
//-----------------------------------------------------------------------------
void CBlackCompsognathus::BuildScheduleTestBits( void )
{
	// Ignore damage if we're attacking or are fleeing and recently flinched.
	if ( IsCurSchedule( SCHED_COMPY_CRAWL_FROM_CANISTER ) || IsCurSchedule( SCHED_RANGE_ATTACK1 ) || ( IsCurSchedule( SCHED_TAKE_COVER_FROM_ENEMY ) && HasMemory( bits_MEMORY_FLINCHED ) ) )
	{
		ClearCustomInterruptCondition( COND_LIGHT_DAMAGE );
		ClearCustomInterruptCondition( COND_HEAVY_DAMAGE );
	}
	else
	{
		SetCustomInterruptCondition( COND_LIGHT_DAMAGE );
		SetCustomInterruptCondition( COND_HEAVY_DAMAGE );
	}

	// If we're committed to jump, carry on even if our enemy hides behind a crate. Or a barrel.
	if ( IsCurSchedule( SCHED_RANGE_ATTACK1 ) && m_bCommittedToJump )
	{
		ClearCustomInterruptCondition( COND_ENEMY_OCCLUDED );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------
int CBlackCompsognathus::SelectSchedule( void )
{
	// don't override inherited behavior when hanging from ceiling
	if ( !IsHangingFromCeiling() )
	{
		if ( HasSpawnFlags(SF_NPC_WAIT_TILL_SEEN) )
		{
			return SCHED_IDLE_STAND;
		}

		if ( HasCondition( COND_LIGHT_DAMAGE ) || HasCondition( COND_HEAVY_DAMAGE ) )
		{
			if ( ( gpGlobals->curtime >= m_flNextHopTime ) && SelectWeightedSequence( ACT_SMALL_FLINCH ) != -1 )
			{
				m_flNextHopTime = gpGlobals->curtime + random->RandomFloat( 1, 3 );
				return SCHED_SMALL_FLINCH;
			}
		}

		if ( m_bPanicState )
		{
			// We're looking for a place to hide, and we've found one. Lurk!
			if ( HasMemory( bits_MEMORY_INCOVER ) )
			{
				m_bPanicState = false;
				m_flPanicStopTime = gpGlobals->curtime;

				return SCHED_COMPY_AMBUSH;
			}

			return SCHED_TAKE_COVER_FROM_ENEMY;
		}
	}

	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
// Purpose: Black headcrab's touch attack damage. Evil!
//-----------------------------------------------------------------------------
void CBlackCompsognathus::TouchDamage( CBaseEntity *pOther )
{
	if ( pOther->m_iHealth > 1 )
	{
		CTakeDamageInfo info;
		if ( CalcDamageInfo( &info ) >= pOther->m_iHealth )
			info.SetDamage( pOther->m_iHealth - 1 );

		pOther->TakeDamage( info  );

		if ( pOther->IsAlive() && pOther->m_iHealth > 1)
		{
			// Episodic change to avoid NPCs dying too quickly from poison bites
			if ( hl2_episodic.GetBool() )
			{
				if ( pOther->IsPlayer() )
				{
					// That didn't finish them. Take them down to one point with poison damage. It'll heal.
					pOther->TakeDamage( CTakeDamageInfo( this, this, pOther->m_iHealth - 1, DMG_POISON ) );
				}
				else
				{
					// Just take some amount of slash damage instead
					pOther->TakeDamage( CTakeDamageInfo( this, this, sk_compsognathus_poison_npc_damage.GetFloat(), DMG_SLASH ) );
				}
			}
			else
			{
				// That didn't finish them. Take them down to one point with poison damage. It'll heal.
				pOther->TakeDamage( CTakeDamageInfo( this, this, pOther->m_iHealth - 1, DMG_POISON ) );
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Bails out of our host zombie, either because he died or was blown
//			into two pieces by an explosion.
// Input  : vecAngles - The yaw direction we should face.
//			flVelocityScale - A multiplier for our ejection velocity.
//			pEnemy - Who we should acquire as our enemy. Usually our zombie host's enemy.
//-----------------------------------------------------------------------------
void CBlackCompsognathus::Eject( const QAngle &vecAngles, float flVelocityScale, CBaseEntity *pEnemy )
{
	SetGroundEntity( NULL );
	m_spawnflags |= SF_NPC_FALL_TO_GROUND;

	SetIdealState( NPC_STATE_ALERT );

	if ( pEnemy )
	{
		SetEnemy( pEnemy );
		UpdateEnemyMemory(pEnemy, pEnemy->GetAbsOrigin());
	}

	SetActivity( ACT_RANGE_ATTACK1 );

	SetNextThink( gpGlobals->curtime );
	PhysicsSimulate();

	GetMotor()->SetIdealYaw( vecAngles.y );

	SetAbsVelocity( flVelocityScale * random->RandomInt( 20, 50 ) * 
		Vector( random->RandomFloat( -1.0, 1.0 ), random->RandomFloat( -1.0, 1.0 ), random->RandomFloat( 0.5, 1.0 ) ) );

	m_bMidJump = false;
	SetTouch( &CBlackCompsognathus::EjectTouch );
}


//-----------------------------------------------------------------------------
// Purpose: Touch function for when we are ejected from the poison zombie.
//			Panic when we hit the ground.
//-----------------------------------------------------------------------------
void CBlackCompsognathus::EjectTouch( CBaseEntity *pOther )
{
	LeapTouch( pOther );
	if ( GetFlags() & FL_ONGROUND )
	{
		// Keep trying to take cover for at least a few seconds.
		Panic( random->RandomFloat( 2, 8 ) );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Puts us in a state in which we just want to hide. We'll stop
//			hiding after the given duration.
//-----------------------------------------------------------------------------
void CBlackCompsognathus::Panic( float flDuration )
{
	m_flPanicStopTime = gpGlobals->curtime + flDuration;
	m_bPanicState = true;
}


#if HL2_EPISODIC
//-----------------------------------------------------------------------------
// Purpose: Black headcrabs have 360-degree vision when they are in the ambush
//			schedule. This is because they ignore sounds when in ambush, and
//			you could walk up behind them without having them attack you.
//			This vision extends only 24 feet.
//-----------------------------------------------------------------------------
#define CRAB_360_VIEW_DIST_SQR	(12 * 12 * 24 * 24)
bool CBlackCompsognathus::FInViewCone( CBaseEntity *pEntity )
{
	if(  IsCurSchedule( SCHED_COMPY_AMBUSH ) &&
		 (( pEntity->IsNPC() || pEntity->IsPlayer() ) && pEntity->GetAbsOrigin().DistToSqr(GetAbsOrigin()) <= CRAB_360_VIEW_DIST_SQR ) )
	{
		// Only see players and NPC's with 360 cone
		// For instance, DON'T tell the eyeball/head tracking code that you can see an object that is behind you!
		return true;
	}
	else
	{
		return BaseClass::FInViewCone( pEntity );
	}
}
#endif


//-----------------------------------------------------------------------------
// Purpose: Does a spastic hop in a random or provided direction.
// Input  : pvecDir - 2D direction to hop, NULL picks a random direction.
//-----------------------------------------------------------------------------
void CBlackCompsognathus::JumpFlinch( const Vector *pvecDir )
{
	SetGroundEntity( NULL );

	//
	// Take him off ground so engine doesn't instantly reset FL_ONGROUND.
	//
	if( HasHeadroom() )
	{
		MoveOrigin( Vector( 0, 0, 1 ) );
	}

	//
	// Jump in a random direction.
	//
	Vector up;
	AngleVectors( GetLocalAngles(), NULL, NULL, &up );

	if (pvecDir)
	{
		SetAbsVelocity( Vector( pvecDir->x * 4, pvecDir->y * 4, up.z ) * random->RandomFloat( 40, 80 ) );
	}
	else
	{
		SetAbsVelocity( Vector( random->RandomFloat( -4, 4 ), random->RandomFloat( -4, 4 ), up.z ) * random->RandomFloat( 40, 80 ) );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Catches the monster-specific messages that occur when tagged
//			animation frames are played.
// Input  : pEvent - 
//-----------------------------------------------------------------------------
void CBlackCompsognathus::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_POISONCOMPY_FOOTSTEP )
	{
		bool walk = ( GetActivity() == ACT_WALK );   // ? 1.0 : 0.6; !!cgreen! old code had bug

		if ( walk )
		{
			EmitSound( "NPC_BlackCompy.FootstepWalk" );
		}
		else
		{
			EmitSound( "NPC_BlackCompy.Footstep" );
		}

		return;
	}

	if ( pEvent->event == AE_COMPY_JUMP_TELEGRAPH )
	{
		EmitSound( "NPC_BlackCompy.Telegraph" );

		CBaseEntity *pEnemy = GetEnemy();

		if ( pEnemy )
		{
			// Once we telegraph, we MUST jump. This is also when commit to what point
			// we jump at. Jump at our enemy's eyes.
			m_vecCommittedJumpPos = pEnemy->EyePosition();
			m_bCommittedToJump = true;
		}
 
		return;
	}

	if ( pEvent->event == AE_POISONCOMPY_THREAT_SOUND )
	{
		EmitSound( "NPC_BlackCompy.Threat" );
		EmitSound( "NPC_BlackCompy.Alert" );

		return;
	}

	if ( pEvent->event == AE_POISONCOMPY_FLINCH_HOP )
	{
		//
		// Hop in a random direction, then run and hide. If we're already running
		// to hide, jump forward -- hopefully that will take us closer to a hiding spot.
		//			
		if (m_bPanicState)
		{
			Vector vecForward;
			AngleVectors( GetLocalAngles(), &vecForward );
			JumpFlinch( &vecForward );
		}
		else
		{
			JumpFlinch( NULL );
		}

		Panic( random->RandomFloat( 2, 5 ) );

		return;
	}

		if ( pEvent->event == AE_COMPY_MELEE1_ANNOUNCE )
	{
		EmitSound( "NPC_FastCompy.MeleeAttack" );
		return;
	}

	BaseClass::HandleAnimEvent( pEvent );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CBlackCompsognathus::IsHeavyDamage( const CTakeDamageInfo &info )
{
	if ( !HasMemory(bits_MEMORY_FLINCHED) && info.GetDamage() > 1.0f )
	{
		// If I haven't flinched lately, any amount of damage is interpreted as heavy.
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBlackCompsognathus::IdleSound( void )
{
	// TODO: hook up "Marco" / "Polo" talking with nearby buddies
	if ( m_NPCState == NPC_STATE_IDLE )
	{
		EmitSound( "NPC_BlackCompy.Idle" );
	}
	else if ( m_NPCState == NPC_STATE_ALERT )
	{
		EmitSound( "NPC_BlackCompy.Talk" );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBlackCompsognathus::AlertSound( void )
{
	EmitSound( "NPC_BlackCompy.AlertVoice" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBlackCompsognathus::PainSound( const CTakeDamageInfo &info )
{
	if( IsOnFire() && random->RandomInt( 0, COMPY_BURN_SOUND_FREQUENCY ) > 0 )
	{
		// Don't squeak every think when burning.
		return;
	}

	EmitSound( "NPC_BlackCompy.Pain" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBlackCompsognathus::DeathSound( const CTakeDamageInfo &info )
{
	EmitSound( "NPC_BlackCompy.Die" );
}


//-----------------------------------------------------------------------------
// Purpose: Played when we jump and hit something that we can't bite.
//-----------------------------------------------------------------------------
void CBlackCompsognathus::ImpactSound( void )
{
	EmitSound( "NPC_BlackCompy.Impact" );

	if ( !( GetFlags() & FL_ONGROUND ) )
	{
		// Hit a wall - make a pissed off sound.
		EmitSound( "NPC_BlackCompy.ImpactAngry" );
	}
}



LINK_ENTITY_TO_CLASS( npc_compsognathus, CCompsognathus );
LINK_ENTITY_TO_CLASS( npc_compsognathus_fast, CFastCompsognathus );
LINK_ENTITY_TO_CLASS( npc_compsognathus_black, CBlackCompsognathus );
LINK_ENTITY_TO_CLASS( npc_Compsognathus_poison, CBlackCompsognathus );


//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_compsognathus, CBaseCompsognathus )

	DECLARE_TASK( TASK_COMPY_HOP_ASIDE )
	//DECLARE_TASK( TASK_COMPY_SWIM )
	DECLARE_TASK( TASK_COMPY_HOP_OFF_NPC )
	//DECLARE_TASK( TASK_COMPY_WAIT_FOR_BARNACLE_KILL )
	DECLARE_TASK( TASK_COMPY_UNHIDE )
	DECLARE_TASK( TASK_COMPY_HARASS_HOP )
	DECLARE_TASK( TASK_COMPY_BURROW )
	DECLARE_TASK( TASK_COMPY_UNBURROW )
	DECLARE_TASK( TASK_COMPY_FIND_BURROW_IN_POINT )
	DECLARE_TASK( TASK_COMPY_BURROW_WAIT )
	DECLARE_TASK( TASK_COMPY_CHECK_FOR_UNBURROW )
	DECLARE_TASK( TASK_COMPY_JUMP_FROM_CANISTER )
	DECLARE_TASK( TASK_COMPY_CLIMB_FROM_CANISTER )

	DECLARE_TASK( TASK_COMPY_CEILING_POSITION )
	DECLARE_TASK( TASK_COMPY_CEILING_WAIT )
	DECLARE_TASK( TASK_COMPY_CEILING_DETACH )
	DECLARE_TASK( TASK_COMPY_CEILING_FALL )
	DECLARE_TASK( TASK_COMPY_CEILING_LAND )

	//climb /jump
	//DECLARE_TASK( TASK_COMPY_TAKE_SAFEPOSITION )
	//DECLARE_TASK( TASK_COMPY_FALL_TO_GROUND )
	DECLARE_TASK( TASK_COMPY_JUMP_BACK )
	DECLARE_TASK( TASK_COMPY_LAND_RECOVER )

	DECLARE_ACTIVITY( ACT_COMPY_THREAT_DISPLAY )
	DECLARE_ACTIVITY( ACT_COMPY_HOP_LEFT )
	DECLARE_ACTIVITY( ACT_COMPY_HOP_RIGHT )
	//DECLARE_ACTIVITY( ACT_COMPY_DROWN )
	DECLARE_ACTIVITY( ACT_COMPY_BURROW_IN )
	DECLARE_ACTIVITY( ACT_COMPY_BURROW_OUT )
	DECLARE_ACTIVITY( ACT_COMPY_BURROW_IDLE )
	DECLARE_ACTIVITY( ACT_COMPY_CRAWL_FROM_CANISTER_LEFT )
	DECLARE_ACTIVITY( ACT_COMPY_CRAWL_FROM_CANISTER_CENTER )
	DECLARE_ACTIVITY( ACT_COMPY_CRAWL_FROM_CANISTER_RIGHT )
	DECLARE_ACTIVITY( ACT_COMPY_CEILING_FALL )

	DECLARE_ACTIVITY( ACT_COMPY_CEILING_IDLE )
	DECLARE_ACTIVITY( ACT_COMPY_CEILING_DETACH )
	DECLARE_ACTIVITY( ACT_COMPY_CEILING_LAND )

	//climb jump
	DECLARE_ACTIVITY( ACT_COMPY_THREAT_DISPLAY )
	DECLARE_ACTIVITY( ACT_COMPY_LAND_RIGHT )
	DECLARE_ACTIVITY( ACT_COMPY_LAND_LEFT )
	DECLARE_ACTIVITY( ACT_FALL_TO_GROUND )

//	DECLARE_ACTIVITY( ACT_CLIMB_DISMOUNT )
//	DECLARE_ACTIVITY( ACT_CLIMB_UP )
//	DECLARE_ACTIVITY( ACT_CLIMB_DOWN )
//	DECLARE_ACTIVITY( ACT_GLIDE )
//	DECLARE_ACTIVITY( ACT_JUMP )
//	DECLARE_ACTIVITY( ACT_LAND )
//	DECLARE_CONDITION( COND_COMPY_IN_WATER )
	DECLARE_CONDITION( COND_COMPY_ILLEGAL_GROUNDENT )
//	DECLARE_CONDITION( COND_COMPY_BARNACLED )
	DECLARE_CONDITION( COND_COMPY_UNHIDE )

	//climb jump
	//DECLARE_CONDITION( COND_COMPY_CLIMB_TOUCH )
	//Adrian: events go here
	DECLARE_ANIMEVENT( AE_COMPY_JUMPATTACK )
	DECLARE_ANIMEVENT( AE_COMPY_JUMP_TELEGRAPH )
	DECLARE_ANIMEVENT( AE_COMPY_BURROW_IN )
	DECLARE_ANIMEVENT( AE_COMPY_BURROW_IN_FINISH )
	DECLARE_ANIMEVENT( AE_COMPY_BURROW_OUT )
	DECLARE_ANIMEVENT( AE_COMPY_CEILING_DETACH )

	DECLARE_ANIMEVENT( AE_COMPY_FOOTSTEP )
	
	DECLARE_ANIMEVENT( AE_COMPY_MELEE1_HIT )
	DECLARE_ANIMEVENT( AE_COMPY_MELEE1_ANNOUNCE )
	
	//climb and jump
	//DECLARE_ANIMEVENT( AE_COMPY_CLIMB_LEFT )
	//DECLARE_ANIMEVENT( AE_COMPY_CLIMB_RIGHT )

	//=========================================================
	// > SCHED_COMPY_RANGE_ATTACK1
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_COMPY_RANGE_ATTACK1,

		"	Tasks"
		//"		TASK_STOP_MOVING			0"
		"		TASK_FACE_ENEMY				0"
		"		TASK_RANGE_ATTACK1			0"
		"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
		"		TASK_FACE_IDEAL				0"
		"		TASK_WAIT_RANDOM			0.5"
		""
		"	Interrupts"
		"		COND_ENEMY_OCCLUDED"
		"		COND_NO_PRIMARY_AMMO"
		"		COND_CAN_MELEE_ATTACK1"
	)


	//=========================================================
	//
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_COMPY_WAKE_ANGRY,

		"	Tasks"
		//"		TASK_STOP_MOVING				0"
		"		TASK_SET_ACTIVITY				ACTIVITY:ACT_IDLE "
		"		TASK_FACE_IDEAL					0"
		"		TASK_SOUND_WAKE					0"
		"		TASK_PLAY_SEQUENCE_FACE_ENEMY	ACTIVITY:ACT_COMPY_THREAT_DISPLAY"
		""
		"	Interrupts"
	)

	//=========================================================
	//
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_COMPY_WAKE_ANGRY_NO_DISPLAY,

		"	Tasks"
		//"		TASK_STOP_MOVING				0"
		"		TASK_SET_ACTIVITY				ACTIVITY:ACT_IDLE "
		"		TASK_FACE_IDEAL					0"
		"		TASK_SOUND_WAKE					0"
		"		TASK_FACE_ENEMY					0"
		""
		"	Interrupts"
	)

	//=========================================================
	// > SCHED_FAST_COMPY_RANGE_ATTACK1
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_FAST_COMPY_RANGE_ATTACK1,
		

		"	Tasks"
		"		TASK_STOP_MOVING			0"
		//"		TASK_FACE_ENEMY				0"
		"		TASK_ANNOUNCE_ATTACK		1"
		"		TASK_RANGE_ATTACK1			0"
		"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
		"		TASK_FACE_IDEAL				0"
		//"		TASK_WAIT_RANDOM			0.5"
		""
		"	Interrupts"
		"		COND_CAN_MELEE_ATTACK1"
	)

	//=========================================================
	// I have landed somewhere that's pathfinding-unfriendly
	// just try to jump out.
	//=========================================================
/*	DEFINE_SCHEDULE
	(
		SCHED_COMPY_UNSTICK_JUMP,

		"	Tasks"
		"		TASK_COMPY_UNSTICK_JUMP	0"
		""
		"	Interrupts"
	)
*/
	//=========================================================
	//=========================================================
/*	DEFINE_SCHEDULE
	(

		SCHED_COMPY_CLIMBING_UNSTICK_JUMP,

		"	Tasks"
		"		TASK_SET_ACTIVITY				ACTIVITY:ACT_IDLE"
		"		TASK_COMPY_UNSTICK_JUMP	0"
		""
		"	Interrupts"
	)
*/

	//=========================================================
	// > BackAwayFromEnemy
	//=========================================================
/*	DEFINE_SCHEDULE
	(
		SCHED_COMPY_TAKE_SAFEPOSITION,

		"	Tasks"
				// If I can't back away from the enemy try to get behind him
	//	"		TASK_STOP_MOVING							0"
		"		TASK_SET_TOLERANCE_DISTANCE					24"
		"		TASK_STORE_ENEMY_POSITION_IN_SAVEPOSITION	0"
		"		TASK_FIND_BACKAWAY_FROM_SAVEPOSITION		0"
		"		TASK_RUN_PATH								0"
		"		TASK_WAIT_FOR_MOVEMENT						0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_CAN_RANGE_ATTACK1"
		"		COND_CAN_RANGE_ATTACK2"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_CAN_MELEE_ATTACK2"
	)
*/

/*
	//=========================================================
	// The irreversible process of drowning
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_COMPY_SWIM,
		"	Tasks"
		//"		TASK_SET_FAIL_SCHEDULE	SCHEDULE:SCHED_COMPY_FAIL_SWIM"
		"		TASK_SET_ACTIVITY	ACTIVITY:ACT_COMPY_DROWN"
		"		TASK_COMPY_SWIM								0" // time value  ?
		"		TASK_SET_TOLERANCE_DISTANCE					128"
		"		TASK_SET_ROUTE_SEARCH_TIME					2"
		"		TASK_WAIT_FOR_MOVEMENT						0"
		//"		TASK_FIND_BACKAWAY_FROM_SAVEPOSITION		0"
		"		TASK_RUN_PATH								0"
		"	Interrupts"
		//no interruption conditions, the  process is irreversible
	)


//=========================================================
// FROM THE HEACRAB FAIL DROWN SCHEDULE
//=========================================================

	DEFINE_SCHEDULE
	(
		SCHED_COMPY_FAIL_SWIM,

		"	Tasks"
		"		TASK_STOP_MOVING							0"
		"		TASK_SET_ACTIVITY	ACTIVITY:ACT_COMPY_DROWN"
		"		TASK_WAIT_FOR_MOVEMENT						0"
		"		TASK_DIE									0"
		"	Interrupts"
	)

*/

/*
//=========================================================
// CUSTOM FAIL DROWN SCHEDULE
//=========================================================

	DEFINE_SCHEDULE
	(
		SCHED_COMPY_FAIL_DROWN,

		"	Tasks"
		//"		TASK_COMPY_DROWN			0"
		//"		TASK_FIND_BACKAWAY_FROM_SAVEPOSITION		0"
		"		TASK_WAIT_FACE_ENEMY			1"  // old value "1"
		"		TASK_SET_ROUTE_SEARCH_TIME		3"	// Spend 2 seconds trying to build a path if stuck
		"		TASK_GET_PATH_TO_RANDOM_NODE	300"
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_RUN_PATH								0"
		//"		TASK_WAIT_FOR_MOVEMENT						0"
		""
		"	Interrupts"
	)

*/



	//=========================================================
	// CUSTOM FALL_TO_GROUND SCHED
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_COMPY_FALL_TO_GROUND,

		"	Tasks"
		"		TASK_SET_ACTIVITY				ACTIVITY:ACT_FALL_TO_GROUND"
		"		TASK_FALL_TO_GROUND				0"
		""
		"	Interrupts"
	)


	//===============================================
	//===============================================
	DEFINE_SCHEDULE
	(
		SCHED_COMPY_AMBUSH,

		"	Tasks"
		//"		TASK_STOP_MOVING			0"
		"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
		"		TASK_WAIT_INDEFINITE		0"
		""
		"	Interrupts"
		"		COND_SEE_ENEMY"
		"		COND_SEE_HATE"
		"		COND_CAN_RANGE_ATTACK1"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_PROVOKED"
	)

	//=========================================================
	// Headcrab has landed atop another NPC or has landed on 
	// a ledge. Get down!
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_COMPY_HOP_RANDOMLY,

		"	Tasks"
		"		TASK_STOP_MOVING	0"
		"		TASK_SET_ACTIVITY			ACTIVITY:ACT_FALL_TO_GROUND"
		"		TASK_COMPY_HOP_OFF_NPC	0"
		"	Interrupts"
	)


	//=========================================================
	// Headcrab is in the clutches of a barnacle
	//=========================================================
/*	DEFINE_SCHEDULE
	(
		SCHED_COMPY_BARNACLED,

		"	Tasks"
		"		TASK_STOP_MOVING						0"
		"		TASK_SET_ACTIVITY						ACTIVITY:ACT_COMPY_DROWN"
		"		TASK_COMPY_WAIT_FOR_BARNACLE_KILL	0"
		""
		"	Interrupts"
	)
*/
	//=========================================================
	// Headcrab is unhiding
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_COMPY_UNHIDE,

		"	Tasks"
		"		TASK_COMPY_UNHIDE			0"
		""
		"	Interrupts"
	)

	DEFINE_SCHEDULE
	(
		SCHED_COMPY_HARASS_ENEMY,
		"	Tasks"
		"		TASK_FACE_ENEMY	0"
		"		TASK_COMPY_HARASS_HOP	0"
		"		TASK_WAIT_FACE_ENEMY	1"  // old value "1"
		//"		TASK_SET_ROUTE_SEARCH_TIME	0.1"	// Spend 2 seconds trying to build a path if stuck
		"		TASK_GET_PATH_TO_RANDOM_NODE	300"
		"		TASK_WALK_PATH	0"
		"		TASK_WAIT_FOR_MOVEMENT	0"
		"	Interrupts"
		"		COND_NEW_ENEMY"
	)

	DEFINE_SCHEDULE
	(
		SCHED_COMPY_CRAWL_FROM_CANISTER,
		"	Tasks"
		"		TASK_COMPY_CLIMB_FROM_CANISTER	0"
		"		TASK_COMPY_JUMP_FROM_CANISTER	0"
		""
		"	Interrupts"
	)

	//==================================================
	// Burrow In
	//==================================================
	DEFINE_SCHEDULE
	(
		SCHED_COMPY_BURROW_IN,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_CHASE_ENEMY_FAILED"
		"		TASK_COMPY_BURROW				0"
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_COMPY_BURROW_IN"
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_COMPY_BURROW_IDLE"
		"		TASK_SET_SCHEDULE					SCHEDULE:SCHED_COMPY_BURROW_WAIT"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
	)

	//==================================================
	// Run to a nearby burrow hint and burrow there
	//==================================================
	DEFINE_SCHEDULE
	(
		SCHED_COMPY_RUN_TO_BURROW_IN,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_CHASE_ENEMY_FAILED"
		"		TASK_COMPY_FIND_BURROW_IN_POINT	512"
		"		TASK_SET_TOLERANCE_DISTANCE			8"
		"		TASK_RUN_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		"		TASK_SET_SCHEDULE					SCHEDULE:SCHED_COMPY_BURROW_IN"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_GIVE_WAY"
		"		COND_CAN_RANGE_ATTACK1"
		"		COND_CAN_MELEE_ATTACK1"
	)

	//==================================================
	// Run to m_pHintNode and burrow there
	//==================================================
	DEFINE_SCHEDULE
	(
		SCHED_COMPY_RUN_TO_SPECIFIC_BURROW,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_CHASE_ENEMY_FAILED"
		"		TASK_SET_TOLERANCE_DISTANCE			8"
		"		TASK_GET_PATH_TO_HINTNODE			0"
		"		TASK_RUN_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		"		TASK_SET_SCHEDULE					SCHEDULE:SCHED_COMPY_BURROW_IN"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_GIVE_WAY"
	)

	//==================================================
	// Wait until we can unburrow and attack something
	//==================================================
	DEFINE_SCHEDULE
	(
		SCHED_COMPY_BURROW_WAIT,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMPY_BURROW_WAIT"
		"		TASK_COMPY_BURROW_WAIT			1"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_NEW_ENEMY"				// HACK: We don't actually choose a new schedule on new enemy, but
											// we need this interrupt so that the headcrab actually acquires
											// new enemies while burrowed. (look in ai_basenpc.cpp for "DO NOT mess")
		"		COND_CAN_RANGE_ATTACK1"
	)

	//==================================================
	// Burrow Out
	//==================================================
	DEFINE_SCHEDULE
	(
		SCHED_COMPY_BURROW_OUT,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_COMPY_BURROW_WAIT"
		"		TASK_COMPY_UNBURROW			0"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_COMPY_BURROW_OUT"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
	)

	//==================================================
	// Wait for it to be clear for unburrowing
	//==================================================
	DEFINE_SCHEDULE
	(
		SCHED_COMPY_WAIT_FOR_CLEAR_UNBURROW,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMPY_BURROW_WAIT"
		"		TASK_COMPY_CHECK_FOR_UNBURROW		1"
		"		TASK_SET_SCHEDULE					SCHEDULE:SCHED_COMPY_BURROW_OUT"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
	)

	//==================================================
	// Wait until we can drop.
	//==================================================
	DEFINE_SCHEDULE
	(
	SCHED_COMPY_CEILING_WAIT,

	"	Tasks"
	"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMPY_CEILING_DROP"
	"		TASK_SET_ACTIVITY					ACTIVITY:ACT_COMPY_CEILING_IDLE"
	"		TASK_COMPY_CEILING_POSITION		0"
	"		TASK_COMPY_CEILING_WAIT			1"
	""
	"	Interrupts"
	"		COND_TASK_FAILED"
	"		COND_NEW_ENEMY"	
	"		COND_CAN_RANGE_ATTACK1"
	)

	//==================================================
	// Deatch from ceiling.
	//==================================================
	DEFINE_SCHEDULE
	(
	SCHED_COMPY_CEILING_DROP,

	"	Tasks"
	"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMPY_CEILING_WAIT"
	"		TASK_COMPY_CEILING_DETACH		0"
	"		TASK_COMPY_CEILING_FALL			0"
	"		TASK_COMPY_CEILING_LAND			0"
	""
	"	Interrupts"
	"		COND_TASK_FAILED"
	)

AI_END_CUSTOM_NPC()

//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_compsognathus_fast, CFastCompsognathus )
	
	//DECLARE_TASK( TASK_COMPY_HOP_ASIDE )

	DECLARE_SQUADSLOT( SQUAD_SLOT_ENGAGE1 )
	DECLARE_SQUADSLOT( SQUAD_SLOT_ENGAGE2 )
	DECLARE_SQUADSLOT( SQUAD_SLOT_ENGAGE3 )
	DECLARE_SQUADSLOT( SQUAD_SLOT_ENGAGE4 )
AI_END_CUSTOM_NPC()

//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_compsognathus_poison, CBlackCompsognathus )

	DECLARE_ACTIVITY( ACT_BLACKCOMPY_RUN_PANIC )

	//Adrian: events go here
	DECLARE_ANIMEVENT( AE_POISONCOMPY_FLINCH_HOP )
	DECLARE_ANIMEVENT( AE_POISONCOMPY_FOOTSTEP )
	DECLARE_ANIMEVENT( AE_POISONCOMPY_THREAT_SOUND )

AI_END_CUSTOM_NPC()



