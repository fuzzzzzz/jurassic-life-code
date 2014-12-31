//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements the compy, a tiny, jumpy dinosaur.
//
// TODO: make poison compy hop in response to nearby bullet impacts?
//
//=============================================================================//


#include "cbase.h"
#include "ai_basenpc.h"
#include "ai_task.h"
#include "ai_default.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "ai_interactions.h"
#include "ai_navigator.h"
#include "ai_motor.h"
#include "activitylist.h"
#include "game.h"
#include "hl2_shareddefs.h"
#include "hl2_gamerules.h"
#include "npcevent.h"
#include "player.h"
#include "entitylist.h"
#include "soundenvelope.h"
#include "shake.h"
#include "ndebugoverlay.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "movevars_shared.h"
#include "physics_prop_ragdoll.h"

#include "physics_collisionevent.h"
#include "jl/npc_fish.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar g_debug_fish( "g_debug_fish", "0", FCVAR_CHEAT );

#define	BRYCON_MODEL	"models/jl/animals/brycon_g.mdl"
#define	PIRANHA_MODEL	"models/jl/animals/piranha.mdl"
#define	CROCODILE_MODEL	"models/jl/animals/crocodile.mdl"


/* cbasefish */
#define	NPC_WAYPOINT_DISTANCE	12.0f //64.0f default
#define	NPC_HEIGHT_PREFERENCE	5.0f //12.0f default
#define	NPC_DEPTH_PREFERENCE	3.0f //3.0f

#define	FISH_SWIM_SPEED_WALK		75 // 250
#define	FISH_SWIM_SPEED_RUN		115 // 500
#define	FISH_MIN_TURN_SPEED		15.0f
#define	FISH_MAX_TURN_SPEED		45.0f


/////////***** BRYCON  ********////////

//#define	BRYCON_HEIGHT_PREFERENCE	5.0f //12.0f default
//#define	BRYCON_DEPTH_PREFERENCE	3.0f //3.0f
//#define	BRYCON_WAYPOINT_DISTANCE	64.0f //64.0f default

#define	BRYCON_SWIM_SPEED_WALK		75 // 250
#define	BRYCON_SWIM_SPEED_RUN		115 // 500
#define	BRYCON_MIN_TURN_SPEED		15.0f
#define	BRYCON_MAX_TURN_SPEED		45.0f


/////////***** PIRANHA  ********////////

//#define	PIRANHA_HEIGHT_PREFERENCE	5.0f //12.0f default
//#define	PIRANHA_DEPTH_PREFERENCE	3.0f //3.0f
//#define	PIRANHA_WAYPOINT_DISTANCE	64.0f //64.0f default


#define	PIRANHA_SWIM_SPEED_WALK		75 // 250
#define	PIRANHA_SWIM_SPEED_RUN		115 // 500
#define	PIRANHA_MIN_TURN_SPEED		15.0f
#define	PIRANHA_MAX_TURN_SPEED		45.0f

/////////***** CROCODILE  ********////////

//#define	CROCODILE_HEIGHT_PREFERENCE	5.0f //12.0f default
//#define	CROCODILE_DEPTH_PREFERENCE	3.0f //3.0f
//#define	CROCODILE_WAYPOINT_DISTANCE	64.0f //64.0f default

#define	CROCODILE_SWIM_SPEED_WALK		75 // 250
#define	CROCODILE_SWIM_SPEED_RUN		115 // 500
#define	CROCODILE_MIN_TURN_SPEED		15.0f
#define	CROCODILE_MAX_TURN_SPEED		45.0f

#define	SF_CROCODILE_IN_WATER	( 1 << 18 )


#define	ENVELOPE_CONTROLLER		(CSoundEnvelopeController::GetController())

#define	FEELER_COLLISION			0
#define	FEELER_COLLISION_VISUALIZE	(FEELER_COLLISION&&0)

#define	LATERAL_NOISE_MAX	2.0f
#define	LATERAL_NOISE_FREQ	1.0f

#define	VERTICAL_NOISE_MAX	2.0f
#define	VERTICAL_NOISE_FREQ	1.0f

enum FishMoveType_t
{
	NPC_MOVETYPE_SEEK = 0,	// Fly through the target without stopping.
	NPC_MOVETYPE_ARRIVE		// Slow down and stop at target.
};



//-----------------------------------------------------------------------------
// Think contexts.
//-----------------------------------------------------------------------------
//static const char *s_pPitchContext = "PitchContext";


//-----------------------------------------------------------------------------
// Animation events.
//-----------------------------------------------------------------------------

//...
int PIRANHA_AE_BITE;
int PIRANHA_AE_BITE_START;

int CROCODILE_AE_BITE;
int CROCODILE_AE_BITE_START;


//=========================================================
// private activities
//=========================================================

int	ACT_PIRANHA_THRASH;
int	ACT_PIRANHA_BITE_HIT;
int	ACT_PIRANHA_BITE_MISS;

int	ACT_CROCODILE_THRASH;
int	ACT_CROCODILE_BITE_HIT;
int	ACT_CROCODILE_BITE_MISS;

//-----------------------------------------------------------------------------
// Custom schedules.
//-----------------------------------------------------------------------------
enum //FishSchedules
{	
	SCHED_FISH_FLEE_ENEMY = LAST_SHARED_SCHEDULE,
	//SCHED_FISH_FLEE_ENEMY,
	SCHED_FISH_PATROL_RUN,
	//SCHED_FISH_PATROL_WALK = LAST_SHARED_SCHEDULE,
	SCHED_FISH_PATROL_WALK,

	SCHED_PIRANHA_CHASE_ENEMY = LAST_SHARED_SCHEDULE,
	//SCHED_PIRANHA_CHASE_ENEMY,
	SCHED_PIRANHA_MELEE_ATTACK1,
	SCHED_PIRANHA_THRASH,

	SCHED_CROCODILE_CHASE_ENEMY = LAST_SHARED_SCHEDULE,
	SCHED_CROCODILE_CHASE_ENEMY_GROUND,
	SCHED_CROCODILE_PATROL_WALK_GROUND,
	SCHED_CROCODILE_PATROL_RUN_GROUND,
	//SCHED_CROCODILE_CHASE_ENEMY,
	SCHED_CROCODILE_MELEE_ATTACK1,
	SCHED_CROCODILE_MELEE_ATTACK2,
	SCHED_CROCODILE_THRASH,	
	SCHED_CROCODILE_DROWN_VICTIM,
	
};


//=========================================================
// tasks
//=========================================================
enum //FishTasks
{
	TASK_FISH_GET_PATH_TO_RANDOM_NODE = LAST_SHARED_TASK,
	
	TASK_PIRANHA_THRASH_PATH,

	TASK_CROCODILE_THRASH_PATH,
	TASK_CROCODILE_GET_PATH_TO_DROWN_NODE,	
};

//-----------------------------------------------------------------------------
// Skill settings.
//-----------------------------------------------------------------------------
ConVar	sk_brycon_health( "sk_brycon_health", "0" );

ConVar	sk_piranha_health( "sk_piranha_health", "0" );
ConVar	sk_piranha_melee_dmg( "sk_piranha_melee_dmg", "0" );

ConVar	sk_crocodile_health( "sk_crocodile_health", "0" );
ConVar	sk_crocodile_melee_dmg( "sk_crocodile_melee_dmg", "0" );

//Data description
BEGIN_DATADESC( CBaseFish )


// Silence classcheck
// Common variable used with the brycon, piranha and crocodile navigation in the water.
//	DEFINE_FIELD( m_LastSteer, FIELD_VECTOR ), // not implemented by vavle dev team !

	DEFINE_FIELD( m_vecLastMoveTarget,	FIELD_VECTOR ), // OverrideMove () class
	DEFINE_FIELD( m_bHasMoveTarget,		FIELD_BOOLEAN ), // OverrideMove() class
	DEFINE_FIELD( m_bIgnoreSurface,		FIELD_BOOLEAN ), // steering classes

	DEFINE_FIELD( m_flNextPingTime,		FIELD_FLOAT ), // PrescheduleThink() class
	DEFINE_FIELD( m_flNextGrowlTime,		FIELD_FLOAT ), // PrescheduleThink() class
	
// Common variable used by SetPoses() ( additionals animation features)
	DEFINE_FIELD( m_flSwimSpeed,			FIELD_FLOAT ), 

/* Common variable used by SetPoses() but disabled */
//	DEFINE_FIELD( m_flTailYaw,			FIELD_FLOAT ),
//	DEFINE_FIELD( m_flTailPitch,			FIELD_FLOAT ),


/* really need to input the pirnaha brycon or crocodile ragdoll from hammer entity ?  */
//	DEFINE_INPUTFUNC( FIELD_VOID,	"Ragdoll", InputRagdoll ),

// !! valve old code !!
// need this for the piranha or croc or brycon ??
//	DEFINE_FUNCTION( PiranhaTouch ), 

//	DEFINE_FIELD( m_pVictim,				FIELD_CLASSPTR ),


END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseFish::Spawn( void )
{
//	Precache();

	//SetHullType(HULL_SMALL_CENTERED);
	//SetHullSizeNormal();

//	SetDefaultEyeOffset();
	m_flFieldOfView			= 0.5;

//	SetDistLook( 512 ); // if not set the piranah see the player from far location
//	SetViewOffset( Vector(6, 0, 11) ) ;		// Position of the eyes relative to NPC's origin.

// ?? Try with HL2COLLISION_GROUP_NPC_ACTOR ( not collide with us)or custom collision group? 
// Take a look in hl2_gamerules.cpp and HL2_sharedef.cpp for the declared NPC groups.
// original value :  NO VALUE
	SetCollisionGroup( JLCOLLISION_GROUP_FISH ); 

	SetNavType( NAV_FLY );
	m_NPCState	= NPC_STATE_NONE;
	SetBloodColor( BLOOD_COLOR_RED );
	
	SetSolid( SOLID_BBOX ); // SOLID_BBOX
	AddSolidFlags( FSOLID_NOT_STANDABLE ); // FSOLID_NOT_STANDABLE
	SetMoveType( MOVETYPE_STEP ); //_FLY );
	AddFlag( FL_FLY | FL_STEPMOVEMENT );


	m_flGroundSpeed			= FISH_SWIM_SPEED_RUN; 
	m_flGroundSpeedWalk		= FISH_SWIM_SPEED_WALK;

	m_bIgnoreSurface		= false;

	m_flSwimSpeed			= 1.0f;
//	m_flTailYaw				= 0.0f;
//	m_flTailPitch			= 0.0f;

//	m_flNextBiteTime		= gpGlobals->curtime;
//	m_flHoldTime			= gpGlobals->curtime;
	m_flNextPingTime		= gpGlobals->curtime;
	m_flNextGrowlTime		= gpGlobals->curtime;

#if FEELER_COLLISION

	Vector	forward;

	GetVectors( &forward, NULL, NULL );

	m_vecCurrentVelocity	= forward * m_flGroundSpeed;

#endif

//	SetTouch( PiranhaTouch ); // valve old code

/* Disabling this , that is activated directly into the spawn class of each npcs */

//	CapabilitiesClear();
//	CapabilitiesAdd( bits_CAP_MOVE_FLY ); | bits_CAP_INNATE_MELEE_ATTACK1 );

//	NPCInit();

	//m_pSwimSound	= ENVELOPE_CONTROLLER.SoundCreate( edict(), CHAN_BODY,	"xxxCONVERTTOGAMESOUNDS!!!npc/ichthyosaur/ich_amb1wav", ATTN_NORM );
	//m_pVoiceSound	= ENVELOPE_CONTROLLER.SoundCreate( edict(), CHAN_VOICE,	"xxxCONVERTTOGAMESOUNDS!!!npc/ichthyosaur/water_breathwav", ATTN_IDLE );

	//ENVELOPE_CONTROLLER.Play( m_pSwimSound,	1.0f, 100 );
	//ENVELOPE_CONTROLLER.Play( m_pVoiceSound,1.0f, 100 );

	
}


//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CBaseFish::Precache( void )
{
	BaseClass::Precache();
}	


//-----------------------------------------------------------------------------
// Purpose: Handles movement towards the last move target.
// Input  : flInterval - 
//-----------------------------------------------------------------------------

bool CBaseFish::OverrideMove( float flInterval )
{
	m_flGroundSpeed = GetGroundSpeed();

	if ( m_bHasMoveTarget )
	{
		DoMovement( flInterval, m_vecLastMoveTarget, NPC_MOVETYPE_ARRIVE );
	}
	else
	{
		DoMovement( flInterval, GetLocalOrigin(), NPC_MOVETYPE_ARRIVE );
	}
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTargetEnt - 
//			vecDir - 
//			flDistance - 
//			flInterval - 
//-----------------------------------------------------------------------------

void CBaseFish::MoveFlyExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flDistance, float flInterval )
{
	FishMoveType_t eMoveType = ( GetNavigator()->CurWaypointIsGoal() ) ? NPC_MOVETYPE_ARRIVE : NPC_MOVETYPE_SEEK;

	m_flGroundSpeed = GetGroundSpeed();

	Vector	moveGoal = GetNavigator()->GetCurWaypointPos();

	//See if we can move directly to our goal
	if ( ( GetEnemy() != NULL ) && ( GetNavigator()->GetGoalTarget() == (CBaseEntity *) GetEnemy() ) )
	{
		trace_t	tr;
		Vector	goalPos = GetEnemy()->GetAbsOrigin() + ( GetEnemy()->GetSmoothedVelocity() * 0.5f );

		AI_TraceHull( GetAbsOrigin(), goalPos, GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, GetEnemy(), COLLISION_GROUP_NONE, &tr );

		if ( tr.fraction == 1.0f )
		{
			moveGoal = tr.endpos;
		}
	}

	//Move
	DoMovement( flInterval, moveGoal, eMoveType );

	//Save the info from that run
	m_vecLastMoveTarget	= moveGoal;
	m_bHasMoveTarget	= true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flInterval - 
//			&m_LastMoveTarget - 
//			eMoveType - 
//-----------------------------------------------------------------------------
void CBaseFish::DoMovement( float flInterval, const Vector &MoveTarget, int eMoveType )
{
	// dvs: something is setting this bit, causing us to stop moving and get stuck that way
	//Forget( bits_MEMORY_TURNING );

	Vector Steer, SteerAvoid, SteerRel;
	Vector forward, right, up;

	//Get our orientation vectors.
	GetVectors( &forward, &right, &up);

	if ( ( GetActivity() == ACT_MELEE_ATTACK1 ) && ( GetEnemy() != NULL ) )
	{
		SteerSeek( Steer, GetEnemy()->GetAbsOrigin() );
	}
	else
	{
		//If we are approaching our goal, use an arrival steering mechanism.
		if ( eMoveType == NPC_MOVETYPE_ARRIVE )
		{
			SteerArrive( Steer, MoveTarget );
		}
		else
		{
			//Otherwise use a seek steering mechanism.
			SteerSeek( Steer, MoveTarget );
		}
	}
	
#if FEELER_COLLISION

	Vector f, u, l, r, d;

	float	probeLength = GetAbsVelocity().Length();

	if ( probeLength < 150 )
		probeLength = 150;

	if ( probeLength > 500 )
		probeLength = 500;

	f = DoProbe( GetLocalOrigin() + (probeLength * forward) );
	r = DoProbe( GetLocalOrigin() + (probeLength/3 * (forward+right)) );
	l = DoProbe( GetLocalOrigin() + (probeLength/3 * (forward-right)) );
	u = DoProbe( GetLocalOrigin() + (probeLength/3 * (forward+up)) );
	d = DoProbe( GetLocalOrigin() + (probeLength/3 * (forward-up)) );

	SteerAvoid = f+r+l+u+d;
	
	//NDebugOverlay::Line( GetLocalOrigin(), GetLocalOrigin()+SteerAvoid, 255, 255, 0, false, 0.1f );	

	if ( SteerAvoid.LengthSqr() )
	{
		Steer = (SteerAvoid*0.5f);
	}

	m_vecVelocity = m_vecVelocity + (Steer*0.5f);

	VectorNormalize( m_vecVelocity );

	SteerRel.x = forward.Dot( m_vecVelocity );
	SteerRel.y = right.Dot( m_vecVelocity );
	SteerRel.z = up.Dot( m_vecVelocity );

	m_vecVelocity *= m_flGroundSpeed;

#else

	//See if we need to avoid any obstacles.
	if ( SteerAvoidObstacles( SteerAvoid, GetAbsVelocity(), forward, right, up ) )
	{
		//Take the avoidance vector
		Steer = SteerAvoid;
	}

	//Clamp our ideal steering vector to within our physical limitations.
	ClampSteer( Steer, SteerRel, forward, right, up );

	ApplyAbsVelocityImpulse( Steer * flInterval );
	
#endif

	Vector vecNewVelocity = GetAbsVelocity();
	float flLength = vecNewVelocity.Length();

	//Clamp our final speed
	if ( flLength > m_flGroundSpeed )
	{
		vecNewVelocity *= ( m_flGroundSpeed / flLength );
		flLength = m_flGroundSpeed;
	}

	Vector	workVelocity = vecNewVelocity;

	AddSwimNoise( &workVelocity );

	// Pose the piranha properly
	SetPoses( SteerRel, flLength );

/*	//Drag our victim before moving
	if ( m_pVictim != NULL )
	{
		DragVictim( (workVelocity*flInterval).Length() );
	}
*/
	//Move along the current velocity vector
	if ( WalkMove( workVelocity * flInterval, MASK_NPCSOLID ) == false )
	{
		//Attempt a half-step
		if ( WalkMove( (workVelocity*0.5f) * flInterval,  MASK_NPCSOLID) == false )
		{
			//Restart the velocity
			//VectorNormalize( m_vecVelocity );
			vecNewVelocity *= 0.5f;
		}
		else
		{
			//Cut our velocity in half
			vecNewVelocity *= 0.5f;
		}
	}

	SetAbsVelocity( vecNewVelocity );

}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &probe - 
// Output : Vector
//-----------------------------------------------------------------------------
#if FEELER_COLLISION

Vector CBaseFish::DoProbe( const Vector &probe )
{
	trace_t	tr;
	float	fraction = 1.0f;
	bool	collided = false;
	Vector	normal	 = Vector( 0, 0, -1 );

	float	waterLevel = UTIL_WaterLevel( GetAbsOrigin(), GetAbsOrigin().z, GetAbsOrigin().z+150 );

	waterLevel -= GetAbsOrigin().z;
	waterLevel /= 150;

	if ( waterLevel < 1.0f )
	{
		collided = true;
		fraction = waterLevel;
	}

	AI_TraceHull( GetAbsOrigin(), probe, GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );
	
	if ( ( collided == false ) || ( tr.fraction < fraction ) )
	{
		fraction	= tr.fraction;
		normal		= tr.plane.normal;
	}

	if ( ( fraction < 1.0f ) && ( GetEnemy() == NULL || tr.u.ent != GetEnemy()->pev ) )
	{
#if FEELER_COLLISION_VISUALIZE
		NDebugOverlay::Line( GetLocalOrigin(), probe, 255, 0, 0, false, 0.1f );
#endif
		
		Vector	probeDir = probe - GetLocalOrigin();

		Vector	normalToProbeAndWallNormal = probeDir.Cross( normal );
		Vector	steeringVector = normalToProbeAndWallNormal.Cross( probeDir );

		Vector	velDir = GetAbsVelocity();
		VectorNormalize( velDir );

		float	steeringForce = m_flGroundSpeed * ( 1.0f - fraction ) * normal.Dot( velDir );

		if ( steeringForce < 0.0f )
		{
			steeringForce = -steeringForce;
		}

		velDir = steeringVector;
		VectorNormalize( velDir );

		steeringVector = steeringForce * velDir;
		
		return steeringVector;
	}

#if FEELER_COLLISION_VISUALIZE
	NDebugOverlay::Line( GetLocalOrigin(), probe, 0, 255, 0, false, 0.1f );
#endif

	return Vector( 0.0f, 0.0f, 0.0f );
}

#endif




//-----------------------------------------------------------------------------
// Purpose: Gets a steering vector to arrive at a target location with a
//			relatively small velocity.
// Input  : Steer - Receives the ideal steering vector.
//			Target - Target position at which to arrive.
//-----------------------------------------------------------------------------
void CBaseFish::SteerArrive(Vector &Steer, const Vector &Target)
{
	Vector Offset = Target - GetLocalOrigin();
	float fTargetDistance = Offset.Length();

	float fIdealSpeed = m_flGroundSpeed * (fTargetDistance / NPC_WAYPOINT_DISTANCE);
	float fClippedSpeed = min( fIdealSpeed, m_flGroundSpeed );
	
	Vector DesiredVelocity( 0, 0, 0 );

	if ( fTargetDistance > NPC_WAYPOINT_DISTANCE )
	{
		DesiredVelocity = (fClippedSpeed / fTargetDistance) * Offset;
	}

	Steer = DesiredVelocity - GetAbsVelocity();
}

//-----------------------------------------------------------------------------
// Purpose: Gets a steering vector to move towards a target position as quickly
//			as possible.
// Input  : Steer - Receives the ideal steering vector.
//			Target - Target position to seek.
//-----------------------------------------------------------------------------
void CBaseFish::SteerSeek( Vector &Steer, const Vector &Target )
{
	Vector offset = Target - GetLocalOrigin();
	
	VectorNormalize( offset );
	
	Vector DesiredVelocity = m_flGroundSpeed * offset;
	
	Steer = DesiredVelocity - GetAbsVelocity();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &Steer - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseFish::SteerAvoidObstacles(Vector &Steer, const Vector &Velocity, const Vector &Forward, const Vector &Right, const Vector &Up)
{
	trace_t	tr;

	bool	collided = false;
	Vector	dir = Velocity;
	float	speed = VectorNormalize( dir );

	//Look ahead one second and avoid whatever is in our way.
	AI_TraceHull( GetAbsOrigin(), GetAbsOrigin() + (dir*speed), GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );

	Vector	forward;

	GetVectors( &forward, NULL, NULL );

	//If we're hitting our enemy, just continue on
	if ( ( GetEnemy() != NULL ) && ( tr.m_pEnt == GetEnemy() ) )
		return false;

	if ( tr.fraction < 1.0f )
	{
		CBaseEntity *pBlocker = tr.m_pEnt;
		
		if ( ( pBlocker != NULL ) && ( pBlocker->MyNPCPointer() != NULL ) )
		{
			DevMsg( 2, "Avoiding an NPC\n" );

			Vector HitOffset = tr.endpos - GetAbsOrigin();

			Vector SteerUp = CrossProduct( HitOffset, Velocity );
			Steer = CrossProduct(  SteerUp, Velocity  );
			VectorNormalize( Steer );

			/*Vector probeDir = tr.endpos - GetAbsOrigin();
			Vector normalToProbeAndWallNormal = probeDir.Cross( tr.plane.normal );
			
			Steer = normalToProbeAndWallNormal.Cross( probeDir );
			VectorNormalize( Steer );*/

			if ( tr.fraction > 0 )
			{
				Steer = (Steer * Velocity.Length()) / tr.fraction;
				//NDebugOverlay::Line( GetLocalOrigin(), GetLocalOrigin()+Steer, 255, 0, 0, false, 0.1f );
			}
			else
			{
				Steer = (Steer * 1000 * Velocity.Length());
				//NDebugOverlay::Line( GetLocalOrigin(), GetLocalOrigin()+Steer, 255, 0, 0, false, 0.1f );
			}
		}
		else
		{
			if ( ( pBlocker != NULL ) && ( pBlocker == GetEnemy() ) )
			{
				DevMsg( "Avoided collision\n" );
				return false;
			}

			DevMsg( 2, "Avoiding the world\n" );
			
			Vector	steeringVector = tr.plane.normal;

			if ( tr.fraction == 0.0f )
				return false;

			Steer = steeringVector * ( Velocity.Length() / tr.fraction );
			
			//NDebugOverlay::Line( GetLocalOrigin(), GetLocalOrigin()+Steer, 255, 0, 0, false, 0.1f );
		}

		//return true;
		collided = true;
	}

	//Try to remain 8 feet above the ground.
	AI_TraceLine( GetAbsOrigin(), GetAbsOrigin() + Vector(0, 0, -NPC_HEIGHT_PREFERENCE), MASK_NPCSOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

	if ( tr.fraction < 1.0f )
	{
		Steer += Vector( 0, 0, m_vecAccelerationMax.z / tr.fraction );
		collided = true;
	}
	
	//Stay under the surface
	if ( m_bIgnoreSurface == false )
	{
		float waterLevel = ( UTIL_WaterLevel( GetAbsOrigin(), GetAbsOrigin().z, GetAbsOrigin().z+NPC_DEPTH_PREFERENCE ) - GetAbsOrigin().z ) / NPC_DEPTH_PREFERENCE;

		if ( waterLevel < 1.0f )
		{
			Steer += -Vector( 0, 0, m_vecAccelerationMax.z / waterLevel );
			collided = true;
		}
	}

	return collided;
}

//-----------------------------------------------------------------------------
// Purpose: Clamps the desired steering vector based on the limitations of this
//			vehicle.
// Input  : SteerAbs - The vector indicating our ideal steering vector. Receives
//				the clamped steering vector in absolute (x,y,z) coordinates.
//			SteerRel - Receives the clamped steering vector in relative (forward, right, up)
//				coordinates.
//			forward - Our current forward vector.
//			right - Our current right vector.
//			up - Our current up vector.
//-----------------------------------------------------------------------------
void CBaseFish::ClampSteer(Vector &SteerAbs, Vector &SteerRel, Vector &forward, Vector &right, Vector &up)
{
	float fForwardSteer = DotProduct(SteerAbs, forward);
	float fRightSteer = DotProduct(SteerAbs, right);
	float fUpSteer = DotProduct(SteerAbs, up);

	if (fForwardSteer > 0)
	{
		fForwardSteer = min(fForwardSteer, m_vecAccelerationMax.x);
	}
	else
	{
		fForwardSteer = max(fForwardSteer, m_vecAccelerationMin.x);
	}

	if (fRightSteer > 0)
	{
		fRightSteer = min(fRightSteer, m_vecAccelerationMax.y);
	}
	else
	{
		fRightSteer = max(fRightSteer, m_vecAccelerationMin.y);
	}

	if (fUpSteer > 0)
	{
		fUpSteer = min(fUpSteer, m_vecAccelerationMax.z);
	}
	else
	{
		fUpSteer = max(fUpSteer, m_vecAccelerationMin.z);
	}

	SteerAbs = (fForwardSteer*forward) + (fRightSteer*right) + (fUpSteer*up);

	SteerRel.x = fForwardSteer;
	SteerRel.y = fRightSteer;
	SteerRel.z = fUpSteer;
}

//-----------------------------------------------------------------------------
// Purpose: Determines the pose parameters for the bending of the body and tail speed
// Input  : moveRel - the dot products for the deviation off of each direction (f,r,u)
//			speed - speed of the piranha
//-----------------------------------------------------------------------------
void CBaseFish::SetPoses( Vector moveRel, float speed )
{
	float	movePerc, moveBase;

	//Find out how fast we're moving in our animations boundaries
	if ( GetIdealActivity() == ACT_WALK )
	{
		moveBase = 0.5f;
		//movePerc = moveBase * ( speed / FISH_SWIM_SPEED_WALK ); // change this value
		movePerc = moveBase * ( speed / m_flGroundSpeedWalk );
	}
	else
	{
		moveBase = 1.0f;
		//movePerc = moveBase * ( speed / FISH_SWIM_SPEED_RUN ); // change this value
		movePerc = moveBase * ( speed / m_flGroundSpeed );
	}
	
//	Vector	tailPosition;
	float	flSwimSpeed = movePerc;

	//Forward deviation
	if ( moveRel.x > 0 )
	{
		flSwimSpeed *= moveBase + (( moveRel.x / m_vecAccelerationMax.x )*moveBase);
	}
	else if ( moveRel.x < 0 )
	{
		flSwimSpeed *= moveBase - (( moveRel.x / m_vecAccelerationMin.x )*moveBase);
	}

	//Vertical deviation
//	if ( moveRel.z > 0 )
//	{
//		tailPosition[PITCH]	= -90.0f * ( moveRel.z / m_vecAccelerationMax.z );
//	}
//	else if ( moveRel.z < 0 )
//	{
//		tailPosition[PITCH]	= 90.0f * ( moveRel.z / m_vecAccelerationMin.z );
//	}
//	else
//	{
//		tailPosition[PITCH]	= 0.0f;
//	}

	//Lateral deviation
//	if ( moveRel.y > 0 )
//	{
//		tailPosition[ROLL]	= 25 * moveRel.y / m_vecAccelerationMax.y;
//		tailPosition[YAW]	= -1.0f * moveRel.y / m_vecAccelerationMax.y;
//	}
//	else if ( moveRel.y < 0 )
//	{
//		tailPosition[ROLL]	= -25 * moveRel.y / m_vecAccelerationMin.y;
//		tailPosition[YAW]	= moveRel.y / m_vecAccelerationMin.y;
//	}
//	else
//	{
//		tailPosition[ROLL]	= 0.0f;
//		tailPosition[YAW]	= 0.0f;
//	}
	
	//Clamp
	flSwimSpeed			= clamp( flSwimSpeed, 0.25f, 1.0f );
//	tailPosition[YAW]	= clamp( tailPosition[YAW], -90.0f, 90.0f );
//	tailPosition[PITCH]	= clamp( tailPosition[PITCH], -90.0f, 90.0f );

	//Blend
//	m_flTailYaw		= ( m_flTailYaw * 0.8f ) + ( tailPosition[YAW] * 0.2f );
//	m_flTailPitch	= ( m_flTailPitch * 0.8f ) + ( tailPosition[PITCH] * 0.2f );
	m_flSwimSpeed	= ( m_flSwimSpeed * 0.8f ) + ( flSwimSpeed * 0.2f );

	//Pose the body
	SetPoseParameter( 0, m_flSwimSpeed );
//	SetPoseParameter( 1, m_flTailYaw );
//	SetPoseParameter( 2, m_flTailPitch );
	
	//FIXME: Until the sequence info is reset properly after SetPoseParameter
	if ( ( GetActivity() == ACT_RUN ) || ( GetActivity() == ACT_WALK ) )
	{
		ResetSequenceInfo();
	}

	//Face our current velocity
	GetMotor()->SetIdealYawAndUpdate( UTIL_AngleMod( CalcIdealYaw( GetAbsOrigin() + GetAbsVelocity() ) ), AI_KEEP_YAW_SPEED );

	float	pitch = 0.0f;

	if ( speed != 0.0f )
	{
		pitch = -RAD2DEG( asin( GetAbsVelocity().z / speed ) );
	}

	//FIXME: Framerate dependant
//	QAngle angles = GetLocalAngles();

//	angles.x = (angles.x * 0.8f) + (pitch * 0.2f);
//	angles.z = (angles.z * 0.9f) + (tailPosition[ROLL] * 0.1f);

//	SetLocalAngles( angles );
//
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : velocity - 
//-----------------------------------------------------------------------------
void CBaseFish::AddSwimNoise( Vector *velocity )
{
	Vector	right, up;

	GetVectors( NULL, &right, &up );

	float	lNoise, vNoise;

	lNoise = LATERAL_NOISE_MAX * sin( gpGlobals->curtime * LATERAL_NOISE_FREQ );
	vNoise = VERTICAL_NOISE_MAX * sin( gpGlobals->curtime * VERTICAL_NOISE_FREQ );

	(*velocity) += ( right * lNoise ) + ( up * vNoise );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseFish::Beached( void )
{
	trace_t	tr;
	Vector	testPos;

	testPos = GetAbsOrigin() - Vector( 0, 0, NPC_DEPTH_PREFERENCE );
	
	AI_TraceHull( GetAbsOrigin(), testPos, GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );

	return ( tr.fraction < 1.0f );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : speed to move at
//-----------------------------------------------------------------------------
float CBaseFish::GetGroundSpeed( void )
{
//	if ( m_flHoldTime > gpGlobals->curtime )
//		return	FISH_SWIM_SPEED_WALK/3.5f; //2.0f;

	if ( GetIdealActivity() == ACT_WALK )
		return FISH_SWIM_SPEED_WALK;

//	if ( GetIdealActivity() == ACT_CROCODILE_THRASH )
//		return FISH_SWIM_SPEED_WALK;

	return FISH_SWIM_SPEED_RUN;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEnemy - 
//			&chasePosition - 
//			&tolerance - 
//-----------------------------------------------------------------------------
void CBaseFish::TranslateNavGoal( CBaseEntity *pEnemy, Vector &chasePosition )
{
	Vector offset = pEnemy->EyePosition() - pEnemy->GetAbsOrigin();
	chasePosition += offset;
}

float CBaseFish::GetDefaultNavGoalTolerance()
{
	return GetHullWidth()*2.0f;	
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEntity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseFish::FVisible( CBaseEntity *pEntity, int traceMask, CBaseEntity **ppBlocker )
{
	// don't look through water
	if ( GetWaterLevel() != pEntity->GetWaterLevel() )
		return false;

	return BaseClass::FVisible( pEntity, traceMask, ppBlocker );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
//-----------------------------------------------------------------------------
void CBaseFish::Event_Killed( const CTakeDamageInfo &info )
{
	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pevInflictor - 
//			*pAttacker - 
//			flDamage - 
//			bitsDamageType - 
//-----------------------------------------------------------------------------
int	CBaseFish::OnTakeDamage_Alive( const CTakeDamageInfo &inputInfo )
{
	CTakeDamageInfo info = inputInfo;
		// add pain to the conditions
	if ( IsLightDamage( inputInfo ) )
	{
		SetCondition( COND_LIGHT_DAMAGE );
	}
	if ( IsHeavyDamage( inputInfo ) )
	{
		SetCondition( COND_HEAVY_DAMAGE );
	}

		//
	// Certain death from melee bludgeon weapons!
	//
	if ( info.GetDamageType() & DMG_CLUB )
	{
		info.SetDamage( 6 );  //( m_iHealth );
	}

	if ( info.GetDamageType() & DMG_DROWN )
	{
		info.SetDamage( 0 );  //( m_iHealth );
	}
	return CAI_BaseNPC::OnTakeDamage_Alive( info );
	//return BaseClass::OnTakeDamage_Alive( inputInfo );
}

//-----------------------------------------------------------------------------
// Purpose: Don't become a ragdoll until we've finished our death anim
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseFish::CanBecomeRagdoll( void )
{
	//if ( IsCurSchedule( SCHED_DIE_RAGDOLL ) )
	//	return true;
	if ( IsCurSchedule( SCHED_DIE ) )
		return true;

	return hl2_episodic.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseFish::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();
	
	//Ambient sounds
	/*
	if ( random->RandomInt( 0, 20 ) == 10 )
	{
		if ( random->RandomInt( 0, 1 ) )
		{
			ENVELOPE_CONTROLLER.SoundChangeVolume( m_pSwimSound, random->RandomFloat( 0.0f, 0.5f ), 1.0f );
		}
		else
		{
			ENVELOPE_CONTROLLER.SoundChangeVolume( m_pVoiceSound, random->RandomFloat( 0.0f, 0.5f ), 1.0f );
		}
	}
	*/

	//Pings
	if ( m_flNextPingTime < gpGlobals->curtime )
	{
		m_flNextPingTime = gpGlobals->curtime + random->RandomFloat( 3.0f, 8.0f );
	}
	
	//Growls
	if ( ( m_NPCState == NPC_STATE_COMBAT || m_NPCState == NPC_STATE_ALERT ) && ( m_flNextGrowlTime < gpGlobals->curtime ) )
	{
		m_flNextGrowlTime = gpGlobals->curtime + random->RandomFloat( 2.0f, 6.0f );
	}

	//Randomly emit bubbles
	if ( random->RandomInt( 0, 10 ) == 0 )
	{
		UTIL_Bubbles( GetAbsOrigin()+(GetHullMins()*0.5f), GetAbsOrigin()+(GetHullMaxs()*0.5f), 1 );
	}

	//Check our water level
	if ( GetWaterLevel() != 2 )
	{
		if ( GetWaterLevel() < 1 )
		{
			DevMsg( 2, "Came out of water\n" );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseFish::SelectSchedule( void )
{
	switch ( m_NPCState )
	{
		case NPC_STATE_ALERT:
		{
			if (HasCondition( COND_LIGHT_DAMAGE ) || HasCondition( COND_HEAVY_DAMAGE ))
			{
				if ( fabs( GetMotor()->DeltaIdealYaw() ) < ( 1.0 - m_flFieldOfView) * 60 ) // roughly in the correct direction
				{
					return SCHED_TAKE_COVER_FROM_ORIGIN;
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
		}
	}
	//return nSchedule;
	return BaseClass::SelectSchedule();
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : type - 
// Output : int
//-----------------------------------------------------------------------------
int CBaseFish::TranslateSchedule( int scheduleType )
{
	//if ( scheduleType == SCHED_CHASE_ENEMY )	return SCHED_CROCODILE_CHASE_ENEMY;
	if ( scheduleType == SCHED_IDLE_STAND )		return SCHED_PATROL_WALK;
	if ( scheduleType == SCHED_PATROL_RUN )		return SCHED_FISH_PATROL_RUN;
	if ( scheduleType == SCHED_PATROL_WALK )	return SCHED_FISH_PATROL_WALK;
	//if ( scheduleType == SCHED_MELEE_ATTACK1 )	return SCHED_CROCODILE_MELEE_ATTACK1;

	return BaseClass::TranslateSchedule( scheduleType );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CBaseFish::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_FISH_GET_PATH_TO_RANDOM_NODE:
		{
			if ( GetEnemy() == NULL || !GetNavigator()->SetRandomGoal( GetEnemy()->GetLocalOrigin(), pTask->flTaskData ) )
			{
				if (!GetNavigator()->SetRandomGoal( pTask->flTaskData ) )
				{
					TaskFail(FAIL_NO_REACHABLE_NODE);
					return;
				}
			}
					
			TaskComplete();
		}
		break;

	default:
		BaseClass::StartTask(pTask);
		break;
	}
	
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CBaseFish::RunTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_FISH_GET_PATH_TO_RANDOM_NODE:
		return;
		break;

	default:
		BaseClass::RunTask(pTask);
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////
///////////////////////          BRYCON         //////////////////////////////
//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CBrycon::Precache( void )
{
	PrecacheModel( BRYCON_MODEL );

	PrecacheScriptSound( "NPC_Brycon.Idle" );
	PrecacheScriptSound( "NPC_Brycon.Die" );

	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBrycon::Spawn( void )
{
	Precache();
	SetModel( BRYCON_MODEL );

	SetHullType(HULL_SMALL_CENTERED);
	SetHullSizeNormal();

//	SetDefaultEyeOffset();
//	m_flFieldOfView			= 0.5; //= 0.5;  //-0.707; // VIEW_FIELD_FULL;
	SetDistLook( 512 ); // if not set the piranah see the player from far location
//	SetViewOffset( Vector(6, 0, 11) ) ;		// Position of the eyes relative to NPC's origin.
	
/*	SetSolid( SOLID_BBOX ); // SOLID_BBOX
	AddSolidFlags( FSOLID_NOT_STANDABLE ); // FSOLID_NOT_STANDABLE
	SetMoveType( MOVETYPE_STEP );
	AddFlag( FL_SWIM | FL_STEPMOVEMENT );
*/	
	BaseClass::Spawn();

	m_iHealth				= sk_brycon_health.GetFloat();
	m_iMaxHealth			= m_iHealth;
	
	m_flGroundSpeed			= BRYCON_SWIM_SPEED_RUN;
	m_flGroundSpeedWalk		= BRYCON_SWIM_SPEED_WALK;
	
	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_FLY | bits_CAP_SKIP_NAV_GROUND_CHECK );

	NPCInit();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBrycon::IdleSound( void )
{
	EmitSound( "NPC_Brycon.Idle" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBrycon::DeathSound( const CTakeDamageInfo &info )
{
	EmitSound( "NPC_Brycon.Die" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBrycon::SelectSchedule( void )
{
	//BaseClass::SelectSchedule( scheduleType );

	if ( m_NPCState == NPC_STATE_ALERT )
	{
		return	SCHED_FISH_FLEE_ENEMY;
	}
	
	return BaseClass::SelectSchedule();
	//return nSchedule;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : speed to move at
//-----------------------------------------------------------------------------
float CBrycon::GetGroundSpeed( void )
{
	// don't require
//	if ( m_flHoldTime > gpGlobals->curtime )
//		return	BRYCON_SWIM_SPEED_WALK/3.5f; //2.0f;

	if ( GetIdealActivity() == ACT_WALK )
		return BRYCON_SWIM_SPEED_WALK;

	return BRYCON_SWIM_SPEED_RUN;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : desired yaw speed
//-----------------------------------------------------------------------------
float CBrycon::MaxYawSpeed( void )
{
// disable to test the navigation of the piranha
//	if ( GetIdealActivity() == ACT_RUN )
//		return 16.0f;

	if ( GetIdealActivity() == ACT_WALK )
		return 8.0f;

	//Ramp up the yaw speed as we increase our speed
	return BRYCON_MIN_TURN_SPEED + ( (BRYCON_MAX_TURN_SPEED-BRYCON_MIN_TURN_SPEED) * ( fabs(GetAbsVelocity().Length()) / BRYCON_SWIM_SPEED_RUN ) );
}


//////////////////////////////////////////////////////////////////////////////
///////////////////////         PIRANHA       //////////////////////////////
//int	ACT_PIRANHA_THRASH;
//int	ACT_PIRANHA_BITE_HIT;
//int	ACT_PIRANHA_BITE_MISS;

BEGIN_DATADESC( CPiranha )

	// melee attack ( piranha+ croco)
	DEFINE_FIELD( m_flNextBiteTime,		FIELD_FLOAT ),
	// also in the Bite() class ( only need by the crocodile.)
	DEFINE_FIELD( m_flHoldTime,			FIELD_FLOAT ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CPiranha::Precache( void )
{
	PrecacheModel( PIRANHA_MODEL );

	PrecacheScriptSound( "NPC_Piranha.Pain" );
	PrecacheScriptSound( "NPC_Piranha.Bite" );
	PrecacheScriptSound( "NPC_Piranha.BiteMiss" );
	PrecacheScriptSound( "NPC_Piranha.Growl" );
	PrecacheScriptSound( "NPC_Piranha.AttackGrowl" );
	PrecacheScriptSound( "NPC_Piranha.Die" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPiranha::Spawn( void )
{
	Precache();
	
	SetModel( PIRANHA_MODEL );

	SetHullType(HULL_SMALL_CENTERED);
	SetHullSizeNormal();

	SetDistLook( 512 ); // if not set the piranah see the player from far location
//	SetViewOffset( Vector(6, 0, 11) ) ;		// Position of the eyes relative to NPC's origin.
	
	SetCollisionGroup( JLCOLLISION_GROUP_FISH ); 

/*	SetNavType( NAV_FLY );
	m_NPCState				= NPC_STATE_NONE;
	SetBloodColor( BLOOD_COLOR_RED );

	SetSolid( SOLID_BBOX ); // SOLID_BBOX
	AddSolidFlags( FSOLID_NOT_STANDABLE ); // FSOLID_NOT_STANDABLE
	SetMoveType( MOVETYPE_STEP );
	AddFlag( FL_FLY | FL_STEPMOVEMENT );
*/
	//BaseClass::Spawn();

	m_iHealth				= sk_piranha_health.GetFloat();
	m_iMaxHealth			= m_iHealth;

	m_flGroundSpeed			= PIRANHA_SWIM_SPEED_RUN;
	m_flGroundSpeedWalk		= PIRANHA_SWIM_SPEED_WALK;

	// melee attack
	m_flNextBiteTime		= gpGlobals->curtime;
	m_flHoldTime			= gpGlobals->curtime;


	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_FLY | bits_CAP_INNATE_MELEE_ATTACK1 |  bits_CAP_SKIP_NAV_GROUND_CHECK );
	
	BaseClass::Spawn();

	NPCInit();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPiranha::IdleSound( void )
{
	EmitSound( "NPC_Piranha.Idle" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPiranha::AlertSound( void )
{
	EmitSound( "NPC_Piranha.Alert" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPiranha::PainSound( const CTakeDamageInfo &info )
{
	EmitSound( "NPC_Piranha.Pain" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPiranha::DeathSound( const CTakeDamageInfo &info )
{
	EmitSound( "NPC_Piranha.Die" );
}

//-----------------------------------------------------------------------------
// Purpose: Get our conditions for a melee attack
// Input  : flDot - 
//			flDist - 
// Output : int
//-----------------------------------------------------------------------------
int CPiranha::MeleeAttack1Conditions( float flDot, float flDist )
{
	Vector	predictedDir	= ( (GetEnemy()->GetAbsOrigin()+(GetEnemy()->GetSmoothedVelocity())) - GetAbsOrigin() );	
	float	flPredictedDist = VectorNormalize( predictedDir );
	
	Vector	vBodyDir;
	GetVectors( &vBodyDir, NULL, NULL );

	float	flPredictedDot	= DotProduct( predictedDir, vBodyDir );

	if ( flPredictedDot < 0.8f )
		return COND_NOT_FACING_ATTACK;

	if ( ( flPredictedDist > ( GetAbsVelocity().Length() * 0.5f) ) && ( flDist > 128.0f ) )
		return COND_TOO_FAR_TO_ATTACK;
/*
	// Assume the this check is in regards to my current enemy
	// for the Manhacks spetial condition
	float deltaZ = GetAbsOrigin().z - GetEnemy()->EyePosition().z;
	if ( (deltaZ > 12.0f) || (deltaZ < -24.0f) )
	{
		return COND_TOO_CLOSE_TO_ATTACK;
	}
*/
	return COND_CAN_MELEE_ATTACK1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPiranha::Bite( void )
{
	//Don't allow another bite too soon
	if ( m_flNextBiteTime > gpGlobals->curtime )
		return;

	CBaseEntity *pHurt;
	
	//FIXME: E3 HACK - Always damage bullseyes if we're scripted to hit them
	if ( ( GetEnemy() != NULL ) && ( GetEnemy()->Classify() == CLASS_BULLSEYE ) )
	{
		pHurt = GetEnemy();
	}
	else
	{
		pHurt = CheckTraceHullAttack( 108, Vector(-8,-8,-8), Vector(8,8,8), 0, DMG_SLASH ); //valeur pardéfaut 32 et -32
	}

	//Hit something
	if ( pHurt != NULL )
	{
		CTakeDamageInfo info( this, this, sk_piranha_melee_dmg.GetInt(), DMG_SLASH );

		if ( pHurt->IsPlayer() )
		{
			CBasePlayer *pPlayer = ToBasePlayer( pHurt );

			if ( pPlayer )
			{
				if ( ( ( m_flHoldTime < gpGlobals->curtime ) && ( pPlayer->m_iHealth < (pPlayer->m_iMaxHealth*0.5f)) ) || ( pPlayer->GetWaterLevel() < 1 ) )
				{
					info.SetDamage( sk_piranha_melee_dmg.GetInt() * 3 );
				}
				CalculateMeleeDamageForce( &info, GetAbsVelocity(), pHurt->GetAbsOrigin() );
				pHurt->TakeDamage( info );
	
				color32 red = {32, 0, 0, 255}; //{64, 0, 0, 255}
				UTIL_ScreenFade( pPlayer, red, 0.2, 0, FFADE_IN  );

				//Disorient the player
				QAngle angles = pPlayer->GetLocalAngles();

				angles.x += random->RandomInt( 10, 5 );
				angles.y += random->RandomInt( 10, 5 );
				angles.z = 0.0f;

				pPlayer->SetLocalAngles( angles );

				pPlayer->SnapEyeAngles( angles );
			}
		}
		else
		{
			CalculateMeleeDamageForce( &info, GetAbsVelocity(), pHurt->GetAbsOrigin() );
			pHurt->TakeDamage( info );
		}

		m_flNextBiteTime = gpGlobals->curtime + random->RandomFloat( 2.0f, 4.0f );

		//Bubbles!
		UTIL_Bubbles( pHurt->GetAbsOrigin()+Vector(-8.0f,-8.0f,-8.0f), pHurt->GetAbsOrigin()+Vector(8.0f,8.0f,0.0f), random->RandomInt( 4, 8 ) );
		
		// Play a random attack hit sound
		EmitSound( "NPC_Piranha.Bite" );

		if ( GetActivity() == ACT_MELEE_ATTACK1 )
		{
			SetActivity( (Activity) ACT_PIRANHA_BITE_HIT );
		}
		
		return;
	}

	//Play the miss animation and sound
	if ( GetActivity() == ACT_MELEE_ATTACK1 )
	{
		SetActivity( (Activity) ACT_PIRANHA_BITE_MISS );
	}

	//Miss sound
	EmitSound( "NPC_Piranha.BiteMiss" );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pevInflictor - 
//			*pAttacker - 
//			flDamage - 
//			bitsDamageType - 
//-----------------------------------------------------------------------------
int	CPiranha::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	//Release the player if he's struck us while being held
	if ( m_flHoldTime > gpGlobals->curtime )
	{
		//ReleaseVictim();
		
		//Don't give them as much time to flee
		m_flNextBiteTime = gpGlobals->curtime + 5.0f; // default : 2.0f

		SetSchedule( SCHED_PIRANHA_THRASH );
	}



	return BaseClass::OnTakeDamage_Alive( info );
}
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//-----------------------------------------------------------------------------
void CPiranha::HandleAnimEvent( animevent_t *pEvent )
{
/* old code
	switch ( pEvent->event )
	{
	case PIRANHA_AE_BITE:
		Bite();
		break;

	case PIRANHA_AE_BITE_START:
		{
			EmitSound( "NPC_Piranha.AttackGrowl" );
		}
		break;
	}
*/

	if ( pEvent->event == PIRANHA_AE_BITE )
	{
		Bite();
	}
	
	if ( pEvent->event == PIRANHA_AE_BITE_START )
	{
		EmitSound( "NPC_Piranha.AttackGrowl" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : type - 
// Output : int
//-----------------------------------------------------------------------------
int CPiranha::TranslateSchedule( int scheduleType )
{
	if ( scheduleType == SCHED_CHASE_ENEMY )	return SCHED_PIRANHA_CHASE_ENEMY;
	if ( scheduleType == SCHED_IDLE_STAND )		return SCHED_PATROL_WALK;
	//if ( scheduleType == SCHED_PATROL_RUN )		return SCHED_PIRANHA_PATROL_RUN;
	//if ( scheduleType == SCHED_PATROL_WALK )	return SCHED_PIRANHA_PATROL_WALK;
	if ( scheduleType == SCHED_MELEE_ATTACK1 )	return SCHED_PIRANHA_MELEE_ATTACK1;

	return BaseClass::TranslateSchedule( scheduleType );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CPiranha::SelectSchedule( void )
{
	BaseClass::SelectSchedule(); // why here ? Red

	if ( m_NPCState == NPC_STATE_COMBAT )
	{
		if ( m_flNextBiteTime > gpGlobals->curtime )
			return	SCHED_PATROL_RUN;

		if ( HasCondition( COND_CAN_MELEE_ATTACK1 ) )
			return	SCHED_MELEE_ATTACK1;

		if ( HasCondition( COND_SEE_ENEMY ) )
			return	SCHED_CHASE_ENEMY;

		return SCHED_CHASE_ENEMY;
	}
	
	return BaseClass::SelectSchedule();
	//return nSchedule;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CPiranha::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_PIRANHA_THRASH_PATH:
		GetNavigator()->SetMovementActivity( (Activity) ACT_PIRANHA_THRASH );
		TaskComplete();
		break;
/*
	case TASK_PIRANHA_GET_PATH_TO_RANDOM_NODE:
		{
			if ( GetEnemy() == NULL || !GetNavigator()->SetRandomGoal( GetEnemy()->GetLocalOrigin(), pTask->flTaskData ) )
			{
				if (!GetNavigator()->SetRandomGoal( pTask->flTaskData ) )
				{
					TaskFail(FAIL_NO_REACHABLE_NODE);
					return;
				}
			}
					
			TaskComplete();
		}
		break;
*/
	case TASK_MELEE_ATTACK1:
		m_flPlaybackRate = 1.0f;
		BaseClass::StartTask(pTask);
		break;

	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

/*
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CPiranha::RunTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	default:
		BaseClass::RunTask(pTask);
		break;
	}
}
*/

//-----------------------------------------------------------------------------
// Purpose: 
// Output : speed to move at
//-----------------------------------------------------------------------------
float CPiranha::GetGroundSpeed( void )
{
	if ( m_flHoldTime > gpGlobals->curtime )
		return	PIRANHA_SWIM_SPEED_WALK/3.5f; //2.0f;

	if ( GetIdealActivity() == ACT_WALK )
		return PIRANHA_SWIM_SPEED_WALK;

	if ( GetIdealActivity() == ACT_PIRANHA_THRASH )
		return PIRANHA_SWIM_SPEED_WALK;

	return PIRANHA_SWIM_SPEED_RUN;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : desired yaw speed
//-----------------------------------------------------------------------------
float CPiranha::MaxYawSpeed( void )
{
	if ( GetIdealActivity() == ACT_MELEE_ATTACK1 )
		return 16.0f;
// disable to test the navigation of the piranha
	if ( GetIdealActivity() == ACT_RUN )
		return 32.0f;

	if ( GetIdealActivity() == ACT_WALK )
		return 16.0f;

	if ( GetIdealActivity() == ACT_PIRANHA_THRASH )
		return 64.0f;
	//Ramp up the yaw speed as we increase our speed
	return PIRANHA_MIN_TURN_SPEED + ( (PIRANHA_MAX_TURN_SPEED-PIRANHA_MIN_TURN_SPEED) * ( fabs(GetAbsVelocity().Length()) / PIRANHA_SWIM_SPEED_RUN ) );
}


//////////////////////////////////////////////////////////////////////////////
///////////////////////         CROCODILE       //////////////////////////////


BEGIN_DATADESC( CCrocodile )

	// melee attack ( piranha+ croco)
	DEFINE_FIELD( m_flNextBiteTime,		FIELD_FLOAT ),
	// Croco code (ensnare, release, drag victim) mixed to the Bite() melee attack
	DEFINE_FIELD( m_pVictim,				FIELD_CLASSPTR ),
	// also in the Bite() class ( only need by the crocodile.)
	DEFINE_FIELD( m_flHoldTime,			FIELD_FLOAT ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Precaches all resources this monster needs.
//-----------------------------------------------------------------------------
void CCrocodile::Precache( void )
{

	PrecacheModel( CROCODILE_MODEL );

	PrecacheScriptSound( "NPC_Crocodile.Idle" );
	PrecacheScriptSound( "NPC_Crocodile.Pain" );
	PrecacheScriptSound( "NPC_Crocodile.Bite" );
	PrecacheScriptSound( "NPC_Crocodile.BiteMiss" );
	PrecacheScriptSound( "NPC_Crocodile.Growl" );
	PrecacheScriptSound( "NPC_Crocodile.AttackGrowl" );
	PrecacheScriptSound( "NPC_Crocodile.Die" );
	
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCrocodile::Spawn( void )
{
	Precache();
	SetModel( CROCODILE_MODEL );

	SetHullType(HULL_LARGE_CENTERED);  // need a custom hull
	SetHullSizeNormal();

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_GROUND /* bits_CAP_MOVE_FLY */ | bits_CAP_INNATE_MELEE_ATTACK2 |  bits_CAP_SKIP_NAV_GROUND_CHECK );

	//m_flFieldOfView			= 0.5; //= 0.5;  //-0.707; // VIEW_FIELD_FULL;
	SetDistLook( 1024 ); // if not set the piranah see the player from far location
//	SetViewOffset( Vector(6, 0, 11) ) ;		// Position of the eyes relative to NPC's origin.

//	BaseClass::Spawn();

	m_iHealth				= sk_crocodile_health.GetFloat();
	m_iMaxHealth			= m_iHealth;


	m_NPCState	=	NPC_STATE_IDLE;
	

	RemoveFlag( FL_FLY );
	SetNavType( NAV_GROUND );
	SetMoveType( MOVETYPE_STEP );
	SetSolid( SOLID_BBOX ); // SOLID_BBOX
	AddSolidFlags( FSOLID_NOT_STANDABLE );

//	SetCollisionGroup( JLCOLLISION_GROUP_FISH ); 

/*	SetNavType( NAV_FLY );
	m_NPCState				= NPC_STATE_NONE;
	SetBloodColor( BLOOD_COLOR_RED );
	
	SetSolid( SOLID_BBOX ); // SOLID_BBOX
	AddSolidFlags( FSOLID_NOT_STANDABLE ); // FSOLID_NOT_STANDABLE
	SetMoveType( MOVETYPE_STEP );
	AddFlag( FL_FLY | FL_STEPMOVEMENT );
*/	

	//CapabilitiesAdd( bits_CAP_MOVE_GROUND );

	if ( m_spawnflags & SF_CROCODILE_IN_WATER )
	{
		SetGroundEntity( NULL );
		//AddFlag( FL_FLY );
		AddFlag( FL_FLY | FL_STEPMOVEMENT );
		SetNavType( NAV_FLY );
		CapabilitiesRemove( bits_CAP_MOVE_GROUND | bits_CAP_INNATE_MELEE_ATTACK2 );
		CapabilitiesAdd( bits_CAP_MOVE_FLY | bits_CAP_INNATE_MELEE_ATTACK1 );
		SetMoveType( MOVETYPE_STEP );
		m_flNextBiteTime		= gpGlobals->curtime;
		m_flHoldTime			= gpGlobals->curtime;
	}
	
	else
	{
		RemoveFlag( FL_FLY | FL_STEPMOVEMENT );
		SetNavType( NAV_GROUND );
		CapabilitiesRemove( bits_CAP_MOVE_FLY | bits_CAP_INNATE_MELEE_ATTACK1 );
		CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_INNATE_MELEE_ATTACK2 );
		SetMoveType( MOVETYPE_STEP );
	}
	
	BaseClass::Spawn();
	NPCInit();


}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCrocodile::IdleSound( void )
{
	EmitSound( "NPC_Crocodile.Idle" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCrocodile::AlertSound( void )
{
	EmitSound( "NPC_Crocodile.Alert" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCrocodile::PainSound( const CTakeDamageInfo &info )
{
	EmitSound( "NPC_Crocodile.Pain" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCrocodile::DeathSound( const CTakeDamageInfo &info )
{
	EmitSound( "NPC_Crocodile.Die" );
}

//-----------------------------------------------------------------------------
// Purpose: Get our conditions for a melee attack
// Input  : flDot - 
//			flDist - 
// Output : int
//-----------------------------------------------------------------------------
int CCrocodile::MeleeAttack1Conditions( float flDot, float flDist )
{
	Vector	predictedDir	= ( (GetEnemy()->GetAbsOrigin()+(GetEnemy()->GetSmoothedVelocity())) - GetAbsOrigin() );	
	float	flPredictedDist = VectorNormalize( predictedDir );
	
	Vector	vBodyDir;
	GetVectors( &vBodyDir, NULL, NULL );

	float	flPredictedDot	= DotProduct( predictedDir, vBodyDir );

	if ( flPredictedDot < 0.8f )
		return COND_NOT_FACING_ATTACK;

	if ( ( flPredictedDist > ( GetAbsVelocity().Length() * 0.5f) ) && ( flDist > 128.0f ) )
		return COND_TOO_FAR_TO_ATTACK;

	return COND_CAN_MELEE_ATTACK1;
}
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//-----------------------------------------------------------------------------
void CCrocodile::HandleAnimEvent( animevent_t *pEvent )
{
/* Old code
	switch ( pEvent->event )
	{
	case CROCODILE_AE_BITE:
		Bite();
		break;

	case CROCODILE_AE_BITE_START:
		{
			EmitSound( "NPC_Crocodile.AttackGrowl" );
		}
		break;
	}
*/
	if ( pEvent->event == CROCODILE_AE_BITE )
	{
		Bite();
	}
	
	if ( pEvent->event == CROCODILE_AE_BITE_START )
	{
		EmitSound( "NPC_Crocodile.AttackGrowl" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : type - 
// Output : int
//-----------------------------------------------------------------------------
int CCrocodile::TranslateSchedule( int scheduleType )
{
	if ( scheduleType == SCHED_CHASE_ENEMY )	
	{
		if ( HasSpawnFlags( SF_CROCODILE_IN_WATER ) == false )
		{
			return SCHED_CROCODILE_CHASE_ENEMY_GROUND;
		}

		if ( HasSpawnFlags( SF_CROCODILE_IN_WATER ) == true )
		{
			return SCHED_CROCODILE_CHASE_ENEMY;
		}
	}

	if ( scheduleType == SCHED_IDLE_STAND )		return SCHED_PATROL_WALK;
	
	if ( scheduleType == SCHED_PATROL_RUN )
	{	
		if ( HasSpawnFlags( SF_CROCODILE_IN_WATER ) == false )
		{
			return SCHED_CROCODILE_PATROL_RUN_GROUND;
		}

		if ( HasSpawnFlags( SF_CROCODILE_IN_WATER ) == true )
		{
			return SCHED_FISH_PATROL_RUN;
		}
	}
	if ( scheduleType == SCHED_PATROL_WALK )
	{	
		if ( HasSpawnFlags( SF_CROCODILE_IN_WATER ) == false )
		{
			return SCHED_CROCODILE_PATROL_WALK_GROUND;
		}

		if ( HasSpawnFlags( SF_CROCODILE_IN_WATER ) == true )
		{
			return SCHED_FISH_PATROL_WALK;
		}
	}
	if ( scheduleType == SCHED_MELEE_ATTACK1 )	return SCHED_CROCODILE_MELEE_ATTACK1;

	return BaseClass::TranslateSchedule( scheduleType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CCrocodile::SelectSchedule( void )
{
	BaseClass::SelectSchedule(); //// THIS IS REALLY NEEED ??!?

	if ( m_NPCState == NPC_STATE_ALERT )
	{
		return SCHED_PATROL_RUN;
	}

	if ( m_NPCState == NPC_STATE_IDLE )
	{
		return SCHED_PATROL_WALK;
	}

	if ( m_NPCState == NPC_STATE_COMBAT )
	{
		if ( m_flHoldTime > gpGlobals->curtime )
			return SCHED_CROCODILE_DROWN_VICTIM;

		if ( m_flNextBiteTime > gpGlobals->curtime )
			return	SCHED_PATROL_RUN;

		if ( HasCondition( COND_CAN_MELEE_ATTACK1 ) )
			return	SCHED_MELEE_ATTACK1;

		return SCHED_CHASE_ENEMY;
	}
	//return nSchedule;
	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CCrocodile::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_CROCODILE_THRASH_PATH:
		GetNavigator()->SetMovementActivity( (Activity) ACT_CROCODILE_THRASH );
		TaskComplete();
		break;

	case TASK_CROCODILE_GET_PATH_TO_DROWN_NODE:
		{
			Vector	drownPos = GetLocalOrigin() - Vector( 0, 0, pTask->flTaskData );

			if ( GetNavigator()->SetGoal( drownPos, AIN_CLEAR_TARGET ) == false )
			{
				TaskFail( FAIL_NO_ROUTE );
				return;
			}

			TaskComplete();
		}
		break;

	case TASK_MELEE_ATTACK1:
		m_flPlaybackRate = 1.0f;
		BaseClass::StartTask(pTask);
		break;

	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CCrocodile::RunTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_CROCODILE_GET_PATH_TO_DROWN_NODE:
		return;
		break;

	default:
		BaseClass::RunTask(pTask);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCrocodile::Bite( void )
{
	//Don't allow another bite too soon
	if ( m_flNextBiteTime > gpGlobals->curtime )
		return;

	CBaseEntity *pHurt;
	
	//FIXME: E3 HACK - Always damage bullseyes if we're scripted to hit them
	if ( ( GetEnemy() != NULL ) && ( GetEnemy()->Classify() == CLASS_BULLSEYE ) )
	{
		pHurt = GetEnemy();
	}
	else
	{
		pHurt = CheckTraceHullAttack( 108, Vector(-8,-8,-8), Vector(8,8,8), 0, DMG_SLASH ); //valeur pardéfaut 32 et -32
	}

	//Hit something
	if ( pHurt != NULL )
	{
		CTakeDamageInfo info( this, this, sk_crocodile_melee_dmg.GetInt(), DMG_SLASH );

		if ( pHurt->IsPlayer() )
		{
			CBasePlayer *pPlayer = ToBasePlayer( pHurt );

			if ( pPlayer )
			{
				if ( ( ( m_flHoldTime < gpGlobals->curtime ) && ( pPlayer->m_iHealth < (pPlayer->m_iMaxHealth*0.5f)) ) || ( pPlayer->GetWaterLevel() < 1 ) )
				{
					info.SetDamage( sk_crocodile_melee_dmg.GetInt() * 3 );
				}
				CalculateMeleeDamageForce( &info, GetAbsVelocity(), pHurt->GetAbsOrigin() );
				pHurt->TakeDamage( info );
	
				color32 red = {32, 0, 0, 255}; //{64, 0, 0, 255}
				UTIL_ScreenFade( pPlayer, red, 0.2, 0, FFADE_IN  );

				//Disorient the player
				QAngle angles = pPlayer->GetLocalAngles();

				angles.x += random->RandomInt( 10, 5 );
				angles.y += random->RandomInt( 10, 5 );
				angles.z = 0.0f;

				pPlayer->SetLocalAngles( angles );

				pPlayer->SnapEyeAngles( angles );
			}
		}
		else
		{
			CalculateMeleeDamageForce( &info, GetAbsVelocity(), pHurt->GetAbsOrigin() );
			pHurt->TakeDamage( info );
		}

		m_flNextBiteTime = gpGlobals->curtime + random->RandomFloat( 2.0f, 4.0f );

		//Bubbles!
		UTIL_Bubbles( pHurt->GetAbsOrigin()+Vector(-8.0f,-8.0f,-8.0f), pHurt->GetAbsOrigin()+Vector(8.0f,8.0f,0.0f), random->RandomInt( 4, 8 ) );
		
		// Play a random attack hit sound
		EmitSound( "NPC_Crocodile.Bite" );

		if ( GetActivity() == ACT_MELEE_ATTACK1 )
		{
			SetActivity( (Activity) ACT_CROCODILE_BITE_HIT );
		}
		
		return;
	}

	//Play the miss animation and sound
	if ( GetActivity() == ACT_MELEE_ATTACK1 )
	{
		SetActivity( (Activity) ACT_CROCODILE_BITE_MISS );
	}

	//Miss sound
	EmitSound( "NPC_Crocodile.BiteMiss" );
}
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pevInflictor - 
//			*pAttacker - 
//			flDamage - 
//			bitsDamageType - 
//-----------------------------------------------------------------------------
int	CCrocodile::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	//Release the player if he's struck us while being held
	if ( m_flHoldTime > gpGlobals->curtime )
	{
		ReleaseVictim();
		
		//Don't give them as much time to flee
		m_flNextBiteTime = gpGlobals->curtime + 5.0f; // default : 2.0f

		SetSchedule( SCHED_CROCODILE_THRASH );
	}
	
	return BaseClass::OnTakeDamage_Alive( info );
}



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCrocodile::EnsnareVictim( CBaseEntity *pVictim )
{
	CBaseCombatCharacter* pBCC = (CBaseCombatCharacter *) pVictim;

	if ( pBCC && pBCC->DispatchInteraction( g_interactionBarnacleVictimGrab, NULL, this ) )
	{
		if ( pVictim->IsPlayer() )
		{
			CBasePlayer	*pPlayer = dynamic_cast< CBasePlayer * >((CBaseEntity *) pVictim);

			if ( pPlayer )
			{
				m_flHoldTime = max( gpGlobals->curtime+3.0f, pPlayer->PlayerDrownTime() - 5.0f ); //2.0f );
			}
		}
		else
		{
			m_flHoldTime	= gpGlobals->curtime + 4.0f;
		}
	
		m_pVictim = pVictim;
		m_pVictim->AddSolidFlags( FSOLID_NOT_SOLID );

		SetSchedule( SCHED_CROCODILE_DROWN_VICTIM );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCrocodile::ReleaseVictim( void )
{
	CBaseCombatCharacter *pBCC = (CBaseCombatCharacter *) m_pVictim;

	pBCC->DispatchInteraction( g_interactionBarnacleVictimReleased, NULL, this );

	m_pVictim->RemoveSolidFlags( FSOLID_NOT_SOLID );

	m_pVictim			= NULL;
	m_flNextBiteTime	= gpGlobals->curtime + 8.0f;
	m_flHoldTime		= gpGlobals->curtime - 0.1f;
}


#define	DRAG_OFFSET	100.0f // 70.0f

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCrocodile::DragVictim( float moveDist )
{
	Vector	mins, maxs;
	float	width;
	
	mins	= WorldAlignMins();
	maxs	= WorldAlignMaxs();
	width	= ( maxs.y - mins.y ) * 0.5f;

	Vector	forward, up;
	GetVectors( &forward, NULL, &up );

	Vector	newPos = GetAbsOrigin() + ( (forward+(up*0.25f)) * ( moveDist + width + DRAG_OFFSET ) );

	trace_t	tr;
	AI_TraceEntity( this, m_pVictim->GetAbsOrigin(), newPos, MASK_NPCSOLID, &tr );

	if ( ( tr.fraction == 1.0f ) && ( tr.m_pEnt != this ) )
	{
		UTIL_SetOrigin( m_pVictim, tr.endpos );
	}
	else
	{
		ReleaseVictim();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : speed to move at
//-----------------------------------------------------------------------------
float CCrocodile::GetGroundSpeed( void )
{
	if ( m_flHoldTime > gpGlobals->curtime )
		return	CROCODILE_SWIM_SPEED_WALK/3.5f; //2.0f;

	if ( GetIdealActivity() == ACT_WALK )
		return CROCODILE_SWIM_SPEED_WALK;

	if ( GetIdealActivity() == ACT_CROCODILE_THRASH )
		return CROCODILE_SWIM_SPEED_WALK;

	return CROCODILE_SWIM_SPEED_RUN;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : desired yaw speed
//-----------------------------------------------------------------------------
float CCrocodile::MaxYawSpeed( void )
{
	if ( GetIdealActivity() == ACT_MELEE_ATTACK1 )
		return 16.0f;
// disable to test the navigation of the piranha
	if ( GetIdealActivity() == ACT_RUN )
		return 64.0f;

	if ( GetIdealActivity() == ACT_WALK )
		return 32.0f;

//	if ( GetIdealActivity() == ACT_FLY )
//		return 50.0f;

	if ( GetIdealActivity() == ACT_CROCODILE_THRASH )
		return 16.0f;
	//Ramp up the yaw speed as we increase our speed
	return CROCODILE_MIN_TURN_SPEED + ( (CROCODILE_MAX_TURN_SPEED-CROCODILE_MIN_TURN_SPEED) * ( fabs(GetAbsVelocity().Length()) / CROCODILE_SWIM_SPEED_RUN ) );
}



LINK_ENTITY_TO_CLASS( npc_brycon, CBrycon );
LINK_ENTITY_TO_CLASS( npc_piranha, CPiranha );
LINK_ENTITY_TO_CLASS( npc_crocodile, CCrocodile );



//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_brycon, CBaseFish )

	DECLARE_TASK( TASK_FISH_GET_PATH_TO_RANDOM_NODE )

	//DECLARE_ACTIVITY( ACT_ )

	//DECLARE_ANIMEVENT( AE_ )


	//==================================================
	// SCHED_FISH_PATROL_WALK
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_FISH_PATROL_WALK,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
		//"		TASK_SET_TOLERANCE_DISTANCE			64"
		"		TASK_SET_ACTIVITY					ACTIVITY:ACT_WALK" // ACT_WALK
		"		TASK_SET_ROUTE_SEARCH_TIME		5"	// Spend 5 seconds trying to build a path if stuck
		"		TASK_FISH_GET_PATH_TO_RANDOM_NODE	175"
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		""
		"	Interrupts"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_GIVE_WAY"
		"		COND_NEW_ENEMY"
		"		COND_SEE_ENEMY"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_SEE_FEAR"
	)

	//==================================================
	// SCHED_FISH_PATROL_RUN
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_FISH_PATROL_RUN,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
		"		TASK_SET_ACTIVITY					ACTIVITY:ACT_RUN" // ACT_WALK
		"		TASK_SET_ROUTE_SEARCH_TIME		5"	// Spend 5 seconds trying to build a path if stuck
		"		TASK_FISH_GET_PATH_TO_RANDOM_NODE	200"
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		""
		"	Interrupts"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_GIVE_WAY"
		"		COND_NEW_ENEMY"
		"		COND_SEE_ENEMY"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
	)

	
	//==================================================
	// SCHED_FISH_FLEE_ENEMY
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_FISH_FLEE_ENEMY,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_FISH_PATROL_WALK"
		"		TASK_SET_ACTIVITY					ACTIVITY:ACT_RUN"
		"		TASK_SET_TOLERANCE_DISTANCE		256"
		//"		TASK_GET_PATH_AWAY_FROM_BEST_SOUND	300"
		"		TASK_FISH_GET_PATH_TO_RANDOM_NODE	300"
		//"		TASK_GET_PATH_TO_GOAL			PATH:TRAVEL"
		//"		TASK_FACE_PATH					0"
		"		TASK_RUN_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
/*
		"		TASK_SET_GOAL					GOAL:ENEMY"
		"		TASK_GET_PATH_TO_GOAL			PATH:TRAVEL"
		"		TASK_RUN_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
*/
		""
		"	Interrupts"
		//"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_UNREACHABLE"
		//"		COND_CAN_MELEE_ATTACK1"
		//"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_LOST_ENEMY"
		"		COND_TASK_FAILED"
	)
	

	//==================================================
	// SCHED_PIRANHA_CHASE_ENEMY
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_PIRANHA_CHASE_ENEMY,

		"	Tasks"
		//"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_PIRANHA_PATROL_WALK"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_CHASE_ENEMY_FAILED"
		"		TASK_SET_TOLERANCE_DISTANCE		256"
		"		TASK_GET_CHASE_PATH_TO_ENEMY	300"
		"		TASK_RUN_PATH					0"
		"		TASK_FACE_ENEMY					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_UNREACHABLE"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_LOST_ENEMY"
		"		COND_TASK_FAILED"
	)
	
	//=========================================================
	// SCHED_PIRANHA_MELEE_ATTACK1
	//=========================================================

	DEFINE_SCHEDULE
	(
		SCHED_PIRANHA_MELEE_ATTACK1,

		"	Tasks"
		"		TASK_ANNOUNCE_ATTACK						1"
		"		TASK_MELEE_ATTACK1							0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_OCCLUDED"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_TOO_FAR_TO_ATTACK"
	)

	//==================================================
	// SCHED_PIRANHA_THRASH
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_PIRANHA_THRASH,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
		"		TASK_SET_TOLERANCE_DISTANCE			512"
		"		TASK_SET_ROUTE_SEARCH_TIME			4"
		"		TASK_FISH_GET_PATH_TO_RANDOM_NODE	200"
		"		TASK_PIRANHA_THRASH_PATH			0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		""
		"	Interrupts"
		"		COND_ENEMY_OCCLUDED"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_TOO_FAR_TO_ATTACK"
	)



//==================================================
// SCHED_CROCODILE_CHASE_ENEMY
//==================================================

	DEFINE_SCHEDULE
	(
	SCHED_CROCODILE_CHASE_ENEMY,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CROCODILE_PATROL_WALK"
		"		TASK_SET_TOLERANCE_DISTANCE		256"
		"		TASK_GET_CHASE_PATH_TO_ENEMY	300"
		"		TASK_RUN_PATH					0"
		"		TASK_FACE_ENEMY					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_UNREACHABLE"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_LOST_ENEMY"
		"		COND_TASK_FAILED"
	)
	
	//=========================================================
	// SCHED_CROCODILE_MELEE_ATTACK1
	//=========================================================

	DEFINE_SCHEDULE
	(
		SCHED_CROCODILE_MELEE_ATTACK1,

		"	Tasks"
		"		TASK_ANNOUNCE_ATTACK						1"
		"		TASK_MELEE_ATTACK1							0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_OCCLUDED"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_TOO_FAR_TO_ATTACK"
	)	
	

	//==================================================
	// SCHED_CROCODILE_THRASH
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_CROCODILE_THRASH,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
		"		TASK_SET_TOLERANCE_DISTANCE			64" //512"
		"		TASK_SET_ROUTE_SEARCH_TIME			4"
		"		TASK_FISH_GET_PATH_TO_RANDOM_NODE		64" //200"
		"		TASK_CROCODILE_THRASH_PATH			0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		""
		"	Interrupts"
	)

	//==================================================
	// SCHED_CROCODILE_DROWN_VICTIM
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_CROCODILE_DROWN_VICTIM,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
		"		TASK_SET_TOLERANCE_DISTANCE			256"
		"		TASK_SET_ROUTE_SEARCH_TIME			1"  // defaultvalue  :4"
		"		TASK_FISH_GET_PATH_TO_DROWN_NODE	64"	//256"
		"		TASK_WALK_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
	)


AI_END_CUSTOM_NPC()


AI_BEGIN_CUSTOM_NPC( npc_piranha, CPiranha )

	DECLARE_TASK( TASK_PIRANHA_THRASH_PATH )
		
	DECLARE_ACTIVITY( ACT_PIRANHA_THRASH )
	DECLARE_ACTIVITY( ACT_PIRANHA_BITE_HIT )
	DECLARE_ACTIVITY( ACT_PIRANHA_BITE_MISS )

	DECLARE_ANIMEVENT( PIRANHA_AE_BITE )
	DECLARE_ANIMEVENT( PIRANHA_AE_BITE_START )


/*
	//==================================================
	// SCHED_PIRANHA_CHASE_ENEMY
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_PIRANHA_CHASE_ENEMY,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_PIRANHA_PATROL_WALK"
		"		TASK_SET_TOLERANCE_DISTANCE		256"
		"		TASK_GET_CHASE_PATH_TO_ENEMY	300"
		"		TASK_RUN_PATH					0"
		"		TASK_FACE_ENEMY					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_UNREACHABLE"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_LOST_ENEMY"
		"		COND_TASK_FAILED"
	)
	
	//=========================================================
	// SCHED_PIRANHA_MELEE_ATTACK1
	//=========================================================

	DEFINE_SCHEDULE
	(
		SCHED_PIRANHA_MELEE_ATTACK1,

		"	Tasks"
		"		TASK_ANNOUNCE_ATTACK						1"
		"		TASK_MELEE_ATTACK1							0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_OCCLUDED"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_TOO_FAR_TO_ATTACK"
	)

	//==================================================
	// SCHED_PIRANHA_THRASH
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_PIRANHA_THRASH,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
		"		TASK_SET_TOLERANCE_DISTANCE			64" //512"
		"		TASK_SET_ROUTE_SEARCH_TIME			4"
		"		TASK_GET_PATH_TO_RANDOM_NODE		64" //200"
		"		TASK_PIRANHA_THRASH_PATH			0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		""
		"	Interrupts"
	)
	
*/
/*
//==================================================
// SCHED_CROCODILE_CHASE_ENEMY
//==================================================

	DEFINE_SCHEDULE
	(
	SCHED_CROCODILE_CHASE_ENEMY,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CROCODILE_PATROL_WALK"
		"		TASK_SET_TOLERANCE_DISTANCE		256"
		"		TASK_GET_CHASE_PATH_TO_ENEMY	300"
		"		TASK_RUN_PATH					0"
		"		TASK_FACE_ENEMY					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_UNREACHABLE"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_LOST_ENEMY"
		"		COND_TASK_FAILED"
	)

	//=========================================================
	// SCHED_CROCODILE_MELEE_ATTACK1
	//=========================================================

	DEFINE_SCHEDULE
	(
		SCHED_CROCODILE_MELEE_ATTACK1,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
		"		TASK_ANNOUNCE_ATTACK						1"
		"		TASK_MELEE_ATTACK1							1"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_OCCLUDED"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_TOO_FAR_TO_ATTACK"
	)	
	

	//==================================================
	// SCHED_CROCODILE_THRASH
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_CROCODILE_THRASH,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
		"		TASK_SET_TOLERANCE_DISTANCE			64" //512"
		"		TASK_SET_ROUTE_SEARCH_TIME			4"
		"		TASK_CROCODILE_GET_PATH_TO_RANDOM_NODE		64" //200"
		"		TASK_CROCODILE_THRASH_PATH			0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		""
		"	Interrupts"
	)

	//==================================================
	// SCHED_CROCODILE_DROWN_VICTIM
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_CROCODILE_DROWN_VICTIM,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
		"		TASK_SET_TOLERANCE_DISTANCE			256"
		"		TASK_SET_ROUTE_SEARCH_TIME			1"  // defaultvalue  :4"
		"		TASK_CROCODILE_GET_PATH_TO_DROWN_NODE	64"	//256"
		"		TASK_WALK_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
	)

		//==================================================
	// SCHED_FISH_PATROL_WALK
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_CROCODILE_PATROL_WALK_GROUND,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
		"		TASK_SET_ACTIVITY					ACTIVITY:ACT_WALK"
		"		TASK_SET_ROUTE_SEARCH_TIME		5"	// Spend 5 seconds trying to build a path if stuck
		"		TASK_GET_PATH_TO_RANDOM_NODE	175"
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		""
		"	Interrupts"
		"		COND_CAN_MELEE_ATTACK2"
		"		COND_GIVE_WAY"
		"		COND_NEW_ENEMY"
		"		COND_SEE_ENEMY"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_SEE_FEAR"
	)

	//==================================================
	// SCHED_FISH_PATROL_RUN
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_CROCODILE_PATROL_RUN_GROUND,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
		"		TASK_SET_ACTIVITY					ACTIVITY:ACT_RUN"
		"		TASK_SET_ROUTE_SEARCH_TIME		5"	// Spend 5 seconds trying to build a path if stuck
		"		TASK_FISH_GET_PATH_TO_RANDOM_NODE	200"
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		""
		"	Interrupts"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_GIVE_WAY"
		"		COND_NEW_ENEMY"
		"		COND_SEE_ENEMY"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
	)

//==================================================
// SCHED_CROCODILE_CHASE_ENEMY_GROUND
//==================================================

	DEFINE_SCHEDULE
	(
	SCHED_CROCODILE_CHASE_ENEMY_GROUND,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CROCODILE_PATROL_WALK_GROUND"
		"		TASK_SET_TOLERANCE_DISTANCE		256"
		"		TASK_GET_CHASE_PATH_TO_ENEMY	300"
		"		TASK_RUN_PATH					0"
		"		TASK_FACE_ENEMY					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_UNREACHABLE"
		"		COND_CAN_MELEE_ATTACK2"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_LOST_ENEMY"
		"		COND_TASK_FAILED"
	)

*/
AI_END_CUSTOM_NPC()



AI_BEGIN_CUSTOM_NPC( npc_crocodile, CCrocodile )

	DECLARE_TASK( TASK_CROCODILE_GET_PATH_TO_DROWN_NODE )
	DECLARE_TASK( TASK_CROCODILE_THRASH_PATH )
//	DECLARE_TASK( TASK_CROCODILE_GET_PATH_TO_RANDOM_NODE )
	
	DECLARE_ACTIVITY( ACT_CROCODILE_THRASH )
	DECLARE_ACTIVITY( ACT_CROCODILE_BITE_HIT )
	DECLARE_ACTIVITY( ACT_CROCODILE_BITE_MISS )

	DECLARE_ANIMEVENT( CROCODILE_AE_BITE )
	DECLARE_ANIMEVENT( CROCODILE_AE_BITE_START )
/*
//==================================================
// SCHED_CROCODILE_CHASE_ENEMY
//==================================================

	DEFINE_SCHEDULE
	(
	SCHED_CROCODILE_CHASE_ENEMY,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CROCODILE_PATROL_WALK"
		"		TASK_SET_TOLERANCE_DISTANCE		256"
		"		TASK_GET_CHASE_PATH_TO_ENEMY	300"
		"		TASK_RUN_PATH					0"
		"		TASK_FACE_ENEMY					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_UNREACHABLE"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_LOST_ENEMY"
		"		COND_TASK_FAILED"
	)

	//=========================================================
	// SCHED_CROCODILE_MELEE_ATTACK1
	//=========================================================

	DEFINE_SCHEDULE
	(
		SCHED_CROCODILE_MELEE_ATTACK1,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
		"		TASK_ANNOUNCE_ATTACK						1"
		"		TASK_MELEE_ATTACK1							1"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_OCCLUDED"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_TOO_FAR_TO_ATTACK"
	)	
	

	//==================================================
	// SCHED_CROCODILE_THRASH
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_CROCODILE_THRASH,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
		"		TASK_SET_TOLERANCE_DISTANCE			64" //512"
		"		TASK_SET_ROUTE_SEARCH_TIME			4"
		"		TASK_CROCODILE_GET_PATH_TO_RANDOM_NODE		64" //200"
		"		TASK_CROCODILE_THRASH_PATH			0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		""
		"	Interrupts"
	)

	//==================================================
	// SCHED_CROCODILE_DROWN_VICTIM
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_CROCODILE_DROWN_VICTIM,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
		"		TASK_SET_TOLERANCE_DISTANCE			256"
		"		TASK_SET_ROUTE_SEARCH_TIME			1"  // defaultvalue  :4"
		"		TASK_CROCODILE_GET_PATH_TO_DROWN_NODE	64"	//256"
		"		TASK_WALK_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
	)

		//==================================================
	// SCHED_CROCODILE_PATROL_WALK
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_CROCODILE_PATROL_WALK_GROUND,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
		"		TASK_SET_ACTIVITY					ACTIVITY:ACT_WALK"
		"		TASK_SET_ROUTE_SEARCH_TIME		5"	// Spend 5 seconds trying to build a path if stuck
		"		TASK_GET_PATH_TO_RANDOM_NODE	175"
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		""
		"	Interrupts"
		"		COND_CAN_MELEE_ATTACK2"
		"		COND_GIVE_WAY"
		"		COND_NEW_ENEMY"
		"		COND_SEE_ENEMY"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_SEE_FEAR"
	)

	//==================================================
	// SCHED_CROCODILE_PATROL_RUN
	//==================================================

	DEFINE_SCHEDULE
	(
		SCHED_CROCODILE_PATROL_RUN_GROUND,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
		"		TASK_SET_ACTIVITY					ACTIVITY:ACT_RUN"
		"		TASK_SET_ROUTE_SEARCH_TIME		5"	// Spend 5 seconds trying to build a path if stuck
		"		TASK_FISH_GET_PATH_TO_RANDOM_NODE	200"
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		""
		"	Interrupts"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_GIVE_WAY"
		"		COND_NEW_ENEMY"
		"		COND_SEE_ENEMY"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
	)

//==================================================
// SCHED_CROCODILE_CHASE_ENEMY_GROUND
//==================================================

	DEFINE_SCHEDULE
	(
	SCHED_CROCODILE_CHASE_ENEMY_GROUND,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CROCODILE_PATROL_WALK_GROUND"
		"		TASK_SET_TOLERANCE_DISTANCE		256"
		"		TASK_GET_CHASE_PATH_TO_ENEMY	300"
		"		TASK_RUN_PATH					0"
		"		TASK_FACE_ENEMY					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_UNREACHABLE"
		"		COND_CAN_MELEE_ATTACK2"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_LOST_ENEMY"
		"		COND_TASK_FAILED"
	)
*/
AI_END_CUSTOM_NPC()