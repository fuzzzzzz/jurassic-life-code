//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef NPC_VELOCIRAPTOR_H
#define NPC_VELOCIRAPTOR_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_blended_movement.h"
#include "soundent.h"
#include "ai_behavior_follow.h"
#include "ai_behavior_assault.h"

//class CAntlionTemplateMaker;

#define	RAPTOR_FOLLOW_DISTANCE	350
#define	RAPTOR_FOLLOW_DISTANCE_SQR	(RAPTOR_FOLLOW_DISTANCE*RAPTOR_FOLLOW_DISTANCE)
#define RAPTOR_SKIN_COUNT 4

class CNPC_Raptor;

// Velociraptor follow behavior
class CAI_RaptorFollowBehavior : public CAI_FollowBehavior
{
	typedef CAI_FollowBehavior BaseClass;

public:

	CAI_RaptorFollowBehavior()
	 :	BaseClass( AIF_RAPTOR )
	{
	}

	bool FarFromFollowTarget( void )
	{ 
		return ( GetFollowTarget() && (GetAbsOrigin() - GetFollowTarget()->GetAbsOrigin()).LengthSqr() > RAPTOR_FOLLOW_DISTANCE_SQR ); 
	}

	bool ShouldFollow( void )
	{
		if ( GetFollowTarget() == NULL )
			return false;
		
		if ( GetEnemy() != NULL )
			return false;

		return true;
	}
};

//
// Velociraptor class
//

enum RaptorMoveState_e
{
	RAPTOR_MOVE_FREE,
	RAPTOR_MOVE_FOLLOW,
	RAPTOR_MOVE_FIGHT_TO_GOAL,
};

#define	SF_RAPTOR_BURROW_ON_ELUDED		( 1 << 16 )
#define	SF_RAPTOR_USE_GROUNDCHECKS		( 1 << 17 )
#define	SF_RAPTOR_WORKER				( 1 << 18 ) // Use the "worker" model

typedef CAI_BlendingHost< CAI_BehaviorHost<CAI_BlendedNPC> > CAI_BaseRaptorBase;

class CNPC_Raptor : public CAI_BaseRaptorBase
{
public:

	DECLARE_CLASS( CNPC_Raptor, CAI_BaseRaptorBase  );

	CNPC_Raptor( void );

	virtual float	InnateRange1MinRange( void ) { return 50*12; }
	virtual float	InnateRange1MaxRange( void ) { return 250*12; }

	bool		IsWorker( void ) const { return HasSpawnFlags( SF_RAPTOR_WORKER ); }	// NOTE: IsVelociraptorWorker function must agree!

	float		GetIdealAccel( void ) const;
	float		MaxYawSpeed( void );
	bool		FInViewCone( CBaseEntity *pEntity );
	bool		FInViewCone( const Vector &vecSpot );
				
	void		Activate( void );
	void		HandleAnimEvent( animevent_t *pEvent );
	void		StartTask( const Task_t *pTask );
	void		RunTask( const Task_t *pTask );
	void		IdleSound( void );
	void		PainSound( const CTakeDamageInfo &info );
	void		Precache( void );
	void		Spawn( void );
	int			OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void		TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );

	float		GetHitgroupDamageMultiplier( int iHitGroup, const CTakeDamageInfo &info );

	void		BuildScheduleTestBits( void );
	void		GatherConditions( void );
	void		PrescheduleThink( void );
	void		ZapThink( void );
	void		BurrowUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	bool		CreateVPhysics();
				
	bool		IsJumpLegal( const Vector &startPos, const Vector &apex, const Vector &endPos ) const;
	bool		HandleInteraction( int interactionType, void *data, CBaseCombatCharacter *sender = NULL );
	bool		QuerySeeEntity( CBaseEntity *pEntity, bool bOnlyHateOrFearIfNPC = false );
	bool		ShouldPlayIdleSound( void );
	bool		OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval );
	bool		IsValidEnemy(CBaseEntity *pEnemy);
	bool		QueryHearSound( CSound *pSound );
	bool		IsLightDamage( const CTakeDamageInfo &info );
	bool		CreateBehaviors( void );
	bool		ShouldHearBugbait( void ) { return ( m_bIgnoreBugbait == false ); }
	int			SelectSchedule( void );

	void		Touch( CBaseEntity *pOther );

	virtual int		RangeAttack1Conditions( float flDot, float flDist );
	virtual int		MeleeAttack1Conditions( float flDot, float flDist );
	virtual int		MeleeAttack2Conditions( float flDot, float flDist );
	virtual int		GetSoundInterests( void ) { return (BaseClass::GetSoundInterests())|(SOUND_DANGER|SOUND_PHYSICS_DANGER|SOUND_BUGBAIT); }
	virtual	bool	IsHeavyDamage( const CTakeDamageInfo &info );

	Class_T		Classify( void ) { return CLASS_RAPTOR; }
	
	void		Event_Killed( const CTakeDamageInfo &info );
	bool		FValidateHintType ( CAI_Hint *pHint );
	void		GatherEnemyConditions( CBaseEntity *pEnemy );
	
	bool		IsAllied( void );
	bool		ShouldGib( const CTakeDamageInfo &info );
	bool		CorpseGib( const CTakeDamageInfo &info );

	float		GetMaxJumpSpeed() const { return 1024.0f; }

	void		SetFightTarget( CBaseEntity *pTarget );
	void		InputFightToPosition( inputdata_t &inputdata );
	void		InputStopFightToPosition( inputdata_t &inputdata );
	void		InputJumpAtTarget( inputdata_t &inputdata );

	void		SetFollowTarget( CBaseEntity *pTarget );
	int			TranslateSchedule( int scheduleType );

	virtual		Activity NPC_TranslateActivity( Activity baseAct );

	bool		ShouldResumeFollow( void );
	bool		ShouldAbandonFollow( void );

	void		SetMoveState( RaptorMoveState_e state );
	int			ChooseMoveSchedule( void );

	DECLARE_DATADESC();

	bool		m_bStartBurrowed;
	float		m_flNextJumpPushTime;

	void		SetParentSpawnerName( const char *szName ) { m_strParentSpawner = MAKE_STRING( szName ); }
	const char *GetParentSpawnerName( void ) { return STRING( m_strParentSpawner ); }

	virtual void StopLoopingSounds( void );
	bool    AllowedToBePushed( void );

	virtual Vector BodyTarget( const Vector &posSrc, bool bNoisy = true );
	virtual float GetAutoAimRadius() { return 36.0f; }

	void	ClearBurrowPoint( const Vector &origin );

	void	Flip( bool bZapped = false );

	bool CanBecomeRagdoll();

	virtual void	NotifyDeadFriend( CBaseEntity *pFriend );

private:

	inline CBaseEntity *EntityToWatch( void );
	void				UpdateHead( void );

	bool	FindChasePosition( const Vector &targetPos, Vector &result );
	bool	GetGroundPosition( const Vector &testPos, Vector &result );
	bool	GetPathToSoundFleePoint( int soundType );
	inline bool	IsFlipped( void );

	void	Burrow( void );
	void	Unburrow( void );
	
	void	InputUnburrow( inputdata_t &inputdata );
	void	InputBurrow( inputdata_t &inputdata );
	void	InputBurrowAway( inputdata_t &inputdata );
	void	InputDisableJump( inputdata_t &inputdata );
	void	InputEnableJump( inputdata_t &inputdata );
	void	InputIgnoreBugbait( inputdata_t &inputdata );
	void	InputHearBugbait( inputdata_t &inputdata );

	bool	FindBurrow( const Vector &origin, float distance, int type, bool excludeNear = true );
	void	CreateDust( bool placeDecal = true );

	bool	ValidBurrowPoint( const Vector &point );
	bool	CheckLanding( void );
	bool	Alone( void );
	bool	CheckAlertRadius( void );
	bool	ShouldJump( void );

	void	MeleeAttack( float distance, float damage, QAngle& viewPunch, Vector& shove );
	//void	SetWings( bool state );
	void	StartJump( void );
	void	LockJumpNode( void );

	bool	IsUnusableNode(int iNodeID, CAI_Hint *pHint);

	bool	OnObstructionPreSteer( AILocalMoveGoal_t *pMoveGoal, float distClear, AIMoveResult_t *pResult );
	
	void	ManageFleeCapabilities( bool bEnable );
	
	int		SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );
	bool	IsFirmlyOnGround( void );
	void	CascadePush( const Vector &vecForce );

	virtual bool CanRunAScriptedNPCInteraction( bool bForced = false );

	virtual void Ignite ( float flFlameLifetime, bool bNPCOnly, float flSize, bool bCalledByLevelDesigner );
	//virtual bool GetSpitVector( const Vector &vecStartPos, const Vector &vecTarget, Vector *vecOut );
	virtual bool InnateWeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions );
	virtual bool FCanCheckAttacks( void );
	
	bool SeenEnemyWithinTime( float flTime );
	void DelaySquadAttack( float flDuration );

#if HL2_EPISODIC
	void DoPoisonBurst();
#endif

	float	m_flIdleDelay;
	float	m_flBurrowTime;
	float	m_flJumpTime;
	float	m_flAlertRadius;

	float	m_flPounceTime;
	int		m_iUnBurrowAttempts;
	int		m_iContext;			//for FValidateHintType context

	//Vector	m_vecSaveSpitVelocity;	// Saved when we start to attack and used if we failed to get a clear shot once we release

	CAI_RaptorFollowBehavior	m_FollowBehavior;
	CAI_AssaultBehavior			m_AssaultBehavior;

	RaptorMoveState_e	m_MoveState;

	COutputEvent	m_OnReachFightGoal;	//Reached our scripted destination to fight to
	COutputEvent	m_OnUnBurrowed;	//Unburrowed
	
	Vector		m_vecSavedJump;
	Vector		m_vecLastJumpAttempt;

	float		m_flIgnoreSoundTime;		// Sound time to ignore if earlier than
	float		m_flNextAcknowledgeTime;	// Next time an velociraptor can make an acknowledgement noise
	float		m_flSuppressFollowTime;		// Amount of time to suppress our follow time
	float		m_flObeyFollowTime;			// A range of time the velociraptors must be obedient

	Vector		m_vecHeardSound;
	bool		m_bHasHeardSound;
	bool		m_bAgitatedSound;	//Playing agitated sound?
	//bool		m_bWingsOpen;		//Are the wings open?
	bool		m_bIgnoreBugbait;	//If the velociraptor should ignore bugbait sounds
	string_t	m_strParentSpawner;	//Name of our spawner

	EHANDLE		m_hFollowTarget;
	EHANDLE		m_hFightGoalTarget;
	float		m_flEludeDistance;	//Distance until the velociraptor will consider himself "eluded" if so flagged
	bool		m_bLeapAttack;
	bool		m_bDisableJump;
	float		m_flTimeDrown;
	float		m_flTimeDrownSplash;
	bool		m_bDontExplode;			// Suppresses worker poison burst when drowning, failing to unburrow, etc.
	bool		m_bLoopingStarted;
	bool		m_bSuppressUnburrowEffects;	// Don't kick up dust when spawning
#if HL2_EPISODIC
	bool		m_bHasDoneAirAttack;  ///< only allowed to apply this damage once per glide
#endif

	bool		m_bForcedStuckJump;
	int			m_nBodyBone;

	// Used to trigger a heavy damage interrupt if sustained damage is taken
	int			m_nSustainedDamage;
	float		m_flLastDamageTime;
	float		m_flZapDuration; // FIXME ?!?

protected:
	int m_poseHead_Yaw, m_poseHead_Pitch;
	virtual void	PopulatePoseParameters( void );

private:

	HSOUNDSCRIPTHANDLE	m_hFootstep;

	DEFINE_CUSTOM_AI;

	//==================================================
	// velociraptor_Conditions
	//==================================================

	enum
	{
		COND_RAPTOR_FLIPPED = LAST_SHARED_CONDITION,
		//COND_RAPTOR_ON_NPC,
		COND_RAPTOR_ILLEGAL_GROUNDENT,
		COND_RAPTOR_CAN_JUMP,
		COND_RAPTOR_FOLLOW_TARGET_TOO_FAR,
		COND_RAPTOR_RECEIVED_ORDERS,
		COND_RAPTOR_IN_WATER,
		COND_RAPTOR_CAN_JUMP_AT_TARGET,
		COND_RAPTOR_SQUADMATE_KILLED
	};

	//==================================================
	// VelociraptorSchedules
	//==================================================

	enum
	{
		SCHED_RAPTOR_CHASE_ENEMY_BURROW = LAST_SHARED_SCHEDULE,
		SCHED_RAPTOR_JUMP,
		SCHED_RAPTOR_RUN_TO_BURROW_IN,
		SCHED_RAPTOR_BURROW_IN,
		SCHED_RAPTOR_BURROW_WAIT,
		SCHED_RAPTOR_BURROW_OUT,
		SCHED_RAPTOR_WAIT_FOR_UNBORROW_TRIGGER,
		SCHED_RAPTOR_WAIT_FOR_CLEAR_UNBORROW,
		SCHED_RAPTOR_WAIT_UNBORROW,
		SCHED_RAPTOR_CHASE_BUGBAIT,
		SCHED_RAPTOR_FLIP,
		SCHED_RAPTOR_DISMOUNT_NPC,
		SCHED_RAPTOR_RUN_TO_FIGHT_GOAL,
		SCHED_RAPTOR_RUN_TO_FOLLOW_GOAL,
		SCHED_RAPTOR_BUGBAIT_IDLE_STAND,
		SCHED_RAPTOR_POUNCE,
		SCHED_RAPTOR_POUNCE_MOVING,
		SCHED_RAPTOR_BURROW_AWAY,
		SCHED_RAPTOR_FLEE_PHYSICS_DANGER,
		SCHED_RAPTOR_FLEE_DANGER,
		SCHED_RAPTOR_DROWN,
		SCHED_RAPTOR_WORKER_RANGE_ATTACK1,
		SCHED_RAPTOR_WORKER_RUN_RANDOM,
		SCHED_RAPTOR_TAKE_COVER_FROM_ENEMY,
		SCHED_RAPTOR_ZAP_FLIP,
		SCHED_RAPTOR_WORKER_FLANK_RANDOM,
		SCHED_RAPTOR_TAKE_COVER_FROM_SAVEPOSITION,
		SCHED_RAPTOR_HOP_RANDOMLY
	};
	//==================================================
	// Velociraptor Tasks
	//==================================================

	enum
	{
		TASK_RAPTOR_SET_CHARGE_GOAL = LAST_SHARED_TASK,
		TASK_RAPTOR_FIND_BURROW_IN_POINT,
		TASK_RAPTOR_FIND_BURROW_OUT_POINT,
		TASK_RAPTOR_BURROW,
		TASK_RAPTOR_UNBURROW,
		TASK_RAPTOR_VANISH,
		TASK_RAPTOR_BURROW_WAIT,
		TASK_RAPTOR_CHECK_FOR_UNBORROW,
		TASK_RAPTOR_JUMP,
		TASK_RAPTOR_WAIT_FOR_TRIGGER,
		TASK_RAPTOR_GET_DANGER_ESCAPE_PATH,
		TASK_RAPTOR_GET_PATH_TO_BUGBAIT,
		TASK_RAPTOR_FACE_BUGBAIT,
		TASK_RAPTOR_DISMOUNT_NPC,
		TASK_RAPTOR_REACH_FIGHT_GOAL,
		TASK_RAPTOR_GET_PHYSICS_DANGER_ESCAPE_PATH,
		TASK_RAPTOR_FACE_JUMP,
		TASK_RAPTOR_DROWN,
		TASK_RAPTOR_GET_PATH_TO_RANDOM_NODE,
		TASK_RAPTOR_FIND_COVER_FROM_SAVEPOSITION,
		TASK_RAPTOR_HOP_OFF_NPC,
	};
};


//-----------------------------------------------------------------------------
// Purpose: Shield
//-----------------------------------------------------------------------------
class CRaptorRepellant : public CPointEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CRaptorRepellant, CPointEntity );
	~CRaptorRepellant();

public:
	void Spawn( void );
	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );
	float GetRadius( void );
	void SetRadius( float flRadius ) { m_flRepelRadius = flRadius; }

	static bool IsPositionRepellantFree( Vector vDesiredPos );

	void OnRestore( void );

private:

	float m_flRepelRadius;
	bool  m_bEnabled;
};

extern bool IsRaptor( CBaseEntity *pEntity );
extern bool IsRaptorWorker( CBaseEntity *pEntity );

#ifdef HL2_EPISODIC
extern float RaptorWorkerBurstRadius( void );
#endif // HL2_EPISODIC

#endif // NPC_VELOCIRAPTOR_H

