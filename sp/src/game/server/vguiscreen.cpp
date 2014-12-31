//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: This is an entity that represents a vgui screen
//
// $NoKeywords: $
//===========================================================================//

#include "cbase.h"
#include "vguiscreen.h"
#include "networkstringtable_gamedll.h"
#include "saverestore_stringtable.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// This is an entity that represents a vgui screen
//-----------------------------------------------------------------------------

IMPLEMENT_SERVERCLASS_ST(CVGuiScreen, DT_VGuiScreen)
	SendPropFloat(SENDINFO(m_flWidth),	0, SPROP_NOSCALE ),
	SendPropFloat(SENDINFO(m_flHeight),	0, SPROP_NOSCALE ),
	SendPropInt(SENDINFO(m_nAttachmentIndex), 5, SPROP_UNSIGNED ),
	SendPropInt(SENDINFO(m_nPanelName), MAX_VGUI_SCREEN_STRING_BITS, SPROP_UNSIGNED ),
	SendPropInt(SENDINFO(m_fScreenFlags), VGUI_SCREEN_MAX_BITS, SPROP_UNSIGNED ),
	SendPropInt(SENDINFO(m_nOverlayMaterial), MAX_MATERIAL_STRING_BITS, SPROP_UNSIGNED ),
	SendPropEHandle(SENDINFO(m_hPlayerOwner)),

	//JL(TH)
	SendPropInt(SENDINFO(m_iCode), VGUI_SCREEN_MAX_BITS, SPROP_UNSIGNED ),
	SendPropStringT( SENDINFO(m_sVar1) ),
	SendPropStringT( SENDINFO(m_sVar2) ),
	SendPropInt(SENDINFO(m_bUseFull), 1, SPROP_UNSIGNED ),
	SendPropInt(SENDINFO(m_iState), 2, SPROP_UNSIGNED ),	

END_SEND_TABLE();

LINK_ENTITY_TO_CLASS( vgui_screen, CVGuiScreen );
LINK_ENTITY_TO_CLASS( vgui_screen_team, CVGuiScreen );

//JL
LINK_ENTITY_TO_CLASS( point_screen_follow, CScreenFollow );

PRECACHE_REGISTER( vgui_screen );


//-----------------------------------------------------------------------------
// Save/load
//-----------------------------------------------------------------------------
BEGIN_DATADESC( CVGuiScreen )

	DEFINE_CUSTOM_FIELD( m_nPanelName, &g_VguiScreenStringOps ),
	DEFINE_FIELD( m_nAttachmentIndex, FIELD_INTEGER ),
//	DEFINE_FIELD( m_nOverlayMaterial, FIELD_INTEGER ),
	DEFINE_FIELD( m_fScreenFlags, FIELD_INTEGER ),
	DEFINE_KEYFIELD( m_flWidth, FIELD_FLOAT, "width" ),
	DEFINE_KEYFIELD( m_flHeight, FIELD_FLOAT, "height" ),
	DEFINE_KEYFIELD( m_strOverlayMaterial, FIELD_STRING, "overlaymaterial" ),
	DEFINE_FIELD( m_hPlayerOwner, FIELD_EHANDLE ),

	//JL(TH)
	DEFINE_KEYFIELD( m_iCode, FIELD_INTEGER, "code" ),
	DEFINE_KEYFIELD( m_sVar1, FIELD_STRING, "var1" ),
	DEFINE_KEYFIELD( m_sVar2, FIELD_STRING, "var2" ),
	DEFINE_FIELD( m_bUseFull, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iState, FIELD_INTEGER ),
	
	DEFINE_INPUTFUNC( FIELD_VOID, "SetActive", InputSetActive ),
	DEFINE_INPUTFUNC( FIELD_VOID, "SetInactive", InputSetInactive ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CVGuiScreen::CVGuiScreen()
{
	m_nOverlayMaterial = OVERLAY_MATERIAL_INVALID_STRING;
	m_hPlayerOwner = NULL;
	//JL
	m_pFollow=NULL;
}


//-----------------------------------------------------------------------------
// Read in worldcraft data...
//-----------------------------------------------------------------------------
bool CVGuiScreen::KeyValue( const char *szKeyName, const char *szValue ) 
{
	//!! temp hack, until worldcraft is fixed
	// strip the # tokens from (duplicate) key names
	char *s = (char *)strchr( szKeyName, '#' );
	if ( s )
	{
		*s = '\0';
	}

	if ( FStrEq( szKeyName, "panelname" ))
	{
		SetPanelName( szValue );
		return true;
	}

	// NOTE: Have to do these separate because they set two values instead of one
	if( FStrEq( szKeyName, "angles" ) )
	{
		Assert( GetMoveParent() == NULL );
		QAngle angles;
		UTIL_StringToVector( angles.Base(), szValue );

		// Because the vgui screen basis is strange (z is front, y is up, x is right)
		// we need to rotate the typical basis before applying it
		VMatrix mat, rotation, tmp;
		MatrixFromAngles( angles, mat );
		MatrixBuildRotationAboutAxis( rotation, Vector( 0, 1, 0 ), 90 );
		MatrixMultiply( mat, rotation, tmp );
		MatrixBuildRotateZ( rotation, 90 );
		MatrixMultiply( tmp, rotation, mat );
		MatrixToAngles( mat, angles );
		SetAbsAngles( angles );

		return true;
	}

	//JL
	if ( FStrEq( szKeyName, "code" ))
	{
		m_iCode = atoi(szValue);
		return true;
	}

	return BaseClass::KeyValue( szKeyName, szValue );
}


//-----------------------------------------------------------------------------
// Precache...
//-----------------------------------------------------------------------------
void CVGuiScreen::Precache()
{
	BaseClass::Precache();
	if ( m_strOverlayMaterial != NULL_STRING )
	{
		PrecacheMaterial( STRING(m_strOverlayMaterial) );
	}
}


//-----------------------------------------------------------------------------
// Spawn...
//-----------------------------------------------------------------------------
void CVGuiScreen::Spawn()
{
	Precache();

	// This has no model, but we want it to transmit if it's in the PVS anyways
	AddEFlags( EFL_FORCE_CHECK_TRANSMIT );
	m_nAttachmentIndex = 0;
	SetSolid( SOLID_OBB );
	AddSolidFlags( FSOLID_NOT_SOLID );
	SetActualSize( m_flWidth, m_flHeight );
	m_fScreenFlags.Set( VGUI_SCREEN_ACTIVE );

	//JL
	SetTransparency( HasSpawnFlags(1) );
	SetNoColorCorrection( HasSpawnFlags(2) );
	
	m_takedamage = DAMAGE_NO;
	AddFlag( FL_NOTARGET );
}

//-----------------------------------------------------------------------------
// Spawn...
//-----------------------------------------------------------------------------
void CVGuiScreen::Activate()
{
	BaseClass::Activate();

	if ( m_nOverlayMaterial == OVERLAY_MATERIAL_INVALID_STRING && m_strOverlayMaterial != NULL_STRING )
	{
		SetOverlayMaterial( STRING(m_strOverlayMaterial) );
	}
}

void CVGuiScreen::OnRestore()
{
	UpdateTransmitState();

	BaseClass::OnRestore();
}

void CVGuiScreen::SetAttachmentIndex( int nIndex )
{
	m_nAttachmentIndex = nIndex;
}

void CVGuiScreen::SetOverlayMaterial( const char *pMaterial )
{
	int iMaterial = GetMaterialIndex( pMaterial );

	if ( iMaterial == 0 )
	{
		m_nOverlayMaterial = OVERLAY_MATERIAL_INVALID_STRING;
	}
	else
	{
		m_nOverlayMaterial = iMaterial;
	}
}

bool CVGuiScreen::IsActive() const 
{ 
	return (m_fScreenFlags & VGUI_SCREEN_ACTIVE) != 0; 
}

void CVGuiScreen::SetActive( bool bActive )
{
	if (bActive != IsActive())
	{
		if (!bActive)
		{
			m_fScreenFlags &= ~VGUI_SCREEN_ACTIVE;
		}
		else
		{
			m_fScreenFlags.Set(  m_fScreenFlags | VGUI_SCREEN_ACTIVE );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CVGuiScreen::IsAttachedToViewModel() const
{
	return (m_fScreenFlags & VGUI_SCREEN_ATTACHED_TO_VIEWMODEL) != 0; 
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bAttached - 
//-----------------------------------------------------------------------------
void CVGuiScreen::SetAttachedToViewModel( bool bAttached )
{
	if (bAttached != IsActive())
	{
		if (!bAttached)
		{
			m_fScreenFlags &= ~VGUI_SCREEN_ATTACHED_TO_VIEWMODEL;
		}
		else
		{
			m_fScreenFlags.Set( m_fScreenFlags | VGUI_SCREEN_ATTACHED_TO_VIEWMODEL );

			// attached screens have different transmit rules
			DispatchUpdateTransmitState();
		}

		// attached screens have different transmit rules
		DispatchUpdateTransmitState();
	}
}

void CVGuiScreen::SetTransparency( bool bTransparent )
{
	if (!bTransparent)
	{
		m_fScreenFlags &= ~VGUI_SCREEN_TRANSPARENT;
	}
	else
	{
		m_fScreenFlags.Set( m_fScreenFlags | VGUI_SCREEN_TRANSPARENT );
	}
}

//[JL
void CVGuiScreen::SetNoColorCorrection( bool bNoColorCorrection )
{
	if ( bNoColorCorrection )
	{
		m_fScreenFlags &= ~VGUI_SCREEN_NO_COLOR_CORRECTION;
	}else{
		m_fScreenFlags.Set( m_fScreenFlags | VGUI_SCREEN_NO_COLOR_CORRECTION );
	}
}

bool CVGuiScreen::HasNoColorCorrection()
{
	return ( (m_fScreenFlags & VGUI_SCREEN_NO_COLOR_CORRECTION) != 0 );
}
//]JL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CVGuiScreen::InputSetActive( inputdata_t &inputdata )
{
	SetActive( true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CVGuiScreen::InputSetInactive( inputdata_t &inputdata )
{
	SetActive( false );
}

//[JL
void CVGuiScreen::InputUseFull( inputdata_t &inputdata )
{
	UseFull(true);
}

void CVGuiScreen::InputSetStates( inputdata_t &inputdata )
{
	//DevMsg("Server VGUI SetState %d\n",m_iState);
	m_iState = inputdata.value.Int();
	//DevMsg("Server VGUI SetState %d\n",m_iState);
}

void CVGuiScreen::InputSetStateTrue( inputdata_t &inputdata )
{
	m_iState |= (1<<inputdata.value.Int());
}

void CVGuiScreen::InputSetStateFalse( inputdata_t &inputdata )
{
	m_iState &= ~(1<<inputdata.value.Int());
}

void CVGuiScreen::UseFull(bool enable)
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (pPlayer)
	{
		pPlayer->SetVguiUseFull(enable);
	}
	//m_bUseFull=!m_bUseFull;
	m_bUseFull=enable;
	if (!m_pFollow)
	{
		m_pFollow = (CScreenFollow*)CreateEntityByName("point_screen_follow");
		m_pFollow->SetParent(NULL);
		DispatchSpawn(m_pFollow);
	}

	m_pFollow->Setup(this,m_bUseFull);
}

bool CVGuiScreen::IsUseFull()
{
	return m_bUseFull;
}
//]JL

bool CVGuiScreen::IsVisibleOnlyToTeammates() const 
{ 
	return (m_fScreenFlags & VGUI_SCREEN_VISIBLE_TO_TEAMMATES) != 0; 
}

void CVGuiScreen::MakeVisibleOnlyToTeammates( bool bActive )
{
	if (bActive != IsVisibleOnlyToTeammates())
	{
		if (!bActive)
		{
			m_fScreenFlags &= ~VGUI_SCREEN_VISIBLE_TO_TEAMMATES;
		}
		else
		{
			m_fScreenFlags.Set(  m_fScreenFlags | VGUI_SCREEN_VISIBLE_TO_TEAMMATES );
		}
	}
}

bool CVGuiScreen::IsVisibleToTeam( int nTeam )
{
	// FIXME: Should this maybe go into a derived class of some sort?
	// Don't bother with screens on the wrong team
	if ( IsVisibleOnlyToTeammates() && (nTeam > 0) )
	{
		// Hmmm... sort of a hack...
		CBaseEntity *pOwner = GetOwnerEntity();
		if ( pOwner && (nTeam != pOwner->GetTeamNumber()) )
			return false;
	}
	
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Screens attached to view models only go to client if viewmodel is being sent, too.
// Input  : *recipient - 
//			*pvs - 
//			clientArea - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
int CVGuiScreen::UpdateTransmitState()
{
	if ( IsAttachedToViewModel() )
	{
		// only send to the owner, or someone spectating the owner.
		return SetTransmitState( FL_EDICT_FULLCHECK );
	}
	else if ( GetMoveParent() )
	{
		// Let the parent object trigger the send. This is more efficient than having it call CBaseEntity::ShouldTransmit
		// for all the vgui screens in the map.
		return SetTransmitState( FL_EDICT_PVSCHECK );
	}
	else
	{
		return BaseClass::UpdateTransmitState();
	}
}

int CVGuiScreen::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	Assert( IsAttachedToViewModel() );

	CBaseEntity *pViewModel = GetOwnerEntity();

	if ( pViewModel )
	{
		return pViewModel->ShouldTransmit( pInfo );
	}

	return BaseClass::ShouldTransmit( pInfo );
}

//-----------------------------------------------------------------------------
// Convert the panel name into an integer
//-----------------------------------------------------------------------------
void CVGuiScreen::SetPanelName( const char *pPanelName )
{
	m_nPanelName = g_pStringTableVguiScreen->AddString( CBaseEntity::IsServer(), pPanelName );
}

const char *CVGuiScreen::GetPanelName() const
{
	return g_pStringTableVguiScreen->GetString( m_nPanelName );
}


//-----------------------------------------------------------------------------
// Sets the screen size + resolution
//-----------------------------------------------------------------------------
void CVGuiScreen::SetActualSize( float flWidth, float flHeight )
{
	m_flWidth = flWidth;
	m_flHeight = flHeight;

	Vector mins, maxs;
	mins.Init( 0.0f, 0.0f, -0.1f );
	maxs.Init( 0.0f, 0.0f, 0.1f );
	if (flWidth > 0)
		maxs.x = flWidth;
	else
		mins.x = flWidth;
	if (flHeight > 0)
		maxs.y = flHeight;
	else
		mins.y = flHeight;

	UTIL_SetSize( this, mins, maxs );
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void CVGuiScreen::SetPlayerOwner( CBasePlayer *pPlayer, bool bOwnerOnlyInput /* = false */ )
{
	m_hPlayerOwner = pPlayer;

	if ( bOwnerOnlyInput )
	{
		m_fScreenFlags.Set( VGUI_SCREEN_ONLY_USABLE_BY_OWNER );
	}
}

//-----------------------------------------------------------------------------
//JL 
//-----------------------------------------------------------------------------
void CVGuiScreen::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	DevMsg("use VGUI screen\n");
}

//-----------------------------------------------------------------------------
// Precaches a vgui screen
//-----------------------------------------------------------------------------
void PrecacheVGuiScreen( const char *pScreenType )
{
	g_pStringTableVguiScreen->AddString( CBaseEntity::IsServer(), pScreenType );
}


//-----------------------------------------------------------------------------
// Creates a vgui screen, attaches it to another player
//-----------------------------------------------------------------------------
CVGuiScreen *CreateVGuiScreen( const char *pScreenClassname, const char *pScreenType, CBaseEntity *pAttachedTo, CBaseEntity *pOwner, int nAttachmentIndex )
{
	Assert( pAttachedTo );
	CVGuiScreen *pScreen = (CVGuiScreen *)CBaseEntity::Create( pScreenClassname, vec3_origin, vec3_angle, pAttachedTo );

	pScreen->SetPanelName( pScreenType );
	pScreen->FollowEntity( pAttachedTo );
	pScreen->SetOwnerEntity( pOwner );
	pScreen->SetAttachmentIndex( nAttachmentIndex );

	return pScreen;
}

void DestroyVGuiScreen( CVGuiScreen *pVGuiScreen )
{
	if (pVGuiScreen)
	{
		UTIL_Remove( pVGuiScreen );
	}
}

//JL(TH)

BEGIN_DATADESC( CScreenFollow )
	//DEFINE_FUNCTION( DevShotThink_Setup ),
	//DEFINE_FUNCTION( DevShotThink_TakeShot ),
	//DEFINE_FUNCTION( DevShotThink_PostShot ),

	//DEFINE_KEYFIELD( m_iszCameraName,	FIELD_STRING,	"cameraname" ),
	//DEFINE_KEYFIELD( m_iFOV,	FIELD_INTEGER,	"FOV" ),
END_DATADESC()

int CScreenFollow::UpdateTransmitState()
{
	// always transmit if currently used by a monitor
	return SetTransmitState( FL_EDICT_ALWAYS );
}

void CScreenFollow::Setup(CVGuiScreen *screen, bool toScreen)
{
	DevMsg("Setup\n");
	m_pScreen=screen;
	if (m_pScreen)
	{
		m_bToScreen=toScreen;
		CBasePlayer *pPlayer = NULL;

		if (!m_hPlayer.Get())
			m_hPlayer = UTIL_GetLocalPlayer();
		pPlayer = UTIL_GetLocalPlayer();

		if ( !m_hPlayer )
		{
			DispatchUpdateTransmitState();
			return;
		}

		if ( m_hPlayer->IsPlayer() )
		{
			pPlayer = ((CBasePlayer*)m_hPlayer.Get());
		}
		
		if (pPlayer)
		{
			Vector xaxis,yaxis,zaxis;
			Vector pLowerLeft,pUpperLeft,pUpperRight;
			Vector vecOrigin = m_pScreen->GetAbsOrigin();
			AngleVectors( m_pScreen->GetAbsAngles(), &xaxis, &yaxis, &zaxis );
			yaxis *= -1.0f;
			VectorCopy( vecOrigin, pLowerLeft );
			VectorMA( vecOrigin, m_pScreen->m_flHeight, yaxis, pUpperLeft );
			VectorMA( pUpperLeft, m_pScreen->m_flWidth, xaxis, pUpperRight );

			m_vStartPos = pPlayer->EyePosition();
			m_vEndPos = pLowerLeft+(pUpperRight - pLowerLeft)/2.0+zaxis*15.0;
			m_aStartAngle = pPlayer->EyeAngles();
			VectorAngles(-zaxis,yaxis,m_aEndAngle);

			m_fStartTime = gpGlobals->curtime;

			FollowScreen();

			//DevMsg("SetViewEntity\n");
			pPlayer->EnableControl(FALSE);
			pPlayer->AddFlag(FL_VGUISCREEN_CONTROL);
			pPlayer->SetViewEntity( this );

			if (pPlayer->GetActiveWeapon())
			{
				pPlayer->GetActiveWeapon()->Holster();
			}

			DispatchUpdateTransmitState();

			SetNextThink( gpGlobals->curtime );
			SetThink( &CScreenFollow::FollowScreen );
		}
	}
}
void CScreenFollow::FollowScreen()
{
	//DevMsg("FollowScreen\n");
	if (m_pScreen)
	{
		float val = (gpGlobals->curtime-m_fStartTime)/1.0;
		if (val>1.0)
			val=1.0;

		val = Gain(val,0.8); // Smooth
		if (!m_bToScreen)
			val=1.0-val;

		// Enabling/Disabling bloom
		ConVar *pId  = cvar->FindVar( "mat_bloom_enable" );
		if (pId)
			pId->SetValue( (float)(1.0-val) );

		// Enabling/Disabling color correction
		if ( !m_pScreen->HasNoColorCorrection() )
		{
			pId  = cvar->FindVar( "mat_colcorrection_scale" );
			if (pId)
				pId->SetValue( (float)(1.0-val) );
		}

		SetAbsOrigin( m_vStartPos + (m_vEndPos - m_vStartPos)*val );
		SetAbsAngles( Lerp<QAngle>(val,m_aStartAngle,m_aEndAngle) );

		SetNextThink( gpGlobals->curtime );

		if (!m_bToScreen && val<=0.0)
		{
			CBasePlayer *pPlayer = NULL;
			pPlayer = UTIL_GetLocalPlayer();
			if (pPlayer)
			{
				pPlayer->EnableControl(true);
				pPlayer->SetViewEntity(NULL);
				pPlayer->RemoveFlag(FL_VGUISCREEN_CONTROL);
				m_pScreen->m_pFollow=NULL;
				delete this;
			}
		}
	}
}
