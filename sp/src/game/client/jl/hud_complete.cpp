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

class CHudComplete : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE( CHudComplete, vgui::Panel );

public:

	CHudComplete( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudComplete" ) 
	{
		vgui::Panel *pParent = g_pClientMode->GetViewport();
		SetParent( pParent );

		SetPaintBorderEnabled( false );
		SetPaintBackgroundEnabled( false );
		
		// Never hide
		//SetHiddenBits( 0 );
		SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
	};

	void	Init( void );
	void	VidInit( void );
	void	Paint( void );
	
private:
	int			m_TBottomLeft;
	int			m_TBottomRight;
	int			m_TBottomStress;
	int			m_TLight;
	int			m_TLightOn;
	wchar_t		m_pText[256];	// Unicode text buffer
	int			newHealth;
	int			oldHealth;
	int			stress;
	int			oldStress;
	int			ammo;
	int			clip;
	wchar_t		unicode[6];
	int			charWidth;
	int			charWidth2;
	vgui::HFont hFont ;
	vgui::HFont hFont2 ;

	float ratio;
	int pos;
	int NextHealth;
};

DECLARE_HUDELEMENT( CHudComplete );

void CHudComplete::VidInit( void )
{
}

void CHudComplete::Init( void )
{
	m_TBottomLeft = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(m_TBottomLeft,"hud/bottomleft",true,false);
	m_TBottomRight = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(m_TBottomRight,"hud/bottomright",true,false);
	m_TBottomStress = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(m_TBottomStress,"hud/bottomleftstress",true,false);
	m_TLight = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(m_TLight,"hud/light",true,false);
	m_TLightOn = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(m_TLightOn,"hud/lighton",true,false);
	// Get our scheme and font information
	vgui::HScheme scheme = vgui::scheme()->GetScheme( "ClientScheme" );
	hFont = vgui::scheme()->GetIScheme(scheme)->GetFont( "HudInfo" );
	hFont2 = vgui::scheme()->GetIScheme(scheme)->GetFont( "HudInfoSmall" );
	charWidth = surface()->GetCharacterWidth(hFont, '0');
	charWidth2 = surface()->GetCharacterWidth(hFont2, '0');
	NextHealth = 0;
	newHealth = 100;
	PrecacheMaterial( "frontbuffer" );
}


void CHudComplete::Paint( void )
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();

	//C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();

	C_BaseCombatWeapon *wpn = GetActiveWeapon();
	//newHealth = 0;
	clip=0;
	ammo=0;
	if ( player )
	{
		// Get Health of player
		// Never below zero		
		oldHealth = max( player->GetHealth(), 0 );
		//stress = pPlayer->m_HL2Local.m_flSuitPower/10+0.5;
		//TO FIX
		//stress = player->GetStress()/10+0.5;
		//stress=50;
		//stress = max( player->GetStress(), 0 );
		//JL --TH
		//stress = player->GetStress();
		
		//Get Ammo of player		
		if ( wpn )
		{
			clip = wpn->Clip1();
			ammo = player->GetAmmoCount(wpn->GetPrimaryAmmoType());
		}		
	}	
	if (NextHealth + 50 > gpGlobals->curtime)
	{
		NextHealth = gpGlobals->curtime;
		if ( newHealth-2 > oldHealth)
			newHealth = newHealth - 2;
		else if (newHealth > oldHealth)
			newHealth--;
		else if ( newHealth+2 < oldHealth)
			newHealth = newHealth + 2;
		else if (newHealth < oldHealth)
			newHealth++;
	}


	//Draw HUD Bottom Left image (Health)
	surface()->DrawSetTexture(m_TBottomLeft);
	surface()->DrawSetColor(255,255,255,255);
	surface()->DrawTexturedRect(0,ScreenHeight()-256,256,ScreenHeight());	

	//Draw HUD Bottom Left Stress image
	surface()->DrawSetTexture(m_TBottomStress);
	surface()->DrawSetColor(255,255-255/100.f*stress,255-255/100.f*stress,32);
	surface()->DrawTexturedRect(0,ScreenHeight()-256+2,256,ScreenHeight());
	surface()->DrawSetColor(255,255-255/100.f*stress,255-255/100.f*stress,255);
	//surface()->DrawTexturedSubRect(0,ScreenHeight()-256+3+191/10*(10-stress),256,ScreenHeight(),0.0f,0.012f+0.0752f*(10-stress),1.0f,1.0f);
	switch ((int)(stress/10+0.5))
	{
	case 0:
		pos = 0;
		break;
	case 1:
		pos = 83;
		break;
	case 2:
		pos = 103;
		break;
	case 3:
		pos = 121;
		break;
	case 4:
		pos = 141;
		break;
	case 5:
		pos = 161;
		break;		
	case 6:
		pos = 180;
		break;
	case 7:
		pos = 199;
		break;
	case 8:
		pos = 218;
		break;
	case 9:
		pos = 237;
		break;
	case 10:
		pos = 256;
		break;
	}
	surface()->DrawTexturedSubRect(0,ScreenHeight()-256+(256-pos)+2,256,ScreenHeight(),0.0f,-float(2/256)+float(256-pos)/256,1.0f,1.0f);

	//Draw HUD Bottom Right image (Ammo)
	surface()->DrawSetColor(255,255,255,255);
	surface()->DrawSetTexture(m_TBottomRight);
	surface()->DrawTexturedRect(ScreenWidth()-256,ScreenHeight()-256,ScreenWidth(),ScreenHeight());
	
	//Draw HUD Bottom (line)
	surface()->DrawSetColor(0,0,0,255);
	surface()->DrawFilledRect(256,ScreenHeight()-2,ScreenWidth()-256,ScreenHeight());	
	surface()->DrawSetColor(0,0,0,128);
	surface()->DrawFilledRect(256,ScreenHeight()-3,ScreenWidth()-256,ScreenHeight());	
	
	swprintf(unicode, L"%d", newHealth);

	//-------------------------------
	//Draw Health
	//-------------------------------
	surface()->DrawSetTextFont( hFont ); // set the font	
	if (newHealth >25)
		surface()->DrawSetTextColor( 252, 205, 125, 255 ); // Orange
	else
		surface()->DrawSetTextColor( 252, 25, 25, 255 ); // Rouge

	// x,y position 44,233    256+218
	if (newHealth>=10)
	{
		if (newHealth>=100)
			surface()->DrawSetTextPos( 30,ScreenHeight()-30 );
		else
			surface()->DrawSetTextPos( 30+charWidth/2,ScreenHeight()-30 );
	} else
		surface()->DrawSetTextPos( 30+charWidth,ScreenHeight()-30 );
	surface()->DrawPrintText( unicode, wcslen(unicode) ); // print text	

	//-------------------------------
	//Draw Clip and Ammo
	//-------------------------------
	if ( wpn )
	{		
		//wpn->GetEntityName()
		int maxclip = wpn->GetWpnData().iMaxClip1;
		
		if ( !strcmp(wpn->GetWpnData().szPrintName,"FlashLight") )
		{
			//pPlayer->SetStress(0);
			surface()->DrawSetTexture(m_TLight);
			surface()->DrawSetColor(255,255*( (float) clip / maxclip ),255*( (float) clip / maxclip ),255);
			if ((float)clip/maxclip > 1.0f/4)
				surface()->DrawSetColor(255,255,255,255);
			else
				surface()->DrawSetColor(255,0,0,255);
			surface()->DrawTexturedRect(ScreenWidth()-93,ScreenHeight()-32,ScreenWidth()-93+64,ScreenHeight());
			surface()->DrawSetTexture(m_TLightOn);		
			//20 - 105
			//85/128
			surface()->DrawTexturedSubRect(ScreenWidth()-93,ScreenHeight()-32,ScreenWidth()-93+64*( (float) 25/128 + (float) 85/128*( (float) clip / maxclip )),ScreenHeight(),0.0f,0.0f, (float) 25/128 + (float) 85/128*( (float) clip / maxclip ),1.0f);
			//surface()->DrawTexturedSubRect(ScreenWidth()-93,ScreenHeight()-32,ScreenWidth()-93+64*( (float) clip / maxclip ),ScreenHeight(),0.0f,0.0f, ( (float) clip / maxclip ),1.0f);
		}
		else 
		if ( wpn->UsesPrimaryAmmo() )
		{
			if ( clip==-1 )
			{	
				swprintf(unicode, L"%d", ammo);
				if (ammo>0)
					//surface()->DrawSetTextColor( 252, 205, 125, 255 ); // Orange
					surface()->DrawSetTextColor( 188, 227, 118, 255 ); // Vert
				else
					surface()->DrawSetTextColor( 252, 25, 25, 255 ); // Rouge
				surface()->DrawSetTextPos( ScreenWidth()-70,ScreenHeight()-30 ); //100, 40
				surface()->DrawPrintText( unicode, wcslen(unicode) ); // print text	
			}else
			{
				swprintf(unicode, L"%d", clip);
				if ( clip>=10 )
				{
					if ( clip>=100 )
						surface()->DrawSetTextPos( ScreenWidth()-90-charWidth,ScreenHeight()-30 ); //100, 40
					else
						surface()->DrawSetTextPos( ScreenWidth()-90,ScreenHeight()-30 ); //100, 40
				} else
					surface()->DrawSetTextPos( ScreenWidth()-90+charWidth,ScreenHeight()-30 ); //100, 40
				if ( clip>0 )
					//surface()->DrawSetTextColor( 252, 205, 125, 255 ); // Orange
					surface()->DrawSetTextColor( 188, 227, 118, 255 ); // Vert
				else
					surface()->DrawSetTextColor( 252, 25, 25, 255 ); // Rouge
				surface()->DrawPrintText( unicode, wcslen(unicode) ); // print text	
			
				surface()->DrawSetTextFont( hFont2 ); // set the font	
				swprintf(unicode, L"%d", ammo);
				if ( ammo>=10 )
				{
					if ( ammo>=100 )
						surface()->DrawSetTextPos( ScreenWidth()-45,ScreenHeight()-15 ); //100, 40
					else
						surface()->DrawSetTextPos( ScreenWidth()-45+charWidth2,ScreenHeight()-15 ); //100, 40
				} else
					surface()->DrawSetTextPos( ScreenWidth()-45+2*charWidth2,ScreenHeight()-15 ); //100, 40
				if ( ammo>0 )
					//surface()->DrawSetTextColor( 252, 205, 125, 255 ); // Orange
					surface()->DrawSetTextColor( 188, 227, 118, 255 ); // Vert
				else
					surface()->DrawSetTextColor( 252, 25, 25, 255 ); // Rouge
				surface()->DrawPrintText( unicode, wcslen(unicode) ); // print text	
			}
		}
	}

	//Crossair
	//vgui::surface()->DrawSetColor(0,0,0,128);
	//vgui::surface()->DrawFilledRect(ScreenWidth()/2-5,ScreenHeight()/2-5,ScreenWidth()/2+5,ScreenHeight()/2+5);
}