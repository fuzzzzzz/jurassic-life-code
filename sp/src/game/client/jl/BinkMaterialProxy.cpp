#include "cbase.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialproxy.h"
#include "materialsystem/ITexture.h"
#include "materialsystem/IMaterialVar.h"
#include "pixelwriter.h"
#include "avi/ibik.h"



class CBinkTextureRegenerator : public ITextureRegenerator
{
public:
	CBinkTextureRegenerator( void );
	virtual void RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect );
	virtual void Release( void );

	virtual void LoadBink(char *pName);

private:
	BIKMaterial_t m_hBIKMaterial;
	IMaterial *m_material;

	int m_numFrames;

	int CurrentFrame;
};


CBinkTextureRegenerator::CBinkTextureRegenerator()
{
	m_hBIKMaterial = BIKMATERIAL_INVALID;
	CurrentFrame=0;
	m_numFrames=0;
}

void CBinkTextureRegenerator::LoadBink(char *pName)
{
	if (pName!=NULL)
	{
		const char *pExt = Q_GetFileExtension( pName );
		bool bIsBIK = pExt && !Q_stricmp( pExt, "bik" );
		if ( bIsBIK )
		{
			m_hBIKMaterial = bik->CreateMaterial( pName, pName, "GAME" );

			if (m_hBIKMaterial != BIKMATERIAL_INVALID )
			{
				m_material = bik->GetMaterial( m_hBIKMaterial );
				//bik->GetFrameSize( m_hBIKMaterial, &m_width, &m_height );
				m_numFrames = bik->GetFrameCount( m_hBIKMaterial );
			}
		}

		//m_material->
	}
}
void CBinkTextureRegenerator::RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect )
{
	if (m_hBIKMaterial != BIKMATERIAL_INVALID )
	{
		if (CurrentFrame>m_numFrames)
			CurrentFrame=0;
		
		bik->SetFrame(m_hBIKMaterial,CurrentFrame);
		bik->Update(m_hBIKMaterial);


		CPixelWriter pixelWriter;
		pixelWriter.SetPixelMemory( pVTFTexture->Format(), 
			pVTFTexture->ImageData( 0, 0, 0 ), pVTFTexture->RowSizeInBytes( 0 ) );

		// Now upload the part we've been asked for
		int xmax = pSubRect->x + pSubRect->width;
		int ymax = pSubRect->y + pSubRect->height;
		int x, y;

		for( y = pSubRect->y; y < ymax; ++y )
		{
			pixelWriter.Seek( pSubRect->x, y );

			for( x=pSubRect->x; x < xmax; ++x )
			{
				pixelWriter.WritePixel( y%256, 0, 0, 255 );
			}
		}
		CurrentFrame++;
	}
}

void CBinkTextureRegenerator::Release()
{
	//delete stuff
}



class CBinkTextureProxy: public IMaterialProxy
{
public:
	CBinkTextureProxy();
	virtual ~CBinkTextureProxy();
	virtual bool Init( IMaterial* pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( void *pC_BaseEntity );
	virtual void Release( void ) { delete this; }

	virtual IMaterial *	GetMaterial() { return m_pMaterial; };

private:
	IMaterialVar			*m_pTextureVar;   // The material variable
	ITexture				*m_pTexture;      // The texture
	CBinkTextureRegenerator	*m_pTextureRegen; // The regenerator
	IMaterial				*m_pMaterial;
};

CBinkTextureProxy::CBinkTextureProxy()
{
	m_pTextureVar = NULL;
	m_pTexture = NULL;
	m_pTextureRegen = NULL;
	m_pMaterial = NULL;
}

CBinkTextureProxy::~CBinkTextureProxy()
{
	if (m_pTexture != NULL)
	{
		m_pTexture->SetTextureRegenerator( NULL );
	}
}

bool CBinkTextureProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	bool found;
	
	m_pMaterial=pMaterial;

	m_pTextureVar = pMaterial->FindVar("$basetexture", &found, false);  // Get a reference to our base texture variable
	if( !found )
	{
		m_pTextureVar = NULL;
		return false;
	}

	m_pTexture = m_pTextureVar->GetTextureValue();  // Now grab a ref to the actual texture
	if (m_pTexture != NULL)
	{
		m_pTextureRegen = new CBinkTextureRegenerator( );  // Here we create our regenerator
		m_pTexture->SetTextureRegenerator(m_pTextureRegen); // And here we attach it to the texture.
	}

	m_pTextureVar = m_pMaterial->FindVar( "$basevideo", &found );

	if (!found)
	{
		m_pTextureVar = NULL;
		return false;
	}

	//m_pTextureRegen->LoadBink(m_pTextureVar->GetStringValue());

	return true;
}

void CBinkTextureProxy::OnBind( void *pEntity )
{
	if( !m_pTexture )  // Make sure we have a texture
		return;

	if(!m_pTextureVar->IsTexture())  // Make sure it is a texture
		return;

	m_pTexture->Download(); // Force the regenerator to redraw
}


EXPOSE_INTERFACE( CBinkTextureProxy, IMaterialProxy, "Bink" IMATERIAL_PROXY_INTERFACE_VERSION );



