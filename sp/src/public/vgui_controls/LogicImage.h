
#ifndef LOGICIMAGE_H
#define LOGICIMAGE_H
 
#ifdef _WIN32
#pragma once
#endif
 

#include <vgui/VGUI.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/LogicControl.h>
 
namespace vgui
{
 
	class LogicImage : public ImagePanel, public LogicControl
	{
		DECLARE_CLASS_SIMPLE( LogicImage, ImagePanel );
	 
	public:
		LogicImage( Panel *parent, const char *panelName);
		~LogicImage();

		virtual void ApplySettings(KeyValues *inResourceData);

		virtual void SetStateInternal(StateControl state);
	private:
		vgui::IImage *m_pStateImage[10];
	};
 
} //namespace vgui
 
#endif //LOGICIMAGE_H