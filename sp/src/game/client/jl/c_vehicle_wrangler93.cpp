//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "c_vehicle_wrangler93.h"
#include "movevars_shared.h"
#include "c_baseplayer.h"
#include "c_te_effect_dispatch.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar default_fov;

ConVar r_Wrangler93ViewBlendTo( "r_Wrangler93ViewBlendTo", "1", FCVAR_CHEAT );
ConVar r_Wrangler93ViewBlendToScale( "r_Wrangler93ViewBlendToScale", "0.03", FCVAR_CHEAT );
ConVar r_Wrangler93ViewBlendToTime( "r_Wrangler93ViewBlendToTime", "1.5", FCVAR_CHEAT );
ConVar r_Wrangler93FOV( "r_Wrangler93FOV", "90", FCVAR_CHEAT );

#define WRANGLER93_DELTA_LENGTH_MAX	12.0f			// 1 foot
#define WRANGLER93_FRAMETIME_MIN		1e-6
#define WRANGLER93_HEADLIGHT_DISTANCE 1000
#define WRANGLER93_BEAM_DISTANCE 192
#define WRANGLER93_BEAM_WIDTH 32
#define WRANGLER93_BEAM_END_WIDTH 256
#define WRANGLER93_BEAM_HALO_WIDTH 48

IMPLEMENT_CLIENTCLASS_DT( C_PropWrangler93, DT_PropWrangler93, CPropWrangler93 )
	RecvPropBool( RECVINFO( m_bHeadlightIsOn ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_PropWrangler93::C_PropWrangler93()
{
	m_vecEyeSpeed.Init();
	m_flViewAngleDeltaTime = 0.0f;
	for (int i=0;i<2;++i)
	{
		m_pHeadlight[i] = new CFlashlightEffect;
		m_pHeadlight[i]->TurnOff();

		m_pHeadlightBeam[i] = C_Beam::BeamCreate( "sprites/glow_test02.vmt", WRANGLER93_BEAM_WIDTH );
		ClientEntityList().AddNonNetworkableEntity( m_pHeadlightBeam[i] );
		m_pHeadlightBeam[i]->SetHDRColorScale( 1.f );
		//const color24 c = GetRenderColor();
		m_pHeadlightBeam[i]->SetColor( 224, 224, 192 );
		m_pHeadlightBeam[i]->SetHaloTexture( PrecacheModel("sprites/light_glow03.vmt") );
		m_pHeadlightBeam[i]->SetHaloScale(WRANGLER93_BEAM_HALO_WIDTH);
		m_pHeadlightBeam[i]->SetWidth(WRANGLER93_BEAM_WIDTH);
		m_pHeadlightBeam[i]->SetEndWidth(WRANGLER93_BEAM_END_WIDTH);
		m_pHeadlightBeam[i]->SetBeamFlags( (FBEAM_SHADEOUT|FBEAM_NOTILE) );
		m_pHeadlightBeam[i]->SetBrightness(128);
		m_pHeadlightBeam[i]->SetNoise(0);

		//m_pHeadlightBeam[i]->TurnOff();
	}
	m_ViewSmoothingData.flFOV = r_Wrangler93FOV.GetFloat();	
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
C_PropWrangler93::~C_PropWrangler93()
{
	for (int i=0;i<2;++i)
	{
		if ( m_pHeadlight[i] )
		{
			delete m_pHeadlight[i];
		}

		/*
		if ( m_pHeadlightBeam[i] )
		{
			delete m_pHeadlightBeam[i];
		}
		*/
	}
}

void C_PropWrangler93::Simulate( void )
{
	// The dim light is the flashlight.
	if ( m_bHeadlightIsOn )
	{
		for (int i=0;i<2;++i)
		{
			if (!m_pHeadlight[i]->IsOn())
			{
				m_pHeadlight[i]->TurnOn();
				//m_pHeadlightBeam[i]->TurnOn();
				m_pHeadlightBeam[i]->TurnOff();
			}

			QAngle vAngle;
			Vector vVector;
			Vector vecForward, vecRight, vecUp;

			char sAttachenemt[32];
			sprintf( sAttachenemt, "headlight%d", i+1 );
			int iAttachment = LookupAttachment( sAttachenemt );

			if ( iAttachment != -1 )
			{
				GetAttachment( iAttachment, vVector, vAngle );
				AngleVectors( vAngle, &vecForward, &vecRight, &vecUp );

				m_pHeadlight[i]->UpdateLight( vVector, vecForward, vecRight, vecUp, WRANGLER93_HEADLIGHT_DISTANCE );
				m_pHeadlightBeam[i]->PointsInit( vVector, vVector + vecForward * WRANGLER93_BEAM_DISTANCE );
			}
		}
	}
	else
	{
		// Turned off the flashlight
		for (int i=0;i<2;++i)
		{
			if (m_pHeadlight[i]->IsOn())
			{
				m_pHeadlight[i]->TurnOff();
				m_pHeadlightBeam[i]->TurnOn();
			}
		}
	}

	BaseClass::Simulate();
}

//-----------------------------------------------------------------------------
// Purpose: Blend view angles.
//-----------------------------------------------------------------------------
void C_PropWrangler93::UpdateViewAngles( C_BasePlayer *pLocalPlayer, CUserCmd *pCmd )
{
	if ( r_Wrangler93ViewBlendTo.GetInt() )
	{
		// Check to see if the mouse has been touched in a bit or that we are not throttling.
		if ( ( pCmd->mousedx != 0 || pCmd->mousedy != 0 ) || ( fabsf( m_flThrottle ) < 0.01f ) )
		{
			m_flViewAngleDeltaTime = 0.0f;
		}
		else
		{
			m_flViewAngleDeltaTime += gpGlobals->frametime;
		}

		if ( m_flViewAngleDeltaTime > r_Wrangler93ViewBlendToTime.GetFloat() )
		{
			// Blend the view angles.
			int eyeAttachmentIndex = LookupAttachment( "vehicle_driver_eyes" );
			Vector vehicleEyeOrigin;
			QAngle vehicleEyeAngles;
			GetAttachmentLocal( eyeAttachmentIndex, vehicleEyeOrigin, vehicleEyeAngles );
			
			QAngle outAngles;
			InterpolateAngles( pCmd->viewangles, vehicleEyeAngles, outAngles, r_Wrangler93ViewBlendToScale.GetFloat() );
			pCmd->viewangles = outAngles;
		}
	}

	BaseClass::UpdateViewAngles( pLocalPlayer, pCmd );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_PropWrangler93::DampenEyePosition( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles )
{
#ifdef HL2_CLIENT_DLL
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
#endif
}


//-----------------------------------------------------------------------------
// Use the controller as follows:
// speed += ( pCoefficientsOut[0] * ( targetPos - currentPos ) + pCoefficientsOut[1] * ( targetSpeed - currentSpeed ) ) * flDeltaTime;
//-----------------------------------------------------------------------------
void C_PropWrangler93::ComputePDControllerCoefficients( float *pCoefficientsOut,
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
void C_PropWrangler93::DampenForwardMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime )
{
	// vecVehicleEyePos = real eye position this frame

	// m_vecLastEyePos = eye position last frame
	// m_vecEyeSpeed = eye speed last frame
	// vecPredEyePos = predicted eye position this frame (assuming no acceleration - it will get that from the pd controller).
	// vecPredEyeSpeed = predicted eye speed
	Vector vecPredEyePos = m_vecLastEyePos + m_vecEyeSpeed * flFrameTime;
	Vector vecPredEyeSpeed = m_vecEyeSpeed;

	// m_vecLastEyeTarget = real eye position last frame (used for speed calculation).
	// Calculate the approximate speed based on the current vehicle eye position and the eye position last frame.
	Vector vecVehicleEyeSpeed = ( vecVehicleEyePos - m_vecLastEyeTarget ) / flFrameTime;
	m_vecLastEyeTarget = vecVehicleEyePos;
	if (vecVehicleEyeSpeed.Length() == 0.0)
		return;

	// Calculate the delta between the predicted eye position and speed and the current eye position and speed.
	Vector vecDeltaSpeed = vecVehicleEyeSpeed - vecPredEyeSpeed;
	Vector vecDeltaPos = vecVehicleEyePos - vecPredEyePos;

	// Forward vector.
	Vector vecForward;
	AngleVectors( vecVehicleEyeAngles, &vecForward );

	float flDeltaLength = vecDeltaPos.Length();
	if ( flDeltaLength > WRANGLER93_DELTA_LENGTH_MAX )
	{
		// Clamp.
		float flDelta = flDeltaLength - WRANGLER93_DELTA_LENGTH_MAX;
		if ( flDelta > 40.0f )
		{
			// This part is a bit of a hack to get rid of large deltas (at level load, etc.).
			m_vecLastEyePos = vecVehicleEyePos;
			m_vecEyeSpeed = vecVehicleEyeSpeed;
		}
		else
		{
			// Position clamp.
			float flRatio = WRANGLER93_DELTA_LENGTH_MAX / flDeltaLength;
			vecDeltaPos *= flRatio;
			Vector vecForwardOffset = vecForward * ( vecForward.Dot( vecDeltaPos ) );
			vecVehicleEyePos -= vecForwardOffset;
			m_vecLastEyePos = vecVehicleEyePos;

			// Speed clamp.
			vecDeltaSpeed *= flRatio;
			float flCoefficients[2];
			ComputePDControllerCoefficients( flCoefficients, r_JeepViewDampenFreq.GetFloat(), r_JeepViewDampenDamp.GetFloat(), flFrameTime );
			m_vecEyeSpeed += ( ( flCoefficients[0] * vecDeltaPos + flCoefficients[1] * vecDeltaSpeed ) * flFrameTime );
		}
	}
	else
	{
		// Generate an updated (dampening) speed for use in next frames position prediction.
		float flCoefficients[2];
		ComputePDControllerCoefficients( flCoefficients, r_JeepViewDampenFreq.GetFloat(), r_JeepViewDampenDamp.GetFloat(), flFrameTime );
		m_vecEyeSpeed += ( ( flCoefficients[0] * vecDeltaPos + flCoefficients[1] * vecDeltaSpeed ) * flFrameTime );
		
		// Save off data for next frame.
		m_vecLastEyePos = vecPredEyePos;
		
		// Move eye forward/backward.
		Vector vecForwardOffset = vecForward * ( vecForward.Dot( vecDeltaPos ) );
		vecVehicleEyePos -= vecForwardOffset;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_PropWrangler93::DampenUpMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime )
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
void C_PropWrangler93::OnEnteredVehicle( C_BasePlayer *pPlayer )
{
	int eyeAttachmentIndex = LookupAttachment( "vehicle_driver_eyes" );
	Vector vehicleEyeOrigin;
	QAngle vehicleEyeAngles;
	GetAttachment( eyeAttachmentIndex, vehicleEyeOrigin, vehicleEyeAngles );

	m_vecLastEyeTarget = vehicleEyeOrigin;
	m_vecLastEyePos = vehicleEyeOrigin;
	m_vecEyeSpeed = vec3_origin;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
/*void WheelDustCallback( const CEffectData &data )
{
	CSmartPtr<CSimpleEmitter> pSimple = CSimpleEmitter::Create( "dust" );
	pSimple->SetSortOrigin( data.m_vOrigin );
	pSimple->SetNearClip( 32, 64 );

	SimpleParticle	*pParticle;

	Vector	offset;

	//FIXME: Better sampling area
	offset = data.m_vOrigin + ( data.m_vNormal * data.m_flScale );
	
	//Find area ambient light color and use it to tint smoke
	Vector	worldLight = WorldGetLightForPoint( offset, true );

	PMaterialHandle	hMaterial = pSimple->GetPMaterial("particle/particle_smokegrenade");;

	//Throw puffs
	offset.Random( -(data.m_flScale*16.0f), data.m_flScale*16.0f );
	offset.z = 0.0f;
	offset += data.m_vOrigin + ( data.m_vNormal * data.m_flScale );

	pParticle = (SimpleParticle *) pSimple->AddParticle( sizeof(SimpleParticle), hMaterial, offset );

	if ( pParticle != NULL )
	{			
		pParticle->m_flLifetime		= 0.0f;
		pParticle->m_flDieTime		= random->RandomFloat( 0.25f, 0.5f );
		
		pParticle->m_vecVelocity = RandomVector( -1.0f, 1.0f );
		VectorNormalize( pParticle->m_vecVelocity );
		pParticle->m_vecVelocity[2] += random->RandomFloat( 16.0f, 32.0f ) * (data.m_flScale*2.0f);

		int	color = random->RandomInt( 100, 150 );

		pParticle->m_uchColor[0] = 16 + ( worldLight[0] * (float) color );
		pParticle->m_uchColor[1] = 8 + ( worldLight[1] * (float) color );
		pParticle->m_uchColor[2] = ( worldLight[2] * (float) color );

		pParticle->m_uchStartAlpha	= random->RandomInt( 64.0f*data.m_flScale, 128.0f*data.m_flScale );
		pParticle->m_uchEndAlpha	= 0;
		pParticle->m_uchStartSize	= random->RandomInt( 16, 24 ) * data.m_flScale;
		pParticle->m_uchEndSize		= random->RandomInt( 32, 48 ) * data.m_flScale;
		pParticle->m_flRoll			= random->RandomInt( 0, 360 );
		pParticle->m_flRollDelta	= random->RandomFloat( -2.0f, 2.0f );
	}
}*/

//DECLARE_CLIENT_EFFECT( "WheelDust", WheelDustCallback );


IMPLEMENT_CLIENTCLASS_DT( C_PropExplorer, DT_PropExplorer, CPropExplorer )
	RecvPropBool( RECVINFO( m_bHeadlightIsOn ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_PropExplorer::C_PropExplorer()
{
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
C_PropExplorer::~C_PropExplorer()
{
}