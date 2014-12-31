//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "c_basehlcombatweapon.h"
#include "iviewrender_beams.h"
#include "beam_shared.h"
#include "c_weapon__stubs.h"
#include "materialsystem/IMaterial.h"
#include "ClientEffectPrecacheSystem.h"
#include "beamdraw.h"
#include "flashlighteffect.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectStunstick )
//CLIENTEFFECT_MATERIAL( "effects/stunstick" )
//CLIENTEFFECT_REGISTER_END()

void FormatViewModelAttachment( Vector &vOrigin, bool bInverse );

class C_WeaponFlashLight : public C_BaseHLBludgeonWeapon
{
	DECLARE_CLASS( C_WeaponFlashLight, C_BaseHLBludgeonWeapon );
public:
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	int DrawModel( int flags )
	{
		//FIXME: This sucks, but I can't easily create temp ents...

		/*if ( 0 )
		{
			Vector	vecOrigin;
			QAngle	vecAngles;
			float	color[3];

			color[0] = color[1] = color[2] = random->RandomFloat( 0.1f, 0.2f );

			GetAttachment( 1, vecOrigin, vecAngles );

			Vector	vForward;
			AngleVectors( vecAngles, &vForward );

			Vector vEnd = vecOrigin - vForward * 1.0f;

			IMaterial *pMaterial = materials->FindMaterial( "effects/stunstick", NULL, false );

			materials->Bind( pMaterial );
			DrawHalo( pMaterial, vEnd, random->RandomFloat( 4.0f, 6.0f ), color );

			color[0] = color[1] = color[2] = random->RandomFloat( 0.9f, 1.0f );

			DrawHalo( pMaterial, vEnd, random->RandomFloat( 2.0f, 3.0f ), color );
		}*/

		return BaseClass::DrawModel( flags );
	}

	// Do part of our effect
	void ClientThink( void )
	{
		//DevMsg("Light State : %d\n",active,NULL);
		if ( m_bActive )
		{
			if ( m_pHeadlight == NULL )
			{
				// Turned on the headlight; create it.
				m_pHeadlight = new CHeadlightEffect;
				//m_pHeadlight = new CFlashlightEffect;

				if ( m_pHeadlight == NULL )
					return;

				m_pHeadlight->TurnOn();
			}
			QAngle Correct;
			QAngle vAngle;
			Vector vVector;
			QAngle vAngle2;
			Vector vVector2;
			Vector vecForward, vecRight, vecUp;

			C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
			C_BaseViewModel *vm = player ? player->GetViewModel( 0 ) : NULL;
			if ( vm )
			{
				int iAttachment = vm->LookupAttachment( "0" );
				vm->GetAttachment( iAttachment, vVector, vAngle );
				iAttachment = vm->LookupAttachment( "muzzle" );
				vm->GetAttachment( iAttachment, vVector2, vAngle2 );
				::FormatViewModelAttachment( vVector2, true );
				//vAngle = vAngle + vAngle2;
				//vAngle.y = vAngle.y +270;
				
				AngleVectors( vAngle2, &vecForward, &vecRight, &vecUp );
				
				//if (m_bPower)
					m_pHeadlight->UpdateLight( vVector+Vector(0,0,0), vecForward, vecRight, vecUp, 1000 );
				//else
					//m_pHeadlight->UpdateLight( vVector, vecForward, vecRight, vecUp, 20 );
			}
		}
		else if ( m_pHeadlight )
		{
			// Turned off the flashlight; delete it.
			delete m_pHeadlight;
			m_pHeadlight = NULL;
		}

	}

	void OnDataChanged( DataUpdateType_t updateType )
	{
		BaseClass::OnDataChanged( updateType );
		if ( updateType == DATA_UPDATE_CREATED )
		{
			SetNextClientThink( CLIENT_THINK_ALWAYS );
		}
	}
	
	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void StartFlashLight( void )
	{
		//TODO: Play startup sound
		DevMsg("Start Light\n",NULL,NULL);
		//active = true;
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void StopFlashLight( void )
	{
		//TODO: Play shutdown sound
		DevMsg("Stop Light\n",NULL,NULL);
		//active = false;
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	// Output : RenderGroup_t
	//-----------------------------------------------------------------------------
	RenderGroup_t GetRenderGroup( void )
	{
		return RENDER_GROUP_TRANSLUCENT_ENTITY;
	}

private:
	CNetworkVar( bool, m_bActive );
	CNetworkVar( bool, m_bPower );
	CFlashlightEffect *m_pHeadlight;
	//bool active;
};


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pData - 
//			*pStruct - 
//			*pOut - 
//-----------------------------------------------------------------------------
void RecvProxy_LightActive( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	bool state = *((bool *)&pData->m_Value.m_Int);

	//C_WeaponFlashLight *pWeapon = (C_WeaponFlashLight *) pStruct;

	if ( state )
	{
		// Turn on the effect
		//pWeapon->StartFlashLight();
	}
	else
	{
		// Turn off the effect
		//pWeapon->StopFlashLight();
	}

	*(bool *)pOut = state;
}

STUB_WEAPON_CLASS_IMPLEMENT( weapon_flashlight, C_WeaponFlashLight );

IMPLEMENT_CLIENTCLASS_DT( C_WeaponFlashLight, DT_WeaponFlashLight, CWeaponFlashLight )
	RecvPropBool( RECVINFO(m_bActive) ),
	RecvPropBool( RECVINFO(m_bPower) ),
END_RECV_TABLE()

