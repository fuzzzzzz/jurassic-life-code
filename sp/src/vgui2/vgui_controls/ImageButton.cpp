
#include <assert.h>
#include <math.h>
#include <stdio.h>

#include <vgui_controls/ImageButton.h>
#include <vgui_controls/Controls.h>

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>
 
using namespace vgui;

//DECLARE_BUILD_FACTORY_DEFAULT_TEXT( ImageButton, ImageButton );
DECLARE_BUILD_FACTORY( ImageButton );
 
ImageButton::ImageButton( Panel *parent, const char *panelName) //, const char *normalImage, const char *mouseOverImage, const char *mouseClickImage, const char *pCmd ) 
: Label( parent, panelName, "") 
{
	SetParent(parent);

	m_pImage=NULL;

	m_pCommand=NULL;
	m_pCommandPressed=NULL;
	m_pCommandReleased=NULL;
	
	m_bEnable=true;
	m_bOver=false;

	SetNormalImage();
}

ImageButton::~ImageButton()
{
	SetCommand(NULL);
	SetCommandPressed(NULL);
	SetCommandReleased(NULL);
}

void ImageButton::SetImage(ImageUV *image)
{
	m_pImage = image;
	Repaint();
}

void ImageButton::PaintBackground()
{
	if (m_pImage && m_pImage->m_iTexture)
	{
		surface()->DrawSetColor( 255, 255, 255, 255 );			
		surface()->DrawSetTexture(m_pImage->m_iTexture);
		surface()->DrawTexturedSubRect( 0, 0, GetWide(), GetTall(), 
			m_pImage->m_fUV[0], m_pImage->m_fUV[1], m_pImage->m_fUV[2] ,m_pImage->m_fUV[3] );
	}
}
//-----------------------------------------------------------------------------
// Purpose: Get control settings for editing
//-----------------------------------------------------------------------------
void ImageButton::ApplySettings( KeyValues *inResourceData )
{
	GetImage( m_oNormalImage, inResourceData, "Image" );
	GetImage( m_oOverImage, inResourceData, "ImageOver" );
	GetImage( m_oClickImage, inResourceData, "ImageClick");
	GetImage( m_oDisableImage, inResourceData, "ImageDisable" );
	SetNormalImage();

	SetCommand( inResourceData->GetString("command") );
	SetCommandPressed( inResourceData->GetString("commandpressed") );
	SetCommandReleased( inResourceData->GetString("commandreleased") );

	SetEnable(inResourceData->GetInt("Enabled",1));

	BaseClass::ApplySettings(inResourceData);
}

void ImageButton::GetImage(ImageUV& oImage, KeyValues* pValues, const char* pName )
{
	if ( pValues->GetDataType(pName) == KeyValues::TYPE_WSTRING )
	{
		oImage.m_iTexture = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(oImage.m_iTexture,pValues->GetString(pName),true,false);
	}else{
		KeyValues* pSub = pValues->FindKey(pName);
		if (pSub)
		{
			oImage.m_iTexture = vgui::surface()->CreateNewTextureID();
			vgui::surface()->DrawSetTextureFile(oImage.m_iTexture,pSub->GetString("Image"),true,false);

			int wide, tall;
			vgui::surface()->DrawGetTextureSize(oImage.m_iTexture, wide, tall );
			oImage.m_fUV[0] = pSub->GetInt( "X", 0 ) / (float)wide;
			oImage.m_fUV[1] = pSub->GetInt( "Y", 0 ) / (float)tall;
			oImage.m_fUV[2] = oImage.m_fUV[0] + pSub->GetInt( "Width", wide ) / (float)wide;
			oImage.m_fUV[3] = oImage.m_fUV[1] + pSub->GetInt( "Height", tall ) / (float)tall;
		}
	}
}
 
void ImageButton::OnCursorEntered()
{
	if (IsEnable())
	{
		m_bOver = true;
		SetMouseOverImage();
	}
}
 
void ImageButton::OnCursorExited()
{
	if (IsEnable())
	{
		if (m_bClicked)
		{
			m_bClicked=false;
			if (GetParent())
			{
				GetParent()->OnCommand(m_pCommandReleased);
			}
		}

		m_bOver = false;
		SetNormalImage();
	}
}
 
void ImageButton::OnMouseReleased( vgui::MouseCode code )
{
	if (IsEnable())
	{
		m_bClicked=false;
		if (GetParent())
		{
			GetParent()->OnCommand(m_pCommand);
			GetParent()->OnCommand(m_pCommandReleased);
		}
	 
		if ( ( code == MOUSE_LEFT ) )
		{
			if (m_bOver)
				SetMouseOverImage();
			else
				SetNormalImage();
		}
	}
}
 
void ImageButton::OnMousePressed( vgui::MouseCode code )
{
	if (IsEnable())
	{
		m_bClicked=true;
		if (GetParent())
		{
			GetParent()->OnCommand(m_pCommandPressed);
		}

		if ( ( code == MOUSE_LEFT ) )
			SetMouseClickImage();
	}
}
 
void ImageButton::SetNormalImage( void )
{
	if (m_oNormalImage.m_iTexture != 0)
	{
		SetImage(&m_oNormalImage);
		Repaint();
	}
}
 
void ImageButton::SetMouseOverImage( void )
{
	if (m_oOverImage.m_iTexture != 0)
	{
		SetImage(&m_oOverImage);
		Repaint();
	}
}
 
void ImageButton::SetMouseClickImage( void )
{
	if (m_oClickImage.m_iTexture != 0)
	{
		SetImage(&m_oClickImage);
		Repaint();
	}
}
 
void ImageButton::SetDisableImage()
{
	if (m_oDisableImage.m_iTexture != 0)
	{
		SetImage(&m_oDisableImage);
		Repaint();
	}
}

void ImageButton::SetCommand(const char *cmd)
{
	if (m_pCommand)
	{
		delete[] m_pCommand;
		m_pCommand=NULL;
	}
	if (cmd)
	{
		m_pCommand = new char[strlen(cmd)+1];
		strcpy(m_pCommand,cmd);
	}
}

void ImageButton::SetCommandPressed(const char *cmd)
{
	if (m_pCommandPressed)
	{
		delete[] m_pCommandPressed;
		m_pCommandPressed=NULL;
	}
	if (cmd)
	{
		m_pCommandPressed = new char[strlen(cmd)+1];
		strcpy(m_pCommandPressed,cmd);
	}
}

void ImageButton::SetCommandReleased(const char *cmd)
{
	if (m_pCommandReleased)
	{
		delete[] m_pCommandReleased;
		m_pCommandReleased=NULL;
	}
	if (cmd)
	{
		m_pCommandReleased = new char[strlen(cmd)+1];
		strcpy(m_pCommandReleased,cmd);
	}
}

void ImageButton::SetEnable(bool enable)
{
	if (m_bEnable!=enable)
	{
		m_bEnable=enable;
		SetDisableImage();
		m_bClicked=false;
	}
}

bool ImageButton::IsEnable()
{
	return m_bEnable;
}
