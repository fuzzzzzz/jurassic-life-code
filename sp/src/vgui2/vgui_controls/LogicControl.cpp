
#include <assert.h>
#include <math.h>
#include <stdio.h>

#include <vgui_controls/LogicControl.h>
#include <vgui_controls/Controls.h>

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>
 
using namespace vgui;

LogicControl::LogicControl()
{
	m_iState=-1;
	//SetState(0);
}

void LogicControl::SetState(int state)
{
	if (state!=m_iState)
	{
		SetStateInternal((StateControl)state);
		m_iState=state;
	}
}
