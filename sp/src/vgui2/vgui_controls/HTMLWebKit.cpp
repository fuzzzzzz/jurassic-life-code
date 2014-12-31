//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
// This class is a message box that has two buttons, ok and cancel instead of
// just the ok button of a message box. We use a message box class for the ok button
// and implement another button here.
//
// $NoKeywords: $
//=============================================================================//
#include "OfflineMode.h"
#include "vgui/Cursor.h"
#include "vgui/IScheme.h"
#include "vgui/ISystem.h"
#include "vgui/ISurface.h"
#include "vgui/IVGUI.h"
#include "vgui/IBorder.h"
#include "filesystem.h"

#include "vgui_controls/HTMLWebKit.h"
#include "vgui_controls/Controls.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Image.h"
#include "vgui_controls/ScrollBar.h"
#include "KeyValues.h"

#include <../game/client/cdll_client_int.h>

#include <stdio.h>
#include <string>

#include <WebCore.h>

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

#include "tier0/memdbgon.h"

using namespace vgui;
using namespace std;
using namespace Awesomium;

#define DEFAULT_ACTION		"trigger"

void getAsDec(char* hex)
{
	char tmp = tolower(hex[0]);
	if(tmp == 'a') {
		strcpy(hex,"10");
	}else if(tmp == 'b') {
		strcpy(hex,"11");
	}else if(tmp == 'c') {
		strcpy(hex,"12");
	}else if(tmp == 'd') {
		strcpy(hex,"13");
	}else if(tmp == 'e') {
		strcpy(hex,"14");
	}else if(tmp == 'f') {
		strcpy(hex,"15");
	}else if(tmp == 'g') {
		strcpy(hex,"16");
	}
}

int convertToDec(const char* hex)
{
	char buff[12];
	sprintf(buff,"%s",hex);
	int ret = 0;
	int len = strlen(buff);
	for(int i=0;i<len;i++) {
		char tmp[4];
		tmp[0] = buff[i];
		tmp[1] = '\0';
		getAsDec(tmp);
		int tmp_i = atoi(tmp);
		int rs = 1;
		for(int j=i;j<(len-1);j++) {
			rs *= 16;
		}
		ret += (rs * tmp_i);
	}
	return ret;
}

string URLDecoder(string str)
{
	int len = str.length();
	char* buff = new char[len + 1];
	strcpy(buff,str.c_str());
	string ret = "";
	for(int i=0;i<len;i++) {
		if(buff[i] == '+') {
			ret = ret + " ";
		}else if(buff[i] == '%') {
			char tmp[4];
			char hex[4];			
			hex[0] = buff[++i];
			hex[1] = buff[++i];
			hex[2] = '\0';		
			//int hex_i = atoi(hex);
			sprintf(tmp,"%c",convertToDec(hex));
			ret = ret + tmp;
		}else {
			ret = ret + buff[i];
		}
	}
	delete[] buff;
	return ret;
}

// converts the wide char array into a multibyte one
std::string wideCharToMultiByte( const wchar_t *lpwstr)
{
	int size = WideCharToMultiByte( CP_UTF8, 0, lpwstr, -1, NULL, 0, NULL, NULL );

	char *buffer = new char[ size+1];
	WideCharToMultiByte( CP_UTF8, 0, lpwstr, -1, buffer, size, NULL, NULL );

	std::string str( buffer);
	delete []buffer;

	return str;
}

int ButtonCodeToWinCode(int code)
{

/* * VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
 * 0x40 : unassigned
 * VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)*/
	//keybd_event()
	switch (code)
	{
	//Number
	case KEY_0:
		return 0x30;
	case KEY_1:
		return 0x31;
	case KEY_2:
		return 0x32;
	case KEY_3:
		return 0x33;
	case KEY_4:
		return 0x34;
	case KEY_5:
		return 0x35;
	case KEY_6:
		return 0x36;
	case KEY_7:
		return 0x37;
	case KEY_8:
		return 0x38;
	case KEY_9:
		return 0x39;
	//Character
	case KEY_A:
		return 0x41;
	case KEY_B:
		return 0x42;
	case KEY_C:
		return 0x43;
	case KEY_D:
		return 0x44;
	case KEY_E:
		return 0x45;
	case KEY_F:
		return 0x46;
	case KEY_G:
		return 0x47;
	case KEY_H:
		return 0x48;
	case KEY_I:
		return 0x49;
	case KEY_J:
		return 0x4A;
	case KEY_K:
		return 0x4B;
	case KEY_L:
		return 0x4C;
	case KEY_M:
		return 0x4D;
	case KEY_N:
		return 0x4E;
	case KEY_O:
		return 0x4F;
	case KEY_P:
		return 0x50;
	case KEY_Q:
		return 0x51;
	case KEY_R:
		return 0x52;
	case KEY_S:
		return 0x53;
	case KEY_T:
		return 0x54;
	case KEY_U:
		return 0x55;
	case KEY_V:
		return 0x56;
	case KEY_W:
		return 0x57;
	case KEY_X:
		return 0x58;
	case KEY_Y:
		return 0x59;
	case KEY_Z:
		return 0x5A;
	case KEY_PAD_0:
		return VK_NUMPAD0;
	case KEY_PAD_1:
		return VK_NUMPAD1;
	case KEY_PAD_2:
		return VK_NUMPAD2;
	case KEY_PAD_3:
		return VK_NUMPAD3;
	case KEY_PAD_4:
		return VK_NUMPAD4;
	case KEY_PAD_5:
		return VK_NUMPAD5;
	case KEY_PAD_6:
		return VK_NUMPAD6;
	case KEY_PAD_7:
		return VK_NUMPAD7;
	case KEY_PAD_8:
		return VK_NUMPAD8;
	case KEY_PAD_9:
		return VK_NUMPAD9;
	case KEY_PAD_DIVIDE:
		return VK_DIVIDE;
	case KEY_PAD_MULTIPLY:
		return VK_MULTIPLY;
	case KEY_PAD_MINUS:
		return VK_SUBTRACT;
	case KEY_PAD_PLUS:
		return VK_ADD;
	//case KEY_PAD_ENTER:
	//	return ;
	case KEY_PAD_DECIMAL:
		return VK_DECIMAL;
	/*case KEY_LBRACKET:
		return ;
	case KEY_RBRACKET:
		return ;
	case KEY_SEMICOLON:
		return ;
	case KEY_APOSTROPHE:
		return ;
	case KEY_BACKQUOTE:
		return ;
	case KEY_COMMA:
		return ;
	case KEY_PERIOD:
		return ;
	case KEY_SLASH:
		return ;
	case KEY_BACKSLASH:
		return ;
	case KEY_MINUS:
		return ;
	case KEY_EQUAL:
		return ;*/
	case KEY_ENTER:
		return VK_RETURN;
	case KEY_SPACE:
		return VK_SPACE;
	case KEY_BACKSPACE:
		return VK_BACK;
	case KEY_TAB:
		return VK_TAB;
	/*case KEY_CAPSLOCK:
		return ;
	case KEY_NUMLOCK:
		return ;*/
	case KEY_ESCAPE:
		return VK_ESCAPE;
	//case KEY_SCROLLLOCK:
	//	return ;
	case KEY_INSERT:
		return VK_INSERT;
	case KEY_DELETE:
		return VK_DELETE;
	case KEY_HOME:
		return VK_HOME;
	case KEY_END:
		return VK_END;
	case KEY_PAGEUP:
		return VK_PRIOR;
	case KEY_PAGEDOWN:
		return VK_NEXT;
	/*case KEY_BREAK:
		return ;
	case KEY_LSHIFT:
		return VK_SHIFT;
	case KEY_RSHIFT:
		return VK_SHIFT;
	case KEY_LALT:
		return ;
	case KEY_RALT:
		return ;
	case KEY_LCONTROL:
		return ;
	case KEY_RCONTROL:
		return ;
	case KEY_LWIN:
		return ;
	case KEY_RWIN:
		return ;
	case KEY_APP:
		return ;*/
	case KEY_UP:
		return VK_UP;
	case KEY_LEFT:
		return VK_LEFT;
	case KEY_DOWN:
		return VK_DOWN;
	case KEY_RIGHT:
		return VK_RIGHT;
	case KEY_F1:
		return VK_F1;
	case KEY_F2:
		return VK_F2;
	case KEY_F3:
		return VK_F3;
	case KEY_F4:
		return VK_F4;
	case KEY_F5:
		return VK_F5;
	case KEY_F6:
		return VK_F6;
	case KEY_F7:
		return VK_F7;
	case KEY_F8:
		return VK_F8;
	case KEY_F9:
		return VK_F9;
	case KEY_F10:
		return VK_F10;
	case KEY_F11:
		return VK_F11;
	case KEY_F12:
		return VK_F12;
	/*case KEY_CAPSLOCKTOGGLE:
		return ;
	case KEY_NUMLOCKTOGGLE:
		return ;
	case KEY_SCROLLLOCKTOGGLE:
		return ;*/
	}
	return 0;
}

class HTMLListener : public Awesomium::WebViewListener
{
public:
	HTMLListener(HTMLWebKit *panel=NULL)
	{
		Panel=panel;
	}
	virtual void onBeginNavigation(const std::string& url, const std::wstring& frameName)
	{
		if (Panel)
			Panel->EntityParse(url.c_str());
		//DevMsg("Begin Navigation %s\n",url.c_str());
	}
	virtual void onBeginLoading(const std::string& url, const std::wstring& frameName, int statusCode, const std::wstring& mimeType)
	{
		//DevMsg("onBeginLoading\n");
	}
	virtual void onFinishLoading()
	{
		//DevMsg("onFinishLoading\n");
	}
	virtual void onCallback(const std::wstring& objectName, const std::wstring& callbackName, const Awesomium::JSArguments& args)
	{
		DevMsg("onCallback\n");
		if (objectName==L"FireEntity")
		{
			DevMsg("%s\n",args[0].toString().c_str());
			//TO FIX
			if (Panel)
				Panel->EntityParse(wideCharToMultiByte(args[0].toString().c_str()).c_str());
		}
	}
	virtual void onCallback(const std::string& name, const Awesomium::JSArguments& args)
	{
		DevMsg("onCallback\n");
		//FireEntity
		if (name=="FireEntity")
		{
			DevMsg("%s\n",args[0].toString().c_str());
			//TO FIX
			//if (Panel)
			//	Panel->EntityParse(args[0].toString().c_str());
		}
	}
	virtual void onReceiveTitle(const std::wstring& title, const std::wstring& frameName)
	{
		//DevMsg("onReceiveTitle\n");
	}
	virtual void onChangeTooltip(const std::wstring& tooltip)
	{
		//DevMsg("onChangeTooltip\n");
	}

#if defined(_WIN32)
	virtual void onChangeCursor(const HCURSOR& cursor)
	{
		//DevMsg("onChangeCursor\n");
	}
#endif
	virtual void onChangeKeyboardFocus(bool isFocused)
	{
		//DevMsg("onChangeKeyboardFocus\n");
	}
	virtual void onChangeTargetURL(const std::string& url)
	{
		//DevMsg("Change URL\n");
	}

	HTMLWebKit *Panel;
};

//-----------------------------------------------------------------------------
// JL(TH) : Include HTML render with Awesomium (WebKit)
//-----------------------------------------------------------------------------

Awesomium::WebCore* HTMLWebKit::m_pCore=NULL;

DECLARE_BUILD_FACTORY( HTMLWebKit );

HTMLWebKit::HTMLWebKit(Panel *parent,const char *name)
: Panel(parent,name)
{
	if (m_pCore==NULL)
	{
		m_pCore = new WebCore(L"",L"",L"",L"",LOG_NORMAL,true,PF_RGBA);
		string dir = engine->GetGameDirectory();
		dir+="\\html\\";
		m_pCore->setBaseDirectory(dir.c_str());

		m_pCore->setCustomResponsePage(404, "404response.html");
	}
 
	m_pView = m_pCore->createWebView(GetWide(), GetTall(), false, true, 60);

	//m_pView->setProperty("welcomeMsg", "Hey there, thanks for checking out the Awesomium v1.0 Demo.");
	//m_pView->setProperty("renderSystem", Root::getSingleton().getRenderSystem()->getName());
	//m_pView->setCallback("requestFPS");
	//m_pView->setListener(this);
	m_pListener = new HTMLListener(this);
	m_pView->setListener((WebViewListener*)m_pListener);
	//m_pView->setCallback("FireEntity");

	m_pView->setTransparent(true);
	//m_pView->zoomOut();
	//m_pView->zoomOut();
	SetURL("test2.html");

	//
	m_iBufferWidth=2;
	m_iBufferHeight=2;
	while (m_iBufferWidth<GetWide())
		m_iBufferWidth*=2;
	while (m_iBufferHeight<GetTall())
		m_iBufferHeight*=2;
	m_pBuffer = new unsigned char[m_iBufferWidth*m_iBufferHeight*4];

	m_iTextureId = surface()->CreateNewTextureID(true);
	m_bRightClickEnable=false;
	m_fLastRefresh=0;
}

HTMLWebKit::~HTMLWebKit()
{
	m_pView->destroy();
	delete[] m_pBuffer;
	delete m_pListener;
}

void HTMLWebKit::ApplyUserConfigSettings(KeyValues *userConfig)
{
	DevMsg("ApplyUserConfigSettings WebKit\n");
	m_DefaultURL = userConfig->GetString("URL","");
	if (m_DefaultURL!="")
		SetURL(m_DefaultURL.c_str());

	BaseClass::ApplyUserConfigSettings(userConfig);
}

void HTMLWebKit::GetUserConfigSettings(KeyValues *userConfig)
{
	DevMsg("GetUserConfigSettings WebKit\n");
	m_DefaultURL = userConfig->GetString("URL","");
	if (m_DefaultURL!="")
		SetURL(m_DefaultURL.c_str());

	BaseClass::ApplyUserConfigSettings(userConfig);
}

bool HTMLWebKit::HasUserConfigSettings()
{
	DevMsg("HasUserConfigSettings WebKit\n");
	return BaseClass::HasUserConfigSettings();
}

const char* HTMLWebKit::GetDefaultURL()
{
	return m_DefaultURL.c_str();
}

void HTMLWebKit::Paint()
{
	//BaseClass::Paint();
	//m_pCore->update();
	int wide,tall;
	if (Plat_FloatTime()-m_fLastRefresh>=(1.0/30.0))
	{
		m_fLastRefresh = Plat_FloatTime();
		m_pView->render(m_pBuffer,4*m_iBufferWidth,4,0);
		surface()->DrawSetTextureRGBA(m_iTextureId,m_pBuffer,m_iBufferWidth,m_iBufferHeight,false,true);
	}
	surface()->DrawGetTextureSize(m_iTextureId,wide,tall);
	//surface()->DrawSetColor( Color( 0, 0, 0, 255 ) ); 
	//surface()->DrawFilledRect(0,0,GetWide(),GetTall());
	surface()->DrawSetColor( Color( 255, 255, 255, 255 ) ); 
	surface()->DrawSetTexture(m_iTextureId);
	
	float x,y;
	x=(float)GetWide()/wide;
	y=(float)GetTall()/tall;
	//DevMsg("%f %f %d %d %d %d\n",x,y,GetWide(),GetTall(),wide,tall);
	surface()->DrawTexturedSubRect(0,0,GetWide(),GetTall(),0,0,x,y);
}

void HTMLWebKit::OnThink()
{
	m_pCore->update();
}

void HTMLWebKit::OnSizeChanged(int wide,int tall)
{
	BaseClass::OnSizeChanged(wide, tall);
	delete[] m_pBuffer;
	m_iBufferWidth=2;
	m_iBufferHeight=2;
	while (m_iBufferWidth<GetWide())
		m_iBufferWidth*=2;
	while (m_iBufferHeight<GetTall())
		m_iBufferHeight*=2;
	m_pBuffer = new unsigned char[m_iBufferWidth*m_iBufferHeight*4];
	m_pView->resize(wide,tall);
}

void HTMLWebKit::OnMousePressed(MouseCode code)
{
	RequestFocus();
	m_pView->focus();

	MouseButton mouse=LEFT_MOUSE_BTN;
	bool OK=false;
	if (code==MOUSE_LEFT)
	{
		mouse=LEFT_MOUSE_BTN;
		OK=true;
	}else if (code==MOUSE_MIDDLE)
	{
		mouse=MIDDLE_MOUSE_BTN;
		OK=true;
	}else if (code==MOUSE_RIGHT && m_bRightClickEnable)
	{
		mouse=RIGHT_MOUSE_BTN;
		OK=true;
	}
	if (OK)
		m_pView->injectMouseDown(mouse);
}

void HTMLWebKit::OnMouseReleased(MouseCode code)
{
	MouseButton mouse=LEFT_MOUSE_BTN;
	bool OK=false;
	if (code==MOUSE_LEFT)
	{
		mouse=LEFT_MOUSE_BTN;
		OK=true;
	}else if (code==MOUSE_MIDDLE)
	{
		mouse=MIDDLE_MOUSE_BTN;
		OK=true;
	}else if (code==MOUSE_RIGHT && m_bRightClickEnable)
	{
		mouse=RIGHT_MOUSE_BTN;
		OK=true;
	}
	if (OK)
		m_pView->injectMouseUp(mouse);
}

void HTMLWebKit::OnCursorMoved(int x,int y)
{
	m_pView->injectMouseMove(x,y);
}

void HTMLWebKit::OnMouseWheeled(int delta)
{
	m_pView->injectMouseWheel(delta);
}

void HTMLWebKit::OnKeyCodePressed(KeyCode code)
{
	DevMsg("OnKeyCodePressed %d %d\n",code, ButtonCodeToWinCode(code));
	//m_pView->injectKeyboardEvent(0,WM_KEYDOWN,MapVirtualKey(ButtonCodeToWinCode(code), 4),KF_EXTENDED); // MAPVK_VSC_TO_VK_EX
	//m_pView->injectKeyboardEvent(0,WM_IME_KEYDOWN,ButtonCodeToWinCode(code),0); // MAPVK_VSC_TO_VK_EX
	//m_pView->injectKeyboardEvent(0,WM_KEYDOWN,(code),KF_EXTENDED); // MAPVK_VSC_TO_VK_EX
	//m_pView->injectKeyboardEvent(0,WM_SYSKEYDOWN,0x41,0);
	WebKeyboardEvent event;
	event.type = WebKeyboardEvent::TYPE_KEY_DOWN;
	event.virtualKeyCode = ButtonCodeToWinCode(code);
	event.modifiers = 0;
    event.text[0] = event.virtualKeyCode;

	//getKeyIdentifierFromVirtualKeyCode()

	m_pView->injectKeyboardEvent(event);
}

void HTMLWebKit::OnKeyCodeReleased(KeyCode code)
{
	DevMsg("OnKeyCodeReleased %d\n",code);
	//m_pView->injectKeyboardEvent(0,WM_KEYUP,MapVirtualKey(ButtonCodeToWinCode(code), 4),KF_EXTENDED);
	//m_pView->injectKeyboardEvent(0,WM_IME_KEYUP,ButtonCodeToWinCode(code),0);
	//m_pView->injectKeyboardEvent(0,WM_KEYUP,(code),KF_EXTENDED);
	//m_pView->injectKeyboardEvent(0,WM_SYSKEYUP,0x41,KF_EXTENDED);
	WebKeyboardEvent event;
	event.type = WebKeyboardEvent::TYPE_KEY_UP;
	event.virtualKeyCode = ButtonCodeToWinCode(code);
	event.modifiers = 0;
    event.text[0] = event.virtualKeyCode;
	m_pView->injectKeyboardEvent(event);
}

//virtual void OnKeyTyped(wchar_t unichar);
bool HTMLWebKit::EntityParse(const char *urlSrc)
{
	char url[1024];
	char newadr[1024];
	char entity[1024];
	char entity_name[1024];
	char entity_action[1024];

	std::string decode = URLDecoder(urlSrc);
	strcpy_s(url,1024,decode.c_str());

	if(strstr(url,"entity://") == 0)
	{
		return false;
	}

	char* sub = strstr((char*)url,";");
	if (sub)
	{
		strcpy_s(newadr,1024,sub+1);
		strncpy_s(entity,1024,url+strlen("entity://"),sub-url-strlen("entity://")-1);
	}else{
		
		strcpy_s(newadr,1024,"");
		strncpy_s(entity,1024,url+strlen("entity://"),strlen(url)-strlen("entity://"));
	}

	sub = strstr(entity,"->");
	if (sub)
	{
		int pos = sub-entity;
		strncpy_s(entity_name,1024,entity,pos);
		strcpy_s(entity_action,1024,entity+pos+2);
	}else{
		strcpy_s(entity_action,1024,DEFAULT_ACTION);
		strcpy_s(entity_name,1024,entity);
	}

	DevMsg("URL : %s\n",url);
	DevMsg("Entity : %s\n",entity);
	DevMsg("Name : %s\n",entity_name);
	DevMsg("Action : %s\n",entity_action);
	DevMsg("NewURL : %s\n",newadr);
	//return false;
	
	// Open the new url
	if(strcmp(newadr,"")!=0)
	{
		DevMsg(newadr);
		SetURL(newadr);
	}

	// Fire the event
	string command ("ent_fire ");
	command += entity_name;
	command += " ";
	command += entity_action;
	{
		string msg ("Execing command: ");
		msg += command + "\n";
		DevMsg(msg.c_str());
	}
	engine->ServerCmd(command.c_str());

	return true;
}

void HTMLWebKit::SetURL(const char* url)
{
	if (EntityParse(url)==false)
	{
		string addr=url;
		/*if (addr.find("http://"))
			m_pView->loadURL(url);
		else*/
			m_pView->loadFile(url);
	}
}

//m_pView->executeJavascript
