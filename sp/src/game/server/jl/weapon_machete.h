//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef WEAPON_MACHETE_H
#define WEAPON_MACHETE_H

#include "basebludgeonweapon.h"

#if defined( _WIN32 )
#pragma once
#endif

#ifdef HL2MP
#error weapon_machete.h must not be included in hl2mp. The windows compiler will use the wrong class elsewhere if it is.
#endif

#define	MACHETE_RANGE	75.0f // default 75.0f
#define	MACHETE_REFIRE	0.5f // default 0.4f

//-----------------------------------------------------------------------------
// CWeaponMachete
//-----------------------------------------------------------------------------

class CWeaponMachete : public CBaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponMachete, CBaseHLBludgeonWeapon );

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponMachete();

	float		GetRange( void )		{	return	MACHETE_RANGE;	}
	float		GetFireRate( void )		{	return	MACHETE_REFIRE;	}
	//void		WeaponIdle( void );						// called when no buttons pressed

	void		AddViewKick( void );
	float		GetDamageForActivity( Activity hitActivity );

	virtual int WeaponMeleeAttack1Condition( float flDot, float flDist );
	virtual int WeaponMeleeAttack2Condition( float flDot, float flDist );
	//void		PrimaryAttack( void );
	void		SecondaryAttack( int bIsSecondary );

	// Animation event
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

private:

	
	// Animation event handlers
	void HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
};

#endif // WEAPON_MACHETE_H
