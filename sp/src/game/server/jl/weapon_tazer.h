//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_TAZER_H
#define WEAPON_TAZER_H
#ifdef _WIN32
#pragma once
#endif

#include "basebludgeonweapon.h"

#define	TAZER_RANGE		75.0f
#define	TAZER_REFIRE	0.6f

class CWeaponTazer : public CBaseHLBludgeonWeapon
{
public:

	DECLARE_CLASS( CWeaponTazer, CBaseHLBludgeonWeapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CWeaponTazer();

	
	DECLARE_ACTTABLE();

	virtual void Precache();

	void		Spawn();

	float		GetRange( void )		{ return TAZER_RANGE; }
	float		GetFireRate( void )		{ return TAZER_REFIRE; }

	int			WeaponMeleeAttack1Condition( float flDot, float flDist );

	bool		Deploy( void );
	bool		Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	
	void		Drop( const Vector &vecVelocity );
	void		ImpactEffect( trace_t &traceHit );
	void		SecondaryAttack( void )	{}
	void		SetTazerState( bool state );
	bool		GetTazerState( void );
	void		Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	
	float		GetDamageForActivity( Activity hitActivity );

	bool		CanBePickedUpByNPCs( void ) { return false;	}		

//private:

	CNetworkVar( bool, m_bActive );
};

#endif // WEAPON_TAZER_H
