//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: This is an entity that represents a vgui screen
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUISCREEN_H
#define VGUISCREEN_H

#ifdef _WIN32
#pragma once
#endif

//JL
class CVGuiScreen;
class CScreenFollow : public CBaseEntity
{
	DECLARE_CLASS( CScreenFollow, CBaseEntity );
public:
	DECLARE_DATADESC();
	//CScreenFollow(CVGuiScreen *screen=NULL);
	void FollowScreen();
	void Setup(CVGuiScreen *screen,bool toScreen);

	//virtual void Spawn( void );
	// Always transmit to clients so they know where to move the view to
	virtual int UpdateTransmitState();
//protected:
	CVGuiScreen *m_pScreen;
	EHANDLE m_hPlayer;
	float m_fState;
	Vector m_vStartPos;
	Vector m_vEndPos;
	QAngle m_aStartAngle;
	QAngle m_aEndAngle;
	float m_fStartTime;
	bool m_bToScreen;
};

//-----------------------------------------------------------------------------
// This is an entity that represents a vgui screen
//-----------------------------------------------------------------------------
class CVGuiScreen : public CBaseEntity
{
	friend class CScreenFollow;
public:
	DECLARE_CLASS( CVGuiScreen, CBaseEntity );
	
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CVGuiScreen();

	virtual void Precache();
	virtual bool KeyValue( const char *szKeyName, const char *szValue );
	virtual void Spawn();
	virtual void Activate();
	virtual void OnRestore();

	//JL
	virtual void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	const char *GetPanelName() const;

	// Sets the screen size + resolution
	void SetActualSize( float flWidth, float flHeight );

	// Activates/deactivates the screen
	bool IsActive() const;
	void SetActive( bool bActive );

	// Is this screen only visible to teammates?
	bool IsVisibleOnlyToTeammates() const;
	void MakeVisibleOnlyToTeammates( bool bActive );
	bool IsVisibleToTeam( int nTeam );

	// Sets the overlay material
	void SetOverlayMaterial( const char *pMaterial );

	void SetAttachedToViewModel( bool bAttached );
	bool IsAttachedToViewModel() const;

	void SetTransparency( bool bTransparent );

	//JL
	void SetNoColorCorrection( bool bNoColorCorrection );
	bool HasNoColorCorrection();

	virtual int UpdateTransmitState( void );
	virtual int ShouldTransmit( const CCheckTransmitInfo *pInfo );

	void SetPlayerOwner( CBasePlayer *pPlayer, bool bOwnerOnlyInput = false );

	//JL
	void UseFull(bool enable);
	bool IsUseFull();

private:
	void SetAttachmentIndex( int nIndex );
 	void SetPanelName( const char *pPanelName );
	void InputSetActive( inputdata_t &inputdata );
	void InputSetInactive( inputdata_t &inputdata );

	//JL
	void InputSetStates( inputdata_t &inputdata );
	void InputSetStateTrue( inputdata_t &inputdata );
	void InputSetStateFalse( inputdata_t &inputdata );
	void InputUseFull( inputdata_t &inputdata );


	string_t m_strOverlayMaterial;
public:
	CNetworkVar( float, m_flWidth ); 
	CNetworkVar( float, m_flHeight );
	CScreenFollow *m_pFollow;
private:		
	CNetworkVar( int, m_nPanelName );	// The name of the panel 
	CNetworkVar( int, m_nAttachmentIndex );
	CNetworkVar( int, m_nOverlayMaterial );
	CNetworkVar( int, m_fScreenFlags );
	CNetworkVar( EHANDLE, m_hPlayerOwner );

	//JL(TH)
	CNetworkVar( int, m_iCode );
	CNetworkVar( string_t, m_sVar1 );
	CNetworkVar( string_t, m_sVar2 );
	CNetworkVar( bool, m_bUseFull );
	CNetworkVar( unsigned int, m_iState );

	

	friend CVGuiScreen *CreateVGuiScreen( const char *pScreenClassname, const char *pScreenType, CBaseEntity *pAttachedTo, CBaseEntity *pOwner, int nAttachmentIndex );
};


void PrecacheVGuiScreen( const char *pScreenType );
void PrecacheVGuiScreenOverlayMaterial( const char *pMaterialName );
CVGuiScreen *CreateVGuiScreen( const char *pScreenClassname, const char *pScreenType, CBaseEntity *pAttachedTo, CBaseEntity *pOwner, int nAttachmentIndex );
void DestroyVGuiScreen( CVGuiScreen *pVGuiScreen );


#endif // VGUISCREEN_H
