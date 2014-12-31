//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef NPC_GALLIMIMUS_H
#define NPC_GALLIMIMUS_H
#ifdef _WIN32
#pragma once
#endif

//#include "ai_blended_movement.h"
#include "ai_basenpc.h"
#include "soundent.h"
//#include "ai_behavior_follow.h"
//#include "ai_behavior_assault.h"




//#define GALLIMIMUS_SKIN_COUNT 4 //need this ?

//class CNPC_Gallimimus;
//
// Gallimimus class
//

/* need this  ? FIXME look to work only with workers
enum GallimimusMoveState_e
{
	GALLIMIMUS_MOVE_FREE,
	GALLIMIMUS_MOVE_FOLLOW,
	GALLIMIMUS_MOVE_FIGHT_TO_GOAL,
};
*/

#define	SF_GALLIMIMUS_USE_GROUNDCHECKS		( 1 << 16 ) // 17 on antlion's code

class CNPC_Gallimimus : public CAI_BaseNPC
{
public:

	DECLARE_CLASS( CNPC_Gallimimus, CAI_BaseNPC  );

	CNPC_Gallimimus( void );

	float		GetIdealAccel( void ) const;
	float		MaxYawSpeed( void );
	bool		FInViewCone( CBaseEntity *pEntity );
	bool		FInViewCone( const Vector &vecSpot );
	
	void		HandleAnimEvent( animevent_t *pEvent );
	void		StartTask( const Task_t *pTask );
	void		RunTask( const Task_t *pTask );
	void		IdleSound( void );
	void		PainSound( const CTakeDamageInfo &info );
	void		Precache( void );
	void		Spawn( void );
	int			OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void		TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );
	void		BuildScheduleTestBits( void );
	void		GatherConditions( void );
	void		PrescheduleThink( void );
	void		ZapThink( void );

	void		ClampRagdollForce( const Vector &vecForceIn, Vector *vecForceOut );
	bool		CreateVPhysics(); // Gibs ?
	bool		ShouldPlayIdleSound( void );
	bool		OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval );
	bool		IsLightDamage( const CTakeDamageInfo &info ); //why not ?
	
//	Activity		GetDeathActivity();
	int			SelectSchedule( void ); // cond_ >sched links
	
	void		Touch( CBaseEntity *pOther ); // by vehicles  to set the COND_FLIPPED and hear the physic danger  // should be problematic
	
	//virtual int		MeleeAttack1Conditions( float flDot, float flDist ); //against raptor or other NPC enemies
	//virtual int		MeleeAttack2Conditions( float flDot, float flDist ); // idem  but for a second meeleattack
	
	virtual int		GetSoundInterests( void ) { return (BaseClass::GetSoundInterests())|(SOUND_DANGER|SOUND_PHYSICS_DANGER|SOUND_BUGBAIT); }
	virtual	bool	IsHeavyDamage( const CTakeDamageInfo &info ); // should be problematic

	Class_T		Classify( void ) { return CLASS_GALLIMIMUS; } //  For AI RelationShip
	
	void		Event_Killed( const CTakeDamageInfo &info );
	//bool		FValidateHintType ( CAI_Hint *pHint ); // FLEE DANGER 
	//void		GatherEnemyConditions( CBaseEntity *pEnemy ); // 
	
	
	bool		ShouldGib( const CTakeDamageInfo &info ); // gibs
	bool		CorpseGib( const CTakeDamageInfo &info ); // gibs

	//float		GetMaxJumpSpeed() const { return 1024.0f; }  // no class code !!!

	//void		InputJumpAtTarget( inputdata_t &inputdata );

	//void		SetFollowTarget( CBaseEntity *pTarget ); /bugbait obey
	
	//int		TranslateSchedule( int scheduleType ); // seems to be not need
	//virtual		Activity NPC_TranslateActivity( Activity baseAct ); // used with antlions worker
	//bool		ShouldResumeFollow( void ); // worker
	//bool		ShouldAbandonFollow( void ); // worker
	//void		SetMoveState( GallimimusMoveState_e state ); // worker
	//int		ChooseMoveSchedule( void ); // worker

	
	DECLARE_DATADESC();
	
	//float		m_flNextJumpPushTime;
	
	//void		SetParentSpawnerName( const char *szName ) { m_strParentSpawner = MAKE_STRING( szName ); } // look on where pointed this
	//const char *GetParentSpawnerName( void ) { return STRING( m_strParentSpawner ); } // look on where pointed this
	
	virtual void StopLoopingSounds( void ); // work with AGITATED States
	
	virtual Vector BodyTarget( const Vector &posSrc, bool bNoisy = true ); // workswith Gibs
	virtual float GetAutoAimRadius() { return 36.0f; } // No class code !!!
	
	void	Flip( bool bZapped = false ); // after vehicle collision or raptor attack ? rename it to Onside.
	
	//bool CanBecomeRagdoll(); // basic client ragdoll  // don't work with a SCHED_DIE ?
	//virtual void	NotifyDeadFriend( CBaseEntity *pFriend ); // works woth SQUADMATES (need this or not ?)
	
private:

	inline CBaseEntity *EntityToWatch( void ); // what entity can be watched
	void				UpdateHead( void );	 // need head poseparameter in the model ?
	
	//bool	FindChasePosition( const Vector &targetPos, Vector &result ); // don't understand but work with flee capabilities
	//bool	GetGroundPosition( const Vector &testPos, Vector &result ); // use with flee capabilities
	bool	GetPathToSoundFleePoint( int soundType ); // flee capabilities
	inline bool	IsFlipped( void ); // rename it to Onside.
	
	//void	InputDisableJump( inputdata_t &inputdata );
	//void	InputEnableJump( inputdata_t &inputdata );
	
	//void	CreateDust( bool placeDecal = true ); // works with CheckLanding Class

	//bool	CheckLanding( void );
	
	//bool	Alone( void );  // work with SQUADMATES
	bool	CheckAlertRadius( void ); // no code class !!!
	//bool	ShouldJump( void ); // look what intereaction is between prethinkschedule class
	
	//void	MeleeAttack( float distance, float damage, QAngle& viewPunch, Vector& shove ); // Attack against an enemy
	//void	StartJump( void );
	//void	LockJumpNode( void );

	//bool	IsUnusableNode(int iNodeID, CAI_Hint *pHint);

	//bool	OnObstructionPreSteer( AILocalMoveGoal_t *pMoveGoal, float distClear, AIMoveResult_t *pResult ); // flee capabilities
	
	void	ManageFleeCapabilities( bool bEnable );
	
	//int		SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );
	//bool	IsFirmlyOnGround( void );
	
	//virtual void Ignite ( float flFlameLifetime, bool bNPCOnly, float flSize, bool bCalledByLevelDesigner ); // need this ?
	//virtual bool FCanCheckAttacks( void ); // looks to be not need
	
	float	m_flIdleDelay;
	//float	m_flJumpTime;
	float	m_flAlertRadius;

	// float	m_flPounceTime;	// need for melee attack
	//int		m_iContext;			//for FValidateHintType context
	
	//GallimimusMoveState_e	m_MoveState;
	//COutputEvent	m_OnReachFightGoal;	//Reached our scripted destination to fight to
	
	//Vector		m_vecSavedJump;
	//Vector		m_vecLastJumpAttempt;

	Vector		m_vecHeardSound;
	bool		m_bHasHeardSound;

	EHANDLE		m_hFollowTarget;

	float		m_flIgnoreSoundTime;		// QuerieHearSounds | Sound time to ignore if earlier than
	
	bool		m_bAgitatedSound;	//Playing agitated sound?
	//string_t	m_strParentSpawner;	//Name of our spawner
	
	//bool		m_bDisableJump; // to enable if gallimimus flee

	float		m_flTimeDrown; // drowning capabilities?
	float		m_flTimeDrownSplash; // idem
	bool		m_bLoopingStarted; //works with agitated
	
	bool		m_bForcedStuckJump; 
	int			m_nBodyBone;	// workwith gibs and mouth attachment
	
	// Used to trigger a heavy damage interrupt if sustained damage is taken
	int			m_nSustainedDamage; // works with heavydamage
	float		m_flLastDamageTime; // linkedto OnTakeDamage Class
	float		m_flZapDuration; // use with flip/onside feature

#if HL2_EPISODIC
	bool		m_bHasDoneAirAttack;  ///< only allowed to apply this damage once per glide
#endif

protected:
	int m_poseHead_Yaw, m_poseHead_Pitch; // need poseparameter ?
	virtual void	PopulatePoseParameters( void );  // need pose parameter ?

private:

	HSOUNDSCRIPTHANDLE	m_hFootstep; // use footstep feature
	
	DEFINE_CUSTOM_AI;

	//==================================================
	// GALLIMIMUS_Conditions
	//==================================================

	enum
	{
		COND_GALLIMIMUS_FLIPPED = LAST_SHARED_CONDITION,
		//COND_GALLIMIMUS_CAN_JUMP,
		//COND_GALLIMIMUS_FOLLOW_TARGET_TOO_FAR,
		COND_GALLIMIMUS_IN_WATER,
		COND_GALLIMIMUS_RECEIVED_ORDERS,
		//COND_GALLIMIMUS_CAN_JUMP_AT_TARGET,
		
	};

	//==================================================
	// GalimimusSchedules
	//==================================================

	enum
	{
		//SCHED_GALLIMIMUS_JUMP = LAST_SHARED_SCHEDULE,
		SCHED_GALLIMIMUS_FLEE_PHYSICS_DANGER = LAST_SHARED_SCHEDULE,
		SCHED_GALLIMIMUS_FLEE_DANGER,
		SCHED_GALLIMIMUS_FLIP,
		SCHED_GALLIMIMUS_ZAP_FLIP,
		SCHED_GALLIMIMUS_DROWN,
		SCHED_GALLIMIMUS_TAKE_COVER_FROM_ENEMY,
		SCHED_GALLIMIMUS_TAKE_COVER_FROM_SAVEPOSITION,
	};

	//==================================================
	// GallimimusTasks
	//==================================================

	enum
	{
		TASK_GALLIMIMUS_SET_CHARGE_GOAL = LAST_SHARED_TASK,
		//TASK_GALLIMIMUS_JUMP,
		TASK_GALLIMIMUS_DROWN,
		TASK_GALLIMIMUS_WAIT_FOR_TRIGGER,
		TASK_GALLIMIMUS_DISMOUNT_NPC,
		TASK_GALLIMIMUS_GET_DANGER_ESCAPE_PATH,
		TASK_GALLIMIMUS_GET_PHYSICS_DANGER_ESCAPE_PATH,
		//TASK_GALLIMIMUS_FACE_JUMP,
		TASK_GALLIMIMUS_GET_PATH_TO_RANDOM_NODE,
		TASK_GALLIMIMUS_FIND_COVER_FROM_SAVEPOSITION,
	};
};


//-----------------------------------------------------------------------------
// Purpose: Shield
//-----------------------------------------------------------------------------
class CGallimimusRepellant : public CPointEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CGallimimusRepellant, CPointEntity );
	~CGallimimusRepellant();

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

extern bool IsGallimimus( CBaseEntity *pEntity );
//extern bool IsGallimimusWorker( CBaseEntity *pEntity );

//#ifdef HL2_EPISODIC
//extern float VelociraptorWorkerBurstRadius( void );
//#endif // HL2_EPISODIC

#endif // NPC_GALLIMIMUS_H
