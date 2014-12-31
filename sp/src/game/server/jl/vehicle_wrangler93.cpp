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
#include "vehicle_Wrangler93.h"
#include "rumble_shared.h"

#include "dlight.h"
#include "vphysics/constraints.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	VEHICLE_HITBOX_DRIVER			1
#define LOCK_SPEED						10
#define WRANGLER93_GUN_YAW				"vehicle_weapon_yaw"
#define WRANGLER93_GUN_PITCH				"vehicle_weapon_pitch"
#define WRANGLER93_GUN_SPIN				"gun_spin"
#define	WRANGLER93_GUN_SPIN_RATE			20

#define CANNON_MAX_UP_PITCH				20
#define CANNON_MAX_DOWN_PITCH			20
#define CANNON_MAX_LEFT_YAW					90
#define CANNON_MAX_RIGHT_YAW			90

#define OVERTURNED_EXIT_WAITTIME		2.0f

#define WRANGLER93_AMMOCRATE_HITGROUP		5

#define WRANGLER93_STEERING_SLOW_ANGLE	50.0f
#define WRANGLER93_STEERING_FAST_ANGLE	15.0f

#define	WRANGLER93_AMMO_CRATE_CLOSE_DELAY	2.0f

#define WRANGLER93_DELTA_LENGTH_MAX		12.0f			// 1 foot
#define WRANGLER93_FRAMETIME_MIN			1e-6

// Seagull perching
const char *g_pWrangler93ThinkContext = "Wrangler93SeagullThink";
#define	WRANGLER93_SEAGULL_THINK_INTERVAL		10.0		// Interval between checks for seagull perches
#define	WRANGLER93_SEAGULL_POOP_INTERVAL		45.0		// Interval between checks for seagull poopage
#define WRANGLER93_SEAGULL_HIDDEN_TIME		15.0		// Time for which the player must be hidden from the Wrangler93 for a seagull to perch
#define WRANGLER93_SEAGULL_MAX_TIME			60.0		// Time at which a seagull will definately perch on the Wrangler93

ConVar	sk_WRANGLER93_gauss_damage( "sk_Wrangler93_gauss_damage", "15" );
ConVar	hud_Wrangler93hint_numentries( "hud_Wrangler93hint_numentries", "10", FCVAR_NONE );
ConVar	g_Wrangler93exitspeed( "g_Wrangler93exitspeed", "100", FCVAR_CHEAT );

extern ConVar autoaim_max_dist;

//=============================================================================
//
// Wrangler93 water data.
//

BEGIN_SIMPLE_DATADESC( Wrangler93WaterData_t )
	DEFINE_ARRAY( m_bWheelInWater,			FIELD_BOOLEAN,	WRANGLER93_WHEEL_COUNT ),
	DEFINE_ARRAY( m_bWheelWasInWater,			FIELD_BOOLEAN,	WRANGLER93_WHEEL_COUNT ),
	DEFINE_ARRAY( m_vecWheelContactPoints,	FIELD_VECTOR,	WRANGLER93_WHEEL_COUNT ),
	DEFINE_ARRAY( m_flNextRippleTime,			FIELD_TIME,		WRANGLER93_WHEEL_COUNT ),
	DEFINE_FIELD( m_bBodyInWater,				FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bBodyWasInWater,			FIELD_BOOLEAN ),
END_DATADESC()	

BEGIN_DATADESC( CPropWrangler93 )
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
	DEFINE_FIELD( m_bHeadlightIsOn, FIELD_BOOLEAN ),
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

	DEFINE_THINKFUNC( Wrangler93SeagullThink ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CPropWrangler93, DT_PropWrangler93 )
	SendPropBool( SENDINFO( m_bHeadlightIsOn ) ),
END_SEND_TABLE();

//IMPLEMENT_SERVERCLASS_ST( CPropExplorer, DT_PropExplorer )
	//SendPropBool( SENDINFO( m_bHeadlightIsOn ) ),
//END_SEND_TABLE(); 

// This is overriden for the episodic Wrangler93
//#ifndef HL2_EPISODIC
LINK_ENTITY_TO_CLASS( prop_vehicle_wrangler93, CPropWrangler93 );

//#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CPropWrangler93::CPropWrangler93( void )
{
	m_bHasGun = false;

	m_flOverturnedTime = 0.0f;
	m_iNumberOfEntries = 0;

	m_bHeadlightIsOn = false;

	m_vecEyeSpeed.Init();

	InitWaterData();
}

CPropWrangler93::~CPropWrangler93( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropWrangler93::Precache( void )
{
	UTIL_PrecacheOther( "npc_seagull" );
	UTIL_PrecacheOther( "npc_quetzal" );

	PrecacheScriptSound( "PropWrangler93.Klaxon" );

	BaseClass::Precache();
}

//------------------------------------------------
// Spawn
//------------------------------------------------
void CPropWrangler93::Spawn( void )
{
	// Setup vehicle as a real-wheels car.
	SetVehicleType( VEHICLE_TYPE_CAR_WHEELS );

	//Precache();

	BaseClass::Spawn();
	m_flHandbrakeTime = gpGlobals->curtime + 0.1;
	m_bInitialHandbrake = false;

	m_flMinimumSpeedToEnterExit = LOCK_SPEED;

	SetBodygroup( 1, false );

	// Initialize pose parameters
	SetPoseParameter( WRANGLER93_GUN_YAW, 0 );
	SetPoseParameter( WRANGLER93_GUN_PITCH, 0 );
	m_nSpinPos = 0;
	SetPoseParameter( WRANGLER93_GUN_SPIN, m_nSpinPos );
	m_aimYaw = 0;
	m_aimPitch = 0;

	AddSolidFlags( FSOLID_NOT_STANDABLE );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CPropWrangler93::Activate()
{
	BaseClass::Activate();

	CBaseServerVehicle *pServerVehicle = dynamic_cast<CBaseServerVehicle *>(GetServerVehicle());
	if ( pServerVehicle )
	{
		if( pServerVehicle->GetPassenger() )
		{
			// If a Wrangler93 comes back from a save game with a driver, make sure the engine rumble starts up.
			pServerVehicle->StartEngineRumble();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &tr - 
//			nDamageType - 
//-----------------------------------------------------------------------------
void CPropWrangler93::DoImpactEffect( trace_t &tr, int nDamageType )
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
void CPropWrangler93::TraceAttack( const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator )
{
	CTakeDamageInfo info = inputInfo;
	if ( ptr->hitbox != VEHICLE_HITBOX_DRIVER )
	{
		if ( inputInfo.GetDamageType() & DMG_BULLET )
		{
			info.ScaleDamage( 0.0001 );
		}
	}

	BaseClass::TraceAttack( inputInfo, vecDir, ptr, pAccumulator );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CPropWrangler93::OnTakeDamage( const CTakeDamageInfo &inputInfo )
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
Vector CPropWrangler93::BodyTarget( const Vector &posSrc, bool bNoisy )
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
// Purpose: Aim Gun at a target
//-----------------------------------------------------------------------------
/*void CPropWrangler93::AimGunAt( Vector *endPos, float flInterval )
{
	Vector	aimPos = *endPos;

	// See if the gun should be allowed to aim
	if ( IsOverturned() || m_bEngineLocked || m_bHasGun == false )
	{
		SetPoseParameter( WRANGLER93_GUN_YAW, 0 );
		SetPoseParameter( WRANGLER93_GUN_PITCH, 0 );
		SetPoseParameter( WRANGLER93_GUN_SPIN, 0 );
		return;

		// Make the gun go limp and look "down"
		Vector	v_forward, v_up;
		AngleVectors( GetLocalAngles(), NULL, &v_forward, &v_up );
		aimPos = WorldSpaceCenter() + ( v_forward * -32.0f ) - Vector( 0, 0, 128.0f );
	}

	matrix3x4_t gunMatrix;
	GetAttachment( LookupAttachment("gun_ref"), gunMatrix );

	// transform the enemy into gun space
	Vector localEnemyPosition;
	VectorITransform( aimPos, gunMatrix, localEnemyPosition );

	// do a look at in gun space (essentially a delta-lookat)
	QAngle localEnemyAngles;
	VectorAngles( localEnemyPosition, localEnemyAngles );
	
	// convert to +/- 180 degrees
	localEnemyAngles.x = UTIL_AngleDiff( localEnemyAngles.x, 0 );	
	localEnemyAngles.y = UTIL_AngleDiff( localEnemyAngles.y, 0 );

	float targetYaw = m_aimYaw + localEnemyAngles.y;
	float targetPitch = m_aimPitch + localEnemyAngles.x;
	
	// Constrain our angles
	float newTargetYaw	= clamp( targetYaw, -CANNON_MAX_LEFT_YAW, CANNON_MAX_RIGHT_YAW );
	float newTargetPitch = clamp( targetPitch, -CANNON_MAX_DOWN_PITCH, CANNON_MAX_UP_PITCH );

	// If the angles have been clamped, we're looking outside of our valid range
	if ( fabs(newTargetYaw-targetYaw) > 1e-4 || fabs(newTargetPitch-targetPitch) > 1e-4 )
	{
		m_bUnableToFire = true;
	}

	targetYaw = newTargetYaw;
	targetPitch = newTargetPitch;

	// Exponentially approach the target
	float yawSpeed = 8;
	float pitchSpeed = 8;

	m_aimYaw = UTIL_Approach( targetYaw, m_aimYaw, yawSpeed );
	m_aimPitch = UTIL_Approach( targetPitch, m_aimPitch, pitchSpeed );

	SetPoseParameter( WRANGLER93_GUN_YAW, -m_aimYaw);
	SetPoseParameter( WRANGLER93_GUN_PITCH, -m_aimPitch );

	InvalidateBoneCache();

	// read back to avoid drift when hitting limits
	// as long as the velocity is less than the delta between the limit and 180, this is fine.
	m_aimPitch = -GetPoseParameter( WRANGLER93_GUN_PITCH );
	m_aimYaw = -GetPoseParameter( WRANGLER93_GUN_YAW );

	// Now draw crosshair for actual aiming point
	Vector	vecMuzzle, vecMuzzleDir;
	QAngle	vecMuzzleAng;

	GetAttachment( "Muzzle", vecMuzzle, vecMuzzleAng );
	AngleVectors( vecMuzzleAng, &vecMuzzleDir );

	trace_t	tr;
	UTIL_TraceLine( vecMuzzle, vecMuzzle + (vecMuzzleDir * MAX_TRACE_LENGTH), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

		// see if we hit something, if so, adjust endPos to hit location
	if ( tr.fraction < 1.0 )
	{
		m_vecGunCrosshair = vecMuzzle + ( vecMuzzleDir * MAX_TRACE_LENGTH * tr.fraction );
	}
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropWrangler93::InitWaterData( void )
{
	m_WaterData.m_bBodyInWater = false;
	m_WaterData.m_bBodyWasInWater = false;

	for ( int iWheel = 0; iWheel < WRANGLER93_WHEEL_COUNT; ++iWheel )
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
void CPropWrangler93::HandleWater( void )
{
	// Only check the wheels and engine in water if we have a driver (player).
	if ( !GetDriver() )
		return;

	// Check to see if we are in water.
	if ( CheckWater() )
	{
		for ( int iWheel = 0; iWheel < WRANGLER93_WHEEL_COUNT; ++iWheel )
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
	for ( int iWheel = 0; iWheel < WRANGLER93_WHEEL_COUNT; ++iWheel )
	{
		m_WaterData.m_bWheelWasInWater[iWheel] = m_WaterData.m_bWheelInWater[iWheel];
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CPropWrangler93::CheckWater( void )
{
	bool bInWater = false;

	// Check all four wheels.
	for ( int iWheel = 0; iWheel < WRANGLER93_WHEEL_COUNT; ++iWheel )
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
void CPropWrangler93::CheckWaterLevel( void )
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

		// Add the Wrangler93's Z view offset
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
void CPropWrangler93::CreateSplash( const Vector &vecPosition )
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
void CPropWrangler93::CreateRipple( const Vector &vecPosition )
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
void CPropWrangler93::Think(void)
{
	BaseClass::Think();		

	/*CBasePlayer	*pPlayer = UTIL_GetLocalPlayer();

	if ( m_bEngineLocked )
	{
		if ( pPlayer != NULL )
		{
			pPlayer->m_Local.m_iHideHUD |= HIDEHUD_VEHICLE_CROSSHAIR;
		}
	}
	else if ( m_bHasGun )
	{
		 Start this as false and update it again each frame
		m_bUnableToFire = false;

		if ( pPlayer != NULL )
		{
			pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_VEHICLE_CROSSHAIR;
		}
	}*/

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

			// HACKHACK: This forces the Wrangler93 to play a sound when it gets entered underwater
			if ( m_VehiclePhysics.IsEngineDisabled() )
			{
				CBaseServerVehicle *pServerVehicle = dynamic_cast<CBaseServerVehicle *>(GetServerVehicle());
				if ( pServerVehicle )
				{
					pServerVehicle->SoundStartDisabled();
				}
			}

			// The first few time we get into the Wrangler93, print the Wrangler93 help
			if ( m_iNumberOfEntries < hud_Wrangler93hint_numentries.GetInt() )
			{
				UTIL_HudHintText( m_hPlayer, "#Valve_Hint_Wrangler93Keys" );
				m_iNumberOfEntries++;
			}
		}

		// If we're exiting and have had the tau cannon removed, we don't want to reset the animation
		GetServerVehicle()->HandleEntryExitFinish( m_bExitAnimOn, !(m_bExitAnimOn) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: If the player uses the Wrangler93 while at the back, he gets ammo from the crate instead
//-----------------------------------------------------------------------------
void CPropWrangler93::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
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
	
	if ( tr.m_pEnt == this && tr.hitgroup == WRANGLER93_AMMOCRATE_HITGROUP )
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
			
			CPASAttenuationFilter sndFilter( this, "PropWrangler93.AmmoOpen" );
			EmitSound( sndFilter, entindex(), "PropWrangler93.AmmoOpen" );
		}

		m_flAmmoCrateCloseTime = gpGlobals->curtime + WRANGLER93_AMMO_CRATE_CLOSE_DELAY;
		return;
	}

	// Fall back and get in the vehicle instead
	BaseClass::Use( pActivator, pCaller, useType, value );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CPropWrangler93::CanExitVehicle( CBaseEntity *pEntity )
{
	return ( !m_bEnterAnimOn && !m_bExitAnimOn && !m_bLocked && (m_nSpeed <= g_Wrangler93exitspeed.GetFloat() ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropWrangler93::DampenEyePosition( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles )
{
	// Get the frametime. (Check to see if enough time has passed to warrent dampening).
	float flFrameTime = gpGlobals->frametime;
	if ( flFrameTime < WRANGLER93_FRAMETIME_MIN )
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
void CPropWrangler93::ComputePDControllerCoefficients( float *pCoefficientsOut,
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
void CPropWrangler93::DampenForwardMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime )
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
	if ( vecDeltaPos.Length() > WRANGLER93_DELTA_LENGTH_MAX )
	{
		float flSign = vecForward.Dot( vecVehicleEyeSpeed ) >= 0.0f ? -1.0f : 1.0f;
		vecVehicleEyePos += flSign * ( vecForward * WRANGLER93_DELTA_LENGTH_MAX );
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
void CPropWrangler93::DampenUpMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime )
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
void CPropWrangler93::SetupMove( CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move )
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
void CPropWrangler93::DriveVehicle( float flFrameTime, CUserCmd *ucmd, int iButtonsDown, int iButtonsReleased )
{
	//Klaxon
	if ( iButtonsDown & IN_ATTACK )
	{
		DevMsg("Wrangler Attack\n");
		CPASAttenuationFilter sndFilter( this, "PropWrangler93.Klaxon" );
		//GetSoundDuration( "PropWrangler93.Klaxon",NULL);
		EmitSound( sndFilter, entindex(), "PropWrangler93.Klaxon" );
	}

	//Adrian: No headlights on Superfly.
	//if ( ucmd->impulse == 100 )
	if ( iButtonsReleased & IN_ATTACK2 )
	{
		DevMsg("Wrangler Attack2\n");
		if (HeadlightIsOn())
		{
			HeadlightTurnOff();
		}
        else 
		{
			HeadlightTurnOn();
		}
	}

	BaseClass::DriveVehicle( flFrameTime, ucmd, iButtonsDown, iButtonsReleased );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//			*pMoveData - 
//-----------------------------------------------------------------------------
void CPropWrangler93::ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMoveData )
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
void CPropWrangler93::UpdateSteeringAngle( void )
{
	float flMaxSpeed = m_VehiclePhysics.GetMaxSpeed();
	float flSpeed = m_VehiclePhysics.GetSpeed();

	float flRatio = 1.0f - ( flSpeed / flMaxSpeed );
	float flSteeringDegrees = WRANGLER93_STEERING_FAST_ANGLE + ( ( WRANGLER93_STEERING_SLOW_ANGLE - WRANGLER93_STEERING_FAST_ANGLE ) * flRatio );
	flSteeringDegrees = clamp( flSteeringDegrees, WRANGLER93_STEERING_FAST_ANGLE, WRANGLER93_STEERING_SLOW_ANGLE );
	m_VehiclePhysics.SetSteeringDegrees( flSteeringDegrees );
}

//-----------------------------------------------------------------------------
// Purpose: Create danger sounds in front of the vehicle.
//-----------------------------------------------------------------------------
/*void CPropWrangler93::CreateDangerSounds( void )
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
	// Make danger sounds ahead of the Wrangler93
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
		// 0.3 seconds ahead of the Wrangler93
		vecSpot = vecStart + vecDir * (speed * 0.3f);
		CSoundEnt::InsertSound( SOUND_DANGER, vecSpot, radius, soundDuration, this, 0 );
		CSoundEnt::InsertSound( SOUND_PHYSICS_DANGER, vecSpot, radius, soundDuration, this, 1 );
		//NDebugOverlay::Box(vecSpot, Vector(-radius,-radius,-radius),Vector(radius,radius,radius), 255, 0, 255, 0, soundDuration);

#if 0
		trace_t	tr;
		// put sounds a bit to left and right but slightly closer to Wrangler93 to make a "cone" of sound 
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
void CPropWrangler93::EnterVehicle( CBasePlayer *pPlayer )
{
	if ( !pPlayer )
		return;

	CheckWater();
	BaseClass::EnterVehicle( pPlayer );

	// Start looking for seagulls to land
	m_hLastPlayerInVehicle = m_hPlayer;
	SetContextThink( NULL, 0, g_pWrangler93ThinkContext );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropWrangler93::ExitVehicle( int nRole )
{
	//HeadlightTurnOff();

	BaseClass::ExitVehicle( nRole );

	// Remember when we last saw the player
	m_flPlayerExitedTime = gpGlobals->curtime;
	m_flLastSawPlayerAt = gpGlobals->curtime;

	if ( GlobalEntity_GetState( "no_seagulls_on_Wrangler93" ) == GLOBAL_OFF )
	{
		// Look for fly nodes
		CHintCriteria hintCriteria;
		hintCriteria.SetHintType( HINT_CROW_FLYTO_POINT );
		hintCriteria.AddIncludePosition( GetAbsOrigin(), 4500 );
		CAI_Hint *pHint = CAI_HintManager::FindHint( GetAbsOrigin(), hintCriteria );
		if ( pHint )
		{
			// Start looking for seagulls to perch on me
			SetContextThink( &CPropWrangler93::Wrangler93SeagullThink, gpGlobals->curtime + WRANGLER93_SEAGULL_THINK_INTERVAL, g_pWrangler93ThinkContext );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: See if we should spawn a seagull on the Wrangler93
//-----------------------------------------------------------------------------
void CPropWrangler93::Wrangler93SeagullThink( void )
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

				SetContextThink( &CPropWrangler93::Wrangler93SeagullThink, gpGlobals->curtime + WRANGLER93_SEAGULL_POOP_INTERVAL, g_pWrangler93ThinkContext );
			}
			else
			{
				// Our seagull's moved off us. 
				m_hSeagull = NULL;
				SetContextThink( &CPropWrangler93::Wrangler93SeagullThink, gpGlobals->curtime + WRANGLER93_SEAGULL_THINK_INTERVAL, g_pWrangler93ThinkContext );
			}
		}
		
		return;
	}

	// Only spawn seagulls if we're upright and out of water
	if ( m_WaterData.m_bBodyInWater || IsOverturned() )
	{
		SetContextThink( &CPropWrangler93::Wrangler93SeagullThink, gpGlobals->curtime + WRANGLER93_SEAGULL_THINK_INTERVAL, g_pWrangler93ThinkContext );
		return;
	}

	// Is the player visible?
	if ( FVisible( m_hLastPlayerInVehicle, MASK_SOLID_BRUSHONLY, &pBlocker ) )
	{
		m_flLastSawPlayerAt = gpGlobals->curtime;
		SetContextThink( &CPropWrangler93::Wrangler93SeagullThink, gpGlobals->curtime + WRANGLER93_SEAGULL_THINK_INTERVAL, g_pWrangler93ThinkContext );
		return;
	}

	// Start checking quickly
	SetContextThink( &CPropWrangler93::Wrangler93SeagullThink, gpGlobals->curtime + 0.2, g_pWrangler93ThinkContext );

	// Not taken enough time yet?
	float flHiddenTime = (gpGlobals->curtime - m_flLastSawPlayerAt);
	if ( flHiddenTime < WRANGLER93_SEAGULL_HIDDEN_TIME )
		return;

	// Random chance based upon the time it's taken
	float flChance = clamp( flHiddenTime / WRANGLER93_SEAGULL_MAX_TIME, 0.0, 1.0 );
	if ( RandomFloat(0,1) < flChance )
	{
		SpawnPerchedSeagull();

		// Don't think for a while
		SetContextThink( &CPropWrangler93::Wrangler93SeagullThink, gpGlobals->curtime + WRANGLER93_SEAGULL_POOP_INTERVAL, g_pWrangler93ThinkContext );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropWrangler93::SpawnPerchedSeagull( void )
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
void CPropWrangler93::AddSeagullPoop( const Vector &vecOrigin )
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
void CPropWrangler93::OnRestore( void )
{
	IServerVehicle *pServerVehicle = GetServerVehicle();
	if ( pServerVehicle != NULL )
	{
		// Restore the passenger information we're holding on to
		pServerVehicle->RestorePassengerInfo();
	}
}

//=========================================
// FORD EXPLORER
//=========================================

BEGIN_DATADESC( CPropExplorer )
	/*
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
	DEFINE_FIELD( m_bHeadlightIsOn, FIELD_BOOLEAN ),
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

	DEFINE_THINKFUNC( Wrangler93SeagullThink ),
	*/
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CPropExplorer, DT_PropExplorer )
	SendPropBool( SENDINFO( m_bHeadlightIsOn ) ),
END_SEND_TABLE();

LINK_ENTITY_TO_CLASS( prop_vehicle_explorer, CPropWrangler93 );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CPropExplorer::CPropExplorer( void )
{
/*	m_bHasGun = false;

	m_flOverturnedTime = 0.0f;
	m_iNumberOfEntries = 0;

	m_bHeadlightIsOn = false;

	m_vecEyeSpeed.Init();

	InitWaterData();
	*/
}

CPropExplorer::~CPropExplorer( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropExplorer::Precache( void )
{
	UTIL_PrecacheOther( "npc_seagull" );
	UTIL_PrecacheOther( "npc_quetzal" );

	PrecacheScriptSound( "PropWrangler93.Klaxon" );

	BaseClass::Precache();
}


//------------------------------------------------
// Spawn
//------------------------------------------------
void CPropExplorer::Spawn( void )
{
	/*
	// Setup vehicle as a real-wheels car.
	SetVehicleType( VEHICLE_TYPE_CAR_WHEELS );

	//Precache();

	BaseClass::Spawn();
	m_flHandbrakeTime = gpGlobals->curtime + 0.1;
	m_bInitialHandbrake = false;

	m_flMinimumSpeedToEnterExit = LOCK_SPEED;

	SetBodygroup( 1, false );

	// Initialize pose parameters
	SetPoseParameter( WRANGLER93_GUN_YAW, 0 );
	SetPoseParameter( WRANGLER93_GUN_PITCH, 0 );
	m_nSpinPos = 0;
	SetPoseParameter( WRANGLER93_GUN_SPIN, m_nSpinPos );
	m_aimYaw = 0;
	m_aimPitch = 0;

	AddSolidFlags( FSOLID_NOT_STANDABLE );
	*/
}

