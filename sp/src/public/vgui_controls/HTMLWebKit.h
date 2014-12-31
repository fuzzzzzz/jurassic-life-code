//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Creates a HTML control
//
// $NoKeywords: $
//=============================================================================//

#ifndef HTMLWEBKIT_H
#define HTMLWEBKIT_H

#ifdef _WIN32
#pragma once
#endif

//JL(TH) : Add HTML WebKit render headers
//#include <WebCore.h>
//#include <WebView.h>

#include <vgui/VGUI.h>
#include <vgui/IHTML.h>
#include <vgui/IImage.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/PHandle.h>

#include <string>

#define ASYNC_RENDER 1
#define SUPER_QUALITY 0

namespace Awesomium
{
	class WebView;
	class WebCore; 
}

namespace vgui
{

//JL(TH) : Mozilla HTML render
class HTMLWebKit : public Panel
{
	DECLARE_CLASS_SIMPLE( HTMLWebKit, Panel );
public:
	HTMLWebKit(Panel *parent,const char *name);
	~HTMLWebKit();

	// user configuration
	virtual void ApplyUserConfigSettings(KeyValues *userConfig);
	virtual void GetUserConfigSettings(KeyValues *userConfig);
	virtual bool HasUserConfigSettings();

	virtual void Paint();
	virtual void OnThink();
	virtual void OnSizeChanged(int wide,int tall);

	virtual void OnMousePressed(MouseCode code);
	virtual void OnMouseReleased(MouseCode code);
	virtual void OnCursorMoved(int x,int y);
	virtual void OnMouseWheeled(int delta);

	virtual void OnKeyCodePressed(KeyCode code);
	virtual void OnKeyCodeReleased(KeyCode code);

	void SetURL(const char* url);
	const char * GetDefaultURL();

	bool EntityParse(const char *url);
private:
	std::string m_DefaultURL;
	static Awesomium::WebCore* m_pCore; 
	Awesomium::WebView* m_pView;
	unsigned char *m_pBuffer;
	int m_iBufferWidth;
	int m_iBufferHeight;
	int m_iTextureId;

	bool m_bRightClickEnable;
	void *m_pListener;

	double m_fLastRefresh;
};

} // namespace vgui

#endif // HTML_H
