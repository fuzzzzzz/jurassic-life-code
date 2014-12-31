//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Defines the headcrab, a tiny, jumpy alien parasite.
//
//=============================================================================//

#ifndef NPC_FISH_H
#define NPC_FISH_H
#ifdef _WIN32
#pragma once
#endif

//#include "ai_squadslot.h"
#include "ai_basenpc.h"
#include "soundent.h"

class CBaseFish : public CAI_BaseNPC
{
	DECLARE_CLASS( CBaseFish, CAI_BaseNPC );

public:

	virtual void	Precache( void ); // ok
	virtual void	Spawn( void ); // ok
	

	//virtual void	HandleAnimEvent( animevent_t *pEvent );
	//virtual void	PrescheduleThink( void );	
	void	PrescheduleThink( void );	
	virtual int		SelectSchedule( void );
	virtual int		TranslateSchedule( int scheduleType );
	//virtual void	StartTask( const Task_t *pTask );
	virtual void	StartTask( const Task_t *pTask );
	virtual  void	RunTask( const Task_t *pTask );

	//float	MaxYawSpeed( void );

	bool	CanBecomeRagdoll( void ); //virtual ! why ? 

/* Sounds */
//	virtual void	BiteSound( void );
//	virtual void	PainSound( const CTakeDamageInfo &info );
//	virtual void	DeathSound( const CTakeDamageInfo &info );
//	virtual void	IdleSound( void );
//	virtual void	AlertSound( void );
	
	virtual void	MoveFlyExecute( CBaseEntity *pTargetEnt, const Vector & vecDir, float flDistance, float flInterval );
	virtual bool	OverrideMove( float flInterval );

	virtual void	DoMovement( float flInterval, const Vector &MoveTarget, int eMoveType );

	void	Event_Killed( const CTakeDamageInfo &info );
	virtual int	OnTakeDamage_Alive( const CTakeDamageInfo &inputinfo );
	
		
	//int		MeleeAttack1Conditions( float flDot, float flDist );

	bool	FVisible( CBaseEntity *pEntity, int traceMask = MASK_BLOCKLOS, CBaseEntity **ppBlocker = NULL );

	void	TranslateNavGoal( CBaseEntity *pEnemy, Vector &chasePosition );
	float	GetDefaultNavGoalTolerance();
	

	
	void	SteerArrive( Vector &Steer, const Vector &Target );
	void	SteerSeek( Vector &Steer, const Vector &Target );
	void	ClampSteer( Vector &SteerAbs, Vector &SteerRel, Vector &forward, Vector &right, Vector &up );
	void	AddSwimNoise( Vector *velocity );
	bool	SteerAvoidObstacles( Vector &Steer, const Vector &Velocity, const Vector &Forward, const Vector &Right, const Vector &Up );
	void	SetPoses( Vector moveRel, float speed );

	//float	GetGroundSpeed( void );
	
	CBaseEntity	*m_pVictim;

	Vector	m_vecLastMoveTarget;
	bool	m_bHasMoveTarget;
	bool	m_bIgnoreSurface;
	float	m_flGroundSpeed;
	float	m_flGroundSpeedWalk;	// 

//	DECLARE_DATADESC();
//	DEFINE_CUSTOM_AI;
//private:
//protected:

	bool	Beached( void );
	//void	Touch( CBaseEntity *pOther ); // valve old code

/* Need ??? */
	float	GetGroundSpeed( void );
	
#if FEELER_COLLISION
	Vector	DoProbe( const Vector &Probe );
	Vector	m_LastSteer;
#endif

	
	static const Vector	m_vecAccelerationMax;
	static const Vector	m_vecAccelerationMin;

	
	
	//CBaseEntity	*m_pVictim;

	float	m_flSwimSpeed;
//	float	m_flTailYaw;
//	float	m_flTailPitch;

	float	m_flNextPingTime;
	float	m_flNextGrowlTime;



	//CSoundPatch	*m_pSwimSound;
	//CSoundPatch	*m_pVoiceSound;
	
	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();
	

};

//Acceleration definitions
const Vector CBaseFish::m_vecAccelerationMax	= Vector(  256,  1024,  512 );
const Vector CBaseFish::m_vecAccelerationMin	= Vector( -256, -1024, -512 );


//=========================================================
// The ever popular chubby classic headcrab
//=========================================================
//=========================================================
class CBrycon : public CBaseFish
{
	DECLARE_CLASS( CBrycon, CBaseFish );

public:
	void Precache( void );
	void Spawn( void );

	//void	PrescheduleThink( void ); // ( baseclass cbasefish)
	//int		TranslateSchedule( int scheduleType );
	int		SelectSchedule( void );
	//void	StartTask( const Task_t *pTask );
	//void	RunTask( const Task_t *pTask );

	float	MaxYawSpeed( void );
//	Activity NPC_TranslateActivity( Activity eNewActivity );  // need ?
	Class_T Classify( void )	{	return CLASS_BRYCON;	}

	//void	DoMovement( float flInterval, const Vector &MoveTarget, int eMoveType );
	//void	MoveFlyExecute( CBaseEntity *pTargetEnt, const Vector & vecDir, float flDistance, float flInterval );
	//bool	OverrideMove( float flInterval );

/* Sounds */
	void	IdleSound( void );	
	void	DeathSound( const CTakeDamageInfo &info );

//	float	GetGroundSpeed( void );


private:

	float	GetGroundSpeed( void );

};


//=========================================================
// The ever popular chubby classic headcrab
//=========================================================
//=========================================================
class CPiranha : public CBaseFish
{
//	DECLARE_DATADESC(); // buggy
	DECLARE_CLASS( CPiranha, CBaseFish );
public:	
//	DECLARE_CLASS( CPiranha, CBaseFish );
	
	void Precache( void );
	void Spawn( void );

	float	MaxYawSpeed( void );
//	Activity NPC_TranslateActivity( Activity eNewActivity ); // need ?
	Class_T Classify( void )	{	return CLASS_PIRANHA;	}

/* Artificiel Intelligence   */
	
	//void	PrescheduleThink( void ); // ( baseclass cbasefish)
	int		SelectSchedule( void );
	void	StartTask( const Task_t *pTask );
	//void	RunTask( const Task_t *pTask );
	void	HandleAnimEvent( animevent_t *pEvent );
	int		TranslateSchedule( int scheduleType );

	int		MeleeAttack1Conditions( float flDot, float flDist );

/* Navigation */
/*	
	void	TranslateNavGoal( CBaseEntity *pEnemy, Vector &chasePosition );
	float	GetDefaultNavGoalTolerance();
	void	DoMovement( float flInterval, const Vector &MoveTarget, int eMoveType );
	void	MoveFlyExecute( CBaseEntity *pTargetEnt, const Vector & vecDir, float flDistance, float flInterval );
	bool	OverrideMove( float flInterval );
*/
	
/* death, damages, ragdoll etc. */

	int	OnTakeDamage_Alive( const CTakeDamageInfo &info );

	//void	Event_Killed( const CTakeDamageInfo &info );
	
/* Sounds */
	void	BiteSound( void );
	void	PainSound( const CTakeDamageInfo &info );
	void	DeathSound( const CTakeDamageInfo &info );
	void	IdleSound( void );
	void	AlertSound( void );
	//void	AttackSound( void );
	
	//float	GetGroundSpeed( void );
	
	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();

private:

/* Piranha melee attack */
	void	Bite( void );
	
	float	GetGroundSpeed( void );

	float	m_flNextBiteTime;
	float	m_flHoldTime;

};


//=========================================================
// The ever popular chubby classic headcrab
//=========================================================
//=========================================================
class CCrocodile : public CBaseFish
{
	DECLARE_CLASS( CCrocodile, CBaseFish );

public:
	void Precache( void );
	void Spawn( void );

	float	MaxYawSpeed( void );
//	Activity NPC_TranslateActivity( Activity eNewActivity ); // need ?
	Class_T Classify( void )	{	return CLASS_PIRANHA;	} // change that later..

	int		OnTakeDamage_Alive( const CTakeDamageInfo &info );
	
/* Artificiel Intelligence   */
	
	//void	PrescheduleThink( void ); // ( baseclass cbasefish)
	int		SelectSchedule( void );
	void	StartTask( const Task_t *pTask );
	void	RunTask( const Task_t *pTask );
	void	HandleAnimEvent( animevent_t *pEvent ); // OK
	int		TranslateSchedule( int ScheduleType );

/* Croco melee attack */

	int		MeleeAttack1Conditions( float flDot, float flDist ); // OK
	
/* Navigation */

//	void	TranslateNavGoal( CBaseEntity *pEnemy, Vector &chasePosition );
//	float	GetDefaultNavGoalTolerance();
//	void	DoMovement( float flInterval, const Vector &MoveTarget, int eMoveType );
//	void	MoveFlyExecute( CBaseEntity *pTargetEnt, const Vector & vecDir, float flDistance, float flInterval );
//	bool	OverrideMove( float flInterval );
	

/* Sounds */

	void	BiteSound( void ); // OK
	void	PainSound( const CTakeDamageInfo &info ); // OK
	void	DeathSound( const CTakeDamageInfo &info ); // OK
	void	IdleSound( void ); // OK
	void	AlertSound( void ); // OK 
	//void	AttackSound( void );  // will depending on the melee attack

	//virtual float	GetGroundSpeed( void );

	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();
	
private:


/* Croco melee attack */
	void	Bite( void ); // OK 
	
/* Crocodile Ensnare victim to drown node */
	void	EnsnareVictim( CBaseEntity *pVictim ); // OK
	void	ReleaseVictim( void ); // OK
	void	DragVictim( float moveDist ); //

	float	GetGroundSpeed( void );

	float	m_flNextBiteTime;
	float	m_flHoldTime;
	//CBaseEntity	*m_pVictim;

};

#endif // NPC_FISH_H