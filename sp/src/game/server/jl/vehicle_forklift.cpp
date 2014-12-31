//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "engine/IEngineSound.h"
#include "in_buttons.h"
#include "ammodef.h"
#include "IEffects.h"
#include "beam_shared.h"
#include "weapon_gauss.h"
#include "soundenvelope.h"
#include "decals.h"
#include "soundent.h"
#include "grenade_ar2.h"
#include "te_effect_dispatch.h"
#include "hl2_player.h"
#include "ndebugoverlay.h"
#include "movevars_shared.h"
#include "bone_setup.h"
#include "ai_basenpc.h"
#include "ai_hint.h"
#include "npc_crow.h"
#include "globalstate.h"
#include "vehicle_Forklift.h"
#include "rumble_shared.h"
#include "CForkliftServerVehicle.h"

#include "dlight.h"
#include "vphysics/constraints.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	VEHICLE_HITBOX_DRIVER			1
#define LOCK_SPEED						10

#define OVERTURNED_EXIT_WAITTIME		2.0f

#define FORKLIFT_AMMOCRATE_HITGROUP		5

#define FORKLIFT_STEERING_SLOW_ANGLE	50.0f
#define FORKLIFT_STEERING_FAST_ANGLE	15.0f

#define	FORKLIFT_AMMO_CRATE_CLOSE_DELAY	2.0f

#define FORKLIFT_DELTA_LENGTH_MAX		12.0f			// 1 foot
#define FORKLIFT_FRAMETIME_MIN			1e-6

// Seagull perching
const char *g_pForkliftThinkContext = "ForkliftSeagullThink";
#define	FORKLIFT_SEAGULL_THINK_INTERVAL		10.0		// Interval between checks for seagull perches
#define	FORKLIFT_SEAGULL_POOP_INTERVAL		45.0		// Interval between checks for seagull poopage
#define FORKLIFT_SEAGULL_HIDDEN_TIME		15.0		// Time for which the player must be hidden from the Forklift for a seagull to perch
#define FORKLIFT_SEAGULL_MAX_TIME			60.0		// Time at which a seagull will definately perch on the Forklift

//JL(TH)
#define ELEVATOR_MODEL "models/jl/forklift_elevator.mdl"

ConVar	sk_FORKLIFT_gauss_damage( "sk_forklift_gauss_damage", "15" );
ConVar	hud_Forklifthint_numentries( "hud_Forklifthint_numentries", "10", FCVAR_NONE );
ConVar	g_Forkliftexitspeed( "g_Forkliftexitspeed", "100", FCVAR_CHEAT );

extern ConVar autoaim_max_dist;

//=============================================================================
//
// Forklift water data.
//

BEGIN_SIMPLE_DATADESC( ForkliftWaterData_t )
	DEFINE_ARRAY( m_bWheelInWater,			FIELD_BOOLEAN,	FORKLIFT_WHEEL_COUNT ),
	DEFINE_ARRAY( m_bWheelWasInWater,			FIELD_BOOLEAN,	FORKLIFT_WHEEL_COUNT ),
	DEFINE_ARRAY( m_vecWheelContactPoints,	FIELD_VECTOR,	FORKLIFT_WHEEL_COUNT ),
	DEFINE_ARRAY( m_flNextRippleTime,			FIELD_TIME,		FORKLIFT_WHEEL_COUNT ),
	DEFINE_FIELD( m_bBodyInWater,				FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bBodyWasInWater,			FIELD_BOOLEAN ),
END_DATADESC()	

BEGIN_DATADESC( CPropForklift )
	//DEFINE_FIELD( m_bGunHasBeenCutOff, FIELD_BOOLEAN ),
	//DEFINE_FIELD( m_flDangerSoundTime, FIELD_TIME ),
	//DEFINE_FIELD( m_nBulletType, FIELD_INTEGER ),
	//DEFINE_FIELD( m_bCannonCharging, FIELD_BOOLEAN ),
	//DEFINE_FIELD( m_flCannonTime, FIELD_TIME ),
	//DEFINE_FIELD( m_flCannonChargeStartTime, FIELD_TIME ),
	//DEFINE_FIELD( m_vecGunOrigin, FIELD_POSITION_VECTOR ),
	//DEFINE_SOUNDPATCH( m_sndCannonCharge ),
	DEFINE_FIELD( m_nSpinPos, FIELD_INTEGER ),
	DEFINE_FIELD( m_aimYaw, FIELD_FLOAT ),
	DEFINE_FIELD( m_aimPitch, FIELD_FLOAT ),
	DEFINE_FIELD( m_throttleDisableTime, FIELD_TIME ),
	DEFINE_FIELD( m_flHandbrakeTime, FIELD_TIME ),
	DEFINE_FIELD( m_bInitialHandbrake, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flOverturnedTime, FIELD_TIME ),
	DEFINE_FIELD( m_flAmmoCrateCloseTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_vecLastEyePos, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vecLastEyeTarget, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vecEyeSpeed, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vecTargetSpeed, FIELD_POSITION_VECTOR ),
	//DEFINE_FIELD( m_bHeadlightIsOn, FIELD_BOOLEAN ),
	DEFINE_EMBEDDED( m_WaterData ),

	DEFINE_FIELD( m_iNumberOfEntries, FIELD_INTEGER ),
	DEFINE_FIELD( m_nAmmoType, FIELD_INTEGER ),

	DEFINE_FIELD( m_flPlayerExitedTime, FIELD_TIME ),
	DEFINE_FIELD( m_flLastSawPlayerAt, FIELD_TIME ),
	DEFINE_FIELD( m_hLastPlayerInVehicle, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hSeagull, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bHasPoop, FIELD_BOOLEAN ),

	//DEFINE_INPUTFUNC( FIELD_VOID, "StartRemoveTauCannon", InputStartRemoveTauCannon ),
	//DEFINE_INPUTFUNC( FIELD_VOID, "FinishRemoveTauCannon", InputFinishRemoveTauCannon ),

	DEFINE_THINKFUNC( ForkliftSeagullThink ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CPropForklift, DT_PropForklift )
	//SendPropBool( SENDINFO( m_bHeadlightIsOn ) ),
END_SEND_TABLE();

// This is overriden for the episodic Forklift
//#ifndef HL2_EPISODIC
LINK_ENTITY_TO_CLASS( prop_vehicle_forklift, CPropForklift );
//#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CPropForklift::CPropForklift( void )
{
	m_bHasGun = false;
	m_flOverturnedTime = 0.0f;
	m_iNumberOfEntries = 0;
	m_fLevelTime = 0;
	m_fLevel = 0.1f;

	m_pElevatorProp = NULL;
	m_pElevatorPropBase = NULL;

	m_vecEyeSpeed.Init();

	InitWaterData();

	//m_VehiclePhysics
	//m_VehiclePhysics.GetVehicleController
	//this->GetPhysics()->VPhysicsUpdate	
	//elevator->GetModel().
}

CPropForklift::~CPropForklift( void )
{
	//elevator->Remove();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropForklift::CreateServerVehicle( void )
{
	// Create our armed server vehicle
	m_pServerVehicle = new CForkLiftServerVehicle();
	m_pServerVehicle->SetVehicle( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropForklift::Precache( void )
{
	UTIL_PrecacheOther( "npc_seagull" );

	//PrecacheScriptSound( "PropForklift.AmmoClose" );
	//PrecacheScriptSound( "PropForklift.FireCannon" );
	//PrecacheScriptSound( "PropForklift.FireChargedCannon" );
	//PrecacheScriptSound( "PropForklift.AmmoOpen" );
	//PrecacheScriptSound( "PropForklift.Klaxon" );

	//PrecacheScriptSound( "Forklift.GaussCharge" );	

	//PrecacheModel( GAUSS_BEAM_SPRITE );
	//PrecacheModel( "models/jl/forklift_elevator.mdl" );
	PrecacheModel(ELEVATOR_MODEL);

	BaseClass::Precache();
}

//------------------------------------------------
// Spawn
//------------------------------------------------
void CPropForklift::Spawn( void )
{
	// Setup vehicle as a real-wheels car.
	SetVehicleType( VEHICLE_TYPE_CAR_WHEELS );

	//Precache();

	BaseClass::Spawn();
	m_flHandbrakeTime = gpGlobals->curtime + 0.1;
	m_bInitialHandbrake = false;

	m_flMinimumSpeedToEnterExit = LOCK_SPEED;

	//AddPhysicsChild(elevator);
	//this->AddEntityToGroundList(elevator);

	SetBodygroup( 1, false );

	AddSolidFlags( FSOLID_NOT_STANDABLE );

	m_pElevatorProp = assert_cast<CDynamicProp*>(CreateNoSpawn("prop_dynamic", GetAbsOrigin(), GetAbsAngles(), this ));
	if ( m_pElevatorProp )
	{
		m_pElevatorProp->AddEffects( EF_NODRAW );
		m_pElevatorProp->SetModelName( MAKE_STRING( ELEVATOR_MODEL ) );
		m_pElevatorProp->SetModel( ELEVATOR_MODEL );
		m_pElevatorProp->SetSolid( SOLID_VPHYSICS );
		m_pElevatorProp->SetSolidFlags(0);
		m_pElevatorProp->AddFlag( FL_STATICPROP );
		m_pElevatorProp->SetCollisionGroup(COLLISION_GROUP_FORKLIFT);
		m_pElevatorProp->CreateVPhysics();
		DispatchSpawn( m_pElevatorProp );
		m_pElevatorProp->SetParent(this,LookupAttachment( "vehicle_elevator" ));
		m_pElevatorProp->SetLocalAngles(QAngle(0,0,0));
		m_pElevatorProp->SetLocalOrigin(Vector(0,0,0));
	}
	m_pElevatorPropBase = assert_cast<CPhysicsProp*>(CreateEntityByName("prop_physics"));
	if (m_pElevatorPropBase)
	{
		m_pElevatorPropBase->AddEffects( EF_NODRAW );
		m_pElevatorPropBase->SetModel( ELEVATOR_MODEL );
		m_pElevatorPropBase->SetSolid( SOLID_VPHYSICS );
		m_pElevatorPropBase->SetCollisionGroup(COLLISION_GROUP_FORKLIFT);
		m_pElevatorPropBase->CreateVPhysics();
		m_VehiclePhysics.SetElevatorEnable(true,m_pElevatorPropBase->VPhysicsGetObject());
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CPropForklift::Activate()
{
	BaseClass::Activate();

	CBaseServerVehicle *pServerVehicle = dynamic_cast<CBaseServerVehicle *>(GetServerVehicle());
	if ( pServerVehicle )
	{
		if( pServerVehicle->GetPassenger() )
		{
			// If a Forklift comes back from a save game with a driver, make sure the engine rumble starts up.
			pServerVehicle->StartEngineRumble();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &tr - 
//			nDamageType - 
//-----------------------------------------------------------------------------
void CPropForklift::DoImpactEffect( trace_t &tr, int nDamageType )
{
	//Draw our beam
	/*DrawBeam( tr.startpos, tr.endpos, 2.4 );

	if ( (tr.surface.flags & SURF_SKY) == false )
	{
		CPVSFilter filter( tr.endpos );
		te->GaussExplosion( filter, 0.0f, tr.endpos, tr.plane.normal, 0 );

		UTIL_ImpactTrace( &tr, m_nBulletType );
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropForklift::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator )
{
	CTakeDamageInfo newInfo = info;
	if ( ptr->hitbox != VEHICLE_HITBOX_DRIVER )
	{
		if ( newInfo.GetDamageType() & DMG_BULLET )
		{
			newInfo.ScaleDamage( 0.0001 );
		}
	}

	BaseClass::TraceAttack( info, vecDir, ptr, pAccumulator );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CPropForklift::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	//Do scaled up physics damage to the car
	CTakeDamageInfo info = inputInfo;
	info.ScaleDamage( 25 );
	
	// HACKHACK: Scale up grenades until we get a better explosion/pressure damage system
	if ( inputInfo.GetDamageType() & DMG_BLAST )
	{
		info.SetDamageForce( inputInfo.GetDamageForce() * 10 );
	}
	VPhysicsTakeDamage( info );

	// reset the damage
	info.SetDamage( inputInfo.GetDamage() );

	// small amounts of shock damage disrupt the car, but aren't transferred to the player
	if ( info.GetDamageType() == DMG_SHOCK )
	{
		if ( info.GetDamage() <= 10 )
		{
			// take 10% damage and make the engine stall
			info.ScaleDamage( 0.1 );
			m_throttleDisableTime = gpGlobals->curtime + 2;
		}
	}

	//Check to do damage to driver
	if ( GetDriver() )
	{
		//Take no damage from physics damages
		if ( info.GetDamageType() & DMG_CRUSH )
			return 0;

		// Take the damage (strip out the DMG_BLAST)
		info.SetDamageType( info.GetDamageType() & (~DMG_BLAST) );
		GetDriver()->TakeDamage( info );
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Vector CPropForklift::BodyTarget( const Vector &posSrc, bool bNoisy )
{
	Vector	shotPos;
	matrix3x4_t	matrix;

	int eyeAttachmentIndex = LookupAttachment("vehicle_driver_eyes");
	GetAttachment( eyeAttachmentIndex, matrix );
	MatrixGetColumn( matrix, 3, shotPos );

	if ( bNoisy )
	{
		shotPos[0] += random->RandomFloat( -8.0f, 8.0f );
		shotPos[1] += random->RandomFloat( -8.0f, 8.0f );
		shotPos[2] += random->RandomFloat( -8.0f, 8.0f );
	}

	return shotPos;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropForklift::InitWaterData( void )
{
	m_WaterData.m_bBodyInWater = false;
	m_WaterData.m_bBodyWasInWater = false;

	for ( int iWheel = 0; iWheel < FORKLIFT_WHEEL_COUNT; ++iWheel )
	{
		m_WaterData.m_bWheelInWater[iWheel] = false;
		m_WaterData.m_bWheelWasInWater[iWheel] = false;
		m_WaterData.m_vecWheelContactPoints[iWheel].Init();
		m_WaterData.m_flNextRippleTime[iWheel] = 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropForklift::HandleWater( void )
{
	// Only check the wheels and engine in water if we have a driver (player).
	if ( !GetDriver() )
		return;

	// Check to see if we are in water.
	if ( CheckWater() )
	{
		for ( int iWheel = 0; iWheel < FORKLIFT_WHEEL_COUNT; ++iWheel )
		{
			// Create an entry/exit splash!
			if ( m_WaterData.m_bWheelInWater[iWheel] != m_WaterData.m_bWheelWasInWater[iWheel] )
			{
				CreateSplash( m_WaterData.m_vecWheelContactPoints[iWheel] );
				CreateRipple( m_WaterData.m_vecWheelContactPoints[iWheel] );
			}
			
			// Create ripples.
			if ( m_WaterData.m_bWheelInWater[iWheel] && m_WaterData.m_bWheelWasInWater[iWheel] )
			{
				if ( m_WaterData.m_flNextRippleTime[iWheel] < gpGlobals->curtime )
				{
					// Stagger ripple times
					m_WaterData.m_flNextRippleTime[iWheel] = gpGlobals->curtime + RandomFloat( 0.1, 0.3 );
					CreateRipple( m_WaterData.m_vecWheelContactPoints[iWheel] );
				}
			}
		}
	}

	// Save of data from last think.
	for ( int iWheel = 0; iWheel < FORKLIFT_WHEEL_COUNT; ++iWheel )
	{
		m_WaterData.m_bWheelWasInWater[iWheel] = m_WaterData.m_bWheelInWater[iWheel];
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CPropForklift::CheckWater( void )
{
	bool bInWater = false;

	// Check all four wheels.
	for ( int iWheel = 0; iWheel < FORKLIFT_WHEEL_COUNT; ++iWheel )
	{
		// Get the current wheel and get its contact point.
		IPhysicsObject *pWheel = m_VehiclePhysics.GetWheel( iWheel );
		if ( !pWheel )
			continue;

		// Check to see if we hit water.
		if ( pWheel->GetContactPoint( &m_WaterData.m_vecWheelContactPoints[iWheel], NULL ) )
		{
			m_WaterData.m_bWheelInWater[iWheel] = ( UTIL_PointContents( m_WaterData.m_vecWheelContactPoints[iWheel] ) & MASK_WATER ) ? true : false;
			if ( m_WaterData.m_bWheelInWater[iWheel] )
			{
				bInWater = true;
			}
		}
	}

	// Check the body and the BONNET.
	int iEngine = LookupAttachment( "vehicle_engine" );
	Vector vecEnginePoint;
	QAngle vecEngineAngles;
	GetAttachment( iEngine, vecEnginePoint, vecEngineAngles );

	m_WaterData.m_bBodyInWater = ( UTIL_PointContents( vecEnginePoint ) & MASK_WATER ) ? true : false;
	if ( m_WaterData.m_bBodyInWater )
	{
		if ( m_bHasPoop )
		{
			RemoveAllDecals();
			m_bHasPoop = false;
		}

		if ( !m_VehiclePhysics.IsEngineDisabled() )
		{
			m_VehiclePhysics.SetDisableEngine( true );
		}
	}
	else
	{
		if ( m_VehiclePhysics.IsEngineDisabled() )
		{
			m_VehiclePhysics.SetDisableEngine( false );
		}
	}

	if ( bInWater )
	{
		// Check the player's water level.
		CheckWaterLevel();
	}

	return bInWater;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropForklift::CheckWaterLevel( void )
{
	CBaseEntity *pEntity = GetDriver();
	if ( pEntity && pEntity->IsPlayer() )
	{
		CBasePlayer *pPlayer = static_cast<CBasePlayer*>( pEntity );
		
		Vector vecAttachPoint;
		QAngle vecAttachAngles;
		
		// Check eyes. (vehicle_driver_eyes point)
		int iAttachment = LookupAttachment( "vehicle_driver_eyes" );
		GetAttachment( iAttachment, vecAttachPoint, vecAttachAngles );

		// Add the Forklift's Z view offset
		Vector vecUp;
		AngleVectors( vecAttachAngles, NULL, NULL, &vecUp );
		vecUp.z = clamp( vecUp.z, 0.0f, vecUp.z );
		vecAttachPoint.z += r_JeepViewZHeight.GetFloat() * vecUp.z;

		bool bEyes = ( UTIL_PointContents( vecAttachPoint ) & MASK_WATER ) ? true : false;
		if ( bEyes )
		{
			pPlayer->SetWaterLevel( WL_Eyes );
			return;
		}

		// Check waist.  (vehicle_engine point -- see parent function).
		if ( m_WaterData.m_bBodyInWater )
		{
			pPlayer->SetWaterLevel( WL_Waist );
			return;
		}

		// Check feet. (vehicle_feet_passenger0 point)
		iAttachment = LookupAttachment( "vehicle_feet_passenger0" );
		GetAttachment( iAttachment, vecAttachPoint, vecAttachAngles );
		bool bFeet = ( UTIL_PointContents( vecAttachPoint ) & MASK_WATER ) ? true : false;
		if ( bFeet )
		{
			pPlayer->SetWaterLevel( WL_Feet );
			return;
		}

		// Not in water.
		pPlayer->SetWaterLevel( WL_NotInWater );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropForklift::CreateSplash( const Vector &vecPosition )
{
	// Splash data.
	CEffectData	data;
	data.m_fFlags = 0;
	data.m_vOrigin = vecPosition;
	data.m_vNormal.Init( 0.0f, 0.0f, 1.0f );
	VectorAngles( data.m_vNormal, data.m_vAngles );
	data.m_flScale = 10.0f + random->RandomFloat( 0, 2 );

	// Create the splash..
	DispatchEffect( "watersplash", data );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropForklift::CreateRipple( const Vector &vecPosition )
{
	// Ripple data.
	CEffectData	data;
	data.m_fFlags = 0;
	data.m_vOrigin = vecPosition;
	data.m_vNormal.Init( 0.0f, 0.0f, 1.0f );
	VectorAngles( data.m_vNormal, data.m_vAngles );
	data.m_flScale = 10.0f + random->RandomFloat( 0, 2 );
	if ( GetWaterType() & CONTENTS_SLIME )
	{
		data.m_fFlags |= FX_WATER_IN_SLIME;
	}

	// Create the ripple.
	DispatchEffect( "waterripple", data );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropForklift::Think(void)
{
	BaseClass::Think();	

	if (m_pElevatorProp)
	{
		//if (m_bUpdateElevator==true)
		{
			/*Vector vecElevator;
			QAngle vecElevatorAng;
			int iAttachment = LookupAttachment( "vehicle_elevator" );
			GetAttachment( iAttachment, vecElevator, vecElevatorAng );

			m_pElevatorProp->SetAbsOrigin(vecElevator);
			m_pElevatorProp->SetAbsAngles(vecElevatorAng);*/
		}
		//m_bUpdateElevator=!m_bUpdateElevator;
	}	
	
	// Water!?
	HandleWater();

	SetSimulationTime( gpGlobals->curtime );
	
	SetNextThink( gpGlobals->curtime );
	SetAnimatedEveryTick( true );

    if ( !m_bInitialHandbrake )	// after initial timer expires, set the handbrake
	{
		m_bInitialHandbrake = true;
		m_VehiclePhysics.SetHandbrake( true );
		m_VehiclePhysics.Think();
	}

	// Check overturned status.
	if ( !IsOverturned() )
	{
		m_flOverturnedTime = 0.0f;
	}
	else
	{
		m_flOverturnedTime += gpGlobals->frametime;
	}

	StudioFrameAdvance();

	// If the enter or exit animation has finished, tell the server vehicle
	if ( IsSequenceFinished() && (m_bExitAnimOn || m_bEnterAnimOn) )
	{
		if ( m_bEnterAnimOn )
		{
			m_VehiclePhysics.ReleaseHandbrake();
			StartEngine();

			// HACKHACK: This forces the Forklift to play a sound when it gets entered underwater
			if ( m_VehiclePhysics.IsEngineDisabled() )
			{
				CBaseServerVehicle *pServerVehicle = dynamic_cast<CBaseServerVehicle *>(GetServerVehicle());
				if ( pServerVehicle )
				{
					pServerVehicle->SoundStartDisabled();
				}
			}

			// The first few time we get into the Forklift, print the Forklift help
			if ( m_iNumberOfEntries < hud_Forklifthint_numentries.GetInt() )
			{
				UTIL_HudHintText( m_hPlayer, "#Valve_Hint_ForkliftKeys" );
				m_iNumberOfEntries++;
			}
		}

		// If we're exiting and have had the tau cannon removed, we don't want to reset the animation
		//GetServerVehicle()->HandleEntryExitFinish( m_bExitAnimOn, !(m_bExitAnimOn && TauCannonHasBeenCutOff()) );
		GetServerVehicle()->HandleEntryExitFinish( m_bExitAnimOn, !(m_bExitAnimOn) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: If the player uses the Forklift while at the back, he gets ammo from the crate instead
//-----------------------------------------------------------------------------
void CPropForklift::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = ToBasePlayer( pActivator );
	
	if ( pPlayer == NULL)
		return;

	// Find out if the player's looking at our ammocrate hitbox 
	Vector vecForward;
	pPlayer->EyeVectors( &vecForward, NULL, NULL );

	trace_t tr;
	Vector vecStart = pPlayer->EyePosition();
	UTIL_TraceLine( vecStart, vecStart + vecForward * 1024, MASK_SOLID | CONTENTS_DEBRIS | CONTENTS_HITBOX, pPlayer, COLLISION_GROUP_NONE, &tr );
	
	if ( tr.m_pEnt == this && tr.hitgroup == FORKLIFT_AMMOCRATE_HITGROUP )
	{
		// Player's using the crate.
		// Fill up his SMG ammo.
		pPlayer->GiveAmmo( 300, "SMG1");
		
		if ( ( GetSequence() != LookupSequence( "ammo_open" ) ) && ( GetSequence() != LookupSequence( "ammo_close" ) ) )
		{
			// Open the crate
			m_flAnimTime = gpGlobals->curtime;
			m_flPlaybackRate = 0.0;
			SetCycle( 0 );
			ResetSequence( LookupSequence( "ammo_open" ) );
			
			CPASAttenuationFilter sndFilter( this, "PropForklift.AmmoOpen" );
			EmitSound( sndFilter, entindex(), "PropForklift.AmmoOpen" );
		}

		m_flAmmoCrateCloseTime = gpGlobals->curtime + FORKLIFT_AMMO_CRATE_CLOSE_DELAY;
		return;
	}

	// Fall back and get in the vehicle instead
	BaseClass::Use( pActivator, pCaller, useType, value );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CPropForklift::CanExitVehicle( CBaseEntity *pEntity )
{
	return ( !m_bEnterAnimOn && !m_bExitAnimOn && !m_bLocked && (m_nSpeed <= g_Forkliftexitspeed.GetFloat() ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropForklift::DampenEyePosition( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles )
{
	// Get the frametime. (Check to see if enough time has passed to warrent dampening).
	float flFrameTime = gpGlobals->frametime;
	if ( flFrameTime < FORKLIFT_FRAMETIME_MIN )
	{
		vecVehicleEyePos = m_vecLastEyePos;
		DampenUpMotion( vecVehicleEyePos, vecVehicleEyeAngles, 0.0f );
		return;
	}

	// Keep static the sideways motion.

	// Dampen forward/backward motion.
	DampenForwardMotion( vecVehicleEyePos, vecVehicleEyeAngles, flFrameTime );

	// Blend up/down motion.
	DampenUpMotion( vecVehicleEyePos, vecVehicleEyeAngles, flFrameTime );
}

//-----------------------------------------------------------------------------
// Use the controller as follows:
// speed += ( pCoefficientsOut[0] * ( targetPos - currentPos ) + pCoefficientsOut[1] * ( targetSpeed - currentSpeed ) ) * flDeltaTime;
//-----------------------------------------------------------------------------
void CPropForklift::ComputePDControllerCoefficients( float *pCoefficientsOut,
												  float flFrequency, float flDampening,
												  float flDeltaTime )
{
	float flKs = 9.0f * flFrequency * flFrequency;
	float flKd = 4.5f * flFrequency * flDampening;

	float flScale = 1.0f / ( 1.0f + flKd * flDeltaTime + flKs * flDeltaTime * flDeltaTime );

	pCoefficientsOut[0] = flKs * flScale;
	pCoefficientsOut[1] = ( flKd + flKs * flDeltaTime ) * flScale;
}
 
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropForklift::DampenForwardMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime )
{
	// Get forward vector.
	Vector vecForward;
	AngleVectors( vecVehicleEyeAngles, &vecForward);

	// Simulate the eye position forward based on the data from last frame
	// (assumes no acceleration - it will get that from the "spring").
	Vector vecCurrentEyePos = m_vecLastEyePos + m_vecEyeSpeed * flFrameTime;

	// Calculate target speed based on the current vehicle eye position and the last vehicle eye position and frametime.
	Vector vecVehicleEyeSpeed = ( vecVehicleEyePos - m_vecLastEyeTarget ) / flFrameTime;
	m_vecLastEyeTarget = vecVehicleEyePos;	

	// Calculate the speed and position deltas.
	Vector vecDeltaSpeed = vecVehicleEyeSpeed - m_vecEyeSpeed;
	Vector vecDeltaPos = vecVehicleEyePos - vecCurrentEyePos;

	// Clamp.
	if ( vecDeltaPos.Length() > FORKLIFT_DELTA_LENGTH_MAX )
	{
		float flSign = vecForward.Dot( vecVehicleEyeSpeed ) >= 0.0f ? -1.0f : 1.0f;
		vecVehicleEyePos += flSign * ( vecForward * FORKLIFT_DELTA_LENGTH_MAX );
		m_vecLastEyePos = vecVehicleEyePos;
		m_vecEyeSpeed = vecVehicleEyeSpeed;
		return;
	}

	// Generate an updated (dampening) speed for use in next frames position extrapolation.
	float flCoefficients[2];
	ComputePDControllerCoefficients( flCoefficients, r_JeepViewDampenFreq.GetFloat(), r_JeepViewDampenDamp.GetFloat(), flFrameTime );
	m_vecEyeSpeed += ( ( flCoefficients[0] * vecDeltaPos + flCoefficients[1] * vecDeltaSpeed ) * flFrameTime );

	// Save off data for next frame.
	m_vecLastEyePos = vecCurrentEyePos;

	// Move eye forward/backward.
	Vector vecForwardOffset = vecForward * ( vecForward.Dot( vecDeltaPos ) );
	vecVehicleEyePos -= vecForwardOffset;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropForklift::DampenUpMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime )
{
	// Get up vector.
	Vector vecUp;
	AngleVectors( vecVehicleEyeAngles, NULL, NULL, &vecUp );
	vecUp.z = clamp( vecUp.z, 0.0f, vecUp.z );
	vecVehicleEyePos.z += r_JeepViewZHeight.GetFloat() * vecUp.z;

	// NOTE: Should probably use some damped equation here.
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropForklift::SetupMove( CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move )
{
	// If we are overturned and hit any key - leave the vehicle (IN_USE is already handled!).
	if ( m_flOverturnedTime > OVERTURNED_EXIT_WAITTIME )
	{
		if ( (ucmd->buttons & (IN_FORWARD|IN_BACK|IN_MOVELEFT|IN_MOVERIGHT|IN_SPEED|IN_JUMP|IN_ATTACK|IN_ATTACK2) ) && !m_bExitAnimOn )
		{
			// Can't exit yet? We're probably still moving. Swallow the keys.
			if ( !CanExitVehicle(player) )
				return;

			if ( !GetServerVehicle()->HandlePassengerExit( m_hPlayer ) && ( m_hPlayer != NULL ) )
			{
				m_hPlayer->PlayUseDenySound();
			}
			return;
		}
	}

	// If the throttle is disabled or we're upside-down, don't allow throttling (including turbo)
	CUserCmd tmp;
	if ( ( m_throttleDisableTime > gpGlobals->curtime ) || ( IsOverturned() ) )
	{
		m_bUnableToFire = true;
		
		tmp = (*ucmd);
		tmp.buttons &= ~(IN_FORWARD|IN_BACK|IN_SPEED);
		ucmd = &tmp;
	}
	
	BaseClass::SetupMove( player, ucmd, pHelper, move );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropForklift::DriveVehicle( float flFrameTime, CUserCmd *ucmd, int iButtonsDown, int iButtonsReleased )
{
	//Klaxon
	if ( iButtonsDown & IN_RELOAD )
	{
		CPASAttenuationFilter sndFilter( this, "PropForklift.Klaxon" );
		//GetSoundDuration( "PropForklift.Klaxon",NULL);
		EmitSound( sndFilter, entindex(), "PropForklift.Klaxon" );
	}

	//Adrian: No headlights on Superfly.
	//if ( ucmd->impulse == 100 )
	/*if ( iButtonsDown & IN_DUCK )
	{
		if (HeadlightIsOn())
		{
			HeadlightTurnOff();
		}
        else 
		{
			HeadlightTurnOn();
		}
	}*/
		
	int iButtons = ucmd->buttons;

	// If we're holding down an attack button, update our state
	if ( iButtons & IN_ATTACK )
	{
		// UP
		if ( m_fLevelTime < gpGlobals->curtime )
		{
			m_fLevelTime = gpGlobals->curtime + 0.005f;
			if (m_fLevel<10)
				m_fLevel = m_fLevel + 0.05;
			
			SetPoseParameter( "vehicle_elevator", m_fLevel);
			m_VehiclePhysics.CallUpdateElevator();
		}
		Vector vecElevator;
		QAngle vecElevatorAng;
		int iAttachment = LookupAttachment( "vehicle_elevator" );
		GetAttachment( iAttachment, vecElevator, vecElevatorAng );
		m_pElevatorProp->SetAbsOrigin(vecElevator);
		m_pElevatorProp->SetAbsAngles(vecElevatorAng);
	}
	else if ( iButtons & IN_ATTACK2 )
	{
		// Down
		if ( m_fLevelTime < gpGlobals->curtime )
		{
			m_fLevelTime = gpGlobals->curtime + 0.005f;
			if (m_fLevel>0.2)
				m_fLevel = m_fLevel - 0.05;
			
			SetPoseParameter( "vehicle_elevator", m_fLevel);
			m_VehiclePhysics.CallUpdateElevator();
		}
	}

	BaseClass::DriveVehicle( flFrameTime, ucmd, iButtonsDown, iButtonsReleased );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//			*pMoveData - 
//-----------------------------------------------------------------------------
void CPropForklift::ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMoveData )
{
	BaseClass::ProcessMovement( pPlayer, pMoveData );

	// Update the steering angles based on speed.
	UpdateSteeringAngle();

	// Create dangers sounds in front of the vehicle.
	//CreateDangerSounds();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropForklift::UpdateSteeringAngle( void )
{
	float flMaxSpeed = m_VehiclePhysics.GetMaxSpeed();
	float flSpeed = m_VehiclePhysics.GetSpeed();

	float flRatio = 1.0f - ( flSpeed / flMaxSpeed );
	float flSteeringDegrees = FORKLIFT_STEERING_FAST_ANGLE + ( ( FORKLIFT_STEERING_SLOW_ANGLE - FORKLIFT_STEERING_FAST_ANGLE ) * flRatio );
	flSteeringDegrees = clamp( flSteeringDegrees, FORKLIFT_STEERING_FAST_ANGLE, FORKLIFT_STEERING_SLOW_ANGLE );
	m_VehiclePhysics.SetSteeringDegrees( flSteeringDegrees );
}

//-----------------------------------------------------------------------------
// Purpose: Create danger sounds in front of the vehicle.
//-----------------------------------------------------------------------------
/*void CPropForklift::CreateDangerSounds( void )
{
	QAngle dummy;
	GetAttachment( "Muzzle", m_vecGunOrigin, dummy );

	if ( m_flDangerSoundTime > gpGlobals->curtime )
		return;

	QAngle vehicleAngles = GetLocalAngles();
	Vector vecStart = GetAbsOrigin();
	Vector vecDir, vecRight;

	GetVectors( &vecDir, &vecRight, NULL );

	const float soundDuration = 0.25;
	float speed = m_VehiclePhysics.GetHLSpeed();
	// Make danger sounds ahead of the Forklift
	if ( fabs(speed) > 120 )
	{
		Vector	vecSpot;

		float steering = m_VehiclePhysics.GetSteering();
		if ( steering != 0 )
		{
			if ( speed > 0 )
			{
				vecDir += vecRight * steering * 0.5;
			}
			else
			{
				vecDir -= vecRight * steering * 0.5;
			}
			VectorNormalize(vecDir);
		}
		const float radius = speed * 0.4;
		// 0.3 seconds ahead of the Forklift
		vecSpot = vecStart + vecDir * (speed * 0.3f);
		CSoundEnt::InsertSound( SOUND_DANGER, vecSpot, radius, soundDuration, this, 0 );
		CSoundEnt::InsertSound( SOUND_PHYSICS_DANGER, vecSpot, radius, soundDuration, this, 1 );
		//NDebugOverlay::Box(vecSpot, Vector(-radius,-radius,-radius),Vector(radius,radius,radius), 255, 0, 255, 0, soundDuration);

#if 0
		trace_t	tr;
		// put sounds a bit to left and right but slightly closer to Forklift to make a "cone" of sound 
		// in front of it
		vecSpot = vecStart + vecDir * (speed * 0.5f) - vecRight * speed * 0.5;
		UTIL_TraceLine( vecStart, vecSpot, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
		CSoundEnt::InsertSound( SOUND_DANGER, vecSpot, 400, soundDuration, this, 1 );

		vecSpot = vecStart + vecDir * (speed * 0.5f) + vecRight * speed * 0.5;
		UTIL_TraceLine( vecStart, vecSpot, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
		CSoundEnt::InsertSound( SOUND_DANGER, vecSpot, 400, soundDuration, this, 2);
#endif
	}

	m_flDangerSoundTime = gpGlobals->curtime + 0.1;
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropForklift::EnterVehicle( CBasePlayer *pPlayer )
{
	if ( !pPlayer )
		return;

	CheckWater();
	BaseClass::EnterVehicle( pPlayer );

	// Start looking for seagulls to land
	m_hLastPlayerInVehicle = m_hPlayer;
	SetContextThink( NULL, 0, g_pForkliftThinkContext );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropForklift::ExitVehicle( int nRole )
{
	//HeadlightTurnOff();

	BaseClass::ExitVehicle( nRole );

	//If the player has exited, stop charging
	//StopChargeSound();
	//m_bCannonCharging = false;

	// Remember when we last saw the player
	m_flPlayerExitedTime = gpGlobals->curtime;
	m_flLastSawPlayerAt = gpGlobals->curtime;

	if ( GlobalEntity_GetState( "no_seagulls_on_Forklift" ) == GLOBAL_OFF )
	{
		// Look for fly nodes
		CHintCriteria hintCriteria;
		hintCriteria.SetHintType( HINT_CROW_FLYTO_POINT );
		hintCriteria.AddIncludePosition( GetAbsOrigin(), 4500 );
		CAI_Hint *pHint = CAI_HintManager::FindHint( GetAbsOrigin(), hintCriteria );
		if ( pHint )
		{
			// Start looking for seagulls to perch on me
			SetContextThink( &CPropForklift::ForkliftSeagullThink, gpGlobals->curtime + FORKLIFT_SEAGULL_THINK_INTERVAL, g_pForkliftThinkContext );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: See if we should spawn a seagull on the Forklift
//-----------------------------------------------------------------------------
void CPropForklift::ForkliftSeagullThink( void )
{
	if ( !m_hLastPlayerInVehicle )
		return;

	CBaseEntity *pBlocker;

	// Do we already have a seagull?
	if ( m_hSeagull )
	{
		CNPC_Seagull *pSeagull = dynamic_cast<CNPC_Seagull *>( m_hSeagull.Get() );

		if ( pSeagull )
		{
			// Is he still on us?
			if ( pSeagull->m_bOnJeep == true )
			{
				// Make the existing seagull spawn more poop over time
				if ( pSeagull->IsAlive() )
				{
					AddSeagullPoop( pSeagull->GetAbsOrigin() );
				}

				SetContextThink( &CPropForklift::ForkliftSeagullThink, gpGlobals->curtime + FORKLIFT_SEAGULL_POOP_INTERVAL, g_pForkliftThinkContext );
			}
			else
			{
				// Our seagull's moved off us. 
				m_hSeagull = NULL;
				SetContextThink( &CPropForklift::ForkliftSeagullThink, gpGlobals->curtime + FORKLIFT_SEAGULL_THINK_INTERVAL, g_pForkliftThinkContext );
			}
		}
		
		return;
	}

	// Only spawn seagulls if we're upright and out of water
	if ( m_WaterData.m_bBodyInWater || IsOverturned() )
	{
		SetContextThink( &CPropForklift::ForkliftSeagullThink, gpGlobals->curtime + FORKLIFT_SEAGULL_THINK_INTERVAL, g_pForkliftThinkContext );
		return;
	}

	// Is the player visible?
	if ( FVisible( m_hLastPlayerInVehicle, MASK_SOLID_BRUSHONLY, &pBlocker ) )
	{
		m_flLastSawPlayerAt = gpGlobals->curtime;
		SetContextThink( &CPropForklift::ForkliftSeagullThink, gpGlobals->curtime + FORKLIFT_SEAGULL_THINK_INTERVAL, g_pForkliftThinkContext );
		return;
	}

	// Start checking quickly
	SetContextThink( &CPropForklift::ForkliftSeagullThink, gpGlobals->curtime + 0.2, g_pForkliftThinkContext );

	// Not taken enough time yet?
	float flHiddenTime = (gpGlobals->curtime - m_flLastSawPlayerAt);
	if ( flHiddenTime < FORKLIFT_SEAGULL_HIDDEN_TIME )
		return;

	// Random chance based upon the time it's taken
	float flChance = clamp( flHiddenTime / FORKLIFT_SEAGULL_MAX_TIME, 0.0, 1.0 );
	if ( RandomFloat(0,1) < flChance )
	{
		SpawnPerchedSeagull();

		// Don't think for a while
		SetContextThink( &CPropForklift::ForkliftSeagullThink, gpGlobals->curtime + FORKLIFT_SEAGULL_POOP_INTERVAL, g_pForkliftThinkContext );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropForklift::SpawnPerchedSeagull( void )
{
	// Find a point on the car to sit
	Vector vecOrigin;
	QAngle vecAngles;
	int iAttachment = Studio_FindRandomAttachment( GetModelPtr(), "seagull_perch" );
	if ( iAttachment == -1 )
		return;

	// Spawn the seagull
	GetAttachment( iAttachment+1, vecOrigin, vecAngles );
	//vecOrigin.z += 16;

	CNPC_Seagull *pSeagull = (CNPC_Seagull*)CBaseEntity::Create("npc_seagull", vecOrigin, vecAngles, NULL );

	if ( !pSeagull )
		return;
	
	pSeagull->AddSpawnFlags( SF_NPC_FADE_CORPSE );
	pSeagull->SetGroundEntity( this );
	pSeagull->AddFlag( FL_ONGROUND );
	pSeagull->SetOwnerEntity( this );
	pSeagull->SetMoveType( MOVETYPE_FLY );
	pSeagull->m_bOnJeep = true;
	
	m_hSeagull = pSeagull;

	AddSeagullPoop( vecOrigin );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &vecOrigin - 
//-----------------------------------------------------------------------------
void CPropForklift::AddSeagullPoop( const Vector &vecOrigin )
{
	// Drop some poop decals!
	int iDecals = RandomInt( 1,2 );
	for ( int i = 0; i < iDecals; i++ )
	{
		Vector vecPoop = vecOrigin;

		// get circular gaussian spread
		float x, y, z;
		do 
		{
			x = random->RandomFloat(-0.5,0.5) + random->RandomFloat(-0.5,0.5);
			y = random->RandomFloat(-0.5,0.5) + random->RandomFloat(-0.5,0.5);
			z = x*x+y*y;
		} while (z > 1);
		vecPoop += Vector( x * 90, y * 90, 128 );
		
		trace_t tr;
		UTIL_TraceLine( vecPoop, vecPoop - Vector(0,0,512), MASK_SHOT, m_hSeagull, COLLISION_GROUP_NONE, &tr );
		UTIL_DecalTrace( &tr, "BirdPoop" );
	}

	m_bHasPoop = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropForklift::OnRestore( void )
{
	IServerVehicle *pServerVehicle = GetServerVehicle();
	if ( pServerVehicle != NULL )
	{
		// Restore the passenger information we're holding on to
		pServerVehicle->RestorePassengerInfo();
	}
}
