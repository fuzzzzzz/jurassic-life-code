
#ifndef PROP_TREE_H
#define PROP_TREE_H
#ifdef _WIN32
#pragma once
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "baseanimating.h"

class CPropTree : public CBaseAnimating
{
	DECLARE_CLASS( CPropTree, CBaseAnimating );
	DECLARE_DATADESC();
	//DECLARE_SERVERCLASS();

public:
	virtual void Precache();
	virtual void Spawn();

	//virtual bool OverridePropdata();
	//virtual bool CreateVPhysics();

private:
};

#endif // PROP_TREE_H
