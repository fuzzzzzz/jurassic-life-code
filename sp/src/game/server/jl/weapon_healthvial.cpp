//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		HealthVial - an old favorite
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// CWeaponHealthVial
//-----------------------------------------------------------------------------

class CWeaponHealthVial : public CBaseCombatWeapon //CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponHealthVial, CBaseCombatWeapon );

	DECLARE_SERVERCLASS();

	CWeaponHealthVial();

	virtual void	PrimaryAttack( void );
};

IMPLEMENT_SERVERCLASS_ST(CWeaponHealthVial, DT_WeaponHealthVial)
END_SEND_TABLE()

#ifndef HL2MP
LINK_ENTITY_TO_CLASS( weapon_healthvial, CWeaponHealthVial );
PRECACHE_WEAPON_REGISTER( weapon_healthvial );
#endif


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponHealthVial::CWeaponHealthVial( void )
{
}


void CWeaponHealthVial::PrimaryAttack( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if (pPlayer->GetHealth()<100)
	{
		pPlayer->TakeHealth( 25, DMG_GENERIC );
		for (int i=0 ; i<8 ; i++)
		{
			if (pPlayer->GetWeapon(i) == (CBaseCombatWeapon*)this)
			{
				pPlayer->SetWeapon(i,NULL);
				break;
			}
		}
		DestroyItem();
	}
}
