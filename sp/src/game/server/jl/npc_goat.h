//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Brachiosaurus header
//
//=============================================================================//

#ifndef NPC_GOAT_H
#define NPC_GOAT_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_basenpc.h"
#include "soundent.h"

class CGoat : public CAI_BaseNPC
{
	DECLARE_CLASS( CGoat, CAI_BaseNPC );
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

public:
	void	Precache( void );
	void	Spawn( void );
	Class_T Classify( void );
private:
	enum
	{
		SCHED_GOAT_SCHEDULE = BaseClass::NEXT_SCHEDULE,
		SCHED_GOAT_SCHEDULE2,
		NEXT_SCHEDULE
	};

	enum 
	{
		TASK_GOAT_TASK = BaseClass::NEXT_TASK,
		TASK_GOAT_TASK2,
		NEXT_TASK
	};

	enum 
	{
		COND_GOAT_CONDITION = BaseClass::NEXT_CONDITION,
		COND_GOAT_CONDITION2,
		NEXT_CONDITION
	};
};

LINK_ENTITY_TO_CLASS( npc_goat, CGoat );

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CGoat )

END_DATADESC()

AI_BEGIN_CUSTOM_NPC( npc_goat, CGoat )

AI_END_CUSTOM_NPC()

#endif // NPC_GOAT_H