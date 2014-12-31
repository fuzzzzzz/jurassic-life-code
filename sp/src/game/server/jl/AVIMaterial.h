// Nate Nichols, InfoLab Northwestern University, August 2006.

#ifndef AVI_MATERIAL_H
#define AVI_MATERIAL_H


//This is the server side of the entity.  The corresponding client entity is C_AVIMaterial.
class CAVIMaterial : public CBaseEntity
{
public:
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_CLASS( CAVIMaterial, CBaseEntity );
	CAVIMaterial();
	void Spawn();
	void Play(inputdata_t &input);
	void Play();
	void Pause(inputdata_t &input);
	void SetMovie(inputdata_t &input);
	void AdvanceFrame(inputdata_t &input);

private:
	CNetworkVar(int, m_iPlay);
	CNetworkVar(bool, m_bLoop);
	CNetworkVar(int, m_iAdvanceFrame);
	CNetworkVar(string_t, m_iszTextureName);
	CNetworkVar(string_t, m_iszMovieName);	
};

LINK_ENTITY_TO_CLASS( AVIMaterial, CAVIMaterial );

BEGIN_DATADESC(CAVIMaterial)
	DEFINE_FIELD( m_iPlay, FIELD_INTEGER ),
	DEFINE_FIELD( m_bLoop, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iAdvanceFrame, FIELD_INTEGER ),
	DEFINE_KEYFIELD( m_iszTextureName, FIELD_STRING, "TextureName" ),
	DEFINE_KEYFIELD( m_iszMovieName, FIELD_STRING, "MovieName" ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Play", Play ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Pause", Pause ),
	DEFINE_INPUTFUNC( FIELD_VOID, "AdvanceFrame", AdvanceFrame ),
	DEFINE_INPUTFUNC( FIELD_STRING, "SetMovie", SetMovie ), 
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CAVIMaterial, DT_AVIMaterial )
	SendPropInt( SENDINFO( m_iPlay ), 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_bLoop ), 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iAdvanceFrame ), 1, SPROP_UNSIGNED ),
	SendPropStringT( SENDINFO(m_iszTextureName)),
	SendPropStringT( SENDINFO(m_iszMovieName)),
END_SEND_TABLE()


#endif //AVI_MATERIAL_H