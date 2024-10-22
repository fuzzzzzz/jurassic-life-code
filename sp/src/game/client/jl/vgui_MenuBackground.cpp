//========= Copyright � 2006, Valde Productions, All rights reserved. ============//
//
// Purpose: Display Main Menu images, handles rollovers as well
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "vgui_MenuBackground.h"
#include "vgui_controls/Frame.h"
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/IInput.h>

#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Displays the logo panel
//-----------------------------------------------------------------------------
class CMenuBackground : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CMenuBackground, vgui::Frame);

public:
	CMenuBackground( vgui::VPANEL parent );
	~CMenuBackground();

	virtual void OnCommand(const char *command);

	void UpdateImageSize(vgui::ImagePanel *Img)
	{
		if (Img && Img->GetImage())
		{
			int Wide, Tall;
			Img->GetImage()->GetSize(Wide,Tall);
			Img->SetSize(Wide,Tall);
		}
	}

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme )
	{
		
		BaseClass::ApplySchemeSettings( pScheme );
	}

	// The panel background image should be square, not rounded.
	virtual void PaintBackground()
	{
		SetBgColor(Color(0,0,0,0));
		SetPaintBackgroundType( 0 );
		BaseClass::PaintBackground();
	}
	virtual void PerformLayout()
	{
		// re-position
		SetPos(vgui::scheme()->GetProportionalScaledValue(defaultX)-posX, vgui::scheme()->GetProportionalScaledValue(defaultY)-posY);

		BaseClass::PerformLayout();
		SetZPos(99);
	}
	void CMenuBackground::PerformDefaultLayout()
	{
		//m_pImgBackground->SetPos(0,0);
		m_pButtonBegin->SetPos(addX+0, addY+0);
		m_pImgBegin->SetPos(addX+0,addY+0);
		m_pButtonLoad->SetPos(addX+0, addY+40);
		m_pImgLoad->SetPos(addX+0,addY+40);
		m_pButtonOptions->SetPos(addX+0, addY+80);
		m_pImgOptions->SetPos(addX+0,addY+80);
		// game pad
		//m_pButtonController->SetPos(addX+0, addY+120);
		//m_pImgController->SetPos(addX+0,addY+120);
		
		// jl option panel
		m_pButtonJLOptions->SetPos(addX+0, addY+120);
		m_pImgJLOptions->SetPos(addX+0,addY+120);
		// achievements
		m_pButtonSuccess->SetPos(addX+0, addY+160);
		m_pImgSuccess->SetPos(addX+0,addY+160);
		// quit
		m_pButtonLeave->SetPos(addX+0, addY+200);
		m_pImgLeave->SetPos(addX+0,addY+200);
		// save game if ingame
		m_pImgSave->SetVisible(false);
		m_pButtonSave->SetVisible(false);
		// resume game ( if ingame)
		m_pImgResume->SetVisible(false);
		m_pButtonResume->SetVisible(false);

		InRolloverResume=false;
		InRolloverBegin=false;
		InRolloverLoad=false;
		InRolloverOptions=false;
		//InRolloverController=false;
		InRolloverJLOptions=false;
		InRolloverSuccess=false;
		InRolloverLeave=false;
	}

	virtual void OnThink()
	{
		// In-game, everything will be in different places than at the root menu!
		if (InGame() && !InGameLayout) {
			DevMsg("Performing menu layout\n");
			int wide, tall;
			surface()->GetScreenSize(wide,tall);
			int dy = 40; // delta y, shift value
			int dy2=0;
			int py=0;
			if (wide==640)
			{
				//dy=40;
				//dy2=0;
				py=-32;
			}

			int x,y;
			// Resume
			m_pButtonResume->SetPos(addX+0,addY+py);
			m_pImgResume->SetPos(addX+0,addY+0+py);
			m_pButtonResume->SetVisible(true);
			m_pImgResume->SetVisible(true);

			m_pButtonBegin->GetPos(x,y);
			m_pButtonBegin->SetPos(x,y+dy+py);
			m_pImgBegin->GetPos(x,y);
			m_pImgBegin->SetPos(x,y+dy+py);

			m_pButtonLoad->GetPos(x,y);
			m_pButtonLoad->SetPos(x,y+dy+py);
			m_pImgLoad->GetPos(x,y);
			m_pImgLoad->SetPos(x,y+dy+py);

			// Save game
			m_pButtonSave->SetPos(x,y+(2*dy)-dy2/2+py);
			m_pImgSave->SetPos(x,y+(2*dy)-dy2/2+py);
			m_pButtonSave->SetVisible(true);
			m_pImgSave->SetVisible(true);

			m_pButtonOptions->GetPos(x,y);
			m_pButtonOptions->SetPos(x,y+(2*dy)-dy2/2+py);
			m_pImgOptions->GetPos(x,y);
			m_pImgOptions->SetPos(x,y+(2*dy)-dy2/2+py); // Options moves under Save game, so twice as far

//			m_pButtonController->GetPos(x,y);
//			m_pButtonController->SetPos(x,y+(2*dy)-dy2/2+py);
//			m_pImgController->GetPos(x,y);
//			m_pImgController->SetPos(x,y+(2*dy)-dy2/2+py); // Options moves under Save game, so twice as far

			m_pButtonJLOptions->GetPos(x,y);
			m_pButtonJLOptions->SetPos(x,y+(2*dy)-dy2/2+py);
			m_pImgJLOptions->GetPos(x,y);
			m_pImgJLOptions->SetPos(x,y+(2*dy)-dy2/2+py); // Options moves under Save game, so twice as far

			m_pButtonSuccess->GetPos(x,y);
			m_pButtonSuccess->SetPos(x,y+(2*dy)-dy2/2+py);
			m_pImgSuccess->GetPos(x,y);
			m_pImgSuccess->SetPos(x,y+(2*dy)-dy2/2+py); // Options moves under Save game, so twice as far

			m_pButtonLeave->GetPos(x,y);
			m_pButtonLeave->SetPos(x,y+(2*dy)-dy2/2+py);
			m_pImgLeave->GetPos(x,y);
			m_pImgLeave->SetPos(x,y+(2*dy)-dy2/2+py); // Leave game moves under Save game, so twice as far

			InGameLayout = true;
		}
		if (!InGame() && InGameLayout)
		{
			PerformDefaultLayout();
			InGameLayout = false;
		}

		// Get mouse coords
		int x,y;
		vgui::input()->GetCursorPos(x,y);

		int fx,fy; // frame xpos, ypos

		GetPos(fx,fy);

		CheckRolloverBegin(x,y,fx,fy);
		CheckRolloverResume(x,y,fx,fy);
		CheckRolloverLoad(x,y,fx,fy);
		CheckRolloverSave(x,y,fx,fy);
		CheckRolloverOptions(x,y,fx,fy);
		//CheckRolloverController(x,y,fx,fy); //controller gamepad --red
		CheckRolloverJLOptions(x,y,fx,fy);
		CheckRolloverSuccess(x,y,fx,fy); //achievements success --red
		CheckRolloverLeave(x,y,fx,fy);
		
		BaseClass::OnThink();		
	}

	void CheckRolloverBegin(int x,int y, int fx, int fy)
	{
		int bx,by,bw,bh; // button xpos, ypos, width, height

		m_pButtonBegin->GetPos(bx,by);
		m_pButtonBegin->GetSize(bw,bh);

		bx = bx+fx; // xpos for button (rel to screen)
		by = by+fy; // ypos for button (rel to screen)

		if (m_pImgBegin==NULL)
			return;
		// Check and see if mouse cursor is within button bounds
		if ((x > bx && x < bx+bw) && (y > by && y < by+bh))
		{
			if(!InRolloverBegin) {
				m_pImgBegin->SetImage("menu_begin_over");
				UpdateImageSize(m_pImgBegin);
				InRolloverBegin = true;
			}
		} else {
			if(InRolloverBegin) {
				m_pImgBegin->SetImage("menu_begin");
				UpdateImageSize(m_pImgBegin);
				InRolloverBegin = false;
			}
		}
	}

	void CheckRolloverResume(int x,int y, int fx, int fy)
	{
		if(m_pButtonResume->IsVisible()) {
			int bx,by,bw,bh; // button xpos, ypos, width, height

			m_pButtonResume->GetPos(bx,by);
			m_pButtonResume->GetSize(bw,bh);

			bx = bx+fx; // xpos for button (rel to screen)
			by = by+fy; // ypos for button (rel to screen)

			// Check and see if mouse cursor is within button bounds
			if ((x > bx && x < bx+bw) && (y > by && y < by+bh))
			{
				if(!InRolloverResume) {
					m_pImgResume->SetImage("menu_Resume_over");
					UpdateImageSize(m_pImgResume);
					InRolloverResume = true;
				}
			} else {
				if(InRolloverResume) {
					m_pImgResume->SetImage("menu_Resume");
					UpdateImageSize(m_pImgResume);
					InRolloverResume = false;
				}
			}
		}
	}
	void CheckRolloverLoad(int x,int y, int fx, int fy)
	{
		int bx,by,bw,bh; // button xpos, ypos, width, height

		m_pButtonLoad->GetPos(bx,by);
		m_pButtonLoad->GetSize(bw,bh);

		bx = bx+fx; // xpos for button (rel to screen)
		by = by+fy; // ypos for button (rel to screen)

		// Check and see if mouse cursor is within button bounds
		if ((x > bx && x < bx+bw) && (y > by && y < by+bh))
		{
			if(!InRolloverLoad) {
				m_pImgLoad->SetImage("menu_load_over");
				UpdateImageSize(m_pImgLoad);
				InRolloverLoad = true;
			}
		} else {
			if(InRolloverLoad) {
				m_pImgLoad->SetImage("menu_load");
				UpdateImageSize(m_pImgLoad);
				InRolloverLoad = false;
			}
		}
	}
	void CheckRolloverSave(int x,int y, int fx, int fy)
	{
		if(m_pButtonSave->IsVisible()) {
			int bx,by,bw,bh; // button xpos, ypos, width, height

			m_pButtonSave->GetPos(bx,by);
			m_pButtonSave->GetSize(bw,bh);

			bx = bx+fx; // xpos for button (rel to screen)
			by = by+fy; // ypos for button (rel to screen)

			// Check and see if mouse cursor is within button bounds
			if ((x > bx && x < bx+bw) && (y > by && y < by+bh))
			{
				if(!InRolloverSave) {
					m_pImgSave->SetImage("menu_Save_over");
					UpdateImageSize(m_pImgSave);
					InRolloverSave = true;
				}
			} else {
				if(InRolloverSave) {
					m_pImgSave->SetImage("menu_Save");
					UpdateImageSize(m_pImgSave);
					InRolloverSave = false;
				}
			}
		}
	}
	void CheckRolloverOptions(int x,int y, int fx, int fy)
	{
		int bx,by,bw,bh; // button xpos, ypos, width, height

		m_pButtonOptions->GetPos(bx,by);
		m_pButtonOptions->GetSize(bw,bh);

		bx = bx+fx; // xpos for button (rel to screen)
		by = by+fy; // ypos for button (rel to screen)

		// Check and see if mouse cursor is within button bounds
		if ((x > bx && x < bx+bw) && (y > by && y < by+bh))
		{
			if(!InRolloverOptions) {
				m_pImgOptions->SetImage("menu_Options_over");
				UpdateImageSize(m_pImgOptions);
				InRolloverOptions = true;
			}
		} else {
			if(InRolloverOptions) {
				m_pImgOptions->SetImage("menu_Options");
				UpdateImageSize(m_pImgOptions);
				InRolloverOptions = false;
			}
		}
	}
/* Console only --red
void CheckRolloverController(int x,int y, int fx, int fy)
	{
		int bx,by,bw,bh; // button xpos, ypos, width, height

		m_pButtonController->GetPos(bx,by);
		m_pButtonController->GetSize(bw,bh);

		bx = bx+fx; // xpos for button (rel to screen)
		by = by+fy; // ypos for button (rel to screen)

		// Check and see if mouse cursor is within button bounds
		if ((x > bx && x < bx+bw) && (y > by && y < by+bh))
		{
			if(!InRolloverController) {
				m_pImgController->SetImage("menu_controller_over");
				UpdateImageSize(m_pImgController);
				InRolloverController = true;
			}
		} else {
			if(InRolloverController) {
				m_pImgController->SetImage("menu_controller");
				UpdateImageSize(m_pImgController);
				InRolloverController = false;
			}
		}
	}
*/
	void CheckRolloverJLOptions(int x,int y, int fx, int fy)
	{
		int bx,by,bw,bh; // button xpos, ypos, width, height

		m_pButtonJLOptions->GetPos(bx,by);
		m_pButtonJLOptions->GetSize(bw,bh);

		bx = bx+fx; // xpos for button (rel to screen)
		by = by+fy; // ypos for button (rel to screen)

		// Check and see if mouse cursor is within button bounds
		if ((x > bx && x < bx+bw) && (y > by && y < by+bh))
		{
			if(!InRolloverJLOptions) {
				m_pImgJLOptions->SetImage("menu_JL_Options_over");
				UpdateImageSize(m_pImgJLOptions);
				InRolloverJLOptions = true;
			}
		} else {
			if(InRolloverJLOptions) {
				m_pImgJLOptions->SetImage("menu_JL_Options");
				UpdateImageSize(m_pImgJLOptions);
				InRolloverJLOptions = false;
			}
		}
	}
	void CheckRolloverSuccess(int x,int y, int fx, int fy)
	{
		int bx,by,bw,bh; // button xpos, ypos, width, height

		m_pButtonSuccess->GetPos(bx,by);
		m_pButtonSuccess->GetSize(bw,bh);

		bx = bx+fx; // xpos for button (rel to screen)
		by = by+fy; // ypos for button (rel to screen)

		// Check and see if mouse cursor is within button bounds
		if ((x > bx && x < bx+bw) && (y > by && y < by+bh))
		{
			if(!InRolloverSuccess) {
				m_pImgSuccess->SetImage("menu_achievements_over");
				UpdateImageSize(m_pImgSuccess);
				InRolloverSuccess = true;
			}
		} else {
			if(InRolloverSuccess) {
				m_pImgSuccess->SetImage("menu_achievements");
				UpdateImageSize(m_pImgSuccess);
				InRolloverSuccess = false;
			}
		}
	}
	void CheckRolloverLeave(int x,int y, int fx, int fy)
	{
		int bx,by,bw,bh; // button xpos, ypos, width, height

		m_pButtonLeave->GetPos(bx,by);
		m_pButtonLeave->GetSize(bw,bh);

		bx = bx+fx; // xpos for button (rel to screen)
		by = by+fy; // ypos for button (rel to screen)

		// Check and see if mouse cursor is within button bounds
		if ((x > bx && x < bx+bw) && (y > by && y < by+bh))
		{
			if(!InRolloverLeave) {
				m_pImgLeave->SetImage("menu_Leave_over");
				UpdateImageSize(m_pImgLeave);
				InRolloverLeave = true;
			}
		} else {
			if(InRolloverLeave) {
				m_pImgLeave->SetImage("menu_Leave");
				UpdateImageSize(m_pImgLeave);
				InRolloverLeave = false;
			}
		}
	}
	bool CMenuBackground::InGame()
	{
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

		
		if(pPlayer && IsVisible() && !engine->IsLevelMainMenuBackground())
		{
			return true;
		} else {
			return false;
		}
	}

private:
	
	vgui::ImagePanel *m_pImgBackground;
	vgui::ImagePanel *m_pImgBegin;
	vgui::ImagePanel *m_pImgResume;
	vgui::ImagePanel *m_pImgLoad;
	vgui::ImagePanel *m_pImgSave;
	vgui::ImagePanel *m_pImgOptions;
	//vgui::ImagePanel *m_pImgController;
	vgui::ImagePanel *m_pImgJLOptions;
	vgui::ImagePanel *m_pImgSuccess;
	vgui::ImagePanel *m_pImgLeave;
	vgui::Button *m_pButtonBegin;
	vgui::Button *m_pButtonResume;
	vgui::Button *m_pButtonLoad;
	vgui::Button *m_pButtonSave;
	vgui::Button *m_pButtonOptions;
	//vgui::Button *m_pButtonController;
	vgui::Button *m_pButtonJLOptions;
	vgui::Button *m_pButtonSuccess;
	vgui::Button *m_pButtonLeave;

	int defaultX;
	int defaultY;
	int addX;
	int addY;
	int posX;
	int posY;
	bool InGameLayout;
	bool InRolloverBegin;
	bool InRolloverResume;
	bool InRolloverLoad;
	bool InRolloverSave;
	bool InRolloverOptions;
	//bool InRolloverController;
	bool InRolloverJLOptions;
	bool InRolloverSuccess;
	bool InRolloverLeave;
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMenuBackground::CMenuBackground( vgui::VPANEL parent ) : BaseClass( NULL, "CMenuBackground" )
{
	LoadControlSettings( "resource/UI/MenuBackground.res" ); // Optional, don't need this

	SetParent( parent );
	SetTitleBarVisible( false );
	SetMinimizeButtonVisible( false );
	SetMaximizeButtonVisible( false );
	SetCloseButtonVisible( false );
	SetSizeable( false );
	SetMoveable( false );
	SetProportional( true );
	SetVisible( true );
	SetKeyBoardInputEnabled( false );
	SetMouseInputEnabled( false );
	//ActivateBuildMode();
	SetScheme("MenuScheme.res");

	SetZPos(99999999);

    // These coords are relative to a 640x480 screen
    // Good to test in a 1024x768 resolution.
	//defaultX = 60; // x-coord for our position 60
	//defaultY = 240; // y-coord for our position 240
	//addX=41;
	//addY=125+20;
	//posX=0;
	//posY=-5;
	//px=21;
	//mx=-6;
	//py=0;
	//my=0;
	InGameLayout = false;

	int wide, tall;
	surface()->GetScreenSize(wide,tall);

	//switch(wide)
	//{
	//case 640:
	//	defaultX = 60;
	//	defaultY = 220;
	//	addX=41;
	//	addY=125+3;
	//	posX=41;
	//	posY=125;
	//	//px=21;
	//	//mx=-6;
	//	//py=0;
	//	//my=0;
	//	break;
	////case 800:
	////case 1024:
	//default:
	//	defaultX = 60;
	//	defaultY = 220;
	//	addX=41;
	//	addY=125+20;
	//	posX=41;
	//	posY=125-13;
	//	//px=21;
	//	//mx=-6;
	//	//py=0;
	//	//my=0;
	//	break;
	//}

	//defaultX = 76;
	//defaultY = 240;
	defaultX = 90;
	defaultY = 120;
	addX=45;
	addY=240-95+4;
	posX=45;
	posY=240-95;

	//this->GetParent()->GetChildCount()

	//80

	// Size of the panel
	SetSize(1024,1024);
	//SetZPos(-1); // we're behind everything

	surface()->MovePopupToBack(GetVPanel());

	m_pImgBackground = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Background"));
	//m_pImgBackground->SetImage("jl/background_fabricstreched");
	m_pImgBackground->SetImage("background_fabricstreched");
	m_pImgBackground->SetSize(128,64);
	//m_pImgBackground->SetPos(-125,-125);
	m_pImgBackground->SetPos(0,40);

	if (m_pImgBackground->GetImage())
	{
		int Wide, Tall;
		m_pImgBackground->GetImage()->GetSize(Wide,Tall);
		m_pImgBackground->SetSize(Wide,Tall);
	}	

	// Load invisible buttons
        // Initialize images
	m_pImgBegin = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Begin"));
	m_pImgResume = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Resume"));
	m_pImgLoad = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Load"));
	m_pImgSave = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Save"));
	m_pImgOptions = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Options"));
	//m_pImgController = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Controller")); // gamepad  --red
	m_pImgJLOptions = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "JLOptions"));
	m_pImgSuccess = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Achievements")); // achievements  --red
	m_pImgLeave = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "Leave"));

	// New game	
	m_pButtonBegin = vgui::SETUP_PANEL(new vgui::Button(this, "btnBegin", ""));	
	m_pButtonBegin->SetSize(256, 32);
	m_pButtonBegin->SetPaintBorderEnabled(false);
	m_pButtonBegin->SetPaintEnabled(true);
	m_pImgBegin->SetImage("menu_begin");
	UpdateImageSize(m_pImgBegin);

	// Resume
	m_pButtonResume = vgui::SETUP_PANEL(new vgui::Button(this, "btnResume", ""));	
	m_pButtonResume->SetSize(256, 32);
	m_pButtonResume->SetPaintBorderEnabled(false);
	m_pButtonResume->SetPaintEnabled(false);
	m_pImgResume->SetImage("menu_resume");
	UpdateImageSize(m_pImgResume);

	// Load
	m_pButtonLoad = vgui::SETUP_PANEL(new vgui::Button(this, "btnLoad", ""));
	m_pButtonLoad->SetSize(256, 32);
	m_pButtonLoad->SetPaintBorderEnabled(false);
	m_pButtonLoad->SetPaintEnabled(false);
	m_pImgLoad->SetImage("menu_load");
	UpdateImageSize(m_pImgLoad);

	// Save
	m_pButtonSave = vgui::SETUP_PANEL(new vgui::Button(this, "btnSave", ""));
	m_pButtonSave->SetSize(256, 32);
	m_pButtonSave->SetPaintBorderEnabled(false);
	m_pButtonSave->SetPaintEnabled(false);
	m_pImgSave->SetImage("menu_save");
	UpdateImageSize(m_pImgSave);

	// Options
	m_pButtonOptions = vgui::SETUP_PANEL(new vgui::Button(this, "btnOptions", ""));
	m_pButtonOptions->SetSize(256, 32);
	m_pButtonOptions->SetPaintBorderEnabled(false);
	m_pButtonOptions->SetPaintEnabled(false);
	m_pImgOptions->SetImage("menu_options");
	UpdateImageSize(m_pImgOptions);

	// Gamepad controller --red
	//m_pButtonController = vgui::SETUP_PANEL(new vgui::Button(this, "btnController", ""));
	//m_pButtonController->SetSize(256, 32);
	//m_pButtonController->SetPaintBorderEnabled(false);
	//m_pButtonController->SetPaintEnabled(false);
	//m_pImgController->SetImage("menu_controller");
	//UpdateImageSize(m_pImgController);

	// JL Options
	m_pButtonJLOptions = vgui::SETUP_PANEL(new vgui::Button(this, "btnJLOptions", ""));
	m_pButtonJLOptions->SetSize(256, 32);
	m_pButtonJLOptions->SetPaintBorderEnabled(false);
	m_pButtonJLOptions->SetPaintEnabled(false);
	m_pImgJLOptions->SetImage("menu_jl_options");
	UpdateImageSize(m_pImgJLOptions);

	// Achievements --red
	m_pButtonSuccess = vgui::SETUP_PANEL(new vgui::Button(this, "btnSuccess", ""));
	m_pButtonSuccess->SetSize(256, 32);
	m_pButtonSuccess->SetPaintBorderEnabled(false);
	m_pButtonSuccess->SetPaintEnabled(false);
	m_pImgSuccess->SetImage("menu_achievements");
	UpdateImageSize(m_pImgSuccess);

	// Leave
	m_pButtonLeave = vgui::SETUP_PANEL(new vgui::Button(this, "btnLeave", ""));
	m_pButtonLeave->SetSize(256, 32);
	m_pButtonLeave->SetPaintBorderEnabled(false);
	m_pButtonLeave->SetPaintEnabled(false);
	m_pImgLeave->SetImage("menu_leave");
	UpdateImageSize(m_pImgLeave);

	PerformDefaultLayout();
}

void CMenuBackground::OnCommand(const char *command)
{

	BaseClass::OnCommand(command);
}


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMenuBackground::~CMenuBackground()
{
}

// Class
// Change CSMenu to CModMenu if you want. Salient is the name of the source mod, 
// hence SMenu. If you change CSMenu, change ISMenu too where they all appear.
class CSMenu : public ISMenu
{
private:
	CMenuBackground *MenuBackground;
	vgui::VPANEL m_hParent;

public:
	CSMenu( void )
	{
		MenuBackground = NULL;
	}

	void Create( vgui::VPANEL parent )
	{
		// Create immediately
		MenuBackground = new CMenuBackground(parent);
	}

	void Destroy( void )
	{
		if ( MenuBackground )
		{
			MenuBackground->SetParent( (vgui::Panel *)NULL );
			delete MenuBackground;
		}
	}

	void MoveOnTop()
	{
		if ( MenuBackground )
		{
			surface()->MovePopupToBack(MenuBackground->GetVPanel());
		}
	}

};

static CSMenu g_SBackground;
ISMenu *SMenu = ( ISMenu * )&g_SBackground;