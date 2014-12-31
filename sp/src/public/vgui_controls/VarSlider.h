//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VARSLIDER_H
#define VARSLIDER_H

#ifdef _WIN32
#pragma once
#endif

#include <string>

#include <vgui/VGUI.h>
#include <vgui_controls/Slider.h>

namespace vgui
{

	//-----------------------------------------------------------------------------
	// Labeled horizontal slider
	//-----------------------------------------------------------------------------
	class VarSlider : public Slider
	{
		DECLARE_CLASS_SIMPLE( VarSlider, Slider );

	public:
		VarSlider(Panel *parent, const char *panelName);

		virtual void ApplySettings(KeyValues *inResourceData);

		virtual bool RequestInfo(KeyValues *data);

		MESSAGE_FUNC( OnSliderMoved, "SliderMoved" );
	protected:
		float m_fCoeff;
		std::string m_sVarName;
	};

}

#endif // SLIDER_H
