//
//#include "lua_functions.h"
//#include "lstate.h"
//
//#include <vgui_controls/Panel.h>
//#include <vgui_controls/Label.h>
//
////#include "game/client/cbase.h"
//#include "tier0/dbg.h"
//
//
//
//
//// vgui Label SetText
//static int LuaSetLabelText(lua_State *pState)
//{
//	if (lua_gettop(pState) == 2)
//	{
//		if (lua_isstring(pState,1) && lua_isstring(pState,2))
//		{
//			if (pState->userData)
//			{
//				VGUILuaBind *m_L = (VGUILuaBind*)pState->userData;
//				if (m_L->GetCustomData())
//				{
//					vgui::Panel *panel = (vgui::Panel*)m_L->GetPanel();
//					
//					//DevMsg("Search label %s\n",lua_tolstring(pState,1,NULL));
//					vgui::Label *label = dynamic_cast<vgui::Label*>(panel->FindChildByName(lua_tolstring(pState,1,NULL)));
//					if (label)
//					{
//						//DevMsg("Label found\n");
//						label->SetText(lua_tolstring(pState,2,NULL));
//					}
//				}
//			}
//		}else{
//			lua_pushstring(pState, "Incorrect argument to 'SetLabelText'");
//			lua_error(pState);
//		}
//	}
//	return 0;
//}
//
//static int LuaGetLabelText(lua_State *pState)
//{
//	/* get number of arguments */
//	int n = lua_gettop(pState);
//
//	if (n==1)
//	{
//		if (lua_isstring(pState,1))
//		{
//			if (pState->userData)
//			{
//				LuaBind *m_L = (LuaBind*)pState->userData;
//				if (m_L->GetBindFor()==LuaBind::LBF_VGUI)
//				{
//					if (m_L->GetCustomData())
//					{
//						vgui::Panel *panel = (vgui::Panel*)m_L->GetCustomData();
//						
//						//DevMsg("Search label %s\n",lua_tolstring(pState,1,NULL));
//						vgui::Label *label = dynamic_cast<vgui::Label*>(panel->FindChildByName(lua_tolstring(pState,1,NULL)));
//						if (label)
//						{
//							//DevMsg("Label found\n");
//							char text[2048];
//							label->GetText(text,2048);							
//							lua_pushstring(pState, text);
//							return 1;
//						}
//					}
//				}
//			}
//		}else{
//			lua_pushstring(pState, "Incorrect argument to 'GetLabelText'");
//			lua_error(pState);
//		}
//	}
//	lua_pushstring(pState, "");
//	return 1;
//}
//
//// vgui Label SetText
//static int LuaSetLogicCaseValue(lua_State *pState)
//{
//	/* get number of arguments */
//	int n = lua_gettop(pState);
//
//	if (n==2)
//	{
//		if (lua_isstring(pState,1) && lua_isstring(pState,2))
//		{
//			
//		}else{
//			lua_pushstring(pState, "Incorrect argument to 'SetLogicCaseValue'");
//			lua_error(pState);
//		}
//	}
//	return 0;
//}
//
//static int LuaServerCommand(lua_State *pState)
//{
//	/* get number of arguments */
//	int n = lua_gettop(pState);
//
//	if (n==1)
//	{
//		if (lua_isstring(pState,1))
//		{
//			//engine->ClientCmd(lua_tolstring(pState,1,NULL))
//			if (pState->userData)
//			{
//				LuaBind *m_L = (LuaBind*)pState->userData;
//				if (m_L->GetBindFor()==LuaBind::LBF_VGUI)
//				{
//					if (m_L->GetCustomData())
//					{
//						vgui::Panel *panel = (vgui::Panel*)m_L->GetCustomData();
//						panel->OnCommand(lua_tolstring(pState,1,NULL));
//					}
//				}
//			}
//		}else{
//			lua_pushstring(pState, "Incorrect argument to 'ServerCommand'");
//			lua_error(pState);
//		}
//	}
//	return 0;
//}
