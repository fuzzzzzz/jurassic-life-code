//========= Copyright © 1996-2004, Valve LLC, All rights reserved. ============
//
// Purpose: Simple HUD element
//
//=============================================================================

#include "stdlib.h"
#include "cbase.h"
#include "c_basehlplayer.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "view.h"

using namespace vgui;

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

class CHUDNoteInfo : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE( CHUDNoteInfo, vgui::Panel );

public:

	CHUDNoteInfo( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HUDNoteInfo" ) 
	{
		vgui::Panel *pParent = g_pClientMode->GetViewport();
		SetParent( pParent );

		SetPaintBorderEnabled( false );
		SetPaintBackgroundEnabled( false );
		
		SetHiddenBits( HIDEHUD_PLAYERDEAD );
	};

	void	Init( void );
	void	Paint( void );
	
private:
	int		m_TNote;
};

DECLARE_HUDELEMENT( CHUDNoteInfo );


void CHUDNoteInfo::Init( void )
{
	m_TNote = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(m_TNote,"hud/NoteInfo",true,false);

}

void CHUDNoteInfo::Paint( void )
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();

	bool draw=false;
	if ( player )
	{
		draw = player->HasNewNote();
	}	
	
	//Draw HUD Bottom Left image (Health)
	//draw=true;
	if (draw)
	{
		surface()->DrawSetTexture(m_TNote);
		surface()->DrawSetColor(255,255,255,255);
		surface()->DrawTexturedRect(0,0,GetWide(),GetTall());	
	}
}