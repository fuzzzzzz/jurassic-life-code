//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_selection.h"
#include "iclientmode.h"
#include "history_resource.h"
#include "input.h"

#include <KeyValues.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/Panel.h>

#include "vgui/ILocalize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SELECTION_TIMEOUT_THRESHOLD		1.0f	// Seconds
#define SELECTION_FADEOUT_TIME			0.4f

//-----------------------------------------------------------------------------
// Purpose: JL weapon selection hud element
//-----------------------------------------------------------------------------
class CHudJLWeaponSelection : public CBaseHudWeaponSelection, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudJLWeaponSelection, vgui::Panel );

public:
	CHudJLWeaponSelection(const char *pElementName );

	virtual bool ShouldDraw();
	virtual void OnWeaponPickup( C_BaseCombatWeapon *pWeapon );

	virtual void CycleToNextWeapon( void );
	virtual void CycleToPrevWeapon( void );

	virtual void SelectSlot( int iSlot )
	{
		SwitchWeaponSlot(iSlot-1);
	}

	virtual C_BaseCombatWeapon	*GetSelectedWeapon( void )
	{ 
		return m_hSelectedWeapon;
	}

	C_BaseCombatWeapon *GetWeapon( int slot )
	{ 
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		if ( !pPlayer )
			return NULL;

		return pPlayer->GetWeapon(slot);
	}

	void SwitchWeaponSlot(int slot)
	{
		C_BaseCombatWeapon *weapon = GetWeapon(slot);
		if (weapon)
		{
			if (weapon->GetWpnData().bIsItem == false)
			{
				input->MakeWeaponSelection( weapon );
			}else if (IsInSelectionMode())
			{
				char temp2[32];
				Q_snprintf(temp2,sizeof(temp2),"primaryattack %d",slot+1);
				engine->ClientCmd(temp2);
			}else{
				OpenSelection();
			}
		}
	}

	virtual void OpenSelection( void );
	virtual void HideSelection( void );

	virtual void LevelInit();

protected:
	virtual void Init( void );
	virtual void OnThink();
	virtual void Paint();
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	virtual bool IsWeaponSelectable()
	{ 
		if (IsInSelectionMode())
			return true;

		return false;
	}

	virtual void SetWeaponSelected()
	{
		if (m_hSelectedWeapon && m_hSelectedWeapon->GetWpnData().bIsItem == false)
		{
			CBaseHudWeaponSelection::SetWeaponSelected();
		}
	}

private:
	C_BaseCombatWeapon *FindNextWeaponInWeaponSelection(int iCurrentSlot, int iCurrentPosition);
	C_BaseCombatWeapon *FindPrevWeaponInWeaponSelection(int iCurrentSlot, int iCurrentPosition);
    
	virtual C_BaseCombatWeapon *GetWeaponInSlot( int iSlot, int iSlotPos )
	{
		return NULL;
	}

	virtual void SelectWeaponSlot( int iSlot ) 
	{
		m_hSelectedWeapon = GetWeapon(iSlot-1);
	}

	void FastWeaponSwitch( int iWeaponSlot );

	virtual	void SetSelectedWeapon( C_BaseCombatWeapon *pWeapon ) 
	{ 
		m_hSelectedWeapon = pWeapon;
	}

	int		m_iTexNormal;
	int		m_iTexSelected;

	float	m_fLastChange;
};

DECLARE_HUDELEMENT( CHudJLWeaponSelection );

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudJLWeaponSelection::CHudJLWeaponSelection( const char *pElementName ) : CBaseHudWeaponSelection(pElementName), BaseClass(NULL, "HudJLWeaponSelection")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
}

void CHudJLWeaponSelection::Init( void )
{
	m_iTexNormal = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(m_iTexNormal,"hud/weapon",true,false);
	m_iTexSelected = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(m_iTexSelected,"hud/weaponselected",true,false);
}

//-----------------------------------------------------------------------------
// Purpose: sets up display for showing weapon pickup
//-----------------------------------------------------------------------------
void CHudJLWeaponSelection::OnWeaponPickup( C_BaseCombatWeapon *pWeapon )
{
	// add to pickup history
	/*CHudHistoryResource *pHudHR = GET_HUDELEMENT( CHudHistoryResource );
	if ( pHudHR )
	{
		pHudHR->AddToHistory( pWeapon );
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: updates animation status
//-----------------------------------------------------------------------------
void CHudJLWeaponSelection::OnThink( void )
{
	if (gpGlobals->curtime - m_fLastChange > SELECTION_TIMEOUT_THRESHOLD)
	{
		if (gpGlobals->curtime - m_fLastChange > SELECTION_TIMEOUT_THRESHOLD + SELECTION_FADEOUT_TIME)
		{
			HideSelection();
		}else{
			SetAlpha(255 - 255*(gpGlobals->curtime - m_fLastChange - SELECTION_TIMEOUT_THRESHOLD)/SELECTION_FADEOUT_TIME);
		}
	}else{
		if (GetAlpha() != 255)
		{
			SetAlpha(255);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the panel should draw
//-----------------------------------------------------------------------------
bool CHudJLWeaponSelection::ShouldDraw()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
	{
		if ( IsInSelectionMode() )
		{
			HideSelection();
		}
		return false;
	}

	bool bret = CBaseHudWeaponSelection::ShouldDraw();
	if ( !bret )
		return false;

	// For fast swtich
	if (gpGlobals->curtime - m_fLastChange < SELECTION_TIMEOUT_THRESHOLD + SELECTION_FADEOUT_TIME)
		return true;

	return ( m_bSelectionVisible ) ? true : false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudJLWeaponSelection::LevelInit()
{
	CHudElement::LevelInit();
}

//-------------------------------------------------------------------------
// Purpose: draws the selection area
//-------------------------------------------------------------------------
void CHudJLWeaponSelection::Paint()
{
	const int StartY = 20; // From top
	const int EndY = 80; // From bottom
	int xpos;
	int ypos;

	if (!ShouldDraw())
		return;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	// find and display our current selection
	C_BaseCombatWeapon *pSelectedWeapon = GetSelectedWeapon();
	C_BaseCombatWeapon *pActiveWeapon = pPlayer->GetActiveWeapon();

	int height = ScreenHeight()-EndY-StartY;
	if (height > MAX_WEAPONS*64)
	{
		height = MAX_WEAPONS*64;
	}
	float sizeX, sizeY;
	sizeY = height/(float)MAX_WEAPONS;
	sizeX = sizeY * 2.0;

	xpos = ScreenWidth() - sizeY*2.0;
	ypos = StartY + ((ScreenHeight()-EndY-StartY)-height)/2.0;

	for (int i=0;i<MAX_WEAPONS;i++)
	{
		C_BaseCombatWeapon *weapon = pPlayer->GetWeapon(i);
	
		if (weapon && weapon == pSelectedWeapon)
			surface()->DrawSetTexture(m_iTexSelected);
		else
			surface()->DrawSetTexture(m_iTexNormal);

		surface()->DrawSetColor(255,255,255,255);
		surface()->DrawTexturedRect(xpos,ypos,xpos+sizeX,ypos+sizeY);

		if (weapon && weapon->GetWpnData().iconInactive)
		{
			float fRatio;
			fRatio = sizeX/(float)weapon->GetWpnData().iconInactive->Width();
			fRatio = min(fRatio,sizeY/(float)weapon->GetWpnData().iconInactive->Height());
			float xoff = (sizeX - weapon->GetWpnData().iconInactive->Width()*fRatio)/2.0;
			float yoff = (sizeY - weapon->GetWpnData().iconInactive->Height()*fRatio)/2.0;
			if (weapon->GetWpnData().iconInactive->bRenderUsingFont)
			{
				//vgui::surface()->DrawSetTextScale(fRatio,fRatio);
			}
			//weapon->GetWpnData().iconInactive->DrawSelf(xpos+xoff+5,ypos+yoff+5,sizeX-xoff*2-10,sizeY-yoff*2-10,Color(255,255,255,255));
			weapon->GetWpnData().iconInactive->DrawSelfCropped(xpos+xoff+5,ypos+yoff+5,
				0,0,
				weapon->GetWpnData().iconInactive->Width(), weapon->GetWpnData().iconInactive->Height(),
				//32,32,
				//sizeX-xoff*2-10,sizeY-yoff*2-10,
				32,32,
				Color(255,255,255,255));
		}

		ypos += sizeY;
	}	
}

//-----------------------------------------------------------------------------
// Purpose: hud scheme settings
//-----------------------------------------------------------------------------
void CHudJLWeaponSelection::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);
}

//-----------------------------------------------------------------------------
// Purpose: Opens weapon selection control
//-----------------------------------------------------------------------------
void CHudJLWeaponSelection::OpenSelection( void )
{
	Assert(!IsInSelectionMode());

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (pPlayer)
		SetSelectedWeapon( pPlayer->GetActiveWeapon() );

	m_fLastChange = gpGlobals->curtime;

	CBaseHudWeaponSelection::OpenSelection();
}

//-----------------------------------------------------------------------------
// Purpose: Closes weapon selection control immediately
//-----------------------------------------------------------------------------
void CHudJLWeaponSelection::HideSelection( void )
{
	CBaseHudWeaponSelection::HideSelection();
}

//-----------------------------------------------------------------------------
// Purpose: Moves the selection to the next item in the menu
//-----------------------------------------------------------------------------
void CHudJLWeaponSelection::CycleToNextWeapon( void )
{
	// Get the local player.
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	C_BaseCombatWeapon *activeWeapon;
	activeWeapon = GetSelectedWeapon();
	if (!activeWeapon)
		activeWeapon = pPlayer->GetActiveWeapon();

	if ( !IsInSelectionMode() )
	{
		OpenSelection();
		//return;
	}

	m_fLastChange = gpGlobals->curtime;

	int pos = -1;
	for (int i=0;i<MAX_WEAPONS;i++)
	{
		if (pPlayer->GetWeapon(i) == activeWeapon)
		{
			pos = i;
			break;
		}
	}
	if (pos != -1)
	{
		int i = pos+1;
		if (i>=MAX_WEAPONS)
			i=0;
		while (pPlayer->GetWeapon(i) != activeWeapon)
		{
			if (pPlayer->GetWeapon(i) && pPlayer->GetWeapon(i)->GetWpnData().bIsItem == false)
			{
				SetSelectedWeapon(pPlayer->GetWeapon(i));
				// Play the "cycle to next weapon" sound
				pPlayer->EmitSound( "Player.WeaponSelectionMoveSlot" );
				return;
			}
			i++;
			if (i>=MAX_WEAPONS)
				i=0;
		}
	}	
}

//-----------------------------------------------------------------------------
// Purpose: Moves the selection to the previous item in the menu
//-----------------------------------------------------------------------------
void CHudJLWeaponSelection::CycleToPrevWeapon( void )
{
	// Get the local player.
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	C_BaseCombatWeapon *activeWeapon;
	activeWeapon = GetSelectedWeapon();
	if (!activeWeapon)
		activeWeapon = pPlayer->GetActiveWeapon();

	if ( !IsInSelectionMode() )
	{
		OpenSelection();
		//return;
	}

	m_fLastChange = gpGlobals->curtime;

	int pos = -1;
	for (int i=0;i<MAX_WEAPONS;i++)
	{
		if (pPlayer->GetWeapon(i) == activeWeapon)
		{
			pos = i;
			break;
		}
	}
	if (pos != -1)
	{
		int i = pos-1;
		if (i<0)
			i=7;
		while (pPlayer->GetWeapon(i) != activeWeapon)
		{
			if (pPlayer->GetWeapon(i) && pPlayer->GetWeapon(i)->GetWpnData().bIsItem == false)
			{
				SetSelectedWeapon(pPlayer->GetWeapon(i));
				// Play the "cycle to next weapon" sound
				pPlayer->EmitSound( "Player.WeaponSelectionMoveSlot" );
				return;
			}
			i--;
			if (i<0)
				i=7;
		}
	}
}
