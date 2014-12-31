//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "networkstringtable_clientdll.h"
#include <KeyValues.h>
#include "panelmetaclassmgr.h"
#include <vgui_controls/Controls.h>
#include "mathlib/vmatrix.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "view.h"
#include "collisionutils.h"
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui/IVGui.h>
//JL
#include <vgui_controls/HTML.h>
#include <vgui_controls/HTMLWebKit.h>
#include <vgui_controls/LogicControl.h>

#include "ienginevgui.h"
#include "in_buttons.h"
#include <vgui/MouseCode.h>
#include "materialsystem/imesh.h"
#include "clienteffectprecachesystem.h"
#include "c_vguiscreen.h"
#include "iclientmode.h"
#include "vgui_bitmapbutton.h"
#include "vgui_bitmappanel.h"
#include "filesystem.h"
#include "iinput.h"

#include <vgui/IInputInternal.h>
extern vgui::IInputInternal *g_InputInternal;

//JL
#include "IGameUIFuncs.h" // for key bindings 
#include <LuaBind.h>
#include <string>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define VGUI_SCREEN_MODE_RADIUS	80

//Precache the materials
CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectVGuiScreen )
CLIENTEFFECT_MATERIAL( "engine/writez" )
CLIENTEFFECT_REGISTER_END()

CUtlVector<C_VGuiScreen*> m_VguiScreens; //JL
// ----------------------------------------------------------------------------- //
// This is a cache of preloaded keyvalues.
// ----------------------------------------------------------------------------- // 

CUtlDict<KeyValues*, int> g_KeyValuesCache;

KeyValues* CacheKeyValuesForFile( const char *pFilename )
{
	MEM_ALLOC_CREDIT();
	int i = g_KeyValuesCache.Find( pFilename );
	if ( i == g_KeyValuesCache.InvalidIndex() )
	{
		KeyValues *rDat = new KeyValues( pFilename );
		rDat->LoadFromFile( filesystem, pFilename, NULL );
		g_KeyValuesCache.Insert( pFilename, rDat );
		return rDat;		
	}
	else
	{
		return g_KeyValuesCache[i];
	}
}

void ClearKeyValuesCache()
{
	MEM_ALLOC_CREDIT();
	for ( int i=g_KeyValuesCache.First(); i != g_KeyValuesCache.InvalidIndex(); i=g_KeyValuesCache.Next( i ) )
	{
		g_KeyValuesCache[i]->deleteThis();
	}
	g_KeyValuesCache.Purge();
}


IMPLEMENT_CLIENTCLASS_DT(C_VGuiScreen, DT_VGuiScreen, CVGuiScreen)
	RecvPropFloat( RECVINFO(m_flWidth) ),
	RecvPropFloat( RECVINFO(m_flHeight) ),
	RecvPropInt( RECVINFO(m_fScreenFlags) ),
	RecvPropInt( RECVINFO(m_nPanelName) ),
	RecvPropInt( RECVINFO(m_nAttachmentIndex) ),
	RecvPropInt( RECVINFO(m_nOverlayMaterial) ),
	RecvPropEHandle( RECVINFO(m_hPlayerOwner) ),

	//JL
	RecvPropInt( RECVINFO(m_iCode) ),
	RecvPropString( RECVINFO(m_sVar1) ),
	RecvPropString( RECVINFO(m_sVar2) ),
	RecvPropInt( RECVINFO(m_bUseFull) ),
	RecvPropInt( RECVINFO(m_iState) ),
END_RECV_TABLE()


//-----------------------------------------------------------------------------
// Constructor 
//-----------------------------------------------------------------------------
C_VGuiScreen::C_VGuiScreen()
{
	m_nOldPanelName = m_nPanelName = -1;
	m_nOldOverlayMaterial = m_nOverlayMaterial = -1;
	m_nOldPx = m_nOldPy = -1;
	m_nButtonState = 0;
	m_bLoseThinkNextFrame = false;
	m_bAcceptsInput = true;

	m_WriteZMaterial.Init( "engine/writez", TEXTURE_GROUP_VGUI );
	m_OverlayMaterial.Init( m_WriteZMaterial );
	
	//JL
	int i = m_VguiScreens.AddToTail( );
	m_VguiScreens[i]=this;	
	m_iState = 0;
	m_iOldState = -1;
}

C_VGuiScreen::~C_VGuiScreen()
{
	//JL
	m_VguiScreens.FindAndRemove(this);

	DestroyVguiScreen();
}

//-----------------------------------------------------------------------------
// Network updates
//-----------------------------------------------------------------------------
void C_VGuiScreen::PreDataUpdate( DataUpdateType_t updateType )
{
	BaseClass::PreDataUpdate( updateType );
	m_nOldPanelName = m_nPanelName;
	m_nOldOverlayMaterial = m_nOverlayMaterial;
}

void C_VGuiScreen::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ((type == DATA_UPDATE_CREATED) || (m_nPanelName != m_nOldPanelName))
	{
		CreateVguiScreen( PanelName() );
		m_nButtonState = 0;
	}

	// Set up the overlay material
	if (m_nOldOverlayMaterial != m_nOverlayMaterial)
	{
		m_OverlayMaterial.Shutdown();

		const char *pMaterialName = GetMaterialNameFromIndex(m_nOverlayMaterial);
		if (pMaterialName)
		{
			m_OverlayMaterial.Init( pMaterialName, TEXTURE_GROUP_VGUI );
		}
		else
		{
			m_OverlayMaterial.Init( m_WriteZMaterial );
		}
	}
}

void FormatViewModelAttachment( Vector &vOrigin, bool bInverse );

//-----------------------------------------------------------------------------
// Returns the attachment render origin + origin
//-----------------------------------------------------------------------------
void C_VGuiScreen::GetAimEntOrigin( IClientEntity *pAttachedTo, Vector *pOrigin, QAngle *pAngles )
{
	C_BaseEntity *pEnt = pAttachedTo->GetBaseEntity();
	if (pEnt && (m_nAttachmentIndex > 0))
	{
		{
			C_BaseAnimating::AutoAllowBoneAccess boneaccess( true, true );
			pEnt->GetAttachment( m_nAttachmentIndex, *pOrigin, *pAngles );
		}
		
		if ( IsAttachedToViewModel() )
		{
			FormatViewModelAttachment( *pOrigin, true );
		}
	}
	else
	{
		BaseClass::GetAimEntOrigin( pAttachedTo, pOrigin, pAngles );
	}
}

//-----------------------------------------------------------------------------
// Create, destroy vgui panels...
//-----------------------------------------------------------------------------
void C_VGuiScreen::CreateVguiScreen( const char *pTypeName )
{
	// Clear out any old screens.
	DestroyVguiScreen();

	AddEFlags( EFL_USE_PARTITION_WHEN_NOT_SOLID );

	// Create the new screen...
	VGuiScreenInitData_t initData( this );
	//JL
	initData.m_iCode = m_iCode;
	initData.m_sVar1 = m_sVar1;
	initData.m_sVar2 = m_sVar2;

	m_PanelWrapper.Activate( pTypeName, NULL, 0, &initData );

	// Retrieve the panel dimensions
	vgui::Panel *pPanel = m_PanelWrapper.GetPanel();
	if (pPanel)
	{
		int x, y;
		pPanel->GetBounds( x, y, m_nPixelWidth, m_nPixelHeight );
	}
	else
	{
		m_nPixelWidth = m_nPixelHeight = 0;
	}
	if (pPanel)
	{
		//pPanel->SetScheme("ClientScheme");
		vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFileEx( enginevgui->GetPanel( PANEL_CLIENTDLL_TOOLS ), "resource/VguiScheme.res", "VguiScheme");
		pPanel->SetScheme( scheme );

		char temp[8192] ;
		for (int i = 0; i < pPanel->GetChildCount(); i++)
		{
			vgui::Button *child2 = dynamic_cast<vgui::Button*>(pPanel->GetChild(i));
			if ( child2)
			{		
				if (child2->GetCommand())
				{
					KeyValues *values = child2->GetCommand();
					while (values)
					{
						strcpy(temp,values->GetString("Command"));
						strReplace(temp,"%var1%",m_sVar1);
						strReplace(temp,"%var2%",m_sVar2);
						values->SetString("Command",temp);
						values = values->GetNextKey();
					}
				}
				
				strcpy(temp,child2->GetScript());
				strReplace(temp,"%var1%",m_sVar1);
				strReplace(temp,"%var2%",m_sVar2);
				child2->SetScript(temp);
			}

			if ( !Q_stricmp(pPanel->GetChild(i)->GetClassName(),"HTML"))
			{
				DevMsg("HTML found\n");

				//strcpy(temp,pPanel->GetChild(i)->GetCommand()->GetString("URL"));
				//strReplace(temp,"%var1%",m_sVar1);
				//strReplace(temp,"%var2%",m_sVar2);
				strcpy(temp,m_sVar1);

				DevMsg("%s\n",temp);
				if (!Q_stricmp(temp,""))
				{				
					vgui::HTML *child3 = (vgui::HTML*)(pPanel->GetChild(i));
					//child3->OpenURL(temp,true);
					if (m_sVar1[0]==':')
					{
						strcpy(temp, engine->GetGameDirectory());
						strcat(temp, "/");
						strcat(temp, m_sVar1+1*sizeof(char));
						DevMsg("%d\n",temp);
					}
					child3->OpenURL(temp, NULL,false);	
				}
			}

			if ( !Q_stricmp(pPanel->GetChild(i)->GetClassName(),"HTMLWebKit"))
			{
				DevMsg("HTMLWebKit found\n");

				vgui::HTMLWebKit *child3 = (vgui::HTMLWebKit*)(pPanel->GetChild(i));
				if (child3)
				{				
					strcpy(temp,child3->GetDefaultURL());
					DevMsg("Default URL %s\n",temp);
					if (!Q_stricmp(temp,""))
					{
						strReplace(temp,"%var1%",m_sVar1);
						//child3->SetURL(temp);
						//if (!Q_stricmp(m_sVar1,""))
							//child3->SetURL(m_sVar1);
					}
				}
			}
		}
	}
}

void C_VGuiScreen::DestroyVguiScreen( )
{
	m_PanelWrapper.Deactivate();
}

void C_VGuiScreen::SetUseFull(bool enable)
{
	m_bUseFull=enable;
}

bool C_VGuiScreen::IsUseFull()
{
	return m_bUseFull;
}

//-----------------------------------------------------------------------------
// Is the screen active?
//-----------------------------------------------------------------------------
bool C_VGuiScreen::IsActive() const
{
	return (m_fScreenFlags & VGUI_SCREEN_ACTIVE) != 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_VGuiScreen::IsAttachedToViewModel() const
{
	return (m_fScreenFlags & VGUI_SCREEN_ATTACHED_TO_VIEWMODEL) != 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_VGuiScreen::AcceptsInput() const
{
	return m_bAcceptsInput;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : acceptsinput - 
//-----------------------------------------------------------------------------
void C_VGuiScreen::SetAcceptsInput( bool acceptsinput )
{
	m_bAcceptsInput = acceptsinput;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : RenderGroup_t
//-----------------------------------------------------------------------------
RenderGroup_t C_VGuiScreen::GetRenderGroup()
{
	if ( IsAttachedToViewModel() )
		return RENDER_GROUP_VIEW_MODEL_TRANSLUCENT;

	return BaseClass::GetRenderGroup();
}

//-----------------------------------------------------------------------------
// Are we only visible to teammates?
//-----------------------------------------------------------------------------
bool C_VGuiScreen::IsVisibleOnlyToTeammates() const
{
	return (m_fScreenFlags & VGUI_SCREEN_VISIBLE_TO_TEAMMATES) != 0;
}

//-----------------------------------------------------------------------------
// Are we visible to someone on this team?
//-----------------------------------------------------------------------------
bool C_VGuiScreen::IsVisibleToTeam( int nTeam )
{
	// FIXME: Should this maybe go into a derived class of some sort?
	// Don't bother with screens on the wrong team
	if (IsVisibleOnlyToTeammates() && (nTeam > 0))
	{
		// Hmmm... sort of a hack...
		C_BaseEntity *pOwner = GetOwnerEntity();
		if ( pOwner && (nTeam != pOwner->GetTeamNumber()) )
			return false;
	}
	
	return true;
}


//-----------------------------------------------------------------------------
// Activate, deactivate the view screen
//-----------------------------------------------------------------------------
void C_VGuiScreen::GainFocus( )
{
	SetNextClientThink( CLIENT_THINK_ALWAYS );
	m_bLoseThinkNextFrame = false;
	m_nOldButtonState = 0;
}

void C_VGuiScreen::LoseFocus()
{
	m_bLoseThinkNextFrame = true;
	m_nOldButtonState = 0;
}

void C_VGuiScreen::SetButtonState( int nButtonState )
{
	m_nButtonState = nButtonState;
}



//-----------------------------------------------------------------------------
// Returns the panel name 
//-----------------------------------------------------------------------------
const char *C_VGuiScreen::PanelName() const
{
	return g_StringTableVguiScreen->GetString( m_nPanelName );
}

//[JL

void C_VGuiScreen::AddMouseDelta(int dx, int dy)
{
	m_iPosX+=dx;
	m_iPosY+=dy;
	if (m_iPosX<0)
		m_iPosX=0;
	if (m_iPosY<0)
		m_iPosY=0;
	if (m_iPosX>m_nPixelWidth)
		m_iPosX=m_nPixelWidth;
	if (m_iPosY>m_nPixelHeight)
		m_iPosY=m_nPixelHeight;
}

void C_VGuiScreen::SetMousePos(int x, int y)
{
	m_iPosX=x;
	m_iPosY=y;
	if (m_iPosX<0)
		m_iPosX=0;
	if (m_iPosY<0)
		m_iPosY=0;
	if (m_iPosX>m_nPixelWidth)
		m_iPosX=m_nPixelWidth;
	if (m_iPosY>m_nPixelHeight)
		m_iPosY=m_nPixelHeight;
}

void C_VGuiScreen::GetMousePos(int& x, int& y)
{
	x=m_iPosX;
	y=m_iPosY;
}
//JL]


//--------------------------------------------------------------------------
// Purpose:
// Given a field of view and mouse/screen positions as well as the current
// render origin and angles, returns a unit vector through the mouse position
// that can be used to trace into the world under the mouse click pixel.
// Input : 
// mousex -
// mousey -
// fov -
// vecRenderOrigin - 
// vecRenderAngles -
// Output :
// vecPickingRay
//--------------------------------------------------------------------------
void ScreenToWorld( int mousex, int mousey, float fov,
					const Vector& vecRenderOrigin,
					const QAngle& vecRenderAngles,
					Vector& vecPickingRay )
{
	float dx, dy;
	float c_x, c_y;
	float dist;
	Vector vpn, vup, vright;

	float scaled_fov = ScaleFOVByWidthRatio( fov, engine->GetScreenAspectRatio() * 0.75f );

	c_x = ScreenWidth() / 2;
	c_y = ScreenHeight() / 2;

	dx = (float)mousex - c_x;
	// Invert Y
	dy = c_y - (float)mousey;

	// Convert view plane distance
	dist = c_x / tan( M_PI * scaled_fov / 360.0 );

	// Decompose view angles
	AngleVectors( vecRenderAngles, &vpn, &vright, &vup );

	// Offset forward by view plane distance, and then by pixel offsets
	vecPickingRay = vpn * dist + vright * ( dx ) + vup * ( dy );

	// Convert to unit vector
	VectorNormalize( vecPickingRay );
} 

//-----------------------------------------------------------------------------
// Purpose: Deal with input
//-----------------------------------------------------------------------------
void C_VGuiScreen::ClientThink( void )
{
	int nButtonsChanged = m_nOldButtonState ^ m_nButtonState;

	m_nOldButtonState = m_nButtonState;

	// Debounced button codes for pressed/released
	// UNDONE: Do we need auto-repeat?
	m_nButtonPressed =  nButtonsChanged & m_nButtonState;		// The changed ones still down are "pressed"
	m_nButtonReleased = nButtonsChanged & (~m_nButtonState);	// The ones not down are "released"

	BaseClass::ClientThink();

	// FIXME: We should really be taking bob, shake, and roll into account
	// but if we did, then all the inputs would be generated multiple times
	// if the world was rendered multiple times (for things like water, etc.)

	vgui::Panel *pPanel = m_PanelWrapper.GetPanel();
	if (!pPanel)
		return;
	
	//[JL
	//LogicImage, LogicImageButton
	if (m_iState!=m_iOldState)
	{
		CVGuiScreenPanel * panel = (CVGuiScreenPanel*)m_PanelWrapper.GetPanel();
		if (panel)
		{
			panel->GetLuaBind().SetVarInteger("m_iState",m_iState);
			panel->GetLuaBind().RunCommand("OnStateChanged()");
			//DevMsg("panel %d %d %s\n", panel, panel->GetLuaBind().GetPanel(), panel->GetLuaBind().m_sFilename.c_str());
		}
		//DevMsg("VGUI State changed%d\n",m_iState);
		m_iOldState=m_iState;
		for (int i = 0; i < pPanel->GetChildCount(); i++)
		{
			vgui::LogicControl *child = dynamic_cast<vgui::LogicControl*>(pPanel->GetChild(i));
			if ( child )
			{
				child->SetState(m_iState);
			}
		}
	}
	CVGuiScreenPanel * panel = (CVGuiScreenPanel*)m_PanelWrapper.GetPanel();
	if (panel)
	{
		//panel->GetLuaBind().RunCommand("OnManage()");
	}
	//JL]

	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pLocalPlayer)
		return;

	//[JL 
	//CBaseEntity *pEntity = NULL;
	bool UseFull=false;
	UseFull = pLocalPlayer->GetVguiUseFull();
	/*if (UseFull)
	{
		if (pLocalPlayer->GetCurrentVguiScreen().Get()==NULL)
		{
			pLocalPlayer->GetCurrentVguiScreen().Set(this);
		}
	}*/
	//JL]

	// Generate a ray along the view direction
	Vector vecEyePosition = pLocalPlayer->EyePosition();
	
	QAngle viewAngles = pLocalPlayer->EyeAngles( );

	//[JL
	if (UseFull && pLocalPlayer->GetViewModel())
	{
		vecEyePosition = pLocalPlayer->GetViewModel()->GetAbsOrigin();
		viewAngles = pLocalPlayer->GetViewModel()->GetAbsAngles();
	}
	//JL]

	// Compute cursor position...
	Ray_t lookDir;
	Vector endPos;
	
	float u, v;

	// Viewmodel attached screens that take input need to have a moving cursor
	// Do a pick under the cursor as our selection
	Vector viewDir;
	AngleVectors( viewAngles, &viewDir );
	VectorMA( vecEyePosition, 1000.0f, viewDir, endPos );
	lookDir.Init( vecEyePosition, endPos );

	if (!IntersectWithRay( lookDir, &u, &v, NULL ))
		return;

	if ( ((u < 0) || (v < 0) || (u > 1) || (v > 1)) && !m_bLoseThinkNextFrame)
		return;

	// This will cause our panel to grab all input!
	g_pClientMode->ActivateInGameVGuiContext( pPanel );

	// Convert (u,v) into (px,py)
	int px = (int)(u * m_nPixelWidth + 0.5f);
	int py = (int)(v * m_nPixelHeight + 0.5f);

	//[JL
	//if (pLocalPlayer && pLocalPlayer->GetFlags() & FL_VGUISCREEN_CONTROL)
	//if (m_bUseFull)
	if (UseFull)
	{
		px = m_iPosX + 0.5f;
		py = m_iPosY + 0.5f;
	}else{
		m_iPosX = px;
		m_iPosY = py;
	}

	//Update all fake cursor
	vgui::Panel *fakeCursor = pPanel->FindChildByName("FakeCursor");
	if (fakeCursor && UseFull)
	{
		if (fakeCursor->IsVisible() == false)
		{
			fakeCursor->SetVisible(true);
			fakeCursor->SetEnabled(false);
			fakeCursor->SetMouseInputEnabled(false);
			fakeCursor->SetSize(32,32);
		}
		fakeCursor->SetPos(px-5,py-5);
	}else if (fakeCursor)
	{
		fakeCursor->SetVisible(false);
	}
	//JL]
	
	// Generate mouse input commands
	if ((px != m_nOldPx) || (py != m_nOldPy))
	{
		g_InputInternal->InternalCursorMoved( px, py );
		m_nOldPx = px;
		m_nOldPy = py;
	}

	if (m_nButtonPressed & IN_ATTACK)
	{
		g_InputInternal->SetMouseCodeState( MOUSE_LEFT, vgui::BUTTON_PRESSED );
		g_InputInternal->InternalMousePressed(MOUSE_LEFT);
	}
	if (m_nButtonPressed & IN_ATTACK2)
	{
		g_InputInternal->SetMouseCodeState( MOUSE_RIGHT, vgui::BUTTON_PRESSED );
		g_InputInternal->InternalMousePressed( MOUSE_RIGHT );
	}
	if ( (m_nButtonReleased & IN_ATTACK) || m_bLoseThinkNextFrame) // for a button release on loosing focus
	{
		g_InputInternal->SetMouseCodeState( MOUSE_LEFT, vgui::BUTTON_RELEASED );
		g_InputInternal->InternalMouseReleased( MOUSE_LEFT );
	}
	if (m_nButtonReleased & IN_ATTACK2)
	{
		g_InputInternal->SetMouseCodeState( MOUSE_RIGHT, vgui::BUTTON_RELEASED );
		g_InputInternal->InternalMouseReleased( MOUSE_RIGHT );
	}

	//[JL
	if (1)
	{
		// START TEDDYS FIX
		for (int i = 0; i < pPanel->GetChildCount(); i++)
		{
			vgui::Button *child = dynamic_cast<vgui::Button*>(pPanel->GetChild(i));
			if ( child )
			{
				int x1, x2, y1, y2;
				child->GetBounds( x1, y1, x2, y2 );
			
				// Generate mouse input commands	
				if ( (m_nButtonReleased & IN_ATTACK) )
				{
					if ( px >= x1 && px <= x1 + x2 && py >= y1 && py <= y1 + y2 )
					{
						KeyValues *data = child->GetCommand();
						if (data)
						{
							OnCommand(data->GetString("Command"));
							//DevMsg("Command %s\n",data->GetString("Command"));
						}
						//DevMsg("Script %s\n",child->GetScript());
						std::string cmd;
						cmd = child->GetScript();
						if (cmd != "")
						{
							cmd = "#lua " + cmd;
							OnCommand(cmd.c_str());
						}
					}
				}
			}
		}
		// FIN TEDDYS FIX
		//JL]
	}

	if ( m_bLoseThinkNextFrame == true )
	{
		m_bLoseThinkNextFrame = false;
		SetNextClientThink( CLIENT_THINK_NEVER );
		//[JL
		if (fakeCursor)
		{
			fakeCursor->SetVisible(false);
		}
		//JL]
	}

	g_pClientMode->DeactivateInGameVGuiContext( );
}
//[JL
void C_VGuiScreen::OnCommand(const char* pcCommand)
{
	// Get the first parameter
	if (Q_strlen(pcCommand)>0)
	{
		m_PanelWrapper.GetPanel()->OnCommand(pcCommand);
	}
}
//JL]

//-----------------------------------------------------------------------------
// Computes control points of the quad describing the screen
//-----------------------------------------------------------------------------
void C_VGuiScreen::ComputeEdges( Vector *pUpperLeft, Vector *pUpperRight, Vector *pLowerLeft )
{
	Vector vecOrigin = GetAbsOrigin();
	Vector xaxis, yaxis;
	AngleVectors( GetAbsAngles(), &xaxis, &yaxis, NULL );

	// NOTE: Have to multiply by -1 here because yaxis goes out the -y axis in AngleVectors actually...
	yaxis *= -1.0f;

	VectorCopy( vecOrigin, *pLowerLeft );
	VectorMA( vecOrigin, m_flHeight, yaxis, *pUpperLeft );
	VectorMA( *pUpperLeft, m_flWidth, xaxis, *pUpperRight );
}


//-----------------------------------------------------------------------------
// Return intersection point of ray with screen in barycentric coords
//-----------------------------------------------------------------------------
bool C_VGuiScreen::IntersectWithRay( const Ray_t &ray, float *u, float *v, float *t )
{
	// Perform a raycast to see where in barycentric coordinates the ray hits
	// the viewscreen; if it doesn't hit it, you're not in the mode
	Vector origin, upt, vpt;
	ComputeEdges( &origin, &upt, &vpt );
	return ComputeIntersectionBarycentricCoordinates( ray, origin, upt, vpt, *u, *v, t );
}


//-----------------------------------------------------------------------------
// Is the vgui screen backfacing?
//-----------------------------------------------------------------------------
bool C_VGuiScreen::IsBackfacing( const Vector &viewOrigin )
{
	// Compute a ray from camera to center of the screen..
	Vector cameraToScreen;
	VectorSubtract( GetAbsOrigin(), viewOrigin, cameraToScreen );

	// Figure out the face normal
	Vector zaxis;
	GetVectors( NULL, NULL, &zaxis );

	// The actual backface cull
	return (DotProduct( zaxis, cameraToScreen ) > 0.0f);
}


//-----------------------------------------------------------------------------
//  Computes the panel center to world transform
//-----------------------------------------------------------------------------
void C_VGuiScreen::ComputePanelToWorld()
{
	// The origin is at the upper-left corner of the screen
	Vector vecOrigin, vecUR, vecLL;
	ComputeEdges( &vecOrigin, &vecUR, &vecLL );
	m_PanelToWorld.SetupMatrixOrgAngles( vecOrigin, GetAbsAngles() );
}


//-----------------------------------------------------------------------------
// a pass to set the z buffer...
//-----------------------------------------------------------------------------
void C_VGuiScreen::DrawScreenOverlay()
{
	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->MatrixMode( MATERIAL_MODEL );
	pRenderContext->PushMatrix();
	pRenderContext->LoadMatrix( m_PanelToWorld );

	unsigned char pColor[4] = {255, 255, 255, 255};

	CMeshBuilder meshBuilder;
	IMesh *pMesh = pRenderContext->GetDynamicMesh( true, NULL, NULL, m_OverlayMaterial );
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

	meshBuilder.Position3f( 0.0f, 0.0f, 0 );
	meshBuilder.TexCoord2f( 0, 0.0f, 0.0f );
	meshBuilder.Color4ubv( pColor );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f( m_flWidth, 0.0f, 0 );
	meshBuilder.TexCoord2f( 0, 1.0f, 0.0f );
	meshBuilder.Color4ubv( pColor );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f( m_flWidth, -m_flHeight, 0 );
	meshBuilder.TexCoord2f( 0, 1.0f, 1.0f );
	meshBuilder.Color4ubv( pColor );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f( 0.0f, -m_flHeight, 0 );
	meshBuilder.TexCoord2f( 0, 0.0f, 1.0f );
	meshBuilder.Color4ubv( pColor );
	meshBuilder.AdvanceVertex();

	meshBuilder.End();
	pMesh->Draw();

	pRenderContext->PopMatrix();
}


//-----------------------------------------------------------------------------
// Draws the panel using a 3D transform...
//-----------------------------------------------------------------------------
int	C_VGuiScreen::DrawModel( int flags )
{
	vgui::Panel *pPanel = m_PanelWrapper.GetPanel();
	if (!pPanel || !IsActive())
		return 0;
	
	// Don't bother drawing stuff not visible to me...
	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pLocalPlayer || !IsVisibleToTeam(pLocalPlayer->GetTeamNumber()) )
		return 0;

	if ( !IsVisibleToPlayer( pLocalPlayer ) )
	{
		return 0;
	}
	
	// Backface cull the entire panel here...
	if (IsBackfacing(CurrentViewOrigin()))
		return 0;

	// Recompute the panel-to-world center
	// FIXME: Can this be cached off?
	ComputePanelToWorld();

	g_pMatSystemSurface->DrawPanelIn3DSpace( pPanel->GetVPanel(), m_PanelToWorld, 
		m_nPixelWidth, m_nPixelHeight, m_flWidth, m_flHeight );

	// Finally, a pass to set the z buffer...
	DrawScreenOverlay();

	return 1;
}

bool C_VGuiScreen::ShouldDraw( void )
{
	return !IsEffectActive(EF_NODRAW);
}


//-----------------------------------------------------------------------------
// Purpose: Hook for vgui screens to determine visibility
//-----------------------------------------------------------------------------
bool C_VGuiScreen::IsVisibleToPlayer( C_BasePlayer *pViewingPlayer )
{
	return true;
}

bool C_VGuiScreen::IsTransparent( void )
{
	return (m_fScreenFlags & VGUI_SCREEN_TRANSPARENT) != 0;
}

//-----------------------------------------------------------------------------
// Purpose: Sometimes we only want a specific player to be able to input to a panel
//-----------------------------------------------------------------------------
C_BasePlayer *C_VGuiScreen::GetPlayerOwner( void )
{
	return ( C_BasePlayer * )( m_hPlayerOwner.Get() );
}

bool C_VGuiScreen::IsInputOnlyToOwner( void )
{
	return (m_fScreenFlags & VGUI_SCREEN_ONLY_USABLE_BY_OWNER) != 0;
}

//-----------------------------------------------------------------------------
//
// Enumator class for finding vgui screens close to the local player
//
//-----------------------------------------------------------------------------
class CVGuiScreenEnumerator : public IPartitionEnumerator
{
	DECLARE_CLASS_GAMEROOT( CVGuiScreenEnumerator, IPartitionEnumerator );
public:
	virtual IterationRetval_t EnumElement( IHandleEntity *pHandleEntity );

	int	GetScreenCount();
	C_VGuiScreen *GetVGuiScreen( int index );

private:
	CUtlVector< CHandle< C_VGuiScreen > > m_VguiScreens;
};

IterationRetval_t CVGuiScreenEnumerator::EnumElement( IHandleEntity *pHandleEntity )
{
	C_BaseEntity *pEnt = ClientEntityList().GetBaseEntityFromHandle( pHandleEntity->GetRefEHandle() );
	if ( pEnt == NULL )
		return ITERATION_CONTINUE;

	// FIXME.. pretty expensive...
	C_VGuiScreen *pScreen = dynamic_cast<C_VGuiScreen*>(pEnt); 
	if ( pScreen )
	{
		int i = m_VguiScreens.AddToTail( );
		m_VguiScreens[i].Set( pScreen );
	}

	return ITERATION_CONTINUE;
}

int	CVGuiScreenEnumerator::GetScreenCount()
{
	return m_VguiScreens.Count();
}

C_VGuiScreen *CVGuiScreenEnumerator::GetVGuiScreen( int index )
{
	return m_VguiScreens[index].Get();
}	


//-----------------------------------------------------------------------------
//
// Look for vgui screens, returns true if it found one ...
//
//-----------------------------------------------------------------------------
C_BaseEntity *FindNearbyVguiScreen( const Vector &viewPosition, const QAngle &viewAngle, int nTeam )
{
	if ( IsX360() )
	{
		// X360TBD: Turn this on if feature actually used
		return NULL;
	}

	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();

	Assert( pLocalPlayer );

	if ( !pLocalPlayer )
		return NULL;

	// Get the view direction...
	Vector lookDir;
	AngleVectors( viewAngle, &lookDir );

	// Create a ray used for raytracing 
	Vector lookEnd;
	VectorMA( viewPosition, 2.0f * VGUI_SCREEN_MODE_RADIUS, lookDir, lookEnd );

	Ray_t lookRay;
	lookRay.Init( viewPosition, lookEnd );

	// Look for vgui screens that are close to the player
	
	//[JL ?
	//CVGuiScreenEnumerator localScreens;
	//partition->EnumerateElementsInSphere( PARTITION_CLIENT_NON_STATIC_EDICTS, viewPosition, VGUI_SCREEN_MODE_RADIUS, false, &localScreens );
	//DevMsg("Screen %d\n",localScreens.GetScreenCount());
	//JL]

	Vector vecOut, vecViewDelta;

	float flBestDist = 2.0f;
	C_VGuiScreen *pBestScreen = NULL;
	//[JL
	//for (int i = localScreens.GetScreenCount(); --i >= 0; )
	for (int i=0;i<m_VguiScreens.Count();i++ )
	{
		//C_VGuiScreen *pScreen = localScreens.GetVGuiScreen(i);
		C_VGuiScreen *pScreen = m_VguiScreens[i];

		if ( pScreen->IsAttachedToViewModel() )
			continue;

		// Don't bother with screens I'm behind...
		// Hax - don't cancel backfacing with viewmodel attached screens.
		// we can get prediction bugs that make us backfacing for one frame and
		// it resets the mouse position if we lose focus.
		if ( pScreen->IsBackfacing(viewPosition) )
			continue;

		// Don't bother with screens that are turned off
		if (!pScreen->IsActive())
			continue;

		// FIXME: Should this maybe go into a derived class of some sort?
		// Don't bother with screens on the wrong team
		if (!pScreen->IsVisibleToTeam(nTeam))
			continue;

		if ( !pScreen->AcceptsInput() )
			continue;

		if ( pScreen->IsInputOnlyToOwner() && pScreen->GetPlayerOwner() != pLocalPlayer )
			continue;

		// Test perpendicular distance from the screen...
		pScreen->GetVectors( NULL, NULL, &vecOut );
		VectorSubtract( viewPosition, pScreen->GetAbsOrigin(), vecViewDelta );
		float flPerpDist = DotProduct(vecViewDelta, vecOut);
		if ( (flPerpDist < 0) || (flPerpDist > VGUI_SCREEN_MODE_RADIUS) )
			continue;

		// Perform a raycast to see where in barycentric coordinates the ray hits
		// the viewscreen; if it doesn't hit it, you're not in the mode
		float u, v, t;
		if (!pScreen->IntersectWithRay( lookRay, &u, &v, &t ))
			continue;

		// Barycentric test
		if ((u < 0) || (v < 0) || (u > 1) || (v > 1))
			continue;

		if ( t < flBestDist )
		{
			flBestDist = t;
			pBestScreen = pScreen;
		}
	}
	
	return pBestScreen;
}

void ActivateVguiScreen( C_BaseEntity *pVguiScreenEnt )
{
	if (pVguiScreenEnt)
	{
		Assert( dynamic_cast<C_VGuiScreen*>(pVguiScreenEnt) );
		C_VGuiScreen *pVguiScreen = static_cast<C_VGuiScreen*>(pVguiScreenEnt);
		pVguiScreen->GainFocus( );
	}
}

void SetVGuiScreenButtonState( C_BaseEntity *pVguiScreenEnt, int nButtonState )
{
	if (pVguiScreenEnt)
	{
		Assert( dynamic_cast<C_VGuiScreen*>(pVguiScreenEnt) );
		C_VGuiScreen *pVguiScreen = static_cast<C_VGuiScreen*>(pVguiScreenEnt);
		pVguiScreen->SetButtonState( nButtonState );
	}
}

void DeactivateVguiScreen( C_BaseEntity *pVguiScreenEnt )
{
	if (pVguiScreenEnt)
	{
		Assert( dynamic_cast<C_VGuiScreen*>(pVguiScreenEnt) );
		C_VGuiScreen *pVguiScreen = static_cast<C_VGuiScreen*>(pVguiScreenEnt);
		pVguiScreen->LoseFocus( );
	}
}

CVGuiScreenPanel::CVGuiScreenPanel( vgui::Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	m_hEntity = NULL;
}

CVGuiScreenPanel::CVGuiScreenPanel( vgui::Panel *parent, const char *panelName, vgui::HScheme hScheme )
	: BaseClass( parent, panelName, hScheme )
{
	m_hEntity = NULL;
}


bool CVGuiScreenPanel::Init( KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData )
{
	const char *pResFile = pKeyValues->GetString( "resfile" );
	if (pResFile[0] != 0)
	{
		KeyValues *pCachedKeyValues = CacheKeyValuesForFile( pResFile );
		LoadControlSettings( pResFile, NULL, pCachedKeyValues );
	}

	// Dimensions in pixels
	int nWidth, nHeight;
	nWidth = pKeyValues->GetInt( "pixelswide", 240 );
	nHeight = pKeyValues->GetInt( "pixelshigh", 160 );
	if ((nWidth <= 0) || (nHeight <= 0))
		return false;

	// If init data isn't specified, then we're just precaching.
	if ( pInitData )
	{
		m_hEntity.Set( pInitData->m_pEntity );

		C_VGuiScreen *screen = dynamic_cast< C_VGuiScreen * >( pInitData->m_pEntity );
		if ( screen )
		{
			bool acceptsInput = pKeyValues->GetInt( "acceptsinput", 1 ) ? true : false;
			screen->SetAcceptsInput( acceptsInput );
		}
	}

	SetBounds( 0, 0, nWidth, nHeight );

	//[JL
	//vgui::ivgui()->AddTickSignal( GetVPanel() );

	//SetParent(enginevgui->GetPanel( PANEL_CLIENTDLL ));

	//vgui::input()->SetMouseCapture(GetVPanel());
	//SetKeyBoardInputEnabled(true);
	//SetMouseInputEnabled(true);
	/*SetVisible(true);
	DisableMouseInputForThisPanel(false);*/

	std::string luaFile = pResFile;
	size_t pos = luaFile.find_last_of('.');
	if (pos != std::string::npos)
	{
		luaFile = luaFile.substr(0,pos);
		luaFile += ".lua";
		m_LuaBind.SetPanel(this);
		m_LuaBind.LoadFile(luaFile.c_str());

		if ( pInitData )
		{
			m_LuaBind.SetVarInteger("m_iCode", pInitData->m_iCode);
			m_LuaBind.SetVarString("m_sVar1", pInitData->m_sVar1.c_str());
			m_LuaBind.SetVarString("m_sVar2", pInitData->m_sVar2.c_str());
		}

		m_LuaBind.Run();

		m_LuaBind.RunCommand("OnStateChanged()");
	}
	//JL]

	return true;
}

vgui::Panel *CVGuiScreenPanel::CreateControlByName(const char *controlName)
{
	// Check the panel metaclass manager to make these controls...
	if (!Q_strncmp(controlName, "MaterialImage", 20))
	{
		return new CBitmapPanel(NULL, "BitmapPanel");
	}

	if (!Q_strncmp(controlName, "MaterialButton", 20))
	{
		return new CBitmapButton(NULL, "BitmapButton", "");
	}

	// Didn't find it? Just use the default stuff
	return BaseClass::CreateControlByName( controlName );
}

//-----------------------------------------------------------------------------
// Purpose: Called when the user presses a button
//-----------------------------------------------------------------------------
void CVGuiScreenPanel::OnCommand( const char *command)
{
	if ( Q_stricmp( command, "vguicancel" ) )
	{
		engine->ClientCmd( const_cast<char *>( command ) );
	}
	//[JL
	// BaseClass::OnCommand(command);
	if (strlen(command)>=5 && strncmp(command,"#lua ",5)==0)
	{
		std::string cmd = command;
		cmd = cmd.substr(5);
		m_LuaBind.RunCommand(cmd.c_str());
	}else if (strlen(command)>=5 && strncmp(command,"#cmd ",5)==0)
	{
		std::string cmd = command;
		cmd = cmd.substr(5);
		engine->ClientCmd( cmd.c_str() );
	}else if (stricmp(command,"exitvgui")==0)
	{
		engine->ClientCmd( command );
	}else{	
		BaseClass::OnCommand(command);
	}
	//Jl]
}

//[JL
void CVGuiScreenPanel::OnThink()
{
	//DevMsg("OnThink %s\n", m_LuaBind.m_sFilename.c_str());
	//m_LuaBind.SetVarInteger("m_iState",m_iState);
	//m_LuaBind.RunCommand("OnStateChanged()");
	m_LuaBind.RunCommand("OnManage()", false);
}

DECLARE_VGUI_SCREEN_FACTORY( CVGuiScreenPanel, "vgui_screen_panel" );

//-----------------------------------------------------------------------------
// Purpose: VGUILuaBind
//-----------------------------------------------------------------------------

// vgui Label SetText
int LuaSetLabelText(lua_State *pState)
{
	if (lua_gettop(pState) == 2)
	{
		if (lua_isstring(pState,1) && lua_isstring(pState,2))
		{
			if (pState->userData)
			{
				VGUILuaBind *m_L = (VGUILuaBind*)pState->userData;
				vgui::Panel *panel = m_L->GetPanel();
				if (panel)
				{
					vgui::Label *label = dynamic_cast<vgui::Label*>(panel->FindChildByName(lua_tolstring(pState,1,NULL)));
					if (label)
					{
						label->SetText(lua_tolstring(pState,2,NULL));
					}
				}
			}
		}else{
			lua_pushstring(pState, "Incorrect argument to 'SetLabelText'");
			lua_error(pState);
		}
	}
	return 0;
}

// vgui Label GetText
int LuaGetLabelText(lua_State *pState)
{
	if (lua_gettop(pState) == 1)
	{
		if (lua_isstring(pState,1))
		{
			if (pState->userData)
			{
				VGUILuaBind *m_L = (VGUILuaBind*)pState->userData;
				
				vgui::Panel *panel = m_L->GetPanel();
				if (panel)
				{	
					vgui::Label *label = dynamic_cast<vgui::Label*>(panel->FindChildByName(lua_tolstring(pState,1,NULL)));
					if (label)
					{
						char text[2048];
						label->GetText(text,2048);							
						lua_pushstring(pState, text);
						return 1;
					}
				}
			}
		}else{
			lua_pushstring(pState, "Incorrect argument to 'GetLabelText'");
			lua_error(pState);
		}
	}
	return 1;
}

int LuaSetImageTexture(lua_State *pState)
{
	return 0;
}

int LuaIsPanelVisible(lua_State *pState)
{
	if (lua_gettop(pState) == 1)
	{
		if (lua_isstring(pState,1))
		{
			if (pState->userData)
			{
				VGUILuaBind *m_L = (VGUILuaBind*)pState->userData;
				
				vgui::Panel *panel = m_L->GetPanel();
				if (panel)
				{	
					vgui::Panel *child = dynamic_cast<vgui::Panel*>(panel->FindChildByName(lua_tolstring(pState,1,NULL)));
					if (child)
					{
						lua_pushboolean(pState, child->IsVisible());
						return 0;
					}
				}
				lua_pushboolean(pState, false);
			}
		}else{
			lua_pushstring(pState, "Incorrect argument to 'GetLabelText'");
			lua_error(pState);
		}
	}
	return 1;
}

int LuaSetPanelVisible(lua_State *pState)
{
	if (lua_gettop(pState) == 2)
	{
		if (lua_isstring(pState,1))
		{
			if (pState->userData)
			{
				VGUILuaBind *m_L = (VGUILuaBind*)pState->userData;
				
				vgui::Panel *panel = m_L->GetPanel();
				if (panel)
				{	
					vgui::Panel *child = dynamic_cast<vgui::Panel*>(panel->FindChildByName(lua_tolstring(pState,1,NULL)));
					if (child)
					{
						child->SetVisible( lua_toboolean(pState, 2) );
						return 0;
					}
				}
			}
		}else{
			lua_pushstring(pState, "Incorrect argument to 'GetLabelText'");
			lua_error(pState);
		}
	}
	return 1;
}

int LuaServerCommand(lua_State *pState)
{
	if (lua_gettop(pState) == 1)
	{
		if (lua_isstring(pState,1))
		{
			engine->ClientCmd(lua_tolstring(pState,1,NULL));
		}else{
			lua_pushstring(pState, "Incorrect argument to 'ServerCommand'");
			lua_error(pState);
		}
	}
	return 0;
}

int LuaAcceptInput(lua_State *pState)
{
	if (lua_gettop(pState) == 2)
	{
		if (lua_isstring(pState,1) && lua_isstring(pState,2))
		{
			std::string command;
			command = "acceptinput ";
			command += lua_tolstring(pState,1,NULL);
			command += " ";
			command += lua_tolstring(pState,2,NULL);
			engine->ClientCmd( command.c_str());
		}else{
			lua_pushstring(pState, "Incorrect argument to 'AcceptInput'");
			lua_error(pState);
		}
	}
	return 0;
}


int LuaGetVGUIState(lua_State *pState)
{
	VGUILuaBind *m_L = (VGUILuaBind*)pState->userData;
	if (m_L)
	{
		if (m_L->GetPanel() && m_L->GetPanel()->GetEntity())
		{
			C_VGuiScreen *screen = dynamic_cast< C_VGuiScreen * >( m_L->GetPanel()->GetEntity() );
			if ( screen )
			{
				lua_pushinteger(pState, screen->m_iState);
			}
		}
	}
	lua_pushinteger(pState, 0);
	return 1;
}

/*
m_hWaitingForPlayersTimer->AcceptInput( "Resume", NULL, NULL, sVariant, 0 );
*/

VGUILuaBind::VGUILuaBind()
{
	m_pPanel = NULL;

	RegisterFunc("SetLabelText", LuaSetLabelText);
	RegisterFunc("GetLabelText", LuaGetLabelText);
	RegisterFunc("SetImageTexture", LuaSetImageTexture);
	RegisterFunc("IsPanelVisible", LuaIsPanelVisible);
	RegisterFunc("SetPanelVisible", LuaSetPanelVisible);
	RegisterFunc("ServerCommand", LuaServerCommand);
	RegisterFunc("AcceptInput", LuaAcceptInput);
	RegisterFunc("GetVGUIState", LuaGetVGUIState);
}

CVGuiScreenPanel* VGUILuaBind::GetPanel()
{
	return m_pPanel;
}

void VGUILuaBind::SetPanel(CVGuiScreenPanel* panel)
{
	m_pPanel = panel;
}

//JL]