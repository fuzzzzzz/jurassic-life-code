//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <stdio.h>
//#define PROTECTED_THINGS_DISABLE

#include <KeyValues.h>
#include <vgui/ILocalize.h>
#include "tier1/convar.h"

#include <vgui_controls/VarSlider.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

DECLARE_BUILD_FACTORY( VarSlider );

//-----------------------------------------------------------------------------
// Purpose: Create a slider bar with ticks underneath it
//-----------------------------------------------------------------------------
VarSlider::VarSlider(Panel *parent, const char *panelName ) : Slider(parent, panelName)
{
	AddActionSignalTarget( this );
	m_fCoeff = 1.0;
}

//-----------------------------------------------------------------------------
// Purpose: Get control settings for editing
//-----------------------------------------------------------------------------
void VarSlider::ApplySettings( KeyValues *inResourceData )
{
	if (inResourceData)
	{
		m_sVarName = inResourceData->GetString("convar");
		m_fCoeff = inResourceData->GetFloat("coeff",1.0);
		SetRange(	inResourceData->GetFloat("minvalue"),
					inResourceData->GetFloat("maxvalue") );
		ConVar *var = cvar->FindVar(m_sVarName.c_str());
		if (var)
		{
			SetValue(var->GetFloat()/m_fCoeff);
		}
	}
	BaseClass::ApplySettings(inResourceData);
}

bool VarSlider::RequestInfo(KeyValues *data)
{
	return true;
}

void VarSlider::OnSliderMoved()
{
	ConVar *var = cvar->FindVar(m_sVarName.c_str());
	if (var)
	{
		var->SetValue(m_fCoeff*GetValue());
	}
}
