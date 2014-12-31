//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
// fish.h
// Simple fish behavior
// Author: Michael S. Booth, April 2005

#ifndef _FLOCK_FAUNA_H_
#define _FLOCK_FAUNA_H_

#include "baseanimating.h"
#include "GameEventListener.h"
#include "props.h"

class CFlockPool;

//----------------------------------------------------------------------------------------------
/**
 * Simple ambient fish
 */
class CFlockFauna : public CDynamicProp //CBaseAnimating
{
public:
	DECLARE_CLASS( CFlockFauna, CDynamicProp ); // CBaseAnimating );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CFlockFauna( void );
	virtual ~CFlockFauna();

	void Initialize( CFlockPool *pool, unsigned int id );
	
	virtual void Precache();
	virtual void Spawn( void );

	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual void Touch( CBaseEntity *other );			///< in contact with "other"

	void Update( float deltaT );						///< invoked each server tick

	void FlockTo( CFlockFauna *other, float amount );			///< influence my motion to flock with other nearby fish
	float Avoid( void );
	void Panic( void );									///< panic for awhile

	void ResetVisible( void );							///< zero the visible vector
	void AddVisible( CFlockFauna *fauna );						///< add this fish to our visible vector

/*	bool IsSeagull( void );
	bool IsCrow( void );
	bool IsButterfly( void );

*/
	bool m_bSeagull;
	bool m_bCrow;
	bool m_bButterfly;


private:
	friend void SendProxy_FishOriginX( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID );
	friend void SendProxy_FishOriginY( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID );
	friend void SendProxy_FishOriginZ( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID );

	CHandle<CFlockPool> m_pool;							///< the pool we are in
	//CHandle<CFlockPool> m_fSpeed;							///< the pool we are in
	unsigned int m_id;									///< our unique ID

	CNetworkVar( float, m_x );							///< have to send position coordinates separately since Z is unused
	CNetworkVar( float, m_y );							///< have to send position coordinates separately since Z is unused
	CNetworkVar( float, m_z );							///< only sent once since fish always swim at the same depth

	CNetworkVar( float, m_angle );						///< only yaw changes
	float m_angleChange;
	Vector m_forward;
	Vector m_perp;

	
public:

	CNetworkVar( Vector, m_poolOrigin );				///< used to efficiently network our relative position
	CNetworkVar( float, m_waterLevel );

private:

	float m_speed;
	float m_desiredSpeed;

	float m_calmSpeed;									///< speed the fish moves when calm
	float m_panicSpeed;									///< speed the fish moves when panicked

	float m_avoidRange;									///< range to avoid obstacles

	CountdownTimer m_turnTimer;							///< every so often our turn preference changes
	bool m_turnClockwise;								///< if true this fish prefers to turn clockwise, else CCW
	
	CountdownTimer m_goTimer;							///< start the fish moving when timer elapses
	CountdownTimer m_moveTimer;							///< dont decay speed while we are moving
	CountdownTimer m_panicTimer;						///< if active, fish is panicked
	CountdownTimer m_disperseTimer;						///< initial non-flocking time

	CUtlVector< CFlockFauna * > m_visible;					///< vector of fish that we can see
};


//----------------------------------------------------------------------------------------------
/**
 * This class defines a volume of air where a number of CFlockFauna move.
 */
class CFlockPool : public CBaseEntity, public CGameEventListener
{
public:
	DECLARE_CLASS( CFlockPool, CBaseEntity );
	DECLARE_DATADESC();

	CFlockPool( void );

	virtual void Spawn();

	virtual bool KeyValue( const char *szKeyName, const char *szValue );

	virtual void FireGameEvent( IGameEvent *event );

	void Update( void );					///< invoked each server tick

	float GetWaterLevel( void ) const;		///< return Z coordinate of water in world coords
	float GetMaxRange( void ) const;		///< return how far a fish is allowed to wander
//	float GetFaunaSpeed( float *fSpeed ) const;		//	JL (Red)

//protected:
	float m_fSpeed;						// JL(Red)



	bool IsSeagull( void );
	bool IsCrow( void );
	bool IsButterfly( void );


	bool m_bSeagull;
	bool m_bCrow;
	bool m_bButterfly;

private:
	int m_faunaCount;						///< number of fish in the pool
	
	float m_maxRange;						///< how far a fish is allowed to wander
	float m_airDepth;						///< the depth the fish swim below the water surface

	float m_waterLevel;						///< Z of water surface

	bool m_isDormant;

	CUtlVector< CHandle<CFlockFauna> > m_fauna;	///< vector of all fish in this pool

	CountdownTimer m_visTimer;				///< for throttling line of sight checks between all fish
};


inline float CFlockPool::GetMaxRange( void ) const
{
	return m_maxRange;
}

inline float CFlockPool::GetWaterLevel( void ) const
{
	return m_waterLevel;
}
/*
inline float CFlockPool::GetFaunaSpeed( float *fSpeed ) const
{
	return m_fSpeed;
}
*/
#endif // _FISH_H_

