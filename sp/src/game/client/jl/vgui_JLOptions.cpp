#include "cbase.h"
#include "vgui_JLOptions.h"
// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"
 
GameUI<CJLOptions> g_JLOptionsPanel;
 
IGameUI* GetJLOptionsPanel()
{
	return &g_JLOptionsPanel;
}
  
CON_COMMAND(ToggleJLOptionsPanel,NULL)
{
	g_JLOptionsPanel.GetPanel()->SetVisible( !g_JLOptionsPanel.GetPanel()->IsVisible() );
	if ( g_JLOptionsPanel.GetPanel()->IsVisible() )
	{
		CJLOptions* pPanel = (CJLOptions*)g_JLOptionsPanel.GetPanel();
		pPanel->Activate();
		pPanel->Center();
	}
}
 
CJLOptions::CJLOptions( vgui::VPANEL parent ) : BaseClass( NULL, "JLOptions" )
{
 	SetParent(parent);
 	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme");
 	SetScheme( scheme );
 	LoadControlSettings("Resource/UI/JLOptions.res");
 	SetVisible(false);//made visible on command later 
 
	int x,w,h; 
	GetBounds(x,x,w,h);
	SetPos(ScreenWidth()-w,ScreenHeight()-h);

 	//Other useful options
 	SetSizeable(false);
 	SetMoveable(true);
	SetCloseButtonVisible(true);
	SetMenuButtonResponsive(false);
	SetMenuButtonVisible(false);
}

void CJLOptions::Center()
{
	int x,w,h; 
	GetBounds(x,x,w,h);
	SetPos(0.5f*(ScreenWidth()-w),0.5f*(ScreenHeight()-h));
}
