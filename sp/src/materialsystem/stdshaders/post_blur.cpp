#include "BaseVSShader.h"  
#include "post_blur_ps20.inc" 
#include "SDK_screenspaceeffect_vs20.inc" 

static ConVar jl_post_blur_power( "jl_post_blur_power", "0.003", FCVAR_CHEAT);

BEGIN_VS_SHADER( Post_Blur, "Help for Post_Blur" ) 

BEGIN_SHADER_PARAMS 
SHADER_PARAM( FBTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "_rt_FullFrameFB", "" )
//_rt_Fullscreen
//_rt_FullFrameDepth
//_rt_SmallFB0
END_SHADER_PARAMS 

// Set up anything that is necessary to make decisions in SHADER_FALLBACK. 
SHADER_INIT_PARAMS() 
{ 
} 

SHADER_FALLBACK 
{ 
	return 0; 
} 

SHADER_INIT 
{
	if( params[FBTEXTURE]->IsDefined() ) 
	{ 
		LoadTexture( FBTEXTURE ); 
	} 
} 

SHADER_DRAW 
{ 
	//First pass
	SHADOW_STATE 
	{ 
		// Reset shadow state manually since we're drawing from two materials
		//SetInitialShadowState( );
		pShaderShadow->EnableDepthWrites( false ); 

		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true ); 	

		int fmt = VERTEX_POSITION; 

		int texCoordDimensions [] = { 2 };
		pShaderShadow->VertexShaderVertexFormat( fmt, 1, texCoordDimensions, 0 );

		DECLARE_STATIC_PIXEL_SHADER( post_blur_ps20 ); 
		SET_STATIC_PIXEL_SHADER( post_blur_ps20 ); 

		DECLARE_STATIC_VERTEX_SHADER( sdk_screenspaceeffect_vs20 ); 
		SET_STATIC_VERTEX_SHADER( sdk_screenspaceeffect_vs20 ); 
	} 

	DYNAMIC_STATE 
	{ 
		BindTexture( SHADER_SAMPLER0, FBTEXTURE, -1 );

		float value = jl_post_blur_power.GetFloat();
		pShaderAPI->SetPixelShaderConstant(1,&value);

		DECLARE_DYNAMIC_PIXEL_SHADER( post_blur_ps20 ); 
		SET_DYNAMIC_PIXEL_SHADER( post_blur_ps20 ); 

		DECLARE_DYNAMIC_VERTEX_SHADER( sdk_screenspaceeffect_vs20 ); 
		SET_DYNAMIC_VERTEX_SHADER( sdk_screenspaceeffect_vs20 ); 
	}
	Draw();
	} 
END_SHADER 
