
#ifndef IMAGEBUTTON_H
#define IMAGEBUTTON_H
 
#ifdef _WIN32
#pragma once
#endif
 

#include <vgui/VGUI.h>
#include "vgui/MouseCode.h"
#include <vgui_controls/Label.h>
#include <vgui_controls/Label.h>
 
namespace vgui
{
 
class ImageButton : public vgui::Label
{
	DECLARE_CLASS_SIMPLE( ImageButton, vgui::Label );
 
public:
	struct ImageUV
	{
		ImageUV()
		{
			m_iTexture = 0;
			m_fUV[0] = m_fUV[1] = 0.f;
			m_fUV[2] = m_fUV[3] = 1.f;
		}
		int m_iTexture;
		float m_fUV[4];
	};
	//ImageButton( Panel *parent, const char *panelName, const char *normalImage, const char *mouseOverImage = NULL, const char *mouseClickImage = NULL, const char *pCmd=NULL ); //: ImagePanel( parent, panelName );
	ImageButton( Panel *parent, const char *panelName);
	~ImageButton();
 
	virtual void PaintBackground();
	virtual void OnCursorEntered(); // When the mouse hovers over this panel, change images
	virtual void OnCursorExited(); // When the mouse leaves this panel, change back
 
	virtual void OnMouseReleased( vgui::MouseCode code );
 
	virtual void OnMousePressed( vgui::MouseCode code );

	virtual void ApplySettings(KeyValues *inResourceData);

	void SetNormalImage( void );
	void SetMouseOverImage( void );
	void SetMouseClickImage( void );
	void SetDisableImage( void );

	void SetEnable(bool enable);
	bool IsEnable();
protected:
	void SetCommand(const char *cmd);
	void SetCommandPressed(const char *cmd);
	void SetCommandReleased(const char *cmd);

	void GetImage(ImageUV& oImage, KeyValues* pValues, const char* pName ); 
private:
 	bool m_bEnable;
	bool m_bOver;
	char *m_pCommand; // The command when it is clicked on
	char *m_pCommandPressed; // The command when it is clicked on
	char *m_pCommandReleased; // The command when it is clicked on

	ImageUV m_oNormalImage;
	ImageUV m_oOverImage;
	ImageUV m_oClickImage;
	ImageUV m_oDisableImage;

	ImageUV* m_pImage;

	bool m_bClicked;
 
	virtual void SetImage( ImageUV *image ); //Private because this really shouldnt be changed
};
 
} //namespace vgui
 
#endif //IMAGEBUTTON_H