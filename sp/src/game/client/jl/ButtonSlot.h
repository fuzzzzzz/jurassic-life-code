//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Creates a HTML control
//
// $NoKeywords: $
//=============================================================================//

#ifndef BUTTONSLOT_H
#define BUTTONSLOT_H

#include <vgui_controls/Button.h>

class C_BaseCombatWeapon;

namespace vgui
{

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class ButtonSlot : public Button
{
	DECLARE_CLASS_SIMPLE( ButtonSlot, Button );

public:
	// You can optionally pass in the panel to send the click message to and the name of the command to send to that panel.
	ButtonSlot(Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget=NULL, const char *pCmd=NULL);
	ButtonSlot(Panel *parent, const char *panelName, const wchar_t *text, Panel *pActionSignalTarget=NULL, const char *pCmd=NULL);

	void LinkSlot(int slot, bool playerWeapon=true);
	int GetSlot();
	bool IsPlayerWeapon();
	C_BaseCombatWeapon* GetWeaponIntSlot() const;

	virtual void ApplySettings( KeyValues *inResourceData );
	virtual void ApplySchemeSettings(IScheme *pScheme);

	virtual void Paint(void);

	virtual bool IsDragEnabled() const;
	virtual bool IsDroppable( CUtlVector< KeyValues * >& msglist );

	virtual void OnPanelDropped( CUtlVector< KeyValues * >& msglist );
	virtual void OnDroppablePanelPaint( CUtlVector< KeyValues * >& msglist, CUtlVector< Panel * >& dragPanels );
	virtual void OnDraggablePanelPaint();
private:
	void Init();
	void Switch(ButtonSlot *slot);

	void DisplayDragWeapon();
protected:
	bool m_bPlayerWeapon;
	int m_iSlot;
	int m_iItemType;

	//vgui::IImage* m_pEmptyTexture;
	int m_iEmptyTexture;
	float m_fEmptyTextureUV[4];
};

} // namespace vgui

#endif // BUTTONSLOT_H
