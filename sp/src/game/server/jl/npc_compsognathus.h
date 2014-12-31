//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Defines the headcrab, a tiny, jumpy alien parasite.
//
//=============================================================================//

#ifndef NPC_COMPSOGNATHUS_H
#define NPC_COMPSOGNATHUS_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_squadslot.h"
#include "ai_basenpc.h"
#include "ai_blended_movement.h"
#include "soundent.h"
#include "ai_behavior.h"

typedef CAI_BlendingHost< CAI_BehaviorHost<CAI_BlendedNPC> > CAICompsognathusBase;


//abstract_class CBaseCompsognathus : public CAI_BaseNPC
//{
//
//	DECLARE_CLASS( CBaseCompsognathus, CAI_BaseNPC );

abstract_class CBaseCompsognathus : public CAICompsognathusBase
{
public:
	DECLARE_CLASS( CBaseCompsognathus, CAICompsognathusBase );

//	CBaseCompsognathus( void );
//public:
	void Spawn( void );
	void Precache( void );
	void RunTask( const Task_t *pTask );
	void StartTask( const Task_t *pTask );

	void OnChangeActivity( Activity NewActivity );

	bool IsFirmlyOnGround();
	void MoveOrigin( const Vector &vecDelta );
	void ThrowAt( const Vector &vecPos );
	void ThrowThink( void );
	virtual void JumpAttack( bool bRandomJump, const Vector &vecPos = vec3_origin, bool bThrown = false );
	void JumpToBurrowHint( CAI_Hint *pHint );

	float		GetHitgroupDamageMultiplier( int iHitGroup, const CTakeDamageInfo &info );

	//climb and jump capability
//	void	ClimbTouch( CBaseEntity *pOther );
	bool	IsJumpLegal(const Vector &startPos, const Vector &apex, const Vector &endPos) const;
	bool	MovementCost( int moveType, const Vector &vecStart, const Vector &vecEnd, float *pCost );
	bool	ShouldFailNav( bool bMovementFailed );
	void	BeginNavJump( void );
	void	EndNavJump( void );
	bool	IsNavJumping( void ) { return m_fIsNavJumping; }
	void	OnNavJumpHitApex( void );
	void	BeginAttackJump( void );
	void	EndAttackJump( void );
	//bool	FCanCheckHintNodes( void );
	bool	BecomeRagdollOnClient( const Vector &force );
	void	InputRagdoll( inputdata_t &inputdata );
	bool	CanBecomeRagdoll( void );
	bool	ShouldGib( const CTakeDamageInfo &info );
	bool	CorpseGib( const CTakeDamageInfo &info );
	bool	CreateVPhysics();
	
	bool	HasHeadroom();
	void	LeapTouch ( CBaseEntity *pOther );
	virtual void TouchDamage( CBaseEntity *pOther );
	void	Touch( CBaseEntity *pOther );
	Vector	BodyTarget( const Vector &posSrc, bool bNoisy = true );
	float	GetAutoAimRadius();
	void	TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );
	void	Ignite( float flFlameLifetime, bool bNPCOnly = true, float flSize = 0.0f, bool bCalledByLevelDesigner = false );

	float	MaxYawSpeed( void );
	void	GatherConditions( void );
	void	PrescheduleThink( void );
	Class_T Classify( void );
	void	HandleAnimEvent( animevent_t *pEvent );
	int		RangeAttack1Conditions ( float flDot, float flDist );
	void	MeleeAttack( float distance, float damage, QAngle &viewPunch, Vector &shove );

	virtual int		MeleeAttack1Conditions( float flDot, float flDist ); // compy melee attack
	
	int		OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void	ClampRagdollForce( const Vector &vecForceIn, Vector *vecForceOut );
	void	Event_Killed( const CTakeDamageInfo &info );
	void	BuildScheduleTestBits( void );
	bool	FValidateHintType( CAI_Hint *pHint );

	bool	IsJumping( void ) { return m_bMidJump; }

	virtual void BiteSound( void ) = 0;
	virtual void AttackSound( void ) {};
	virtual void ImpactSound( void ) {};
	virtual void TelegraphSound( void ) {};

	virtual int		SelectSchedule( void );
	virtual int		SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );
	virtual int		TranslateSchedule( int scheduleType );

	virtual float	GetReactionDelay( CBaseEntity *pEnemy ) { return 0.0; }

	//bool		AddTurnGesture( float flYD ) { return true; };

	bool			HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);

	void	CrawlFromCanister();

	virtual	bool		AllowedToIgnite( void ) { return true; }

	virtual bool CanBeAnEnemyOf( CBaseEntity *pEnemy );
private:
	//upddate head
	void	UpdateHead( void );
	//bool	ShouldWatchEnemy( void ); 
	inline CBaseEntity *EntityToWatch( void );

public:

	bool	IsHangingFromCeiling( void ) 
	{ 
#ifdef HL2_EPISODIC
		return m_bHangingFromCeiling;	
#else
		return false;
#endif
	}

	virtual void PlayerHasIlluminatedNPC( CBasePlayer *pPlayer, float flDot );

	void DropFromCeiling( void );

	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();

protected:
	void CompsognathusInit();

	void Leap( const Vector &vecVel );

	void GrabHintNode( CAI_Hint *pHint );
	bool FindBurrow( const Vector &origin, float distance, bool excludeNear );
	bool ValidBurrowPoint( const Vector &point );
	void ClearBurrowPoint( const Vector &origin );
	void Burrow( void );
	void Unburrow( void );
	void SetBurrowed( bool bBurrowed );
	void JumpFromCanister();

	// Begins the climb from the canister
	void BeginClimbFromCanister();

	void InputBurrow( inputdata_t &inputdata );
	void InputBurrowImmediate( inputdata_t &inputdata );
	void InputUnburrow( inputdata_t &inputdata );

	void InputStartHangingFromCeiling( inputdata_t &inputdata );
	void InputDropFromCeiling( inputdata_t &inputdata );

	int CalcDamageInfo( CTakeDamageInfo *pInfo );
	void CreateDust( bool placeDecal = true );

	// Eliminates roll + pitch potentially in the headcrab at canister jump time
	void EliminateRollAndPitch();

	float InnateRange1MinRange( void );
	float InnateRange1MaxRange( void );

protected:

	int m_poseHead_Yaw, m_poseHead_Pitch;
	virtual void	PopulatePoseParameters( void );

	
	static const char *pMoanSounds[];
	int		m_nGibCount;
	float	m_flTimeDrown;
	Vector	m_vecCommittedJumpPos;	// The position of our enemy when we locked in our jump attack.

	float	m_flNextNPCThink;
	float	m_flIgnoreWorldCollisionTime;

	bool	m_bCommittedToJump;		// Whether we have 'locked in' to jump at our enemy.
	bool	m_bCrawlFromCanister;
	bool	m_bStartBurrowed;
	bool	m_bBurrowed;
	bool	m_bHidden;
	bool	m_bMidJump;
	bool	m_bAttackFailed;		// whether we ran into a wall during a jump.

	float	m_flBurrowTime;
	int		m_nContext;			// for FValidateHintType context
	int		m_nJumpFromCanisterDir;

	bool	m_bHangingFromCeiling;
	float	m_flIlluminatedTime;
	// climb/jump capability (Sound stuff)
//	unsigned char	m_iClimbCount; // counts rungs climbed (for sound)
	bool			m_fIsNavJumping;
	mutable float	m_flJumpDist;
	bool			m_fHitApex;

	// update head
	//int m_poseHead_Yaw, m_poseHead_Pitch;
	//virtual void	PopulatePoseParameters( void );

private: // climb/jump capability

	EHANDLE		m_hFollowTarget;
	int			m_nBodyBone;
	bool	m_fJustJumped;
	float	m_flJumpStartAltitude;
	CSoundPatch	*m_pLayer2; // used for climbing ladders, and when jumping (pre apex)

};

//=========================================================
//=========================================================
// The ever popular chubby classic headcrab
//=========================================================
//=========================================================


class CCompsognathus : public CBaseCompsognathus
{
	DECLARE_CLASS( CCompsognathus, CBaseCompsognathus );

public:
	void Precache( void );
	void Spawn( void );

	float	MaxYawSpeed( void );
	Activity NPC_TranslateActivity( Activity eNewActivity );

	void	BiteSound( void );
	void	PainSound( const CTakeDamageInfo &info );
	void	DeathSound( const CTakeDamageInfo &info );
	void	IdleSound( void );
	void	AlertSound( void );
	void	AttackSound( void );
	void	TelegraphSound( void );
};

//=========================================================
//=========================================================
// The spindly, fast headcrab
//=========================================================
//=========================================================

class CFastCompsognathus : public CBaseCompsognathus
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CFastCompsognathus, CBaseCompsognathus );

	void	Precache( void );
	void	Spawn( void );
	bool	QuerySeeEntity(CBaseEntity *pSightEnt, bool bOnlyHateOrFearIfNPC = false);
	
	float	MaxYawSpeed( void );
	
//climb and jump
//	Activity NPC_TranslateActivity( Activity baseAct );
	void	PrescheduleThink( void );
	void	RunTask( const Task_t *pTask );
	void	StartTask( const Task_t *pTask );

	int		SelectSchedule( void );
	int		TranslateSchedule( int scheduleType );

	int		m_iRunMode;
	float	m_flRealGroundSpeed;
	float	m_flSlowRunTime;
	float	m_flPauseTime;
	Vector	m_vecJumpVel;

	void	BiteSound( void );
	void	PainSound( const CTakeDamageInfo &info );
	void	DeathSound( const CTakeDamageInfo &info );
	void	IdleSound( void );
	void	AlertSound( void );
	void	AttackSound( void );


	enum SquadSlot_t
	{	
		SQUAD_SLOT_ENGAGE1 = LAST_SHARED_SQUADSLOT,
		SQUAD_SLOT_ENGAGE2,
		SQUAD_SLOT_ENGAGE3,
		SQUAD_SLOT_ENGAGE4,
	};
protected:
	void CreateDust( bool placeDecal = true );

	DEFINE_CUSTOM_AI;
};


//=========================================================
//=========================================================
// Treacherous black headcrab
//=========================================================
//=========================================================
class CBlackCompsognathus : public CBaseCompsognathus
{
	DECLARE_CLASS( CBlackCompsognathus, CBaseCompsognathus );

public:
	void Eject( const QAngle &vecAngles, float flVelocityScale, CBaseEntity *pEnemy );
	void EjectTouch( CBaseEntity *pOther );

	//
	// CBaseHeadcrab implementation.
	//
	void TouchDamage( CBaseEntity *pOther );
	void BiteSound( void );
	void AttackSound( void );

	//
	// CAI_BaseNPC implementation.
	//
	virtual void PrescheduleThink( void );
	virtual void BuildScheduleTestBits( void );
	virtual int SelectSchedule( void );
	virtual int TranslateSchedule( int scheduleType );

	virtual Activity NPC_TranslateActivity( Activity eNewActivity );
	virtual void HandleAnimEvent( animevent_t *pEvent );
	virtual float MaxYawSpeed( void );

	virtual int	GetSoundInterests( void ) { return (BaseClass::GetSoundInterests() | SOUND_DANGER | SOUND_BULLET_IMPACT); }

	bool IsHeavyDamage( const CTakeDamageInfo &info );

	virtual void PainSound( const CTakeDamageInfo &info );
	virtual void DeathSound( const CTakeDamageInfo &info );
	virtual void IdleSound( void );
	virtual void AlertSound( void );
	virtual void ImpactSound( void );
	virtual void TelegraphSound( void );
#if HL2_EPISODIC
	virtual bool FInViewCone( CBaseEntity *pEntity );
#endif

	//
	// CBaseEntity implementation.
	//
	virtual void Precache( void );
	virtual void Spawn( void );

	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();

private:


	void JumpFlinch( const Vector *pvecAwayFromPos );
	void Panic( float flDuration );

	bool m_bPanicState;
	float m_flPanicStopTime;
	float m_flNextHopTime;		// Keeps us from hopping too often due to damage.
};


#endif // NPC_COMPSOGNATHUS_H
