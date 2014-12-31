//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		HealthKit - an old favorite
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "vstdlib/random.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// CWeaponHealthKit
//-----------------------------------------------------------------------------

class CWeaponHealthKit : public CBaseCombatWeapon //CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponHealthKit, CBaseCombatWeapon );

	DECLARE_SERVERCLASS();

	CWeaponHealthKit();

	virtual void	PrimaryAttack( void );
};

IMPLEMENT_SERVERCLASS_ST(CWeaponHealthKit, DT_WeaponHealthKit)
END_SEND_TABLE()

#ifndef HL2MP
LINK_ENTITY_TO_CLASS( weapon_healthkit, CWeaponHealthKit );
PRECACHE_WEAPON_REGISTER( weapon_healthkit );
#endif


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponHealthKit::CWeaponHealthKit( void )
{
}

void CWeaponHealthKit::PrimaryAttack( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if (pPlayer->GetHealth()<100)
	{
		pPlayer->TakeHealth( 50, DMG_GENERIC );
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
