
#include <assert.h>
#include <math.h>
#include <stdio.h>

#include <vgui_controls/LogicImage.h>
#include <vgui_controls/Controls.h>

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>
 
using namespace vgui;

DECLARE_BUILD_FACTORY( LogicImage );
 
LogicImage::LogicImage( Panel *parent, const char *panelName) //, const char *normalImage, const char *mouseOverImage, const char *mouseClickImage, const char *pCmd ) 
: ImagePanel( parent, panelName ) 
{
	for (int i=0;i<10;i++)
	{
		m_pStateImage[i] = NULL;
	}
	SetStateInternal(STATE_0);
}

LogicImage::~LogicImage()
{
	for (int i=0;i<10;i++)
	{
		if (m_pStateImage[i])
		{
			delete m_pStateImage[i];
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get control settings for editing
//-----------------------------------------------------------------------------
void LogicImage::ApplySettings( KeyValues *inResourceData )
{
	if (inResourceData)
	{
		inResourceData->GetString("State0");
		vgui::scheme()->GetImage(  inResourceData->GetString("State0"), false );
		/*m_pStateImage[0] = vgui::scheme()->GetImage(  inResourceData->GetString("State0"), false );
		m_pStateImage[1] = vgui::scheme()->GetImage(  inResourceData->GetString("State1"), false );
		m_pStateImage[2] = vgui::scheme()->GetImage(  inResourceData->GetString("State2"), false );
		m_pStateImage[3] = vgui::scheme()->GetImage(  inResourceData->GetString("State3"), false );
		m_pStateImage[4] = vgui::scheme()->GetImage(  inResourceData->GetString("State4"), false );
		m_pStateImage[5] = vgui::scheme()->GetImage(  inResourceData->GetString("State5"), false );
		m_pStateImage[6] = vgui::scheme()->GetImage(  inResourceData->GetString("State6"), false );
		m_pStateImage[7] = vgui::scheme()->GetImage(  inResourceData->GetString("State7"), false );
		m_pStateImage[8] = vgui::scheme()->GetImage(  inResourceData->GetString("State8"), false );
		m_pStateImage[9] = vgui::scheme()->GetImage(  inResourceData->GetString("State9"), false );*/
	}

	SetImage(m_pStateImage[0]);
	Repaint();

	BaseClass::ApplySettings(inResourceData);
}

void LogicImage::SetStateInternal(StateControl state)
{
	if (state>=STATE_0 && state<=STATE_9)
	{
		SetVisible(m_pStateImage[state-STATE_0]!=NULL?true:false);
		SetImage(m_pStateImage[state-STATE_0]);
	}
}
