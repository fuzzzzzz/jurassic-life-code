//-----------------------------------------------------------------------------
// Purpose: Compares a single string input to up to 16 case values, firing an
//			output corresponding to the case value that matched, or a default
//			output if the input value didn't match any of the case values.
//
//			This can also be used to fire a random output from a set of outputs.
//-----------------------------------------------------------------------------
#define MAX_LOGIC_CASES 16

class CLogicCase : public CLogicalEntity
{
	DECLARE_CLASS( CLogicCase, CLogicalEntity );
private:
	string_t m_nCase[MAX_LOGIC_CASES];

	int m_nShuffleCases;
	int m_nLastShuffleCase;
	unsigned char m_uchShuffleCaseMap[MAX_LOGIC_CASES];

	void Spawn(void);

	int BuildCaseMap(unsigned char *puchMap);

public:
	// Inputs
	void InputValue( inputdata_t &inputdata );
	void InputPickRandom( inputdata_t &inputdata );
	void InputPickRandomShuffle( inputdata_t &inputdata );

	// Outputs
	COutputEvent m_OnCase[MAX_LOGIC_CASES];		// Fired when the input value matches one of the case values.
	COutputVariant m_OnDefault;					// Fired when no match was found.

	DECLARE_DATADESC();
};
