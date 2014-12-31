
#ifndef IMAGEBUTTON_H
#define IMAGEBUTTON_H
 
#ifdef _WIN32
#pragma once
#endif
 

#include <vgui/VGUI.h>
#include "vgui/MouseCode.h"
#include <vgui_controls/ImagePanel.h>
 
namespace vgui
{
 
class FakeCursor : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( FakeCursor, vgui::Panel );
public:
	FakeCursor( Panel *parent, const char *panelName);
	~FakeCursor();

	virtual void ApplySchemeSettings(IScheme *pScheme);

	virtual void Paint();
	virtual void OnThink();
private:
	int m_iTexture;
};
 
} //namespace vgui
 
#endif //IMAGEBUTTON_H