
#include <assert.h>
#include <math.h>
#include <stdio.h>

#include <vgui_controls/FakeCursor.h>
#include <vgui_controls/Controls.h>

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include <vgui/isurface.h>
#include <KeyValues.h>

#include <vgui/IInputInternal.h>
extern vgui::IInputInternal *g_InputInternal;

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>
 
using namespace vgui;

DECLARE_BUILD_FACTORY( FakeCursor );
 
FakeCursor::FakeCursor( Panel *parent, const char *panelName) //, const char *normalImage, const char *mouseOverImage, const char *mouseClickImage, const char *pCmd ) 
: Panel( parent, panelName ) 
{
	SetParent(parent);
	SetSize(32,32);
	SetZPos(999); // Always on top

	m_iTexture = 0;
}

FakeCursor::~FakeCursor()
{
}

void FakeCursor::ApplySchemeSettings(IScheme *pScheme)
{
	m_iTexture = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(m_iTexture,"vgui/screens/cursor",true,false);
}

void FakeCursor::Paint()
{
	vgui::surface()->DrawSetTexture(m_iTexture);
	vgui::surface()->DrawSetColor(255,255,255,255);
	vgui::surface()->DrawTexturedRect(0, 0, 32, 32);
	//vgui::surface()->DrawGetTextureSize(m_iTexture,w,t);

	/*
	CHudTexture *pArrow	= gHUD.GetIcon( "arrow" );
	if (pArrow)
	{
		//DevMsg("Found\n");
		//pArrow->DrawSelf(-10,-10,20,20,Color(255,192,0,255));
		pArrow->DrawSelf(-10,-10,pArrow->Width(),pArrow->Height(),Color(255,255,255,255));		
		//pArrow->textureId
		//vgui::surface()->DrawSetTexture(pArrow->textureId);
		//vgui::surface()->DrawTexturedRect(0,0,30,30);
	}else{
		vgui::surface()->DrawSetColor(128,128,0,128);
		//vgui::surface()->DrawFilledRect(0,0,10,10);
		vgui::surface()->DrawOutlinedRect(0,0,10,10);
	}
	*/
}

void FakeCursor::OnThink()
{
	//int x,y;
	//input()->GetCursorPos( x, y );
	//g_InputInternal->GetCursorPos(x,y);
	//g_InputInternal->GetCursorPosition(x,y);
	//SetPos(x,y);	
	//DevMsg("Cursor pos %d %d\n",x,y);
}
