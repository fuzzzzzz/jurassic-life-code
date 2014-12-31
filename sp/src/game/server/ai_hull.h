//========= Copyright Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef AI_HULL_H
#define AI_HULL_H
#pragma once

class Vector;
//=========================================================
// Link Properties. These hulls must correspond to the hulls
// in AI_Hull.cpp!
//=========================================================
enum Hull_t
{
	HULL_HUMAN,				// jl = Human, player,								hl2 =  Combine, Stalker, Zombie...
	HULL_SMALL_CENTERED,	// jl = Brycon, Piranha,							hl2 = Scanner
	HULL_WIDE_HUMAN,		// jl = Free,										hl2 = Vortigaunt
	HULL_TINY,				// jl = compies , chicken,							hl2 = Headcrab
	HULL_WIDE_SHORT,		// jl = Cows, Raptor gallimimus, dilophosaurus,		hl2 = Bullsquid
	HULL_MEDIUM,			// jl = goat,										hl2 = Cremator
	HULL_TINY_CENTERED,		// jl = free 										hl2 = Manhack 
	HULL_LARGE,				// jl = triceratop, parasorolophus,					hl2 = Antlion Guard
	HULL_LARGE_CENTERED,	// jl = Crocodile,									hl2 = Mortar Synth
	HULL_MEDIUM_TALL,		// jl = Brachiosaurus								hl2 = Hunter
	
	// we also got the possibility to add some custom hulls if we really need one specific

	// JL Customs HULLs
	HULL_TREX, // tyrannosaurus rex
	HULL_VEHICLE, // maybe it could be need with npc collisions, not sure what i speaking about 

/*	HULL_BRYCON,			// Brycon (npc_fish)
	HULL_PIRANHA,			// PIRANHA (npc_fish)
	HULL_CROCODILE,		// CROCODILE (npc_fish)
	HULL_COMPY,				// COMPY
	HULL_GALLI,				// GALLIMIMUS
	HULL_RAPTOR,			// VELOCIRAPTOR
*/
//--------------------------------------------
	NUM_HULLS,
	HULL_NONE				// No Hull (appears after num hulls as we don't want to count it)
};

enum Hull_Bits_t
{
	bits_HUMAN_HULL				=	0x00000001, // 1
	bits_SMALL_CENTERED_HULL	=	0x00000002, // 2
	bits_WIDE_HUMAN_HULL		=	0x00000004, // 4
	bits_TINY_HULL				=	0x00000008, // 8
	bits_WIDE_SHORT_HULL		=	0x00000010, // 16
	bits_MEDIUM_HULL			=	0x00000020, //
	bits_TINY_CENTERED_HULL		=	0x00000040, // 64
	bits_LARGE_HULL				=	0x00000080, // 128
	bits_LARGE_CENTERED_HULL	=	0x00000100, // 256
	bits_MEDIUM_TALL_HULL		=	0x00000200, // 512
	bits_HULL_BITS_MASK			=	0x00000400, // 767 -0x000002ff

	bits_TREX_HULL				=	0x00000800,
	bits_VEHICLE_HULL			=	0x00001000,

/*	bits_COMPY_HULL				=	0x00000800,  // 1024
	bits_PIRANHA_HULL			=	0x00001000, // 2048
	//bits_BRYCON_HULL			=	0x00002000,  // 4096
	bits_GALLI_HULL				=	0x00004000, // 8192
	bits_CROCODILE_HULL			=	0x00008000, // 16384
	bits_RAPTOR_HULL			=	0x00010000, // 32768
*/
};

inline int HullToBit( Hull_t hull )
{
	return ( 1 << hull );
}



//=============================================================================
//	>> CAI_Hull
//=============================================================================
namespace NAI_Hull
{
	const Vector &Mins(int id);
	const Vector &Maxs(int id);

	const Vector &SmallMins(int id);
	const Vector &SmallMaxs(int id);

	float		Length(int id);
	float		Width(int id);
	float		Height(int id);

	int			Bits(int id);
 
	const char*	Name(int id);

	Hull_t		LookupId(const char *szName);
};

#endif // AI_HULL_H
