//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		flashlight - an old favorite
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "soundent.h"
#include "basebludgeonweapon.h"
#include "vstdlib/random.h"
#include "npcevent.h"
#include "ai_basenpc.h"
#include "weapon_crowbar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar    sk_plr_dmg_flashlight		( "sk_plr_dmg_flashlight","5");
ConVar    sk_npc_dmg_flashlight		( "sk_npc_dmg_flashlight","5");

//-----------------------------------------------------------------------------
// CWeaponFlashLight
//-----------------------------------------------------------------------------

class CWeaponFlashLight : public CBaseHLBludgeonWeapon //CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponFlashLight, CBaseHLBludgeonWeapon );

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponFlashLight();

	float		GetRange( void )		{	return	75.0;	}
	float		GetFireRate( void )		{	return	0.4;	}

	void		AddViewKick( void );
	float		GetDamageForActivity( Activity hitActivity );

	virtual int WeaponMeleeAttack1Condition( float flDot, float flDist );
	void		SecondaryAttack( void );

	// Animation event
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	bool Holster( CBaseCombatWeapon *pSwitchingTo );
	bool Deploy( void );

	virtual bool	Reload( void );
	void	WeaponIdle();

private:
	// Animation event handlers
	void HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	CNetworkVar( bool, m_bActive );
	float NextSecondAttack;
	float m_iNextPower;
	CNetworkVar( bool, m_bPower );
};

IMPLEMENT_SERVERCLASS_ST(CWeaponFlashLight, DT_WeaponFlashlight)
	SendPropBool( SENDINFO( m_bActive ) ),
	SendPropBool( SENDINFO( m_bPower ) ),
END_SEND_TABLE()

#ifndef HL2MP
LINK_ENTITY_TO_CLASS( weapon_flashlight, CWeaponFlashLight );
PRECACHE_WEAPON_REGISTER( weapon_flashlight );
#endif

acttable_t CWeaponFlashLight::m_acttable[] = 
{
	{ ACT_MELEE_ATTACK1,	ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE,				ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_MELEE,	false },
};

IMPLEMENT_ACTTABLE(CWeaponFlashLight);

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponFlashLight::CWeaponFlashLight( void )
{
	m_bPower=true;
}

//-----------------------------------------------------------------------------
// Purpose: Get the damage amount for the animation we're doing
// Input  : hitActivity - currently played activity
// Output : Damage amount
//-----------------------------------------------------------------------------
float CWeaponFlashLight::GetDamageForActivity( Activity hitActivity )
{
	if ( ( GetOwner() != NULL ) && ( GetOwner()->IsPlayer() ) )
		return sk_plr_dmg_flashlight.GetFloat();

	return sk_npc_dmg_flashlight.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: Add in a view kick for this weapon
//-----------------------------------------------------------------------------
void CWeaponFlashLight::AddViewKick( void )
{
	CBasePlayer *pPlayer  = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	QAngle punchAng;

	punchAng.x = random->RandomFloat( 1.0f, 2.0f );
	punchAng.y = random->RandomFloat( -2.0f, -1.0f );
	punchAng.z = 0.0f;
	
	pPlayer->ViewPunch( punchAng ); 
}


//-----------------------------------------------------------------------------
// Attempt to lead the target (needed because citizens can't hit manhacks with the flashlight!)
//-----------------------------------------------------------------------------
ConVar sk_flashlight_lead_time( "sk_flashlight_lead_time", "0.9" );

int CWeaponFlashLight::WeaponMeleeAttack1Condition( float flDot, float flDist )
{
	m_bPower = !m_bPower;
	// Attempt to lead the target (needed because citizens can't hit manhacks with the flashlight!)
	CAI_BaseNPC *pNPC	= GetOwner()->MyNPCPointer();
	CBaseEntity *pEnemy = pNPC->GetEnemy();
	if (!pEnemy)
		return COND_NONE;

	Vector vecVelocity;
	vecVelocity = pEnemy->GetSmoothedVelocity( );

	// Project where the enemy will be in a little while
	float dt = sk_flashlight_lead_time.GetFloat();
	dt += random->RandomFloat( -0.3f, 0.2f );
	if ( dt < 0.0f )
		dt = 0.0f;

	Vector vecExtrapolatedPos;
	VectorMA( pEnemy->WorldSpaceCenter(), dt, vecVelocity, vecExtrapolatedPos );

	Vector vecDelta;
	VectorSubtract( vecExtrapolatedPos, pNPC->WorldSpaceCenter(), vecDelta );

	if ( fabs( vecDelta.z ) > 70 )
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	Vector vecForward = pNPC->BodyDirection2D( );
	vecDelta.z = 0.0f;
	float flExtrapolatedDist = Vector2DNormalize( vecDelta.AsVector2D() );
	if ((flDist > 64) && (flExtrapolatedDist > 64))
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	float flExtrapolatedDot = DotProduct2D( vecDelta.AsVector2D(), vecForward.AsVector2D() );
	if ((flDot < 0.7) && (flExtrapolatedDot < 0.7))
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_MELEE_ATTACK1;
}


//-----------------------------------------------------------------------------
// Animation event handlers
//-----------------------------------------------------------------------------
void CWeaponFlashLight::HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	// Trace up or down based on where the enemy is...
	// But only if we're basically facing that direction
	Vector vecDirection;
	AngleVectors( GetAbsAngles(), &vecDirection );

	CBaseEntity *pEnemy = pOperator->MyNPCPointer() ? pOperator->MyNPCPointer()->GetEnemy() : NULL;
	if ( pEnemy )
	{
		Vector vecDelta;
		VectorSubtract( pEnemy->WorldSpaceCenter(), pOperator->Weapon_ShootPosition(), vecDelta );
		VectorNormalize( vecDelta );
		
		Vector2D vecDelta2D = vecDelta.AsVector2D();
		Vector2DNormalize( vecDelta2D );
		if ( DotProduct2D( vecDelta2D, vecDirection.AsVector2D() ) > 0.8f )
		{
			vecDirection = vecDelta;
		}
	}

	Vector vecEnd;
	VectorMA( pOperator->Weapon_ShootPosition(), 50, vecDirection, vecEnd );
	CBaseEntity *pHurt = pOperator->CheckTraceHullAttack( pOperator->Weapon_ShootPosition(), vecEnd, 
		Vector(-16,-16,-16), Vector(36,36,36), sk_npc_dmg_flashlight.GetFloat(), DMG_CLUB, 0.75 );
	
	// did I hit someone?
	if ( pHurt )
	{
		// play sound
		WeaponSound( MELEE_HIT );

		// Fake a trace impact, so the effects work out like a player's crowbaw
		trace_t traceHit;
		UTIL_TraceLine( pOperator->Weapon_ShootPosition(), pHurt->GetAbsOrigin(), MASK_SHOT_HULL, pOperator, COLLISION_GROUP_NONE, &traceHit );
		ImpactEffect( traceHit );
	}
	else
	{
		WeaponSound( MELEE_MISS );
	}
}


//-----------------------------------------------------------------------------
// Animation event
//-----------------------------------------------------------------------------
void CWeaponFlashLight::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{

	if ( m_bActive )
	{
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
		DevMsg("Light Active1\n",NULL,NULL);
	}
	else
	{
	}
	switch( pEvent->event )
	{
	case EVENT_WEAPON_MELEE_HIT:
		HandleAnimEventMeleeHit( pEvent, pOperator );
		break;

	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}

void CWeaponFlashLight::SecondaryAttack( void )
{	
	if (gpGlobals->curtime > NextSecondAttack )
	{
		if (m_bActive)
			m_bActive = false;
		else if ( m_iClip1>0 )
			m_bActive = true;
		NextSecondAttack  = gpGlobals->curtime+0.25f;
		m_iNextPower  = gpGlobals->curtime+1.0f;
	}
	return;
}

bool CWeaponFlashLight::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	//StopEffects();
	m_bActive = false;
	return BaseClass::Holster( pSwitchingTo );
}

bool CWeaponFlashLight::Deploy( void )
{
	NextSecondAttack  = gpGlobals->curtime;
	return BaseClass::Deploy( );
}

void CWeaponFlashLight::WeaponIdle( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner->m_nButtons & IN_RELOAD)
		Reload();


	if ( m_bActive )
	{
		//CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		//pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
		
		if (gpGlobals->curtime > m_iNextPower )
		{
			m_iClip1--;
			m_iNextPower  = gpGlobals->curtime+1.0f;
		}
		if ( m_iClip1 <= 0 )
			m_bActive = false;

		//DevMsg("Light Active2\n",NULL,NULL);
	}
	else
	{
	}
}

bool CWeaponFlashLight::Reload( void )
{
	DevMsg("Reload Light\n",NULL,NULL);
	m_iClip1 = 50;
	return BaseClass::Reload();
}
