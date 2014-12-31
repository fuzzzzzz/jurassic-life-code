//The following include files are necessary to allow your MyPanel.cpp to compile.
#include "cbase.h"
#include "IInventory.h"
#include "ButtonSlot.h"

using namespace vgui;

#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/imagepanel.h>
#include <vgui/ISurface.h>
#include <vgui/IInput.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Scrollbar.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/ImageList.h>
#include "vgui/ILocalize.h"
#include "c_basehlplayer.h"
#include "c_baseentity.h"
#include <game/client/iviewport.h>
#include "view_shared.h"
#include "view.h"
#include "ivrenderview.h"
#include "model_types.h"
#include "IGameUIFuncs.h" // for key bindings 
#include "in_buttons.h"
#include "ivrenderview.h"
#include "r_efx.h"
#include "dlight.h"
//#include "c_basecombatcharacter.h"
#include "tier0/memdbgon.h"
//struct model_t;
//#include "math_base.h"

extern IGameUIFuncs *gameuifuncs; // for key binding details 

CUtlVector<CInventory*> g_ClassInventory;

int extx, exty, extwidth, extheight;

extern IVEfx *effects;

//CInventory class: 

CInventory::~CInventory()
{
	g_ClassInventory.FindAndRemove( this );
	if (m_pImageList)
	{
		delete m_pImageList;
	}
};		

// Constuctor: Initializes the Panel
CInventory::CInventory(vgui::VPANEL parent): BaseClass(NULL, "Inventory")
{
	//PrecacheMaterial(
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
	SetPaintBorderEnabled( false );
	SetPaintBackgroundEnabled( false );
	
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));

	LoadControlSettings("resource/UI/Inventory.res");

	vgui::ivgui()->AddTickSignal( GetVPanel(), 100 );

	m_i3Dx = extx;
	m_i3Dy = exty;
	m_i3Dwidth = extwidth;
	m_i3Dheight = extheight;

	m_bZoomMore=false;
	m_bZoomLess=false;
	m_bMoveUp=false;
	m_bMoveDown=false;
	m_bMoveLeft=false;
	m_bMoveRight=false;

	//m_TCase = vgui::surface()->CreateNewTextureID();
	//vgui::surface()->DrawSetTextureFile(m_TCase,"hud/case",true,false);

	PrecacheMaterial("hud/Inventory");
	Texture = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(Texture,"hud/Inventory",true,false);
	
	
	DevMsg("Inventory has been constructed\n");

	g_ClassInventory.AddToTail( this );

	//vgui::Label *test = dynamic_cast<vgui::Label*>( FindChildByName( "ScrollBar" ) );
	//test->SetRange(0,100);
	//test->setr
	//test->

	/*vgui::Label *test = new vgui::Button(this,"Test","test");
	test->SetPos(10,10);
	test->SetSize(100,50);*/
	/*list = new vgui::z(this,"NoteList");
	//list = dynamic_cast<vgui::ListPanel*>( FindChildByName( "NoteList" ) );
	if (list)
	{
		list->SetPos(207,52);
		list->SetSize(450,285);
		list->SetPaintBorderEnabled(false);
		list->SetPaintBackgroundEnabled(false);
		//list->setp
		list->AddColumnHeader(0,"Title","",200,0,200);
		list->AddColumnHeader(1,"Texture","",10,0,200);
		list->AddColumnHeader(2,"Message","",10,0,200);
		list->SetColumnVisible(1,false);
		list->SetColumnVisible(2,false);
		list->SetFgColor(Color(255,0,0));

		//list->SetColumnHeaderHeight(0);
		//list->SetColumnVisible(0,false);

		//list->SetDragEnabled(true);
		//list->SetDropEnabled(true);


		KeyValues *Data = new KeyValues("data");
		Data->SetString("Title","Premier titre");
		Data->SetString("Texture","texture du message");
		Data->SetString("Message","premier message");
		
		list->AddItem(Data,0,0,0);
		Data->SetString("Title","Deuxiéme titre");
		list->AddItem(Data,0,0,0);
		Data->deleteThis();
		list->SetVisible(false);
	}*/
	m_bIsInNoteTab=false;

	m_pImageList = new vgui::ImageList(false);
	if (m_pImageList)
	{
		m_pImageList->AddImage(vgui::scheme()->GetImage("NoteUnread",false));
		m_pImageList->AddImage(vgui::scheme()->GetImage("NoteRead",false));
	}

	vgui::ListPanel *list = dynamic_cast<vgui::ListPanel*>( FindChildByName( "NoteList" ) );
	if (list)
	{
		//list->SetPaintBorderEnabled(false);
		//list->SetPaintBackgroundEnabled(false);		
		//g_pVGuiLocalize->GetNameByIndex(g_pVGuiLocalize->FindIndex("JL_Note"))

		//list->AddColumnToSection( m_iSectionId, "avatar", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_RIGHT, m_iAvatarWidth );
		list->AddColumnHeader(0,"Read","",20,20,20,ListPanel::COLUMN_IMAGE);
		list->AddColumnHeader(1,"Title","#JL_Note",180);
		//list->AddColumnHeader(1,"Message","",10,0,200);
		//list->AddColumnHeader(2,"Texture","",10,0,200);
		//list->SetColumnVisible(1,false);
		//list->SetColumnVisible(2,false);

		list->SetMultiselectEnabled(false);
		list->SetColumnSortable(0,false);

		list->SetImageList(m_pImageList,false);
	}

	//SetDragEnabled(true);
	/*SetDropEnabled(true);

	vgui::Label* m_pSlot;
	char temp[6];
	for (int i=0;i<8;i++)
	{
		Q_snprintf(temp,sizeof(temp),"Slot%d",i+1);
		m_pSlot = dynamic_cast<vgui::Label*>( FindChildByName( temp ) );
		
		m_pSlot->SetDragEnabled(true);
		m_pSlot->SetDropEnabled(true);
	}*/

	//PropertySheet* sheet = new PropertySheet();
	pAnimModel = NULL;
	m_p3DModelZone = dynamic_cast<CClass3DModelZone*>( FindChildByName( "3DModel" ) );
}

//Class: CMyPanelInterface Class. Used for construction.
class CInventoryInterface : public IInventory
{
private:
	CInventory *Inventory;
public:
	CInventoryInterface()
	{
		Inventory = NULL;
	}
	void Create(vgui::VPANEL parent)
	{
		Inventory = new CInventory(parent);
	}
	void Destroy()
	{
		if (Inventory)
		{
			Inventory->SetParent( (vgui::Panel *)NULL);
			delete Inventory;
		}
	}

	CInventory* GetInventory()
	{
		return Inventory;
	}
};
static CInventoryInterface g_Inventory;
IInventory* inventory = (IInventory*)&g_Inventory;

ConVar jl_showinventory("jl_showinventory", "0", FCVAR_CLIENTDLL, "Sets the state of Inventory <state>");
extern ConVar jl_post_blur_fade_enable;

void CInventory::OnTick()
{
	BaseClass::OnTick();

	if (jl_showinventory.GetBool()==false && IsVisible())
	{
		// disable -red
		//jl_post_blur_fade_enable.SetValue(0);
		SetVisible(false);
		if ( pAnimModel )
		{
			pAnimModel->Remove();
			pAnimModel = NULL;
		}
	}
	if (jl_showinventory.GetBool() && !IsVisible())
	{
		//jl_post_blur_fade_enable.SetValue(1);
		if ( pAnimModel )
		{
			pAnimModel->Remove();
			pAnimModel = NULL;
		}
		selecteditem = -1;
		vgui::Label *m_pInfo;
		m_pInfo = dynamic_cast<vgui::Label*>( FindChildByName( "Info" ) );		
		wchar_t *tempString = g_pVGuiLocalize->Find("JL_Info_Null");
		char buffer[4096];
		buffer[0]=0;
		g_pVGuiLocalize->ConvertUnicodeToANSI(tempString,buffer,4096);
		m_pInfo->SetText(buffer);
		ShowItem();

		SetVisible(true);

		//Center position
		Center();

		int iX,iY;
		GetPos(iX,iY);
		m_i3Dx = extx+iX;
		m_i3Dy = exty+iY;

		//UpdateInventory();
	}

	if (jl_showinventory.GetBool() /*&& HasFocus()*/ && !engine->IsPaused())
	{
		UpdateInventory();

		//if (input()->WasKeyPressed(gameuifuncs->GetButtonCodeForBind("inventory"))/* && code!=BUTTON_CODE_NONE*/)
		//if (input()->IsKeyDown(gameuifuncs->GetButtonCodeForBind("inventory"))/* && code!=BUTTON_CODE_NONE*/)
		if (input()->WasKeyTyped(gameuifuncs->GetButtonCodeForBind("inventory")))
			jl_showinventory.SetValue(false);

		if (input()->IsKeyDown(gameuifuncs->GetButtonCodeForBind("+reload")) || m_bZoomMore)
			m_fZoom--;

		if (input()->IsKeyDown(gameuifuncs->GetButtonCodeForBind("+use")) || m_bZoomLess)
			m_fZoom++;

		if (input()->IsKeyDown(gameuifuncs->GetButtonCodeForBind("moveright")) || m_bMoveRight)
			m_fy++;

		if (input()->IsKeyDown(gameuifuncs->GetButtonCodeForBind("moveleft")) || m_bMoveLeft)
			m_fy--;

		if (input()->IsKeyDown(gameuifuncs->GetButtonCodeForBind("forward")) || m_bMoveUp)
			m_fx++;

		if (input()->IsKeyDown(gameuifuncs->GetButtonCodeForBind("back")) || m_bMoveDown)
			m_fx--;

		if (input()->IsKeyDown(gameuifuncs->GetButtonCodeForBind("walk")) || m_bMoveUp)
			m_fz++;

		if (input()->IsKeyDown(gameuifuncs->GetButtonCodeForBind("duck")) || m_bMoveDown)
			m_fz--;		

		if (input()->IsKeyDown(gameuifuncs->GetButtonCodeForBind("+lookup")))
			m_fTop+=0.1;

		if (input()->IsKeyDown(gameuifuncs->GetButtonCodeForBind("+lookdown")))
			m_fTop-=0.1;

		if (input()->IsKeyDown(gameuifuncs->GetButtonCodeForBind("+right")))
			m_fLeft+=0.25;

		if (input()->IsKeyDown(gameuifuncs->GetButtonCodeForBind("+left")))
			m_fLeft-=0.25;		

		if (m_fZoom>100)
			m_fZoom=100;
		
		if (m_fZoom<10)
			m_fZoom=10;
	}
}

CON_COMMAND(ToggleInventory, "Toggles CasePanel on or off")
{
	jl_showinventory.SetValue(!jl_showinventory.GetBool());
	if (jl_showinventory.GetBool())
	{
		//GetVPanel()
		if (g_Inventory.GetInventory())
			vgui::ivgui()->AddTickSignal( g_Inventory.GetInventory()->GetVPanel(), 10 );
	}else{
		if (g_Inventory.GetInventory())
			vgui::ivgui()->AddTickSignal( g_Inventory.GetInventory()->GetVPanel(), 250 );
	}
};

CON_COMMAND(_JLShowWeaponPreviewPosition, "show inventory weapon preview position")
{
	if (g_Inventory.GetInventory())
	{
		g_Inventory.GetInventory()->PrintPosition();
	}
};

void CInventory::OnCommand(const char* pcCommand)
{
	if(!Q_stricmp(pcCommand, "drop"))
	{
		if (selecteditem !=-1)
		{
			C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
			DevMsg("Weapon Drop : %s\n",player->GetWeapon(selecteditem));
			if (Q_stricmp(player->GetWeapon(selecteditem-1)->GetClassname(),"weapon_flashlight"))
			{
				char tempcmd[16];
				Q_snprintf(tempcmd,sizeof(tempcmd),"dropitem %d",selecteditem);				
				engine->ClientCmd(tempcmd); // Drop item

				selecteditem = -1;
			}
		}
	}

	
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();

	char tempslot[6];
		
	for (int i=1;i<=8;i++)
	{
		Q_snprintf(tempslot,sizeof(tempslot),"slot%d",i);
		if(!Q_stricmp(pcCommand, tempslot))
		{
			if (player->GetWeapon(i-1))
			{
				if ( selecteditem == i )
					UseItem(i);

				selecteditem = i;

				const FileWeaponInfo_t &weaponInfo = player->GetWeapon(i-1)->GetWpnData();
				if ( pAnimModel )
					pAnimModel->Remove();
				
				m_fZoom=weaponInfo.fPosZoom;
				m_fLeft=weaponInfo.fPosLeft;
				m_fTop=weaponInfo.fPosTop;
				m_fx=weaponInfo.fPosX;
				m_fy=weaponInfo.fPosY;
				m_fz=weaponInfo.fPosZ;

				pAnimModel = new C_BaseAnimatingOverlay;
				//if (weaponInfo)
					pAnimModel->InitializeAsClientEntity( weaponInfo.szWorldModel, RENDER_GROUP_OPAQUE_ENTITY );
				pAnimModel->AddEffects( EF_NODRAW ); // don't let the renderer draw the model normally

				const char *item;
				char temp[32];
				char temp2[32];
				strcpy(temp,player->GetWeapon(i-1)->GetClassname());
				item = nexttoken( temp2, temp, '_' );
				char info[32];
				Q_snprintf(info,sizeof(info),"JL_%s_Info",item );	
				vgui::Label *m_pInfo;
				m_pInfo = dynamic_cast<vgui::Label*>( FindChildByName( "Info" ) );
				if (m_pInfo)
				{
					wchar_t *tempString = g_pVGuiLocalize->Find(info);
					DevMsg("Info %s %s\n",info,tempString);
					m_pInfo->SetText(tempString);
				}
			}
		}
	}

	if(!Q_stricmp(pcCommand, "turnoff"))
	{
		jl_showinventory.SetValue(0);
	}

	if(!Q_stricmp(pcCommand, "+zoommore"))
	{
		m_bZoomMore=true;
	}else if(!Q_stricmp(pcCommand, "-zoommore"))
	{
		m_bZoomMore=false;
	}
	if(!Q_stricmp(pcCommand, "+zoomless"))
	{
		m_bZoomLess=true;
	}else if(!Q_stricmp(pcCommand, "-zoomless"))
	{
		m_bZoomLess=false;
	}
	if(!Q_stricmp(pcCommand, "+moveup"))
	{
		m_bMoveUp=true;
	}else if(!Q_stricmp(pcCommand, "-moveup"))
	{
		m_bMoveUp=false;
	}
	if(!Q_stricmp(pcCommand, "+movedown"))
	{
		m_bMoveDown=true;
	}else if(!Q_stricmp(pcCommand, "-movedown"))
	{
		m_bMoveDown=false;
	}
	if(!Q_stricmp(pcCommand, "+moveleft"))
	{
		m_bMoveLeft=true;
	}else if(!Q_stricmp(pcCommand, "-moveleft"))
	{
		m_bMoveLeft=false;
	}
	if(!Q_stricmp(pcCommand, "+moveright"))
	{
		m_bMoveRight=true;
	}else if(!Q_stricmp(pcCommand, "-moveright"))
	{
		m_bMoveRight=false;
	}

	if(!Q_stricmp(pcCommand, "showItem"))
	{
		ShowItem();
	}

	if(!Q_stricmp(pcCommand, "showNote"))
	{
		ShowNote();
	}

}

void CInventory::Paint( void )
{
	/*surface()->DrawSetColor(255,255,255,255);
	surface()->DrawSetTexture(Texture);
	surface()->DrawTexturedRect(0,0,1024,512);*/

	//surface()->DrawSetColor(0,96,0,64);
	//surface()->DrawFilledRect( 205,52,205+453,52+286);

	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if (player==NULL)
		return;

	/*vgui::Label *m_pSlot;
	char temp[6];
	
	for (int i=0;i<8;i++)
	{
		Q_snprintf(temp,sizeof(temp),"Slot%d",i+1);
		m_pSlot = dynamic_cast<vgui::Label*>( FindChildByName( temp ) );

		if (player->GetWeapon(i))
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
	}

	if (selecteditem!=-1)
	{
		char tempslot[6];
		Q_snprintf(tempslot,sizeof(tempslot),"slot%d",selecteditem);
		vgui::Label *m_pSlot;
		m_pSlot = dynamic_cast<vgui::Label*>( FindChildByName( tempslot ) );
		int x, y, width, height;
		m_pSlot->GetSize(width, height);
		m_pSlot->GetPos(x, y);
		surface()->DrawSetColor(255,0,0,255);
		surface()->DrawOutlinedRect(x,y,x+width,y+height);
		surface()->DrawOutlinedRect(x+1,y+1,x+width+1,y+height+1);
		surface()->DrawOutlinedRect(x-1,y-1,x+width-1,y+height-1);
	}*/
}

void CInventory::UpdateInventory()
{
	//C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
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
}

//----------------------------------------------
//Post VGUI Render
//----------------------------------------------
void CInventory::PostRenderVGui()
{
	//DevMsg("Post Renderer VGUI\n");
	// Draw the model
	bool visible=true;
	if (m_p3DModelZone)
	{
		visible = m_p3DModelZone->IsVisible();
	}

	if (pAnimModel && visible)
	{
		C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
		trace_t tr;
		Vector vecDir, vecForward, vecRight, vecUp;

		// get the angles
		//AngleVectors( player->EyeAngles(), &vecDir );
		AngleVectors( player->EyeAngles(), &vecForward, &vecRight, &vecUp );

		//DevMsg( "Dir %f %f %f\n",vecDir.x,vecDir.y,vecDir.z);
		Vector origin = player->EyePosition();

			Vector lightOrigin = origin;
		    // find a spot inside the world for the dlight's origin, or it won't illuminate the model
			Vector testPos( origin.x - 100, origin.y, origin.z + 100 );
			//trace_t tr;
			UTIL_TraceLine( origin, testPos, MASK_OPAQUE, player, COLLISION_GROUP_NONE, &tr );
			if ( tr.fraction == 1.0f )
			{
				lightOrigin = tr.endpos;
			}
			else
			{
				// Now move the model away so we get the correct illumination
				lightOrigin = tr.endpos + Vector( 1, 0, -1 );	// pull out from the solid
				Vector start = lightOrigin;
				Vector end = lightOrigin + Vector( 100, 0, -100 );
				UTIL_TraceLine( start, end, MASK_OPAQUE, player, COLLISION_GROUP_NONE, &tr );
				origin = tr.endpos;
			}

			float ambient = engine->GetLightForPoint( origin, true ).Length();

			// Make a light so the model is well lit.
			// use a non-zero number so we cannibalize ourselves next frame
			dlight_t *dl = effects->CL_AllocDlight( LIGHT_INDEX_TE_DYNAMIC+1 );

			dl->flags = DLIGHT_NO_WORLD_ILLUMINATION;
			dl->origin = lightOrigin;
			// Go away immediately so it doesn't light the world too.
			dl->die = gpGlobals->curtime + 0.1f;

			dl->color.r = dl->color.g = dl->color.b = 250;
			if ( ambient < 1.0f )
			{
				dl->color.exponent = 1 + (1 - ambient) * 2;
			}
			dl->radius	= 400;
		
		pAnimModel->SetAbsOrigin( origin + vecForward*m_fZoom + vecRight*m_fLeft + vecUp*m_fTop );
		//DevMsg("Top : %d\n",m_fTop);
		//pAnimModel->SetModelWidthScale(5);
		
		pAnimModel->SetAbsAngles(  player->EyeAngles() + QAngle(m_fx,m_fy,m_fz) );//m_fx,y,0.

		Quaternion quat1,quat2,quat3;
		QAngle angle;
		AngleQuaternion(player->EyeAngles(),quat1);
		AngleQuaternion(QAngle(m_fx,m_fy,m_fz),quat2);
		QuaternionMult(quat1,quat2,quat3);
		QuaternionAngles(quat3,angle);

		pAnimModel->SetAbsAngles(angle);
		
		CViewSetup view;
		// setup the views location, size and fov (amongst others)
		view.x = m_i3Dx;
		view.y = m_i3Dy;
		view.width = m_i3Dwidth;
		view.height = m_i3Dheight;

		view.m_bOrtho = false;
		view.fov = 54;

		view.origin = origin ;
		
		// make sure that we see all of the player model
		Vector vMins, vMaxs;
		pAnimModel->C_BaseAnimating::GetRenderBounds( vMins, vMaxs );
		view.origin.z += ( vMins.z + vMaxs.z ) * 0.55f;

		view.angles.Init();
		//view.m_vUnreflectedOrigin = view.origin;
		view.zNear = VIEW_NEARZ;
		view.zFar = 1000;
		
		view.angles = player->EyeAngles();

		// render it out to the new CViewSetup area
		// it's possible that ViewSetup3D will be replaced in future code releases
		Frustum dummyFrustum;
		//VPlane dummyFrustum;
		
		//render->
		//render->ViewSetup3D( &view, 0, dummyFrustum );
		//render->SetMainView(view.origin,view.angles);
		
		//render->Push3DView( view, 16384, /*false,*/ NULL, dummyFrustum ); //DF_CLEARCOLOR=16384
		render->Push3DView( view, 0, NULL, dummyFrustum ); //DF_CLEARCOLOR=16384
		//render->Push3DView( view, VIEW_CLEAR_DEPTH | VIEW_CLEAR_COLOR, NULL, dummyFrustum );

			pAnimModel->DrawModel( STUDIO_RENDER );	
			
		render->PopView( dummyFrustum ); 
	}
}


Panel *CInventory::CreateControlByName(const char *controlName)
{
    if ( Q_stricmp( controlName, "3DModel" ) == 0 )
    {
		DevMsg("Create 3D Model\n");
        return new CClass3DModelZone( NULL, controlName );
    }

	/*if ( Q_stricmp( controlName, "ButtonSlot" ) == 0 )
    {
		DevMsg("Create ButtonSlot\n");
        return new ButtonSlot( this, controlName, controlName );
    }*/	

    return BaseClass::CreateControlByName( controlName );
}

CClass3DModelZone::CClass3DModelZone( vgui::Panel *pParent, const char *pName )
    : vgui::ImagePanel( pParent, pName )
{
}

CClass3DModelZone::~CClass3DModelZone()
{
}

void CClass3DModelZone::ApplySettings( KeyValues *inResourceData )
{
	extx = inResourceData->GetInt("xpos");
	exty = inResourceData->GetInt("ypos");
	extwidth = inResourceData->GetInt("wide");
	extheight = inResourceData->GetInt("tall");

    BaseClass::ApplySettings( inResourceData );
}

void CClass3DModelZone::Paint()
{
    BaseClass::Paint();
	surface()->DrawFilledRect(0,0,GetWide(), GetTall());
}

void CInventory::UseItem(int i)
{
	DevMsg("Use Item : %d\n",i);
	char tempslot[6];
	Q_snprintf(tempslot,sizeof(tempslot),"slot%d",i);			
	char temp[32];
	char temp2[32];
	char type[8];
	vgui::Label *m_pSlot;
	m_pSlot = dynamic_cast<vgui::Label*>( FindChildByName( tempslot ) );
	//DevMsg("Slot : %s \n",tempslot);
	m_pSlot->GetText(temp,sizeof(temp));
	nexttoken( type, temp, '_' );
	//DevMsg("Selected : %s\n",type);

	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if (player->GetWeapon(i-1)->GetWpnData().bIsItem==true)
	{
		//DevMsg("Use Item\n");
		/*C_BaseCombatWeapon *weapon = player->GetWeapon(i-1);
		weapon->PrimaryAttack();*/
		if ( pAnimModel )
		{
			pAnimModel->Remove();
			pAnimModel = NULL;
		}
		Q_snprintf(temp2,sizeof(temp2),"primaryattack %d",i);				
		engine->ClientCmd(temp2);
	}else{
		//DevMsg("Is Not Item\n");
		//DevMsg("Weapon Switch : %s\n",temp);
		Q_snprintf(temp2,sizeof(temp2),"switchweapon %d",i);				
		engine->ClientCmd(temp2);			
	}
}

void CInventory::ShowItem()
{
	vgui::Panel *infoPanel = FindChildByName( "Info" );
	vgui::Panel *modelPanel = FindChildByName( "3DModel" );
	vgui::Panel *noteListPanel = FindChildByName( "NoteList" );
	vgui::Panel *noteTextPanel = FindChildByName( "NoteText" );
	if (infoPanel)
	{
		infoPanel->SetVisible(true);
	}
	if (modelPanel)
	{
		modelPanel->SetVisible(true);
	}
	if (noteListPanel)
	{
		noteListPanel->SetVisible(false);
	}
	if (noteTextPanel)
	{
		noteTextPanel->SetVisible(false);
	}

	return;
	vgui::Label *m_pSlot=NULL;
	char temp[6];
	
	//Show Item Case
	for (int i=0;i<8;i++)
	{
		Q_snprintf(temp,sizeof(temp),"Slot%d",i+1);
		m_pSlot = dynamic_cast<vgui::Label*>( FindChildByName( temp ) );
		if (m_pSlot)
			m_pSlot->SetVisible(true);
	}

	if (list)
		list->SetVisible(false);

	m_pSlot = dynamic_cast<vgui::Label*>( FindChildByName( "Info" ) );		
	wchar_t *tempString = g_pVGuiLocalize->Find("JL_Info_Null");
	if (m_pSlot)
		m_pSlot->SetText(tempString);

	if (selecteditem==-1)
	{
		vgui::Label *m_pInfo;
		m_pInfo = dynamic_cast<vgui::Label*>( FindChildByName( "Info" ) );		
		wchar_t *tempString = g_pVGuiLocalize->Find("JL_Info_Null");
		m_pInfo->SetText(tempString);
	}
	m_bIsInNoteTab=false;	
}

void CInventory::ShowNote()
{
	UpdateNotes();
	vgui::Panel *infoPanel = FindChildByName( "Info" );
	vgui::Panel *modelPanel = FindChildByName( "3DModel" );
	vgui::Panel *noteListPanel = FindChildByName( "NoteList" );
	vgui::Panel *noteTextPanel = FindChildByName( "NoteText" );
	if (infoPanel)
	{
		infoPanel->SetVisible(false);
	}
	if (modelPanel)
	{
		modelPanel->SetVisible(false);
	}
	if (noteListPanel)
	{
		noteListPanel->SetVisible(true);
	}
	if (noteTextPanel)
	{
		noteTextPanel->SetVisible(true);
	}

	return;
	vgui::Label *m_pSlot=NULL;
	char temp[6];
	
	//Show Item Case
	for (int i=0;i<8;i++)
	{
		Q_snprintf(temp,sizeof(temp),"Slot%d",i+1);
		m_pSlot = dynamic_cast<vgui::Label*>( FindChildByName( temp ) );
		if (m_pSlot)
			m_pSlot->SetVisible(false);
	}
	
	if (list)
		list->SetVisible(true);

	m_pSlot = dynamic_cast<vgui::Label*>( FindChildByName( "Info" ) );
	if (m_pSlot)
		m_pSlot->SetText("");

	selecteditem = -1;
	if ( pAnimModel )
	{
		pAnimModel->Remove();
		pAnimModel = NULL;
	}
	m_bIsInNoteTab=true;
}

void CInventory::ShowMap()
{
	vgui::Panel *infoPanel = FindChildByName( "Info" );
	vgui::Panel *modelPanel = FindChildByName( "3DModel" );
	vgui::Panel *noteListPanel = FindChildByName( "NoteList" );
	vgui::Panel *noteTextPanel = FindChildByName( "NoteText" );
	if (infoPanel)
	{
		infoPanel->SetVisible(false);
	}
	if (modelPanel)
	{
		modelPanel->SetVisible(false);
	}
	if (noteListPanel)
	{
		noteListPanel->SetVisible(false);
	}
	if (noteTextPanel)
	{
		noteTextPanel->SetVisible(false);
	}
}

void CInventory::UpdateNotes()
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	//list->DeleteAllItems();
	//player->
	vgui::ListPanel *list = dynamic_cast<vgui::ListPanel*>( FindChildByName( "NoteList" ) );
	vgui::Label *note = dynamic_cast<vgui::Label*>( FindChildByName( "NoteText" ) );
	if (list)
	{
		DevMsg("Client note count %d\n",player->GetNoteCount());
		list->DeleteAllItems();
		if (player)
		{
			char bufferTitle[2048];
			char bufferText[2048];			
			//for (int i=1;i<=player->GetNoteCount();i++)
			for (int i=player->GetNoteCount();i>0;i--)
			{
				bufferTitle[0]=0;
				bufferText[0]=0;
				g_pVGuiLocalize->ConvertUnicodeToANSI(player->GetNoteTitle(i),bufferTitle,2048);
				g_pVGuiLocalize->ConvertUnicodeToANSI(player->GetNote(i),bufferText,2048);
				DevMsg("Note id %d\n",player->GetNoteId(i));
				KeyValues *Data = new KeyValues("data");
				Data->SetInt("Read",player->GetNoteRead(i)+1);
				Data->SetString("Title",bufferTitle);
				Data->SetString("Message",bufferText);
				Data->SetInt("NotePos",i);
				//Data->SetString("Texture","texture du message");				
				list->AddItem(Data,player->GetNoteId(i),false,false);
			}
		}
	}
	if (note)
	{
		note->SetText("");
	}
}

void CInventory::GetSelectedNote()
{
	vgui::ListPanel *m_pList = dynamic_cast<vgui::ListPanel*>( FindChildByName( "NoteList" ) );
	if (m_pList)
	{
		int selCount = m_pList->GetSelectedItemsCount();
		if ( selCount <= 0 )
		{
			return;
		}
		C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
		vgui::Label *note = dynamic_cast<vgui::Label*>( FindChildByName( "NoteText" ) );
		if (player && note)
		{
			int itemId = m_pList->GetSelectedItem(0);
			KeyValues *kv = m_pList->GetItem( itemId );
			//int noteid = m_pList->GetItemUserData(itemId);
			//DevMsg("Selected note %d %d %s\n",itemId,noteid,player->GetNote(noteid));
			//note->SetText(player->GetNote(noteid));
			if (kv)
			{
				note->SetText(kv->GetString("Message"));
				kv->SetInt("Read",2);
				player->SetNoteRead(kv->GetInt("NotePos",0));
			}
		}
	}
}

void CInventory::SetSelecteditem(int i)
{
	selecteditem = i;

	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if (player->GetWeapon(i-1))
	{
		if ( selecteditem == i )
			UseItem(i);

		selecteditem = i;

		m_fZoom=50;
		m_fLeft=0;
		m_fTop=0;
		m_fx=0;
		m_fy=0;
		m_fz=0;
		m_fLeft = 0;

		const FileWeaponInfo_t &weaponInfo = player->GetWeapon(i-1)->GetWpnData();
		if ( pAnimModel )
			pAnimModel->Remove();

		pAnimModel = new C_BaseAnimatingOverlay;
		pAnimModel->InitializeAsClientEntity( weaponInfo.szWorldModel, RENDER_GROUP_OPAQUE_ENTITY );
		pAnimModel->AddEffects( EF_NODRAW ); // don't let the renderer draw the model normally

		const char *item;
		char temp[32];
		char temp2[32];
		strcpy(temp,player->GetWeapon(i-1)->GetClassname());
		item = nexttoken( temp2, temp, '_' );
		char info[32];
		Q_snprintf(info,sizeof(info),"JL_%s_Info",item );	
		vgui::Label *m_pInfo;
		m_pInfo = dynamic_cast<vgui::Label*>( FindChildByName( "Info" ) );
		wchar_t *tempString = g_pVGuiLocalize->Find(info);
		if (m_pInfo)
		{
			m_pInfo->SetText(tempString);
		}
	}
}

void CInventory::PrintPosition()
{
	Msg("-----------\nWeapon Preview Position\nZoom : %f\nX : %f\nY : %f\nZ : %f\nTop : %f\nLeft : %f\n-----------\n",m_fZoom,m_fx,m_fy,m_fz,m_fTop,m_fLeft);
}
