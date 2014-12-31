//The following include files are necessary to allow your MyPanel.cpp to compile.
#include "cbase.h"
#include "HUD_Macros.h"
#include "Icase_panel.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui/IInput.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Scrollbar.h>
#include "vgui/ILocalize.h"
#include <KeyValues.h> 
#include "IGameUIFuncs.h" // for key bindings 
#include <igameresources.h>
#include "iclientmode.h"
//#include "iinventory.h"
#include "ButtonSlot.h"

extern IGameUIFuncs *gameuifuncs; // for key binding details 

//CMyPanel class: Tutorial example class
class CCasePanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CCasePanel, vgui::Frame); 
	//CMyPanel : This Class / vgui::Frame : BaseClass

	CCasePanel(vgui::VPANEL parent); 	// Constructor
	~CCasePanel(){};				// Destructor

protected:
	//VGUI overrides:
	virtual void OnTick();
	virtual void OnCommand(const char* pcCommand);
	void	CCasePanel::OnKeyCodePressed(KeyCode code);
	void	Paint( void );	
	//void	Init( void );

	void Center()
	{
		int x,w,h; 
		GetBounds(x,x,w,h);
		SetPos(0.5f*(ScreenWidth()-w),0.5f*(ScreenHeight()-h));
	}

	Panel	*CreateControlByName(const char *controlName);

	/*int m_iSlot1;
	int m_iSlot2;
	int m_iSlot3;
	int m_iSlot4;
	int m_iSlot5;
	int m_iSlot6;*/
	int m_iQuit;

	int selecteditem;

	int m_iState;

	wchar_t		unicode[6];
	vgui::HFont hFont ;
private:
	void UpdateCasePanel();
private:
	//Other used VGUI control Elements:
	int m_iScrollPos;

};

// Constuctor: Initializes the Panel
CCasePanel::CCasePanel(vgui::VPANEL parent): BaseClass(NULL, "CasePanel")
{
	SetParent( parent );
	
	SetKeyBoardInputEnabled( true );
	SetMouseInputEnabled( true );
	
	SetProportional( false );
	SetTitleBarVisible( false );
	SetMinimizeButtonVisible( false );
	SetMaximizeButtonVisible( false );
	SetCloseButtonVisible( false );
	SetSizeable( false );
	SetMoveable( false );
	SetVisible( true );
	SetSize(640,512);
	SetPos(ScreenWidth()/2-640/2,ScreenHeight()/2-512/2);
	SetPaintBorderEnabled( false );
	SetPaintBackgroundEnabled( false );
	
	// test for menu background
	this->FlashWindowStop();
	//this->GetParent()->GetParent()


	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));

	LoadControlSettings("resource/UI/CasePanel.res");

	vgui::ivgui()->AddTickSignal( GetVPanel(), 100 );

	//vgui::TextEntry* m_pTime; // Panel class declaration, private section

	//m_pTime = new vgui::TextEntry(this, "MyTextEntry");
	//m_pTime->SetPos(15, 310);
	//m_pTime->SetSize(50, 20);

	// Button Label
	{
		/*vgui::Label *m_pBL1;   
		m_pBL1 = dynamic_cast<vgui::Label*>( FindChildByName( "BL1" ) ); 
		m_pBL1->SetPaintBorderEnabled(false);
		m_pBL1->SetPaintBackgroundEnabled(false);
		vgui::Label *m_pBL2;   
		m_pBL2 = dynamic_cast<vgui::Label*>( FindChildByName( "BL2" ) ); 
		m_pBL2->SetPaintBorderEnabled(false);
		m_pBL2->SetPaintBackgroundEnabled(false);
		vgui::Label *m_pBL3;   
		m_pBL3 = dynamic_cast<vgui::Label*>( FindChildByName( "BL3" ) ); 
		m_pBL3->SetPaintBorderEnabled(false);
		m_pBL3->SetPaintBackgroundEnabled(false);
		vgui::Label *m_pBL4;   
		m_pBL4 = dynamic_cast<vgui::Label*>( FindChildByName( "BL4" ) ); 
		m_pBL4->SetPaintBorderEnabled(false);
		m_pBL4->SetPaintBackgroundEnabled(false);*/

		/*vgui::Label *m_pBP1;   
		m_pBP1 = dynamic_cast<vgui::Label*>( FindChildByName( "BP1" ) ); 
		m_pBP1->SetPaintBorderEnabled(false);
		m_pBP1->SetPaintBackgroundEnabled(false);
		vgui::Label *m_pBP2;   
		m_pBP2 = dynamic_cast<vgui::Label*>( FindChildByName( "BP2" ) ); 
		m_pBP2->SetPaintBorderEnabled(false);
		m_pBP2->SetPaintBackgroundEnabled(false);
		vgui::Label *m_pBP3;   
		m_pBP3 = dynamic_cast<vgui::Label*>( FindChildByName( "BP3" ) ); 
		m_pBP3->SetPaintBorderEnabled(false);
		m_pBP3->SetPaintBackgroundEnabled(false);*/

		vgui::ScrollBar *m_pSroll = dynamic_cast<vgui::ScrollBar*>( FindChildByName( "Scrollbar" ) );
		if (m_pSroll)
		{
			m_pSroll->SetRange(1,8);
			m_pSroll->SetRangeWindow(1);
			m_pSroll->SetTabPosition(1);
			m_pSroll->SetButtonPressedScrollValue( 1 );
			m_pSroll->SetValue(1);
		}
	}

	DevMsg("CasePanel has been constructed\n");

	//m_TCase = vgui::surface()->CreateNewTextureID();
	//vgui::surface()->DrawSetTextureFile(m_TCase,"hud/case",true,false);

	vgui::HScheme scheme = vgui::scheme()->GetScheme( "ClientScheme" );
	hFont = vgui::scheme()->GetIScheme(scheme)->GetFont( "HudInfoSmall" );

	m_iScrollPos=-1;
}

//Class: CMyPanelInterface Class. Used for construction.
class CCasePanelInterface : public ICasePanel
{
private:
	CCasePanel *CasePanel;
public:
	CCasePanelInterface()
	{
		CasePanel = NULL;
	}
	void Create(vgui::VPANEL parent)
	{
		CasePanel = new CCasePanel(parent);
	}
	void Destroy()
	{
		if (CasePanel)
		{
			CasePanel->SetParent( (vgui::Panel *)NULL);
			delete CasePanel;
		}
	}
};
static CCasePanelInterface g_CasePanel;
ICasePanel* casepanel = (ICasePanel*)&g_CasePanel;

ConVar jl_showcasepanel("jl_showcasepanel", "0", FCVAR_CLIENTDLL, "Sets the state of casepanel : <state>");

void CCasePanel::OnTick()
{
	BaseClass::OnTick();

	if (jl_showcasepanel.GetBool()==false && IsVisible())
	{
		SetVisible(false);
	}
	if (jl_showcasepanel.GetBool() && !IsVisible())
	{
		SetVisible(true);
		Center();

		//UpdateCasePanel();
	}

	if (jl_showcasepanel.GetBool())
	{
		UpdateCasePanel();
		/*vgui::ScrollBar *scrollbar = dynamic_cast<vgui::ScrollBar*>( FindChildByName( "ScrollBar" ) );

		if (scrollbar && m_iScrollPos!=scrollbar->GetValue())
		{
			m_iScrollPos=scrollbar->GetValue();
			UpdateCasePanel();
		}*/
	}

	//m_pLabel->SetEnabled( true );
	//m_pLabel->SetSize(100,100);
	//int key = gameuifuncs->GetEngineKeyCodeForBind( "slot1" );
	//const char[] temp ;//= FindKey(key)->GetName();
	//m_pLabel->SetText(" Shotgun");
	
	//KeyValues *data = new KeyValues("CasePanel");
	//DevMsg("%d \n",data->GetInt("Flags"));
	//data->deleteThis();
}

CON_COMMAND(ToggleCasePanel, "Toggles CasePabel on or off")
{
	jl_showcasepanel.SetValue(!jl_showcasepanel.GetBool());
};

void CCasePanel::OnCommand(const char* pcCommand)
{
	for (int i=1; i<=8;i++)
	{
		char tempslot[6];
		Q_snprintf(tempslot,sizeof(tempslot),"slot%d",i);
		if(!Q_stricmp(pcCommand, tempslot))
		{
			char command[16];
			Q_snprintf(command,sizeof(command),"invtocase %d",i);			
			engine->ClientCmd(command);
			break;
		}
	}

	int val = (dynamic_cast<vgui::ScrollBar*>( FindChildByName( "Scrollbar" ) ))->GetValue();
	//casetoinv
	for (int i=1; i<=4;i++)
	{
		char tempslot[6];
		Q_snprintf(tempslot,sizeof(tempslot),"BL%d",i);
		if(!Q_stricmp(pcCommand, tempslot))
		{
			char command[16];
			Q_snprintf(command,sizeof(command),"casetoinv %d",7*(val-1)+i-1);	
			DevMsg("Case : %s\n",command);
			engine->ClientCmd(command);
			break;
		}
	}

	for (int i=1; i<=3;i++)
	{
		char tempslot[6];
		Q_snprintf(tempslot,sizeof(tempslot),"BP%d",i);
		if(!Q_stricmp(pcCommand, tempslot))
		{
				char command[16];
				Q_snprintf(command,sizeof(command),"casetoinv %d",7*(val-1)+4+i-1);			
				engine->ClientCmd(command);
				break;
		}
	}

	if(!Q_stricmp(pcCommand, "turnoff"))
		jl_showcasepanel.SetValue(0);

	BaseClass::OnCommand( pcCommand );
}

void CCasePanel::UpdateCasePanel()
{
	vgui::ScrollBar* scrollbar = dynamic_cast<vgui::ScrollBar*>( FindChildByName( "Scrollbar" ) );
	int val=0;
	if (scrollbar)
	{
		val = scrollbar->GetValue();
	}

	vgui::ButtonSlot *m_pSlot;
	char temp[6];
	for (int i=0;i<8;i++)
	{
		Q_snprintf(temp,sizeof(temp),"Slot%d",i+1);
		m_pSlot = dynamic_cast<vgui::ButtonSlot*>( FindChildByName( temp ) );

		if (m_pSlot)
		{
			m_pSlot->LinkSlot(i,true);
		}
	}

	for (int i=0;i<3;i++)
	{
		Q_snprintf(temp,sizeof(temp),"BP%d",i+1);
		m_pSlot = dynamic_cast<vgui::ButtonSlot*>( FindChildByName( temp ) );
		if (m_pSlot)
		{
			m_pSlot->LinkSlot(7*(val-1)+4+i,false);
		}		
	}

	for (int i=0;i<4;i++)
	{
		Q_snprintf(temp,sizeof(temp),"BL%d",i+1);
		m_pSlot = dynamic_cast<vgui::ButtonSlot*>( FindChildByName( temp ) );
		if (m_pSlot)
		{
			m_pSlot->LinkSlot(7*(val-1)+i,false);
		}		
	}
}

void CCasePanel::Paint( void )
{
	//Case
	/*surface()->DrawSetColor(255,255,255,255);
	surface()->DrawSetTexture(m_TCase);
	surface()->DrawTexturedRect(0,0,512,512);*/

	//Shotgun
	/*if (jl_caseflags.GetInt() & SF_CASE_SHOTGUN)
	{
		surface()->DrawSetColor(255,255,255,255);
		surface()->DrawSetTexture(m_TCaseShotgun);
		surface()->DrawTexturedRect(426,0,426+128,512);
	}

	if (jl_caseflags.GetInt() & SF_CASE_MEDUSA)
	{
		surface()->DrawSetColor(255,255,255,255);
		surface()->DrawSetTexture(m_TCaseMedusa);
		surface()->DrawTexturedRect(600,0,600+196,196);
	}*/
	// Draw Key link name
	/*{
		surface()->DrawSetTextFont( hFont );
		surface()->DrawSetTextColor(255,255,255,255);
		int key = gameuifuncs->GetEngineKeyCodeForBind( "slot1" );
		const char *temp = gameuifuncs->Key_NameForKey(key);
		swprintf(unicode, L"%s", temp);
		surface()->DrawSetTextPos( 175,105 );
		surface()->DrawPrintText( unicode, wcslen(unicode) ); // print text	
		key = gameuifuncs->GetEngineKeyCodeForBind( "slot2" );
		temp = gameuifuncs->Key_NameForKey(key);
		swprintf(unicode, L"%s", temp);
		surface()->DrawSetTextPos( 270,105 );
		surface()->DrawPrintText( unicode, wcslen(unicode) ); // print text	
		key = gameuifuncs->GetEngineKeyCodeForBind( "slot3" );
		temp = gameuifuncs->Key_NameForKey(key);
		swprintf(unicode, L"%s", temp);
		surface()->DrawSetTextPos( 365,105 );
		surface()->DrawPrintText( unicode, wcslen(unicode) ); // print text	
		key = gameuifuncs->GetEngineKeyCodeForBind( "slot4" );
		temp = gameuifuncs->Key_NameForKey(key);
		swprintf(unicode, L"%s", temp);
		surface()->DrawSetTextPos( 460,105 );
		surface()->DrawPrintText( unicode, wcslen(unicode) ); // print text	

		key = gameuifuncs->GetEngineKeyCodeForBind( "+reload" );
		temp = gameuifuncs->Key_NameForKey(key);
		swprintf(unicode, L"%s", temp);
		surface()->DrawSetTextPos( 616,175 );
		surface()->DrawPrintText( unicode, wcslen(unicode) ); // print text	
		key = gameuifuncs->GetEngineKeyCodeForBind( "+jump" );
		temp = gameuifuncs->Key_NameForKey(key);
		swprintf(unicode, L"%s", temp);
		surface()->DrawSetTextPos( 616,280);
		surface()->DrawPrintText( unicode, wcslen(unicode) ); // print text	
		key = gameuifuncs->GetEngineKeyCodeForBind( "+duck" );
		temp = gameuifuncs->Key_NameForKey(key);
		swprintf(unicode, L"%s", temp);
		surface()->DrawSetTextPos( 616,385 );
		surface()->DrawPrintText( unicode, wcslen(unicode) ); // print text	
	}*/

	//Draw the item name of player inventory
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();

	//Case items
	//int val = (dynamic_cast<vgui::ScrollBar*>( FindChildByName( "Scrollbar" ) ))->GetValue();
	//DevMsg("Scroll : %d\n",val);
	if (player)
	{
		/*vgui::Label *m_pSlot;
		char temp[6];

		for (int i=0;i<3;i++)
		{
			Q_snprintf(temp,sizeof(temp),"BP%d",i+1);
			m_pSlot = dynamic_cast<vgui::Label*>( FindChildByName( temp ) );
			if (player->GetCaseItem(7*(val-1)+4+i))
				m_pSlot->SetText(player->GetCaseItem(7*(val-1)+4+i)->GetClassname());
			else
				m_pSlot->SetText("");
			//DevMsg("CaseItem L: %d : %s\n",7*(val-1)+4+i,player->GetCaseItem(7*(val-1)+4+i));
		}

		for (int i=0;i<4;i++)
		{
			Q_snprintf(temp,sizeof(temp),"BL%d",i+1);
			m_pSlot = dynamic_cast<vgui::Label*>( FindChildByName( temp ) );
			if (player->GetCaseItem(7*(val-1)+i))
				m_pSlot->SetText(player->GetCaseItem(7*(val-1)+i)->GetClassname());
			else
				m_pSlot->SetText("");
			//DevMsg("CaseItem P: %d\n",7*(val-1)+i);
		}*/
	}
	
	//Inventory Item
	if (player)
	{
		/*vgui::Label *m_pSlot;
		char temp[6];
		for (int i=0;i<8;i++)
		{
			Q_snprintf(temp,sizeof(temp),"Slot%d",i+1);
			m_pSlot = dynamic_cast<vgui::Label*>( FindChildByName( temp ) );
			//m_pSlot->SetText(player->GetItem(i)->GetClassname());
			if (player->GetWeapon(i)!=NULL)
				m_pSlot->SetText(player->GetWeapon(i)->GetClassname());
			else
				m_pSlot->SetText("");
			
			if (player->GetWeapon(i))
			{
				const FileWeaponInfo_t &weaponInfo = player->GetWeapon(i)->GetWpnData();	
				wchar_t text[128];
				wchar_t *tempString = g_pVGuiLocalize->Find(weaponInfo.szPrintName);
				// setup our localized string
				if ( tempString )
				{
					_snwprintf(text, sizeof(text)/sizeof(wchar_t) - 1, L"%s", tempString);
					text[sizeof(text)/sizeof(wchar_t) - 1] = 0;
				}
				else
				{
					// string wasn't found by g_pVGuiLocalize->Find()
					g_pVGuiLocalize->ConvertANSIToUnicode(weaponInfo.szPrintName, text, sizeof(text));
				}								
			}			
		}*/
	}
}
void CCasePanel::OnKeyCodePressed(KeyCode code)
{
	//int lastPressedEngineKey = engine->GetLastPressedEngineKey();
	/*m_iSlot1 = gameuifuncs->GetEngineKeyCodeForBind( "slot1" );
	m_iSlot2 = gameuifuncs->GetEngineKeyCodeForBind( "slot2" );
	m_iSlot3 = gameuifuncs->GetEngineKeyCodeForBind( "slot3" );
	m_iSlot4 = gameuifuncs->GetEngineKeyCodeForBind( "slot4" );
	m_iSlot5 = gameuifuncs->GetEngineKeyCodeForBind( "slot5" );
	m_iSlot6 = gameuifuncs->GetEngineKeyCodeForBind( "slot6" );*/

	//m_iQuit = gameuifuncs->GetEngineKeyCodeForBind( "+use" );

	//char *temp = new char[1024];
	//Q_snprintf(temp, 1024, "updatecase %d %d %d", jl_caseid.GetInt(), 1, jl_caseid.GetInt());
	//engine->ServerCmd(temp);

	/*if( lastPressedEngineKey == m_iSlot1 &&  jl_caseflags.GetInt() & SF_CASE_SHOTGUN )
	{
		engine->ClientCmd("jl_case_getshotgun");
	}
	if( lastPressedEngineKey == m_iSlot2 &&  jl_caseflags.GetInt() & SF_CASE_MEDUSA )
	{
		engine->ClientCmd("jl_case_getmedusa");
	}
	else*/
	//if(lastPressedEngineKey == m_iQuit )
	if (input()->IsKeyDown(gameuifuncs->GetButtonCodeForBind("+use")))
	{
		jl_showcasepanel.SetValue(false);
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}

Panel *CCasePanel::CreateControlByName(const char *controlName)
{
  	/*if ( Q_stricmp( controlName, "ButtonSlot" ) == 0 )
    {
		DevMsg("Create ButtonSlot\n");
        return new ButtonSlot( this, controlName, controlName );
    }*/

    return BaseClass::CreateControlByName( controlName );
}