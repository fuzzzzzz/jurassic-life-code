
#ifndef LOGICCONTROL_H
#define LOGICCONTROL_H
 
#ifdef _WIN32
#pragma once
#endif
 
#include <vgui/VGUI.h>
#include "tier1/KeyValues.h"

namespace vgui
{
 
	class LogicControl
	{
	public:
		enum StateControl
		{			
			STATE_0,
			STATE_1,
			STATE_2,
			STATE_3,
			STATE_4,
			STATE_5,
			STATE_6,
			STATE_7,
			STATE_8,
			STATE_9,
		};
	public:
		LogicControl();

		virtual void SetStateInternal(StateControl state)=0;
		virtual void SetState(int state);
	private:
		int m_iState;
	};
 
} //namespace vgui
 
#endif //LOGICIMAGE_H