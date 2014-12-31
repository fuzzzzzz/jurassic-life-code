//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: BASE Mastodont Dinosaurs header ( trex, triceratops)
//
//=============================================================================//


#ifndef NPC_MASTODON_H
#define NPC_MASTODON_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "ai_basenpc.h"
#include "ai_blended_movement.h"
#include "ai_navigator.h"
#include "ai_network.h"
#include "soundent.h"
#include "sprite.h"


#define	MASTODON_FARTHEST_PHYSICS_OBJECT	350

inline void TraceHull_SkipPhysics( const Vector &vecAbsStart, const Vector &vecAbsEnd, const Vector &hullMin, 
					 const Vector &hullMax,	unsigned int mask, const CBaseEntity *ignore, 
					 int collisionGroup, trace_t *ptr, float minMass );

struct PhysicsObjectCriteria_t
{
	CBaseEntity *pTarget;
	Vector	vecCenter;		// Center point to look around
	float	flRadius;		// Radius to search within
	float	flTargetCone;
	bool	bPreferObjectsAlongTargetVector;	// Prefer objects that we can strike easily as we move towards our target
	float	flNearRadius;						// If we won't hit the player with the object, but get this close, throw anyway
};

#define MAX_FAILED_PHYSOBJECTS 8

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CBaseMastodon : public CAI_BlendedNPC
{
public:
	DECLARE_CLASS( CBaseMastodon, CAI_BlendedNPC );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CBaseMastodon( void );

	Class_T	Classify( void ) { return CLASS_ANTLION; }
	virtual int		GetSoundInterests( void ) { return (SOUND_WORLD|SOUND_COMBAT|SOUND_PLAYER|SOUND_DANGER); }
	virtual bool	QueryHearSound( CSound *pSound );

	const impactdamagetable_t &GetPhysicsImpactDamageTable( void );

	virtual int		MeleeAttack1Conditions( float flDot, float flDist );
	virtual int		SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );

	virtual int		TranslateSchedule( int scheduleType );
	virtual int		OnTakeDamage_Alive( const CTakeDamageInfo &info );
	virtual void	DeathSound( const CTakeDamageInfo &info );
	virtual void	Event_Killed( const CTakeDamageInfo &info );
	virtual int		SelectSchedule( void );

	virtual float GetAutoAimRadius() { return 36.0f; }
	
	virtual void	Precache( void );
	virtual void	Spawn( void );
	virtual void	Activate( void );
	virtual void	HandleAnimEvent( animevent_t *pEvent );
	virtual void	UpdateEfficiency( bool bInPVS )	{ SetEfficiency( ( GetSleepState() != AISS_AWAKE ) ? AIE_DORMANT : AIE_NORMAL ); SetMoveEfficiency( AIME_NORMAL ); }
	virtual void	PrescheduleThink( void );
	virtual void	GatherConditions( void );
	virtual void	TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );
	virtual void	StartTask( const Task_t *pTask );
	virtual void	RunTask( const Task_t *pTask );
	virtual void	StopLoopingSounds();
	virtual bool	HandleInteraction( int interactionType, void *data, CBaseCombatCharacter *sender );
	
	// Input handlers.
	void	InputSetShoveTarget( inputdata_t &inputdata );
	void	InputSetChargeTarget( inputdata_t &inputdata );
	void	InputClearChargeTarget( inputdata_t &inputdata );
	void	InputUnburrow( inputdata_t &inputdata );
	void	InputRagdoll( inputdata_t &inputdata );
	void	InputEnableBark( inputdata_t &inputdata );
	void	InputDisableBark( inputdata_t &inputdata );
	void	InputSummonedAntlionDied( inputdata_t &inputdata );
	void	InputEnablePreferPhysicsAttack( inputdata_t &inputdata );
	void	InputDisablePreferPhysicsAttack( inputdata_t &inputdata );

	virtual bool	IsLightDamage( const CTakeDamageInfo &info );
	virtual bool	IsHeavyDamage( const CTakeDamageInfo &info );
	virtual bool	OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval );
	virtual bool	BecomeRagdollOnClient( const Vector &force );
	virtual void	UpdateOnRemove( void );
	virtual bool	IsUnreachable( CBaseEntity* pEntity );			// Is entity is unreachable?

	virtual float	MaxYawSpeed( void );
	virtual bool	OverrideMove( float flInterval );
	virtual bool	CanBecomeRagdoll( void );

	virtual bool	ShouldProbeCollideAgainstEntity( CBaseEntity *pEntity );

	virtual Activity	NPC_TranslateActivity( Activity baseAct );

#if HL2_EPISODIC
	//---------------------------------
	// Navigation & Movement -- prevent stopping paths for the guard
	//---------------------------------
	class CNavigator : public CAI_ComponentWithOuter<CBaseMastodon, CAI_Navigator>
	{
		typedef CAI_ComponentWithOuter<CBaseMastodon, CAI_Navigator> BaseClass;
	public:
		CNavigator( CBaseMastodon *pOuter )
			:	BaseClass( pOuter )
		{
		}

		bool GetStoppingPath( CAI_WaypointList *pClippedWaypoints );
	};
	CAI_Navigator *	CreateNavigator()	{ return new CNavigator( this );	}
#endif

	DEFINE_CUSTOM_AI;

private:

	inline bool CanStandAtPoint( const Vector &vecPos, Vector *pOut );
	bool	RememberFailedPhysicsTarget( CBaseEntity *pTarget );
	void	GetPhysicsShoveDir( CBaseEntity *pObject, float flMass, Vector *pOut );
	void	CreateGlow( CSprite **pSprite, const char *pAttachName );
	void	DestroyGlows( void );
	void	Footstep( bool bHeavy );
	int		SelectCombatSchedule( void );
	int		SelectUnreachableSchedule( void );
	bool	CanSummon( bool bIgnoreTime );
	void	SummonAntlions( void );
					
	void	ChargeLookAhead( void );
	bool	EnemyIsRightInFrontOfMe( CBaseEntity **pEntity );
	bool	HandleChargeImpact( Vector vecImpact, CBaseEntity *pEntity );
	bool	ShouldCharge( const Vector &startPos, const Vector &endPos, bool useTime, bool bCheckForCancel );
	bool	ShouldWatchEnemy( void );
					
	void	ImpactShock( const Vector &origin, float radius, float magnitude, CBaseEntity *pIgnored = NULL );
	void	BuildScheduleTestBits( void );
	void	Shove( void );
	void	FoundEnemy( void );
	void	LostEnemy( void );
	void	UpdateHead( void );
	void	UpdatePhysicsTarget( bool bPreferObjectsAlongTargetVector, float flRadius = MASTODON_FARTHEST_PHYSICS_OBJECT );
	void	MaintainPhysicsTarget( void );
	void	ChargeDamage( CBaseEntity *pTarget );
	void	StartSounds( void );
	void	SetHeavyDamageAnim( const Vector &vecSource );
	float	ChargeSteer( void );
	CBaseEntity *FindPhysicsObjectTarget( const PhysicsObjectCriteria_t &criteria );
	Vector	GetPhysicsHitPosition( CBaseEntity *pObject, CBaseEntity *pTarget, Vector *vecTrajectory, float *flClearDistance );
	bool	CanStandAtShoveTarget( CBaseEntity *pShoveObject, CBaseEntity *pTarget, Vector *pOut );
	CBaseEntity *GetNextShoveTarget( CBaseEntity *pLastEntity, AISightIter_t &iter );

	int				m_nFlinchActivity;

	bool			m_bStopped;
	bool			m_bIsBurrowed;
	bool			m_bBarkEnabled;
	float			m_flNextSummonTime;
	int				m_iNumLiveRaptors;
							
	float			m_flSearchNoiseTime;
	float			m_flAngerNoiseTime;
	float			m_flBreathTime;
	float			m_flChargeTime;
	float			m_flPhysicsCheckTime;
	float			m_flNextHeavyFlinchTime;
	float			m_flNextRoarTime;
	int				m_iChargeMisses;
	bool			m_bDecidedNotToStop;
	bool			m_bPreferPhysicsAttack;

	CNetworkVar( bool, m_bCavernBreed );	// If this guard is meant to be a cavern dweller (uses different assets)
	CNetworkVar( bool, m_bInCavern );		// Behavioral hint telling the guard to change his behavior
					
	Vector			m_vecPhysicsTargetStartPos;
	Vector			m_vecPhysicsHitPosition;
					
	EHANDLE			m_hShoveTarget;
	EHANDLE			m_hChargeTarget;
	EHANDLE			m_hChargeTargetPosition;
	EHANDLE			m_hOldTarget;
	EHANDLE			m_hPhysicsTarget;
					
	CUtlVectorFixed<EHANDLE, MAX_FAILED_PHYSOBJECTS>		m_FailedPhysicsTargets;

	COutputEvent	m_OnSummon;

	CSoundPatch		*m_pGrowlHighSound;
	CSoundPatch		*m_pGrowlLowSound;
	CSoundPatch		*m_pGrowlIdleSound;
	CSoundPatch		*m_pBreathSound;
	CSoundPatch		*m_pConfusedSound;

	string_t		m_iszPhysicsPropClass;
	string_t		m_strShoveTargets;

	CSprite			*m_hCaveGlow[2];

//#if TREX_BLOOD_EFFECTS
//	CNetworkVar( uint8, m_iBleedingLevel );
//
//	unsigned char GetBleedingLevel( void ) const;
//#endif

protected:

	int m_poseThrow;
	int m_poseHead_Yaw, m_poseHead_Pitch;
	virtual void	PopulatePoseParameters( void );


// inline accessors
public:	
	inline bool IsCavernBreed( void ) const { return m_bCavernBreed; }
	inline bool IsInCavern( void ) const { return m_bInCavern; }

	
};

//==================================================
// CNPC_Trex
//==================================================

class CNPC_TRex : public CBaseMastodon
{
public:
	DECLARE_CLASS( CNPC_TRex, CBaseMastodon );

	CNPC_TRex( void );

	void	Precache( void );
	void	Spawn( void );
		
	Class_T	Classify( void ) { return CLASS_TREX; }
};

//==================================================
// CNPC_Triceratops
//==================================================

class CNPC_Triceratops : public CBaseMastodon
{
public:
	DECLARE_CLASS( CNPC_Triceratops, CBaseMastodon );

	CNPC_Triceratops( void );

	void	Precache( void );
	void	Spawn( void );
		
	Class_T	Classify( void ) { return CLASS_TRICERATOPS; }
};


#endif //NPC_MASTODON_H
