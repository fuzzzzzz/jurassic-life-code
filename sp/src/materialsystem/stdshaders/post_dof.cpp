#include "BaseVSShader.h"  
#include "post_dof_ps20.inc"
#include "SDK_screenspaceeffect_vs20.inc" 

static ConVar jl_post_dof_power( "jl_post_dof_power", "0.003", FCVAR_SPONLY );
//static ConVar jl_post_dof_pass( "jl_post_dof_pass", "1", FCVAR_SPONLY );
static ConVar jl_post_dof_min( "jl_post_dof_min", "0.1", FCVAR_SPONLY );
static ConVar jl_post_dof_max( "jl_post_dof_max", "1", FCVAR_SPONLY );
static ConVar jl_post_dof_delta( "jl_post_dof_delta", "0.2", FCVAR_SPONLY );

static ConVar jl_post_dof_debug( "jl_post_dof_debug", "0", FCVAR_SPONLY );

BEGIN_VS_SHADER( Post_DOF, "Help for Post_DOF" ) 

BEGIN_SHADER_PARAMS 
SHADER_PARAM( FBTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "_rt_FullFrameFB1", "" ) 
SHADER_PARAM( BLURTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "_rt_FullFrameFB", "" )

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
	if( params[BLURTEXTURE]->IsDefined() ) 
	{ 
		LoadTexture( BLURTEXTURE ); 
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
		pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
		//pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );
		//pShaderShadow->SetShadowDepthFiltering(SHADER_SAMPLER0);

		int fmt = VERTEX_POSITION; 

		int texCoordDimensions [] = { 2 };
		pShaderShadow->VertexShaderVertexFormat( fmt, 1, texCoordDimensions, 0 );

		//pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0, 0 ); 

		DECLARE_STATIC_PIXEL_SHADER( post_dof_ps20 ); 
		SET_STATIC_PIXEL_SHADER( post_dof_ps20 ); 

		DECLARE_STATIC_VERTEX_SHADER( sdk_screenspaceeffect_vs20 ); 
		SET_STATIC_VERTEX_SHADER( sdk_screenspaceeffect_vs20 ); 
	} 

	DYNAMIC_STATE 
	{ 
		BindTexture( SHADER_SAMPLER0, FBTEXTURE, -1 );
		BindTexture( SHADER_SAMPLER1, BLURTEXTURE, -1 );
		//pShaderAPI->BindStandardTexture( SHADER_SAMPLER2, TEXTURE_FRAME_BUFFER_FULL_DEPTH );

		float value[3];
		value[0] = jl_post_dof_min.GetFloat();
		value[1] = jl_post_dof_max.GetFloat();
		value[2] = jl_post_dof_delta.GetFloat();
		pShaderAPI->SetPixelShaderConstant(1,value,3);

		//pShaderAPI->SetDepthFeatheringPixelShaderConstant( 2, 50 );

		float valueDebug = jl_post_dof_debug.GetBool();
		pShaderAPI->SetPixelShaderConstant(2,&valueDebug,1);

		DECLARE_DYNAMIC_PIXEL_SHADER( post_dof_ps20 ); 
		SET_DYNAMIC_PIXEL_SHADER( post_dof_ps20 ); 

		DECLARE_DYNAMIC_VERTEX_SHADER( sdk_screenspaceeffect_vs20 ); 
		SET_DYNAMIC_VERTEX_SHADER( sdk_screenspaceeffect_vs20 ); 
	}
	Draw();
} 
END_SHADER 
