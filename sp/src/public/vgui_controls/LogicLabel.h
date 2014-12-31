
#ifndef LOGICLABEL_H
#define LOGICLABEL_H
 
#ifdef _WIN32
#pragma once
#endif
 

#include <vgui/VGUI.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/LogicControl.h>
 
namespace vgui
{
 
	class LogicLabel : public Label, public LogicControl
	{
		DECLARE_CLASS_SIMPLE( LogicLabel, Label );
	 
	public:
		LogicLabel( Panel *parent, const char *panelName);
		~LogicLabel();

		virtual void ApplySettings(KeyValues *inResourceData);

		virtual void SetStateInternal(StateControl state);
	private:
		void SetStateText(int state, const char *text);
	private:
		char *m_pStateText[10];
	};
 
} //namespace vgui
 
#endif //LOGICIMAGE_H