
#include <assert.h>
#include <math.h>
#include <stdio.h>

#include <vgui_controls/LogicImageButton.h>
#include <vgui_controls/Controls.h>

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>
 
using namespace vgui;

DECLARE_BUILD_FACTORY( LogicImageButton );
 
LogicImageButton::LogicImageButton( Panel *parent, const char *panelName) //, const char *normalImage, const char *mouseOverImage, const char *mouseClickImage, const char *pCmd ) 
: ImageButton( parent, panelName ) 
{
	for (int i=0;i<10;i++)
	{
		m_iState[i] = (LogicState)(STATE_VISIBLE & STATE_ENABLE);
	}
}

LogicImageButton::~LogicImageButton()
{
}

//-----------------------------------------------------------------------------
// Purpose: Get control settings for editing
//-----------------------------------------------------------------------------
void LogicImageButton::ApplySettings( KeyValues *inResourceData )
{
	m_iState[0] = (LogicState)inResourceData->GetInt("State0",0);
	m_iState[1] = (LogicState)inResourceData->GetInt("State1",0);
	m_iState[2] = (LogicState)inResourceData->GetInt("State2",0);
	m_iState[3] = (LogicState)inResourceData->GetInt("State3",0);
	m_iState[4] = (LogicState)inResourceData->GetInt("State4",0);
	m_iState[5] = (LogicState)inResourceData->GetInt("State5",0);
	m_iState[6] = (LogicState)inResourceData->GetInt("State6",0);
	m_iState[7] = (LogicState)inResourceData->GetInt("State7",0);
	m_iState[8] = (LogicState)inResourceData->GetInt("State8",0);
	m_iState[9] = (LogicState)inResourceData->GetInt("State9",0);

	BaseClass::ApplySettings(inResourceData);
}

void LogicImageButton::SetStateInternal(StateControl state)
{
	if (state>=STATE_0 && state<=STATE_9)
	{
		SetVisible( m_iState[state]&STATE_VISIBLE );
		SetEnable( m_iState[state]&STATE_ENABLE );
	}
}
