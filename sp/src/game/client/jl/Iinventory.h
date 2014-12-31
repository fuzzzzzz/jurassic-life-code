
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/imagepanel.h>
#include <vgui/ISurface.h>
#include <vgui/IInput.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ListPanel.h>
#include <vgui_controls/Button.h>

#include "vgui/ILocalize.h"


// IInventory.h
class IInventory
{
public:
	virtual void		Create( vgui::VPANEL parent ) = 0;
	virtual void		Destroy( void ) = 0;
};

extern IInventory* inventory;

class CClass3DModelZone : public vgui::ImagePanel
{
    public:
        typedef vgui::ImagePanel BaseClass;

		CClass3DModelZone( vgui::Panel *pParent, const char *pName );
        virtual ~CClass3DModelZone();
        virtual void ApplySettings( KeyValues *inResourceData );
		virtual void Paint();
};

class CInventory : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CInventory, vgui::Frame); 
	//CInventory : This Class / vgui::Frame : BaseClass

	CInventory(vgui::VPANEL parent); 	// Constructor
	~CInventory();	// Destructor
	
	void	PostRenderVGui();
	void	SetSelecteditem(int i);

	void	PrintPosition();
protected:
	//VGUI overrides:
	virtual void OnTick();
	virtual void OnCommand(const char* pcCommand);
	void	Paint( void );

	Panel	*CreateControlByName(const char *controlName);
	//void	PaintBuildOverlay();

	void	UseItem(int i);

	void	ShowNote();
	void	ShowItem();
	void	ShowMap();
	void	UpdateNotes();

	void Center()
	{
		int x,w,h; 
		GetBounds(x,x,w,h);
		SetPos(0.5f*(ScreenWidth()-w),0.5f*(ScreenHeight()-h));
	}

	void	GetSelectedNote();
	MESSAGE_FUNC( OnItemSelected, "ItemSelected" )
	{
		GetSelectedNote();
	}
private:
	void UpdateInventory();
private:
	bool m_bIsInNoteTab;
	vgui::ListPanel *list;
	vgui::ImageList *m_pImageList;
	//Other used VGUI control Elements:
	const model_t *pModel;
	C_BaseAnimating *pAnimModel;
	CClass3DModelZone *m_p3DModelZone;

	int m_TCase;

	float m_fZoom;
	float m_fLeft;
	float m_fTop;
	float m_fx;
	float m_fy;
	float m_fz;

	int selecteditem;

	int m_i3Dx, m_i3Dy, m_i3Dwidth, m_i3Dheight;

	int Texture;

	//int sx,sy,sw,sh,si;

	bool m_bZoomMore;
	bool m_bZoomLess;
	bool m_bMoveUp;
	bool m_bMoveDown;
	bool m_bMoveLeft;
	bool m_bMoveRight;
};

extern CUtlVector<CInventory*> g_ClassInventory;

