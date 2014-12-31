//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_VEHICLE_WRANGLER93_H
#define C_VEHICLE_WRANGLER93_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "c_prop_vehicle.h"
#include "flashlighteffect.h"
#include "beam_shared.h"

//=============================================================================
//
// Client-side Wrangler93 Class
//
class C_PropWrangler93 : public C_PropVehicleDriveable
{
	DECLARE_CLASS( C_PropWrangler93, C_PropVehicleDriveable );
public:
	DECLARE_CLIENTCLASS();
	DECLARE_INTERPOLATION();

	C_PropWrangler93();
	~C_PropWrangler93();
public:
	void UpdateViewAngles( C_BasePlayer *pLocalPlayer, CUserCmd *pCmd );
	void DampenEyePosition( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles );

	void OnEnteredVehicle( C_BasePlayer *pPlayer );
	void Simulate( void );
private:
	void DampenForwardMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime );
	void DampenUpMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime );
	void ComputePDControllerCoefficients( float *pCoefficientsOut, float flFrequency, float flDampening, float flDeltaTime );
private:
	Vector		m_vecLastEyePos;
	Vector		m_vecLastEyeTarget;
	Vector		m_vecEyeSpeed;
	Vector		m_vecTargetSpeed;

	float		m_flViewAngleDeltaTime;

	float		m_flWrangler93FOV;
	CFlashlightEffect* m_pHeadlight[2];
	C_Beam*		m_pHeadlightBeam[2];

	bool		m_bHeadlightIsOn;
	bool		m_bHorn;
};

//=============================================================================
//
// Client-side Wrangler93 Class
//
class C_PropExplorer : public C_PropWrangler93
{
	DECLARE_CLASS( C_PropExplorer, C_PropWrangler93 );
public:
	DECLARE_CLIENTCLASS();
	DECLARE_INTERPOLATION();

	C_PropExplorer();
	~C_PropExplorer();
};

#endif
