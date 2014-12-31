
#include <assert.h>
#include <math.h>
#include <stdio.h>

#include <vgui_controls/LogicLabel.h>
#include <vgui_controls/Controls.h>

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>
 
using namespace vgui;

DECLARE_BUILD_FACTORY( LogicLabel );
 
LogicLabel::LogicLabel( Panel *parent, const char *panelName) //, const char *normalImage, const char *mouseOverImage, const char *mouseClickImage, const char *pCmd ) 
: Label( parent, panelName, "" ) 
{
	for (int i=0;i<10;i++)
	{
		m_pStateText[i]=NULL;
	}
}

LogicLabel::~LogicLabel()
{
	for (int i=0;i<10;i++)
	{
		SetStateText(i,NULL);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get control settings for editing
//-----------------------------------------------------------------------------
void LogicLabel::ApplySettings( KeyValues *inResourceData )
{
	if (inResourceData)
	{
		SetStateText(0,inResourceData->GetString("State0"));
		SetStateText(1,inResourceData->GetString("State1"));
		SetStateText(2,inResourceData->GetString("State2"));
		SetStateText(3,inResourceData->GetString("State3"));
		SetStateText(4,inResourceData->GetString("State4"));
		SetStateText(5,inResourceData->GetString("State5"));
		SetStateText(6,inResourceData->GetString("State6"));
		SetStateText(7,inResourceData->GetString("State7"));
		SetStateText(8,inResourceData->GetString("State8"));
		SetStateText(9,inResourceData->GetString("State9"));
	}

	SetText(m_pStateText[0]);
	Repaint();

	BaseClass::ApplySettings(inResourceData);
}

void LogicLabel::SetStateInternal(StateControl state)
{
	SetVisible(m_pStateText[state]!=NULL?true:false);
	SetText(m_pStateText[state]);
	Repaint();
}

void LogicLabel::SetStateText(int state, const char *text)
{	
	if (m_pStateText[state])
	{
		delete[] m_pStateText[state];
		m_pStateText[state]=NULL;
	}
	if (text)
	{
		m_pStateText[state] = new char[strlen(text)+1];
		strcpy(m_pStateText[state],text);
	}
}
