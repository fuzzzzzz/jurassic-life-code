//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Basic button control
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui/MouseCode.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>

#include "ButtonSlot.h"
//#include "cbase.h"
//#include </*../../game/shared/*/basecombatweapon_shared.h>


// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

DECLARE_BUILD_FACTORY_DEFAULT_TEXT( ButtonSlot, ButtonSlot );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
ButtonSlot::ButtonSlot(Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget, const char *pCmd)
: Button(parent,panelName,text,pActionSignalTarget,pCmd)
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
ButtonSlot::ButtonSlot(Panel *parent, const char *panelName, const wchar_t *text, Panel *pActionSignalTarget, const char *pCmd)
: Button(parent,panelName,text,pActionSignalTarget,pCmd)
{
	Init();
}

void ButtonSlot::Init()
{
	SetDragEnabled(true);
	SetDropEnabled(true,0.5);
	//LinkWeaponSlot(NULL);
	LinkSlot(-1,true);
	m_iItemType = -1;
	
	SetPaintBackgroundEnabled( false );

	m_iEmptyTexture = 0;
	m_fEmptyTextureUV[0] = 0.f;
	m_fEmptyTextureUV[1] = 0.f;
	m_fEmptyTextureUV[2] = 1.f;
	m_fEmptyTextureUV[3] = 1.f;
}

/*void ButtonSlot::LinkWeaponSlot(C_BaseCombatWeapon** slot)
{
	m_pWeaponSlot=slot;
	if (m_pWeaponSlot && (*m_pWeaponSlot))
	{
		SetText((*m_pWeaponSlot)->GetClassname());
	}else{
		SetText("");
	}
}

C_BaseCombatWeapon** ButtonSlot::GetWeaponSlot()
{
	return m_pWeaponSlot;
}*/

void ButtonSlot::LinkSlot(int slot, bool playerWeapon)
{
	m_bPlayerWeapon=playerWeapon;
	m_iSlot=slot;

	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if (player)
	{
		C_BaseCombatWeapon *weapon=NULL;
		if (m_iSlot!=-1)
		{
			if (m_bPlayerWeapon)
				weapon = player->GetWeapon(m_iSlot);
			else
				weapon = player->GetCaseItem(m_iSlot);
		}
		if (weapon)
			SetText(weapon->GetClassname());
		else
			SetText("");
	}
}

int ButtonSlot::GetSlot()
{
	return m_iSlot;
}

bool ButtonSlot::IsPlayerWeapon()
{
	return m_bPlayerWeapon;
}

C_BaseCombatWeapon* ButtonSlot::GetWeaponIntSlot() const
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	C_BaseCombatWeapon* pWeapon = NULL;
	if (m_iSlot!=-1 && pPlayer)
	{
		if (m_bPlayerWeapon)
			pWeapon = pPlayer->GetWeapon(m_iSlot);
		else
			pWeapon = pPlayer->GetCaseItem(m_iSlot);
	}
	return pWeapon;
}

void ButtonSlot::Switch(ButtonSlot *slot)
{
	if (slot)
	{
		C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
		if (player)
		{
			C_BaseCombatWeapon *weapon1=NULL;
			C_BaseCombatWeapon *weapon2=NULL;

			if (slot->m_bPlayerWeapon)
			{
				weapon1 = player->GetWeapon(slot->m_iSlot);
			}else{
				weapon1 = player->GetCaseItem(slot->m_iSlot);
			}

			if (m_bPlayerWeapon)
			{
				weapon2 = player->GetWeapon(m_iSlot);
			}else{
				weapon2 = player->GetCaseItem(m_iSlot);
			}

			/*if (slot->m_bPlayerWeapon)
			{
				player->SetWeapon(slot->m_iSlot,weapon2);
			}else{
				player->SetCaseItem(slot->m_iSlot,weapon2);
			}

			if (m_bPlayerWeapon)
			{
				player->SetWeapon(m_iSlot,weapon1);
			}else{
				player->SetCaseItem(m_iSlot,weapon1);
			}*/

			char temp[32];
			if (slot->m_bPlayerWeapon)
			{
				if (m_bPlayerWeapon)
				{
					Q_snprintf(temp,sizeof(temp),"switchInv %d %d",slot->m_iSlot,m_iSlot);
				}else{
					Q_snprintf(temp,sizeof(temp),"switchInvToCase %d %d",slot->m_iSlot,m_iSlot);
				}
			}else{
				if (m_bPlayerWeapon)
				{
					Q_snprintf(temp,sizeof(temp),"switchCaseToInv %d %d",slot->m_iSlot,m_iSlot);
				}else{
					Q_snprintf(temp,sizeof(temp),"switchCase %d %d",slot->m_iSlot,m_iSlot);
				}
			}
			engine->ClientCmd(temp); // Switch item

			slot->LinkSlot(slot->m_iSlot,slot->m_bPlayerWeapon);
			LinkSlot(m_iSlot,m_bPlayerWeapon);

			/*int iSlot = slot->m_iSlot;
			bool bPlayer = slot->m_bPlayerWeapon;
			slot->LinkSlot(m_iSlot,m_bPlayerWeapon);
			LinkSlot(iSlot,bPlayer);*/
		}
	}
}

void ButtonSlot::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	const char *emptyTexture =	inResourceData->GetString( "emptyTexture", NULL );
	if (emptyTexture)
	{
		m_iEmptyTexture = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(m_iEmptyTexture,emptyTexture,true,false);

		if ( m_iEmptyTexture != 0 )
		{
			int wide, tall;
			vgui::surface()->DrawGetTextureSize(m_iEmptyTexture, wide, tall );
			m_fEmptyTextureUV[0] = inResourceData->GetInt( "emptyTextureUVMinX", 0 ) / (float)wide;
			m_fEmptyTextureUV[1] = inResourceData->GetInt( "emptyTextureUVMinY", 0 ) / (float)tall;
			m_fEmptyTextureUV[2] = inResourceData->GetInt( "emptyTextureUVMaxX", wide ) / (float)wide;
			m_fEmptyTextureUV[3] = inResourceData->GetInt( "emptyTextureUVMaxY", tall ) / (float)tall;
		}
	}
	m_iItemType = inResourceData->GetInt("itemType", -1);
}

void ButtonSlot::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	if ( m_iEmptyTexture != 0 )
	{
		SetButtonBorderEnabled(false);
		SetDefaultBorder(NULL);
		SetDepressedBorder( pScheme->GetBorder("ButtonSlotDepressedBorder") );
		SetBorder(NULL);
	}
}

void ButtonSlot::Paint(void)
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	C_BaseCombatWeapon *weapon=NULL;
	if (m_iSlot!=-1 && player)
	{
		if (m_bPlayerWeapon)
			weapon = player->GetWeapon(m_iSlot);
		else
			weapon = player->GetCaseItem(m_iSlot);
	}

	if (weapon)
	{	
		if (weapon->GetWpnData().iconInactive)
		{
			float fRatio;
			fRatio = GetWide()/(float)weapon->GetWpnData().iconInactive->Width();
			fRatio = min(fRatio,GetTall()/(float)weapon->GetWpnData().iconInactive->Height());
			float xoff = (GetWide() - weapon->GetWpnData().iconInactive->Width()*fRatio)/2.0;
			float yoff = (GetTall() - weapon->GetWpnData().iconInactive->Height()*fRatio)/2.0;
			if (weapon->GetWpnData().iconInactive->bRenderUsingFont)
			{	
				//vgui::surface()->DrawSetTextScale(fRatio,fRatio);
			}
			//weapon->GetWpnData().iconInactive->DrawSelf(xoff,yoff,GetWide()-xoff*2,GetTall()-yoff*2,Color(255,255,255,255));
			weapon->GetWpnData().iconInactive->DrawSelfCropped(xoff,yoff,
				0,0,
				weapon->GetWpnData().iconInactive->Width(), weapon->GetWpnData().iconInactive->Height(),
				GetWide()-xoff*2,GetTall()-yoff*2,
				Color(255,255,255,255));
		}
	}else{
		if ( m_iEmptyTexture != 0 )
		{
			surface()->DrawSetColor( 255, 255, 255, 255 );
			/*
			m_pEmptyTexture->SetPos(0, 0);
			m_pEmptyTexture->SetSize(GetWide(),GetTall());
			m_pEmptyTexture->Paint();
			*/
			surface()->DrawSetTexture(m_iEmptyTexture);
			surface()->DrawTexturedSubRect( 0, 0, GetWide(), GetTall(), 
			m_fEmptyTextureUV[0], m_fEmptyTextureUV[1], m_fEmptyTextureUV[2] ,m_fEmptyTextureUV[3] );
		}
	}

	//BaseClass::Paint();
}

bool ButtonSlot::IsDragEnabled() const
{
	if ( GetWeaponIntSlot() )
	{
		return BaseClass::IsDragEnabled();
	}
	return false;
}

bool ButtonSlot::IsDroppable( CUtlVector< KeyValues * >& msglist )
{
	DevMsg("IsDroppable %s\n", GetName());
	ButtonSlot *slot = static_cast<vgui::ButtonSlot*>(msglist[0]->GetPtr("panel"));
	if (slot)
	{
		DevMsg("Droppable %d %d\n", m_iItemType, slot->m_iItemType );
		if ( m_iItemType == slot->m_iItemType )
		{
			return true;
		}else
		{
			C_BaseCombatWeapon* pWeapon1 = slot->GetWeaponIntSlot();
			C_BaseCombatWeapon* pWeapon2 = GetWeaponIntSlot();
			//If empty slot empty
			if ( pWeapon2 == NULL )
			{
				if ( m_iItemType == -1 || 
					 m_iItemType == pWeapon1->m_iItemType )
				{
					return true;
				}
			}else{
				if ( pWeapon1->m_iItemType == pWeapon2->m_iItemType )
					return true;
			}
		}		
	}
	return false;
}

void ButtonSlot::OnPanelDropped( CUtlVector< KeyValues * >& msglist )
{
	vgui::Panel* panel = (vgui::Panel*)msglist[0]->GetPtr("panel");
	if (panel)
	{
		ButtonSlot *slot = dynamic_cast<vgui::ButtonSlot*>(panel);
		if (slot)
		{
			Switch(slot);
		}
	}

	ivgui()->PostMessage( 
		GetVPanel(), 
		new KeyValues( "MouseReleased", "code", MOUSE_LEFT ), 
		GetVPanel() );
	ivgui()->PostMessage( 
		GetParent()->GetVPanel(), 
		new KeyValues( "MouseReleased", "code", MOUSE_LEFT ), 
		GetVPanel() );
	if (panel)
	{
		ivgui()->PostMessage( 
			panel->GetVPanel(), 
			new KeyValues( "MouseReleased", "code", MOUSE_LEFT ), 
			GetVPanel() );
	}
	
	Panel::OnPanelDropped( msglist );
}

void ButtonSlot::OnDroppablePanelPaint( CUtlVector< KeyValues * >& msglist, CUtlVector< Panel * >& dragPanels )
{
	//BaseClass::Paint();	

	if ( dragPanels.Count()>=1 && dragPanels[0] )
	{
		((ButtonSlot*)dragPanels[0])->DisplayDragWeapon();		
	}

	/*int w, h;
	GetSize( w, h );

	int x, y;
	x = y = 0;
	LocalToScreen( x, y );

	surface()->DrawSetColor( 128,128,128,128 );
	// Draw 2 pixel frame
	surface()->DrawOutlinedRect( x, y, x + w, y + h );
	surface()->DrawOutlinedRect( x+1, y+1, x + w-1, y + h-1 );
	*/

	// Draw all droppable area
	
	//bool isitem;

	//C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();

	/*const char* name = GetName();
	char *num = "0";
	num[0] = name[strlen(name)-1];
	isitem = player->GetWeapon(atoi(num))->IsItem();*/

	//Panel *target = GetParent()->GetDropTarget( msglist ); 

	/*
	Panel *child;
	int w, h;
	int x, y;
	int sw, sh;
	
	for (int i=0;i<GetParent()->GetChildCount();i++)
	{
		child = GetParent()->GetChild(i);
		if (!child)
			continue;
		
		if (child->IsDropEnabled())
		{
			x = y = 0;
			child->LocalToScreen( x, y );
			child->GetSize( sw, sh );
			//w = min( sw, 80 );
			//h = min( sh, 80 );
			w=sw;
			h=sh;

			surface()->DrawSetColor( 128,128,128,128 );
			
			// Draw 2 pixel frame
			surface()->DrawOutlinedRect( x, y, x + w, y + h );
			surface()->DrawOutlinedRect( x+1, y+1, x + w-1, y + h-1 );
		}
	}

	if (target)
	{
		target->LocalToScreen( x, y );
		target->GetSize( sw, sh );
		surface()->DrawSetColor( 128,128,0,128 );
			
		// Draw 2 pixel frame
		surface()->DrawOutlinedRect( x, y, x + w, y + h );
		surface()->DrawOutlinedRect( x+1, y+1, x + w-1, y + h-1 );
	}*/
	BaseClass::OnDroppablePanelPaint( msglist, dragPanels ); 
}

void ButtonSlot::OnDraggablePanelPaint()
{
	//BaseClass::Paint();

	DisplayDragWeapon();

	/*int x, y;
	int w, h;
	int sw, sh;

	GetSize( sw, sh );
	input()->GetCursorPos( x, y );
	
	w = min( sw, 80 );
	h = min( sh, 80 );
	x -= ( w >> 1 );
	y -= ( h >> 1 );

	surface()->DrawSetColor( 128,128,128,128 );
	surface()->DrawOutlinedRect( x, y, x + w, y + h );
	wchar_t weapon[32];
	GetText(weapon,sizeof(weapon));
	surface()->DrawSetTextPos(x, y);
	surface()->DrawUnicodeString(weapon);

	/*if ( m_pDragDrop->m_DragPanels.Count() > 1 )
	{
		surface()->DrawSetTextColor( m_clrDragFrame );
		surface()->DrawSetTextFont( m_infoFont );
		surface()->DrawSetTextPos( x + 5, y + 2 );

		wchar_t sz[ 64 ];
		_snwprintf( sz, 64, L"[ %i ]", m_pDragDrop->m_DragPanels.Count() );

		surface()->DrawPrintText( sz, wcslen( sz ) );
	}*/
	
	//// Draw all droppable area
	//Panel *child;
	////int x, y;
	//for (int i=0;i<GetParent()->GetChildCount();i++)
	//{
	//	child = GetParent()->GetChild(i);
	//	if (!child)
	//		continue;

	//	if (child->IsDropEnabled())
	//	{			
	//		x = y = 0;
	//		child->LocalToScreen( x, y );
	//		child->GetSize( sw, sh );
	//		//w = min( sw, 80 );
	//		//h = min( sh, 80 );

	//		surface()->DrawSetColor( 128,128,128,128 );
	//		// Draw 2 pixel frame
	//		surface()->DrawOutlinedRect( x, y, x + w, y + h );
	//		surface()->DrawOutlinedRect( x+1, y+1, x + w-1, y + h-1 );
	//	}
	//}*/
}

void ButtonSlot::DisplayDragWeapon()
{
	int x, y;
	int sw, sh;

	GetSize( sw, sh );
	sw=128;
	sh=128;
	input()->GetCursorPos( x, y );

	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	C_BaseCombatWeapon *weapon=NULL;
	if (m_iSlot!=-1 && player)
	{
		if (m_bPlayerWeapon)
			weapon = player->GetWeapon(m_iSlot);
		else
			weapon = player->GetCaseItem(m_iSlot);
	}

	if (weapon)
	{	
		if (weapon->GetWpnData().iconInactive)
		{
			float fRatio;
			fRatio = sw/(float)weapon->GetWpnData().iconInactive->Width();
			fRatio = min(fRatio,sh/(float)weapon->GetWpnData().iconInactive->Height());
			float xoff = (sw - weapon->GetWpnData().iconInactive->Width()*fRatio)/2.0;
			float yoff = (sh - weapon->GetWpnData().iconInactive->Height()*fRatio)/2.0;
			if (weapon->GetWpnData().iconInactive->bRenderUsingFont)
			{	
				vgui::surface()->DrawSetTextScale(fRatio,fRatio);
			}
			weapon->GetWpnData().iconInactive->DrawSelf(x-sw/2+xoff,y-sh/2+yoff,sw-xoff*2,sh-yoff*2,Color(255,255,255,255));
		}
	}
}
