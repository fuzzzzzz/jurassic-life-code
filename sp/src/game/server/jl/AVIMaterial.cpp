// Nate Nichols, InfoLab Northwestern University, August 2006.

#include "cbase.h"
#include "AVIMaterial.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SF_AVI_MATERIAL_BEGIN_PLAYING		0x00000001
#define SF_AVI_MATERIAL_LOOP				0x00000002

CAVIMaterial::CAVIMaterial()
{
	m_iPlay = 0;
	m_iAdvanceFrame = 0;
}

void CAVIMaterial::Spawn()
{
	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_NONE);
	UTIL_SetSize(this, -Vector(2, 2, 2), Vector(2, 2, 2));
	AddEFlags(EFL_FORCE_CHECK_TRANSMIT);
	m_bLoop = HasSpawnFlags(SF_AVI_MATERIAL_LOOP);
	if (HasSpawnFlags(SF_AVI_MATERIAL_BEGIN_PLAYING))
	{
		Play();
	}
}

void CAVIMaterial::Play(inputdata_t &input)
{
	Play();
}

void CAVIMaterial::Play()
{
	m_iPlay++;
}

void CAVIMaterial::Pause(inputdata_t &input)
{
	m_iPlay--;
}

void CAVIMaterial::SetMovie(inputdata_t &input)
{
	m_iszMovieName = input.value.StringID();
}

void CAVIMaterial::AdvanceFrame(inputdata_t &input)
{
	//C_AVIMaterial keeps track of the last m_iAdvanceFrame, so if it changes, 
	//C_AVIMaterial knows to advance one frame.
	m_iAdvanceFrame++;
}