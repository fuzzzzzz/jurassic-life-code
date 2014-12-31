
#ifndef LOGICIMAGEBUTTON_H
#define LOGICIMAGEBUTTON_H
 
#ifdef _WIN32
#pragma once
#endif
 

#include <vgui/VGUI.h>
#include <vgui_controls/ImageButton.h>
#include <vgui_controls/LogicControl.h>
 
namespace vgui
{
 
	class LogicImageButton : public ImageButton, public LogicControl
	{
		DECLARE_CLASS_SIMPLE( LogicImageButton, ImageButton );
	 
		enum LogicState
		{
			STATE_VISIBLE = 1<<1,
			STATE_ENABLE = 1<<2,		
		};
	public:
		//ImageButton( Panel *parent, const char *panelName, const char *normalImage, const char *mouseOverImage = NULL, const char *mouseClickImage = NULL, const char *pCmd=NULL ); //: ImagePanel( parent, panelName );
		LogicImageButton( Panel *parent, const char *panelName);
		~LogicImageButton();

		virtual void ApplySettings(KeyValues *inResourceData);

		virtual void SetStateInternal(StateControl state);
	 
	private:
		LogicState m_iState[10];
	};
 
} //namespace vgui
 
#endif //LOGICIMAGEBUTTON_H