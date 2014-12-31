//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//
// fish.cpp
// Simple fish behavior
// Author: Michael S. Booth, April 2005

#include "cbase.h"
#include "baseanimating.h"
#include "flock_fauna.h"
#include "saverestore_utlvector.h"
#include "ai_activity.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar fauna_dormant( "fauna_dormant", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Turns off interactive fauna behavior. Creature become immobile and unresponsive." );


//-----------------------------------------------------------------------------------------------------
LINK_ENTITY_TO_CLASS( fauna, CFlockFauna );


//-----------------------------------------------------------------------------------------------------
BEGIN_DATADESC( CFlockFauna )
	DEFINE_FIELD( m_pool, FIELD_EHANDLE ),
	DEFINE_FIELD( m_id, FIELD_INTEGER ),
	DEFINE_FIELD( m_angle, FIELD_FLOAT ),
	DEFINE_FIELD( m_angleChange, FIELD_FLOAT ),
	DEFINE_FIELD( m_forward, FIELD_VECTOR ),
	DEFINE_FIELD( m_perp, FIELD_VECTOR ),
	DEFINE_FIELD( m_poolOrigin, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_waterLevel, FIELD_FLOAT ),
	DEFINE_FIELD( m_speed, FIELD_FLOAT ),
	DEFINE_FIELD( m_desiredSpeed, FIELD_FLOAT ),
	
//	DEFINE_FIELD( m_faunaSpeed, FIELD_FLOAT ),
	
	
	DEFINE_FIELD( m_calmSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( m_panicSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( m_avoidRange, FIELD_FLOAT ),
	DEFINE_FIELD( m_turnClockwise, FIELD_BOOLEAN ),
	
	//DEFINE_KEYFIELD( m_iszDefaultAnim, FIELD_STRING, "DefaultAnim"),
	//DEFINE_INPUTFUNC( FIELD_STRING,	"SetDefaultAnimation",	InputSetDefaultAnimation ),
END_DATADESC()


//-----------------------------------------------------------------------------------------------------
/**
 * Send creature position relative to pool origin
 */
void SendProxy_FaunaOriginX( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CFlockFauna *fauna = (CFlockFauna *)pStruct;
	Assert( fauna );

	const Vector &v = fauna->GetAbsOrigin();
	Vector origin = fauna->m_poolOrigin;

	pOut->m_Float = v.x - origin.x;
}

void SendProxy_FaunaOriginY( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CFlockFauna *fauna = (CFlockFauna *)pStruct;
	Assert( fauna );

	const Vector &v = fauna->GetAbsOrigin();
	Vector origin = fauna->m_poolOrigin;

	pOut->m_Float = v.y - origin.y;
}

void SendProxy_FaunaOriginZ( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CFlockFauna *fauna = (CFlockFauna *)pStruct;
	Assert( fauna );

	const Vector &v = fauna->GetAbsOrigin();
	Vector origin = fauna->m_poolOrigin;

	pOut->m_Float = v.z - origin.z;
}


// keep angle in normalized range when sending it
void SendProxy_FaunaAngle( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	float value = *((float *)pData);

	while( value > 360.0f )
		value -= 360.0f;

	while (value < 0.0f)
		value += 360.0f;

	pOut->m_Float = value;
}


/**
 * NOTE: Do NOT use SPROP_CHANGES_OFTEN, as it will reorder this list.
 * The pool origin must arrive befoore m_x and m_y or the fish will
 * respawn at the origin and zip back to their proper places.
 */
IMPLEMENT_SERVERCLASS_ST_NOBASE( CFlockFauna, DT_CFlockFauna )

	SendPropVector( SENDINFO(m_poolOrigin), -1, SPROP_COORD, 0.0f, HIGH_DEFAULT ),	// only sent once

	//SendPropFloat( SENDINFO(m_angle), 7, 0 /*SPROP_CHANGES_OFTEN*/, 0.0f, 360.0f, SendProxy_FishAngle ),
	SendPropFloat( SENDINFO(m_angle), 7, 0 /*SPROP_CHANGES_OFTEN*/, 0.0f, 360.0f, SendProxy_FaunaAngle ), //red
	
	//SendPropFloat( SENDINFO(m_x), 7, 0 /*SPROP_CHANGES_OFTEN*/, -255.0f, 255.0f ),
	//SendPropFloat( SENDINFO(m_y), 7, 0 /*SPROP_CHANGES_OFTEN*/, -255.0f, 255.0f ),
	//SendPropFloat( SENDINFO(m_z), -1, SPROP_COORD ),								// only sent once
	
	SendPropFloat( SENDINFO(m_x), 7, 0 /*SPROP_CHANGES_OFTEN*/, -512.0f, 512.0f ), //red
	SendPropFloat( SENDINFO(m_y), 7, 0 /*SPROP_CHANGES_OFTEN*/, -512.0f, 512.0f ), //red
	SendPropFloat( SENDINFO(m_z), 7, 0, -512.0f, 512.0f ),								// only sent once //red
	//SendPropFloat( SENDINFO(m_z), -1, 10 ),								// only sent once //red

	SendPropModelIndex( SENDINFO(m_nModelIndex) ),
	SendPropInt( SENDINFO(m_lifeState) ),

	SendPropFloat( SENDINFO(m_waterLevel) ),										// only sent once //red

END_SEND_TABLE()



//-------------------------------------------------------------------------------------------------------------
CFlockFauna::CFlockFauna( void )
{
}


//-------------------------------------------------------------------------------------------------------------
CFlockFauna::~CFlockFauna()
{
}


//-------------------------------------------------------------------------------------------------------------
void CFlockFauna::Initialize( CFlockPool *pool, unsigned int id )
{
	m_pool = pool;
	m_id = id;

	m_poolOrigin = pool->GetAbsOrigin();
//	m_waterLevel = pool->GetWaterLevel();  //red

	// pass relative position to the client
	Vector deltaPos = GetAbsOrigin() - m_poolOrigin;
	m_x = deltaPos.x;
	m_y = deltaPos.y;
	m_z = deltaPos.z; //red
	//m_z = m_poolOrigin->z;
	
	//m_calmSpeed = RandomFloat( 10.0f, 20.0f );
	//m_calmSpeed = m_fSpeed; // I finally think than i don't need this speed value here
	//m_fSpeed = m_fSpeed;
	//m_calmSpeed = fSpeed; 
	//m_faunaSpeed = CFlockPool::GetFaunaSpeed( *faunaSpeed );
	//m_fSpeed = fSpeed;
	//fSpeed->GetFaunaSpeed();
	//m_calmSpeed = m_fSpeed; //m_fSpeed; 
	/*
	if ( m_bSeagull == true )
	{
		m_calmSpeed= 175.0f;
		m_panicSpeed = m_calmSpeed * RandomFloat( 0.3f, 0.7f );
		m_desiredSpeed = m_calmSpeed;
		m_turnClockwise = (RandomInt( 0, 100 ) < 50);
		m_avoidRange = RandomFloat( 40.0f, 75.0f );
		m_iHealth = 1;
		m_iMaxHealth = 1;
		m_takedamage = DAMAGE_NO;
	}
	if ( m_bCrow == true )
	{
		m_calmSpeed= 200.0f;
		m_panicSpeed = m_calmSpeed * RandomFloat( 0.3f, 0.7f );
		m_desiredSpeed = m_calmSpeed;
		m_turnClockwise = (RandomInt( 0, 100 ) < 50);
		m_avoidRange = RandomFloat( 40.0f, 75.0f );
		m_iHealth = 1;
		m_iMaxHealth = 1;
		m_takedamage = DAMAGE_NO;
	}
	if ( m_bButterfly == true )
	{
		m_calmSpeed= 50.0f;
		m_panicSpeed = m_calmSpeed * RandomFloat( 0.3f, 0.7f );
		m_desiredSpeed = m_calmSpeed;
		m_turnClockwise = (RandomInt( 0, 100 ) < 50);
		m_avoidRange = RandomFloat( 40.0f, 75.0f );
		m_iHealth = 1;
		m_iMaxHealth = 1;
		m_takedamage = DAMAGE_NO;
	}
	else
	{
		return;
	}
	*/

	SetModel( pool->GetModelName().ToCStr() );
}
/*	//-------------------------------------------------------------------------------------------------------------
bool CFlockFauna::IsSeagull( void )
{
	if ( Q_strcmp( STRING( GetModelName() ), "models/seagull.mdl" ) == 1 )
	{
		return m_bSeagull;
	}
	else
	{
		return false;
	}
}


//-------------------------------------------------------------------------------------------------------------
bool CFlockFauna::IsCrow( void )
{
//	if ( Q_strstr( GetModelName(), "models/animals/" ) )
	if ( Q_strcmp( STRING( GetModelName() ), "models/crow.mdl" ) == 1 )
	{
		return m_bCrow;
	}
	else
	{
		return false;
	}
}


//-------------------------------------------------------------------------------------------------------------
bool CFlockFauna::IsButterfly( void )
{
	if ( Q_strcmp( STRING( GetModelName() ), "models/jl/animals/butterfly/blue_morpho.mdl" ) == 1 )
	{
		return m_bButterfly;
	}
	else
	{
		return false;
	}
}
*/

#define CROW_MODEL "models/crow.mdl"
#define SEAGULL_MODEL "model/seagull.mdl"
#define BUTTERFLY_MODEL "models/jl/animals/blue_morpho.mdl"

//------------------------------------
// Spawnflags
//------------------------------------
#define SF_SEAGULL_MODEL		(1 << 16)
#define SF_CROW_MODEL		(1 << 17)
#define	SF_BUTTERFLY_MODEL	( 1 << 18 )

//-------------------------------------------------------------------------------------------------------------
void CFlockFauna::Precache( void )
{
	PrecacheModel( SEAGULL_MODEL );
	PrecacheModel( CROW_MODEL );
	PrecacheModel( BUTTERFLY_MODEL );
	
/*
	if ( m_spawnflags & SF_SEAGULL_MODEL )
	{
		PrecacheModel( SEAGULL_MODEL );	
	}
	if ( m_spawnflags & SF_CROW_MODEL )
	{
		PrecacheModel( CROW_MODEL );	
	}
	if ( m_spawnflags & SF_BUTTERFLY_MODEL )
	{
		PrecacheModel( BUTTERFLY_MODEL );	
	}
	else
	{
		PrecacheModel (SEAGULL_MODEL );
	}
*/
	BaseClass::Precache();
}
//-------------------------------------------------------------------------------------------------------------
void CFlockFauna::Spawn( void )
{
	Precache();

	SetSolid( SOLID_BBOX ); //default : BBOX
	AddSolidFlags( FSOLID_NOT_STANDABLE | FSOLID_NOT_SOLID | FSOLID_TRIGGER );
	SetMoveType( MOVETYPE_FLY );

	
	m_angle = RandomFloat( 0.0f, 360.0f ); //red
	//m_angle = RandomFloat( 0.0f, 360.0f );
	m_angleChange = 0.0f;

	m_forward = Vector( 1.0f, 0.0, 0.0f );
	m_perp.x = m_forward.y;
	m_perp.y = m_forward.x;
	//m_perp.z = RandomFloat( 40.0f, 75.0f ); // 0.0f; //red m_forward.z; 
	m_perp.z = m_forward.z; //RandomFloat( 40.0f, 75.0f );

	m_speed = 0.0f;

	//m_calmSpeed = m_fSpeed;
	m_calmSpeed = 175.0f; // RandomFloat( 25.0f, 220.0f ); 
	m_panicSpeed = m_calmSpeed * RandomFloat( 0.3f, 0.7f );
	m_desiredSpeed = m_calmSpeed;

	m_turnClockwise = (RandomInt( 0, 100 ) < 50);

	m_avoidRange = RandomFloat( 40.0f, 75.0f );

	m_iHealth = 1;
	m_iMaxHealth = 1;
	m_takedamage = DAMAGE_NO;

	// spread out a bit
	m_disperseTimer.Start( RandomFloat( 0.0f, 10.0f ) );
	m_goTimer.Start( RandomFloat( 10.0f, 60.0f ) );
	m_moveTimer.Start( RandomFloat( 2.0f, 10.0 ) );
	m_desiredSpeed = m_calmSpeed;

	

	if ( m_bSeagull )
	{
		m_calmSpeed= 175.0f;
		m_panicSpeed = m_calmSpeed * RandomFloat( 0.3f, 0.7f );
		m_desiredSpeed = m_calmSpeed;
		m_turnClockwise = (RandomInt( 0, 100 ) < 50);
		m_avoidRange = RandomFloat( 40.0f, 75.0f );
		m_iHealth = 1;
		m_iMaxHealth = 1;
		m_takedamage = DAMAGE_NO;
	}
	if ( m_bCrow )
	{
		m_calmSpeed= 200.0f;
		m_panicSpeed = m_calmSpeed * RandomFloat( 0.3f, 0.7f );
		m_desiredSpeed = m_calmSpeed;
		m_turnClockwise = (RandomInt( 0, 100 ) < 50);
		m_avoidRange = RandomFloat( 40.0f, 75.0f );
		m_iHealth = 1;
		m_iMaxHealth = 1;
		m_takedamage = DAMAGE_NO;
	}
	if ( m_bButterfly )
	{
		m_calmSpeed= 50.0f;
		m_panicSpeed = m_calmSpeed * RandomFloat( 0.3f, 0.7f );
		m_desiredSpeed = m_calmSpeed;
		m_turnClockwise = (RandomInt( 0, 100 ) < 50);
		m_avoidRange = RandomFloat( 40.0f, 75.0f );
		m_iHealth = 1;
		m_iMaxHealth = 1;
		m_takedamage = DAMAGE_NO;
	}
}


//-------------------------------------------------------------------------------------------------------------
void CFlockFauna::Event_Killed( const CTakeDamageInfo &info )
{
	m_takedamage = DAMAGE_NO;
	m_lifeState = LIFE_DEAD;
}


//-------------------------------------------------------------------------------------------------------------
/**
 * In contact with "other"
 */

void CFlockFauna::Touch( CBaseEntity *other )
{
	if (other && other->IsPlayer())
	{
		// touched a Player - panic!
		Panic();
	}
}


//-------------------------------------------------------------------------------------------------------------
/**
 * Influence my motion to flock with other nearby creature
 * 'amount' ranges from zero to one, representing the amount of flocking influence allowed
 * If 'other' is NULL, flock to the center of the pool.
 */
void CFlockFauna::FlockTo( CFlockFauna *other, float amount )
{
	// allow creatures to disperse a bit at round start
	if (!m_disperseTimer.IsElapsed())
		return;

	//const float maxRange = (other) ? 100.0f : 300.0f;
	const float maxRange = (other) ? 300.0f : 900.0f; //red

	Vector to = (other) ? (other->GetAbsOrigin() - GetAbsOrigin()) : (m_pool->GetAbsOrigin() - GetAbsOrigin());
	float range = to.NormalizeInPlace();

	if (range > maxRange)
		return;

	// if they are close and we are moving together, avoid them
	//const float avoidRange = 25.0f;
	const float avoidRange = 150.0f; //Red
	if (other && range < avoidRange)
	{
		// compute their relative velocity to us
		Vector relVel = other->GetAbsVelocity() - GetAbsVelocity();

		if (DotProduct( to, relVel ) < 0.0f)
		{
			const float avoidPower = 5.0f;

			// their comin' right at us! - avoid
			if (DotProduct( m_perp, to ) > 0.0f)
			{
				m_angleChange -= avoidPower * (1.0f - range/avoidRange);
			}
			else
			{
				m_angleChange += avoidPower * (1.0f - range/avoidRange);
			}
			return;
		}
	}

	// turn is 2 if 'other' is behind us, 1 perpendicular, and 0 straight ahead
	float turn = 1.0f + DotProduct( -m_forward, to );

	//Vector perp( -m_forward.y, m_forward.x, 0.0f );
	Vector perp( -m_forward.y, m_forward.x, m_forward.z ); // red
	float side = (DotProduct( perp, to ) > 1.0f) ? 1.0f : -1.0f;

	if (turn > 1.0f)
	{
		// always turn one way to avoid dithering if many creatures are behind us
		side = (m_turnClockwise) ? 1.0f : -1.0f;
	}

	float power = 1.0f - (range / maxRange);

	const float flockInfluence = 1.0f; // 0.7f; // 0.3f;	// 0.3
	m_angleChange += amount * flockInfluence * power * side * turn;
}


//-------------------------------------------------------------------------------------------------------------
/**
 * Returns a value between zero (no danger of hitting an obstacle)
 * and one (extreme danger of hitting an obstacle).
 * This is used to modulate later flocking behaviors.
 */
float CFlockFauna::Avoid( void )
{
	const float avoidPower = 75.0f; // 50.0f; // 25.0f;

	// 
	// Stay within pool bounds.
	// This may cause problems with pools with oddly concave portions
	// right at the max range.
	//
	Vector toCenter = m_pool->GetAbsOrigin() - GetAbsOrigin();
	const float avoidZone = 20.0f;
	if (toCenter.IsLengthGreaterThan( m_pool->GetMaxRange() - avoidZone ))
	{
		// turn away from edge
		if (DotProduct( toCenter, m_forward ) < 0.0f)
		{
			m_angleChange += (m_turnClockwise) ? -avoidPower : avoidPower;
		}

		// take total precedence over flocking
		return 1.0f;
	}

	trace_t result;
	const float sideOffset = 0.2f;

	float rightDanger = 0.0f;
	float leftDanger = 0.0f;

	// slightly right of forward
	UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + m_avoidRange * (m_forward + sideOffset * m_perp), MASK_PLAYERSOLID, this, COLLISION_GROUP_NONE, &result );
	if (result.fraction < 1.0f)
	{
		rightDanger = 1.0f - result.fraction;
	}

	// slightly left of forward
	UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + m_avoidRange * (m_forward - sideOffset * m_perp), MASK_PLAYERSOLID, this, COLLISION_GROUP_NONE, &result );
	if (result.fraction < 1.0f)
	{
		// steer away
		leftDanger = 1.0f - result.fraction;
	}

	// steer away - prefer one side to avoid cul-de-sacs
	if (m_turnClockwise)
	{
		if (rightDanger > 0.0f)
		{
			m_angleChange -= avoidPower * rightDanger;
		}
		else
		{
			m_angleChange += avoidPower * leftDanger;
		}
	}
	else
	{
		if (leftDanger > 0.0f)
		{
			m_angleChange += avoidPower * leftDanger;
		}
		else
		{
			m_angleChange -= avoidPower * rightDanger;
		}
	}


	return (leftDanger > rightDanger) ? leftDanger : rightDanger;
}


//-------------------------------------------------------------------------------------------------------------
void CFlockFauna::Panic( void )
{
	// start to panic
	m_panicTimer.Start( RandomFloat( 5.0f, 15.0f ) );
	m_moveTimer.Start( RandomFloat( 10.0f, 20.0f ) );
	m_desiredSpeed = m_panicSpeed;
}


//-------------------------------------------------------------------------------------------------------------
/**
 * Invoked each server tick
 */
void CFlockFauna::Update( float deltaT )
{
	Vector deltaPos = GetAbsOrigin() - m_poolOrigin;
	const float safetyMargin = 5.0f;

	// pass relative position to the client
	// clamp them here to cover the rare cases where a creature's high velocity skirts the range limit
//	m_x = clamp( deltaPos.x, -255.0f, 255.0f );
//	m_y = clamp( deltaPos.y, -255.0f, 255.0f );
//	m_z = m_poolOrigin->z;

	//red
	m_x = clamp( deltaPos.x, -1024.0f, 1024.0f );
	m_y = clamp( deltaPos.y, -1024.0f, 1024.0f );
	m_z = clamp( deltaPos.z, -1024.0f, 1024.0f ); //red

	//AnimThink();
	GetSequence();

	//SetSequence( ACT_FLY );
	SelectWeightedSequence(Activity (ACT_FLY) );
	//LookupSequence( "fly" );
	SetCycle( random->RandomFloat( 0.5f, 2.5f ) );
	
	//
	// Dead creature just coast to a stop.  All floating to the
	// surface and bobbing motion is handled client-side.
	//
	if (m_lifeState == LIFE_DEAD)
	{
		// don't allow creatures to leave maximum range of pool
		if (deltaPos.IsLengthGreaterThan( m_pool->GetMaxRange() - safetyMargin ))
		{
			SetAbsVelocity( Vector( 0, 0, 0 ) );
		}
		else
		{
			// decay movement speed to zero
			Vector vel = GetAbsVelocity();

			const float drag = 1.0f;
			vel -= drag * vel * deltaT;

			SetAbsVelocity( vel );
	
			
			GetSequence();
			//SetSequence( ACT_DIERAGDOLL );
			SelectWeightedSequence( (Activity)ACT_DIERAGDOLL );
		}

		return;
	}


	//
	// Living creatures behavior
	//

	// periodically change our turning preference
	if (m_turnTimer.IsElapsed())
	{
		m_turnTimer.Start( RandomFloat( 5.0f, 10.0f ) );
		//m_turnTimer.Start( RandomFloat( 10.0f, 30.0f ) );
		m_turnClockwise = !m_turnClockwise;
	}

	if (m_panicTimer.GetRemainingTime() > 0.0f)
	{
		// panicking
		m_desiredSpeed = m_panicSpeed;
	}
	else if (m_moveTimer.GetRemainingTime() > 0.0f)
	{
		// normal movement
		m_desiredSpeed = m_calmSpeed;
	}
	else if (m_goTimer.IsElapsed())
	{
		// move every so often
		m_goTimer.Start( RandomFloat( 10.0f, 60.0f ) );
		m_moveTimer.Start( RandomFloat( 2.0f, 10.0 ) );
		m_desiredSpeed = m_calmSpeed;
	}

	// avoid obstacles
	float danger = Avoid();

	// flock towards visible creature
	for( int i=0; i<m_visible.Count(); ++i )
	{
		FlockTo( m_visible[i], (1.0f - danger) );
	}

	// flock towards center of pool
	FlockTo( NULL, (1.0f - danger) );

	
	//
	// Update orientation
	//

	// limit rate of angular change - proportional to movement rate
	//const float maxAngleChange = (25.0f + 175.0f * (m_speed/m_panicSpeed)) * deltaT;
	const float maxAngleChange = (345.0f + 375.0f * (m_speed/m_panicSpeed)) * deltaT; //red
	if (m_angleChange > maxAngleChange)
	{
		m_angleChange = maxAngleChange;
	}
	else if (m_angleChange < -maxAngleChange)
	{
		m_angleChange = -maxAngleChange;
	}

	m_angle += m_angleChange;
	m_angleChange = 0.0f;

	//m_forward.x = cos( m_angle * M_PI/180.0f );

	m_forward.x = cos( m_angle * M_PI/180.0f );
	m_forward.y = sin( m_angle * M_PI/180.0f );
	m_forward.z = cos( m_angle * M_PI/180.0f ); //0.0f;  //red


	m_perp.x = -m_forward.x;
	m_perp.y = m_forward.y;
	m_perp.z = m_forward.z; //0.0f;
	// look strange ! red
	//m_perp.x = -m_forward.y;
	//m_perp.y = m_forward.x;
	//m_perp.z = m_forward.z; //0.0f;

	//
	// Update speed
	//
	const float rate = 1.0f; //2.0f;
	m_speed += rate * (m_desiredSpeed - m_speed) * deltaT;

	// decay desired speed if done moving
	if (m_moveTimer.IsElapsed())
	{
		const float decayRate = 1.0f; // 1.0f;
		m_desiredSpeed -= decayRate * deltaT;
		if (m_desiredSpeed < 0.0f)
		{
			m_desiredSpeed = 0.0f;
		}
	}

	Vector vel = m_speed * m_forward;

	// don't allow creatures to leave maximum range of pool

	
	if (deltaPos.IsLengthGreaterThan( m_pool->GetMaxRange() - safetyMargin ))
	{
		Vector toCenter = -deltaPos;

		float radial = DotProduct( toCenter, vel );
		if (radial < 0.0f)
		{
			// heading out of range, zero the radial velocity component
			toCenter.NormalizeInPlace();
			Vector perp( -toCenter.y, toCenter.x, toCenter.z ); // 0.0f ); jl (Red)

			float side = DotProduct( perp, vel );

			vel = side * perp;
		}
	}

	SetAbsVelocity( vel );

	m_flSpeed = m_speed;
}


//-------------------------------------------------------------------------------------------------------------
/**
 * Zero the visible vector
 */
void CFlockFauna::ResetVisible( void )
{
	m_visible.RemoveAll();
}


//-------------------------------------------------------------------------------------------------------------
/**
 * Add this fish to our visible vector
 */
void CFlockFauna::AddVisible( CFlockFauna *fauna )
{
	m_visible.AddToTail( fauna );
}


//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
/**
 * A CFishPool manages a collection of CFish, and defines where the "pool" is in the world.
 */

LINK_ENTITY_TO_CLASS( func_fauna_pool, CFlockPool );

BEGIN_DATADESC( CFlockPool )

	DEFINE_FIELD( m_faunaCount, FIELD_INTEGER ),
	DEFINE_FIELD( m_maxRange, FIELD_FLOAT ),
	DEFINE_FIELD( m_airDepth, FIELD_FLOAT ),
	DEFINE_FIELD( m_waterLevel, FIELD_FLOAT ),
	DEFINE_FIELD( m_isDormant, FIELD_BOOLEAN ),
	DEFINE_UTLVECTOR( m_fauna, FIELD_EHANDLE ),

	// JL (Red)
	DEFINE_FIELD( m_fSpeed, FIELD_FLOAT ),

	DEFINE_THINKFUNC( Update ),

END_DATADESC()


//-------------------------------------------------------------------------------------------------------------
CFlockPool::CFlockPool( void )
{
	m_faunaCount = 0;
	m_maxRange = 512.0f; //1024.0f; //red
	//m_maxRange = 255.0f; 
	//m_swimDepth = 0.0f;
	m_airDepth = 0.0f; //RandomFloat( 10.0f, 60.0f ); // red
	m_isDormant = false;
	//m_fSpeed = *fSpeed;

	m_visTimer.Start( 0.5f );

	ListenForGameEvent( "player_shoot" );
	ListenForGameEvent( "player_footstep" );
	ListenForGameEvent( "weapon_fire" );
	ListenForGameEvent( "hegrenade_detonate" );
	ListenForGameEvent( "flashbang_detonate" );
	ListenForGameEvent( "smokegrenade_detonate" );
	ListenForGameEvent( "bomb_exploded" );
}

//-------------------------------------------------------------------------------------------------------------
/**
 * Initialize the creature pool
 */
void CFlockPool::Spawn()
{
	SetThink( &CFlockPool::Update );
	SetNextThink( gpGlobals->curtime );

//	m_waterLevel = UTIL_WaterLevel( GetAbsOrigin(), GetAbsOrigin().z, GetAbsOrigin().z + 1000.0f );

	trace_t result;
	for( int i=0; i<m_faunaCount; ++i )
	{
		QAngle heading( 0.0f, RandomFloat( 0, 360.0f ), 0.0f );

		CFlockFauna *fauna = (CFlockFauna *)Create( "fauna", GetAbsOrigin(), heading, this );
		fauna->Initialize( this, i );

		if (fauna)
		{
			CHandle<CFlockFauna> hFauna;
			hFauna.Set( fauna );
			m_fauna.AddToTail( hFauna );
		}
	}
}


//-------------------------------------------------------------------------------------------------------------
/**
 * Parse KeyValue pairs
 */
bool CFlockPool::KeyValue( const char *szKeyName, const char *szValue )
{
	if (FStrEq( szKeyName, "fauna_count" ))
	{
		m_faunaCount = atoi(szValue);
		return true;
	}
	else if (FStrEq( szKeyName, "max_range" ))
	{
		m_maxRange = atof(szValue);
		if (m_maxRange <= 1.0f)
		{
			m_maxRange = 1.0f;
		}
/*
	else if (m_maxRange > 1024.0f)
	{
		// stay within 8 bits range
		m_maxRange = 1024.0f;
	}

return true;		
*/
	}
/*	else if (FStrEq( szKeyName, "fauna_speed" ))
	{
		m_fSpeed = atof(szValue);
	}
*/
	else if (FStrEq( szKeyName, "model" ))
	{
		PrecacheModel( szValue );
		SetModelName( AllocPooledString( szValue ) );
	}

	return BaseClass::KeyValue( szKeyName, szValue );
}


//-------------------------------------------------------------------------------------------------------------
/**
 * Game event processing
 */
void CFlockPool::FireGameEvent( IGameEvent *event )
{
	CBasePlayer *player = UTIL_PlayerByUserId( event->GetInt( "userid" ) );
	
	// the creatures panic
	const float loudRange = 250.0f; //red
	//const float loudRange = 500.0f;
	
	const float quietRange = 75.0f;

	float range = (Q_strcmp( "player_footstep", event->GetName() )) ? loudRange : quietRange;

	for( int i=0; i<m_fauna.Count(); ++i )
	{
		// if player is NULL, assume a game-wide event
		if (player && (player->GetAbsOrigin() - m_fauna[i]->GetAbsOrigin()).IsLengthGreaterThan( range ))
		{
			// event too far away to care
			continue;
		}

		m_fauna[i]->Panic();
	}
}


//-------------------------------------------------------------------------------------------------------------
/**
 * Invoked each server tick
 */
void CFlockPool::Update( void )
{
	float deltaT = 0.1f;
	SetNextThink( gpGlobals->curtime + deltaT );

	//GetFaunaSpeed();
	// @todo Go dormant when no players are around to see us

	if (fauna_dormant.GetBool())
	{
		if (!m_isDormant)
		{
			// stop all the creatures
			for( int i=0; i<m_fauna.Count(); ++i )
			{
				m_fauna[i]->SetAbsVelocity( Vector( 0, 0, 0 ) );
			}
			
			m_isDormant = true;

//			GetSequence();
//			SetSequence( (Activity)ACT_IDLE );
		}

		return;
	}
	else
	{
		m_isDormant = false;
		
//			GetSequence();
//			SetSequence( (Activity)ACT_FLY );
	}

	// update creature to creature visibility
	if (m_visTimer.IsElapsed())
	{
		m_visTimer.Reset();

		int i, j;
		trace_t result;

		// reset each creatures vis list
		for( i=0; i<m_fauna.Count(); ++i )
		{
			m_fauna[i]->ResetVisible();
		}

		// build new vis lists - line of sight is symmetric
		for( i=0; i<m_fauna.Count(); ++i )
		{
			if (!m_fauna[i]->IsAlive())
				continue;

			for( j=i+1; j<m_fauna.Count(); ++j )
			{
				if (!m_fauna[j]->IsAlive())
					continue;

				UTIL_TraceLine( m_fauna[i]->GetAbsOrigin(), m_fauna[j]->GetAbsOrigin(), MASK_PLAYERSOLID, m_fauna[i], COLLISION_GROUP_NONE, &result );
				if (result.fraction >= 1.0f)
				{
					// the creature can see each other
					m_fauna[i]->AddVisible( m_fauna[j] );
					m_fauna[j]->AddVisible( m_fauna[i] );
				}
			}
		}		
	}

	// simulate the creatures behavior
	for( int i=0; i<m_fauna.Count(); ++i )
	{
		m_fauna[i]->Update( deltaT );
	}
}
	//-------------------------------------------------------------------------------------------------------------
bool CFlockPool::IsSeagull( void )
{
	if ( Q_strcmp( STRING( GetModelName() ), "models/seagull.mdl" ) == 1 )
	{
		return m_bSeagull;
	}
	else
	{
		return false;
	}
}


//-------------------------------------------------------------------------------------------------------------
bool CFlockPool::IsCrow( void )
{
//	if ( Q_strstr( GetModelName(), "models/animals/" ) )
	if ( Q_strcmp( STRING( GetModelName() ), "models/crow.mdl" ) == 1 )
	{
		return m_bCrow;
	}
	else
	{
		return false;
	}
}


//-------------------------------------------------------------------------------------------------------------
bool CFlockPool::IsButterfly( void )
{
	if ( Q_strcmp( STRING( GetModelName() ), "models/jl/animals/butterfly/blue_morpho.mdl" ) == 1 )
	{
		return m_bButterfly;
	}
	else
	{
		return false;
	}
}
