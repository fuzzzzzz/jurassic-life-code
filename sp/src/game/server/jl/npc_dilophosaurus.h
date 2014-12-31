//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Dilophosaurus header
//
//=============================================================================//

#ifndef NPC_DILOPHOSAURUS_H
#define NPC_DILOPHOSAURUS_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_basenpc.h"
#include "soundent.h"

class CDilophosaurus : public CAI_BaseNPC
{
	DECLARE_CLASS( CDilophosaurus, CAI_BaseNPC );
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

public:
	void	Precache( void );
	void	Spawn( void );
	Class_T Classify( void );
private:
	enum
	{
		SCHED_DILOPHOSAURUS_SCHEDULE = BaseClass::NEXT_SCHEDULE,
		SCHED_DILOPHOSAURUS_SCHEDULE2,
		NEXT_SCHEDULE
	};

	enum 
	{
		TASK_DILOPHOSAURUS_TASK = BaseClass::NEXT_TASK,
		TASK_DILOPHOSAURUS_TASK2,
		NEXT_TASK
	};

	enum 
	{
		COND_DILOPHOSAURUS_CONDITION = BaseClass::NEXT_CONDITION,
		COND_DILOPHOSAURUS_CONDITION2,
		NEXT_CONDITION
	};
};

LINK_ENTITY_TO_CLASS( npc_dilophosaurus, CDilophosaurus );

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CDilophosaurus )

END_DATADESC()

AI_BEGIN_CUSTOM_NPC( npc_dilophosaurus, CDilophosaurus )

AI_END_CUSTOM_NPC()

#endif // NPC_DILOPHOSAURUS_H