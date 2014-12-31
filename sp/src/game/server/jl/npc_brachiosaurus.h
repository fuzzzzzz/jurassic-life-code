//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Brachiosaurus header
//
//=============================================================================//

#ifndef NPC_BRACHIOSAURUS_H
#define NPC_BRACHIOSAURUS_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_basenpc.h"
#include "soundent.h"

class CBrachiosaurus : public CAI_BaseNPC
{
	DECLARE_CLASS( CBrachiosaurus, CAI_BaseNPC );
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

public:
	void	Precache( void );
	void	Spawn( void );
	Class_T Classify( void );
private:
	enum
	{
		SCHED_BRACHIOSAURUS_SCHEDULE = BaseClass::NEXT_SCHEDULE,
		SCHED_BRACHIOSAURUS_SCHEDULE2,
		NEXT_SCHEDULE
	};

	enum 
	{
		TASK_BRACHIOSAURUS_TASK = BaseClass::NEXT_TASK,
		TASK_BRACHIOSAURUS_TASK2,
		NEXT_TASK
	};

	enum 
	{
		COND_BRACHIOSAURUS_CONDITION = BaseClass::NEXT_CONDITION,
		COND_BRACHIOSAURUS_CONDITION2,
		NEXT_CONDITION
	};
};

LINK_ENTITY_TO_CLASS( npc_brachiosaurus, CBrachiosaurus );

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CBrachiosaurus )

END_DATADESC()

AI_BEGIN_CUSTOM_NPC( npc_brachiosaurus, CBrachiosaurus )

AI_END_CUSTOM_NPC()

#endif // NPC_BRACHIOSAURUS_H