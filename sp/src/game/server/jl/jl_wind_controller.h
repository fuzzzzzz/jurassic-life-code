//=====================================================================================================================================

// JURASSIC LIFE WIND CONTROLLER

//===================================================================================================================================

#ifdef _WIN32
#pragma once
#endif

class CJuraLifeWindController : public CLogicalEntity {

public:
	DECLARE_CLASS( CJuraLifeWindController, CLogicalEntity );

	DECLARE_DATADESC();

	CJuraLifeWindController(){
          
		m_bWind = false;
		m_iDirection = 0;
		m_iIntensity = 0;
          
	}

	void Spawn( void );
	void Activate(){ BaseClass::Activate(); }
	void AddToWindList( CBaseEntity *pOther ){ m_vecFoliage.AddToTail( pOther ); }

	void InputToggleWind(inputdata_t &inputdata){ m_bWind = !m_bWind; UpdateFoliage(); }
	void InputStartWind(inputdata_t &inputdata){ m_bWind = true; UpdateFoliage(); }
	void InputEndWind(inputdata_t &inputdata){ m_bWind = false; UpdateFoliage(); }
	void InputChangeDirection(inputdata_t &inputdata){ m_iDirection = inputdata.value.Int(); UpdateFoliage(); }
	void InputChangeIntensity(inputdata_t &inputdata){ m_bWind = inputdata.value.Int(); UpdateFoliage(); }

	void UpdateFoliage();

	int GetIntensity(){ return m_iIntensity; }
	int GetDirection(){ return m_iDirection; }
	bool GetShouldAnimate(){ return m_bWind; }

private:

	bool m_bWind;
	int m_iDirection;
	int m_iIntensity;

		CUtlVector <CBaseEntity *>m_vecFoliage;

};

CJuraLifeWindController *GP_JL_GetWindController();