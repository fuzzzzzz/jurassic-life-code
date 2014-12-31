

#include "cbase.h"
#include "prop_tree.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"



LINK_ENTITY_TO_CLASS( prop_tree, CPropTree );

//-----------------------------------------------------------------------------
// Save/load: 
//-----------------------------------------------------------------------------
BEGIN_DATADESC( CPropTree )
	//DEFINE_FIELD( m_flLastBounceTime, FIELD_TIME ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Precache 
//-----------------------------------------------------------------------------
void CPropTree::Precache( void )
{
	BaseClass::Precache();

	PrecacheModel(STRING(GetModelName()));
}

//-----------------------------------------------------------------------------
// Spawn: 
//-----------------------------------------------------------------------------
void CPropTree::Spawn( void )
{
	Precache();

	SetModel(STRING(GetModelName()));

	SetSolid( SOLID_VPHYSICS );
	SetMoveType( MOVETYPE_NONE );
	VPhysicsInitStatic();

	BaseClass::Spawn();
}

/*
//-----------------------------------------------------------------------------
// Spherical vphysics
//-----------------------------------------------------------------------------
bool CPropTree::OverridePropdata() 
{ 
	return true; 
}

//-----------------------------------------------------------------------------
// Create vphysics
//-----------------------------------------------------------------------------
bool CPropTree::CreateVPhysics()
{
	SetSolid( SOLID_BBOX );

	float flSize = m_flRadius;

	SetCollisionBounds( Vector(-flSize, -flSize, -flSize), Vector(flSize, flSize, flSize) );
	objectparams_t params = g_PhysDefaultObjectParams;
	params.pGameData = static_cast<void *>(this);
	int nMaterialIndex = physprops->GetSurfaceIndex("metal_bouncy");
	IPhysicsObject *pPhysicsObject = physenv->CreateSphereObject( flSize, nMaterialIndex, GetAbsOrigin(), GetAbsAngles(), &params, false );
	if ( !pPhysicsObject )
		return false;

	VPhysicsSetObject( pPhysicsObject );
	SetMoveType( MOVETYPE_VPHYSICS );
	pPhysicsObject->Wake();

	pPhysicsObject->SetMass( 750.0f );
	pPhysicsObject->EnableGravity( false );
	pPhysicsObject->EnableDrag( false );

	float flDamping = 0.0f;
	float flAngDamping = 0.5f;
	pPhysicsObject->SetDamping( &flDamping, &flAngDamping );
	pPhysicsObject->SetInertia( Vector( 1e30, 1e30, 1e30 ) );

	if( WasFiredByNPC() )
	{
		// Don't do impact damage. Just touch them and do your dissolve damage and move on.
		PhysSetGameFlags( pPhysicsObject, FVPHYSICS_NO_NPC_IMPACT_DMG );
	}
	else
	{
		PhysSetGameFlags( pPhysicsObject, FVPHYSICS_DMG_DISSOLVE | FVPHYSICS_HEAVY_OBJECT );
	}

	return true;
}
*/

