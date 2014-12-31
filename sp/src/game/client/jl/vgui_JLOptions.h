#ifndef VGUI_JLOPTIONS_H
#define VGUI_JLOPTIONS_H
#ifdef _WIN32
#pragma once
#endif
 
#include "vgui_helpers.h"
#include <vgui_controls/Frame.h>
 
 
class CJLOptions : public vgui::Frame
{
 	DECLARE_CLASS_SIMPLE(CJLOptions, vgui::Frame);
 public:
 	CJLOptions( vgui::VPANEL parent );

	void Center();
};
 
IGameUI* GetJLOptionsPanel();
 
#endif // VGUI_JLOPTIONS_H