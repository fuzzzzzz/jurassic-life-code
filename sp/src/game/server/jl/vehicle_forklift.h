//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef VEHICLE_FORKLIFT_H
#define VEHICLE_FORKLIFT_H
#ifdef _WIN32
#pragma once
#endif

#include "vehicle_base.h"
//#include "CForkLiftVehiclePhysics.h"

#define FORKLIFT_WHEEL_COUNT	4

struct ForkliftWaterData_t
{
	bool		m_bWheelInWater[FORKLIFT_WHEEL_COUNT];
	bool		m_bWheelWasInWater[FORKLIFT_WHEEL_COUNT];
	Vector		m_vecWheelContactPoints[FORKLIFT_WHEEL_COUNT];
	float		m_flNextRippleTime[FORKLIFT_WHEEL_COUNT];
	bool		m_bBodyInWater;
	bool		m_bBodyWasInWater;

	DECLARE_SIMPLE_DATADESC();
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CPropForklift : public CPropVehicleDriveable
{
	DECLARE_CLASS( CPropForklift, CPropVehicleDriveable );

public:

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CPropForklift( void );
	~CPropForklift( void );

	// CPropVehicle
	virtual void	ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMoveData );
	virtual void	DriveVehicle( float flFrameTime, CUserCmd *ucmd, int iButtonsDown, int iButtonsReleased );
	virtual void	SetupMove( CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move );
	virtual void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual void	DampenEyePosition( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles );
	virtual bool	AllowBlockedExit( CBasePlayer *pPlayer, int nRole ) { return false; }
	virtual bool	CanExitVehicle( CBaseEntity *pEntity );
	virtual bool	IsVehicleBodyInWater() { return m_WaterData.m_bBodyInWater; }

	// CBaseEntity
	void			Think(void);
	void			Precache( void );
	void			Spawn( void ); 
	void			Activate( void );
	void			OnRestore( void );

	virtual void	CreateServerVehicle( void );
	virtual Vector	BodyTarget( const Vector &posSrc, bool bNoisy = true );
	virtual void	TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );
	virtual int		OnTakeDamage( const CTakeDamageInfo &info );
	virtual void	EnterVehicle( CBasePlayer *pPlayer );
	virtual void	ExitVehicle( int nRole );

	//void			AimGunAt( Vector *endPos, float flInterval );
	//bool			TauCannonHasBeenCutOff( void ) { return m_bGunHasBeenCutOff; }

	// NPC Driving
	//bool			NPC_HasPrimaryWeapon( void ) { return true; }
	//void			NPC_AimPrimaryWeapon( Vector vecTarget );

	const char		*GetTracerType( void ) { return "AR2Tracer"; }
	void			DoImpactEffect( trace_t &tr, int nDamageType );

	//bool HeadlightIsOn( void ) { return m_bHeadlightIsOn; }
	//void HeadlightTurnOn( void ) { m_bHeadlightIsOn = true; }
	//void HeadlightTurnOff( void ) { m_bHeadlightIsOn = false; }

private:

	void		UpElevator( void );
	void		DownElevator( void );
	//void		FireCannon( void );
	//void		ChargeCannon( void );
	//void		FireChargedCannon( void );

	//void		DrawBeam( const Vector &startPos, const Vector &endPos, float width );
	//void		StopChargeSound( void );
	//void		GetCannonAim( Vector *resultDir );

	void		InitWaterData( void );
	void		HandleWater( void );
	bool		CheckWater( void );
	void		CheckWaterLevel( void );
	void		CreateSplash( const Vector &vecPosition );
	void		CreateRipple( const Vector &vecPosition );

	void		UpdateSteeringAngle( void );
	//void		CreateDangerSounds( void );

	void		ComputePDControllerCoefficients( float *pCoefficientsOut, float flFrequency, float flDampening, float flDeltaTime );
	void		DampenForwardMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime );
	void		DampenUpMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime );

	void		ForkliftSeagullThink( void );
	void		SpawnPerchedSeagull( void );
	void		AddSeagullPoop( const Vector &vecOrigin );

	//virtual CFourWheelVehiclePhysics *GetPhysics( void ) { return (CFourWheelVehiclePhysics*)&m_ForkLiftPhysics; }

private:
	
	int				m_nSpinPos;
	float			m_aimYaw;
	float			m_aimPitch;
	float			m_throttleDisableTime;
	float			m_flAmmoCrateCloseTime;

	// handbrake after the fact to keep vehicles from rolling
	float			m_flHandbrakeTime;
	bool			m_bInitialHandbrake;

	float			m_flOverturnedTime;

	Vector			m_vecLastEyePos;
	Vector			m_vecLastEyeTarget;
	Vector			m_vecEyeSpeed;
	Vector			m_vecTargetSpeed;

	ForkliftWaterData_t	m_WaterData;

	int				m_iNumberOfEntries;
	int				m_nAmmoType;

	// Seagull perching
	float			m_flPlayerExitedTime;	// Time at which the player last left this vehicle
	float			m_flLastSawPlayerAt;	// Time at which we last saw the player
	EHANDLE			m_hLastPlayerInVehicle;
	EHANDLE			m_hSeagull;
	bool			m_bHasPoop;

	//float			m_fLevel; // 0-10
	float			m_fLevelTime;

	CPhysicsProp	*m_pElevatorPropBase;
	CDynamicProp	*m_pElevatorProp;

	CNetworkVar( float, m_fLevel );
	//CNetworkVar( bool, m_bHeadlightIsOn );
};

#endif // VEHICLE_Forklift_H
