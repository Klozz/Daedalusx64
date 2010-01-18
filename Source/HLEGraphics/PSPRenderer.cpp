/*
Copyright (C) 2001 StrmnNrmn

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "stdafx.h"

#include "PSPRenderer.h"
#include "Texture.h"
#include "TextureCache.h"
#include "RDP.h"
#include "RDPStateManager.h"
#include "BlendModes.h"
#include "DebugDisplayList.h"

#include "Combiner/RenderSettings.h"
#include "Combiner/BlendConstant.h"
#include "Combiner/CombinerTree.h"

#include "Graphics/NativeTexture.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/ColourValue.h"
#include "Graphics/PngUtil.h"

#include "Math/MathUtil.h"

#include "Debug/Dump.h"
#include "Debug/DBGConsole.h"

#include "Core/Memory.h"			// We access the memory buffers
#include "Core/ROM.h"

#include "../OSHLE/ultra_gbi.h"

#include "Utility/Profiler.h"
#include "Utility/Preferences.h"
#include "Utility/IO.h"

#include <pspgu.h>
#include <pspgum.h>
#include <psputils.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspdisplay.h>

#include <vector>

#include "../SysPSP/Utility/pspmath.h"

float DECAL_Z_OFFSET = +3.14f;		// Found through trial an error for the PSP

#include "PushStructPack1.h"

#include "PopStructPack.h"

extern SImageDescriptor g_CI;		// XXXX SImageDescriptor g_CI = { G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, 0 };
extern SImageDescriptor g_DI;		// XXXX SImageDescriptor g_DI = { G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, 0 };

extern "C"
{
void	_TransformVerticesWithLighting_f0_t0( const Matrix4x4 * world_matrix, const Matrix4x4 * projection_matrix, const FiddledVtx * p_in, const DaedalusVtx4 * p_out, u32 num_vertices, const TnLParams * params, const DaedalusLight * p_lights, u32 num_lights );
void	_TransformVerticesWithLighting_f0_t1( const Matrix4x4 * world_matrix, const Matrix4x4 * projection_matrix, const FiddledVtx * p_in, const DaedalusVtx4 * p_out, u32 num_vertices, const TnLParams * params, const DaedalusLight * p_lights, u32 num_lights );
void	_TransformVerticesWithLighting_f0_t2( const Matrix4x4 * world_matrix, const Matrix4x4 * projection_matrix, const FiddledVtx * p_in, const DaedalusVtx4 * p_out, u32 num_vertices, const TnLParams * params, const DaedalusLight * p_lights, u32 num_lights );
void	_TransformVerticesWithLighting_f1_t0( const Matrix4x4 * world_matrix, const Matrix4x4 * projection_matrix, const FiddledVtx * p_in, const DaedalusVtx4 * p_out, u32 num_vertices, const TnLParams * params, const DaedalusLight * p_lights, u32 num_lights );
void	_TransformVerticesWithLighting_f1_t1( const Matrix4x4 * world_matrix, const Matrix4x4 * projection_matrix, const FiddledVtx * p_in, const DaedalusVtx4 * p_out, u32 num_vertices, const TnLParams * params, const DaedalusLight * p_lights, u32 num_lights );
void	_TransformVerticesWithLighting_f1_t2( const Matrix4x4 * world_matrix, const Matrix4x4 * projection_matrix, const FiddledVtx * p_in, const DaedalusVtx4 * p_out, u32 num_vertices, const TnLParams * params, const DaedalusLight * p_lights, u32 num_lights );

void	_TransformVerticesWithColour_f0_t0( const Matrix4x4 * world_matrix, const Matrix4x4 * projection_matrix, const FiddledVtx * p_in, const DaedalusVtx4 * p_out, u32 num_vertices, const TnLParams * params );
void	_TransformVerticesWithColour_f0_t1( const Matrix4x4 * world_matrix, const Matrix4x4 * projection_matrix, const FiddledVtx * p_in, const DaedalusVtx4 * p_out, u32 num_vertices, const TnLParams * params );
void	_TransformVerticesWithColour_f1_t0( const Matrix4x4 * world_matrix, const Matrix4x4 * projection_matrix, const FiddledVtx * p_in, const DaedalusVtx4 * p_out, u32 num_vertices, const TnLParams * params );
void	_TransformVerticesWithColour_f1_t1( const Matrix4x4 * world_matrix, const Matrix4x4 * projection_matrix, const FiddledVtx * p_in, const DaedalusVtx4 * p_out, u32 num_vertices, const TnLParams * params );

void	_ConvertVertices( DaedalusVtx * dest, const DaedalusVtx4 * source, u32 num_vertices );
void	_ConvertVerticesIndexed( DaedalusVtx * dest, const DaedalusVtx4 * source, u32 num_vertices, const u16 * indices );

u32		_ClipToHyperPlane( DaedalusVtx4 * dest, const DaedalusVtx4 * source, const v4 * plane, u32 num_verts );
}

// Bits for clipping
// +-+-+-
// xxyyzz

// NB: These are ordered such that the VFPU can generate them easily - make sure you keep the VFPU code up to date if changing these.
#define X_NEG  0x01
#define Y_NEG  0x02
#define Z_NEG  0x04
#define X_POS  0x08
#define Y_POS  0x10
#define Z_POS  0x20

// Test all but Z_NEG (for No Near Plane microcodes)
static const u32 CLIP_TEST_FLAGS( X_POS | X_NEG | Y_POS | Y_NEG | Z_POS );

#define GL_TRUE                           1
#define GL_FALSE                          0

#undef min
#undef max

enum CycleType
{
	CYCLE_1CYCLE = 0,		// Please keep in this order - matches RDP
	CYCLE_2CYCLE,
	CYCLE_COPY,
	CYCLE_FILL,
};


extern int HAVE_DVE;
extern int PSP_TV_CABLE;
extern int PSP_TV_LACED;

extern u32 BUF_WIDTH;
extern u32 SCR_WIDTH;
extern u32 SCR_HEIGHT;
#define PIXEL_SIZE (4) /* change this if you change to another screenmode */
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * PIXEL_SIZE)
#define DEPTH_SIZE (BUF_WIDTH * SCR_HEIGHT * 2)

static u32 sViWidth = 320;
static u32 sViHeight = 240;

static const u32 gPlaceholderTextureWidth( 16 );
static const u32 gPlaceholderTextureHeight( 16 );
static const float gFillRectDepth( 0.0f );
static const float gTexRectDepth( 0.0f );

static const float gMaxZValue( 1.0f);

static u32		__attribute__((aligned(16)))	gWhiteTexture[gPlaceholderTextureWidth * gPlaceholderTextureHeight ];
static u32		__attribute__((aligned(16)))	gPlaceholderTexture[gPlaceholderTextureWidth * gPlaceholderTextureHeight ];
static u32		__attribute__((aligned(16)))	gSelectedTexture[gPlaceholderTextureWidth * gPlaceholderTextureHeight ];

void			DaedalusVtx4::Interpolate( const DaedalusVtx4 & lhs, const DaedalusVtx4 & rhs, float factor )
{
	ProjectedPos = lhs.ProjectedPos + (rhs.ProjectedPos - lhs.ProjectedPos) * factor;
	TransformedPos = lhs.TransformedPos + (rhs.TransformedPos - lhs.TransformedPos) * factor;
	Colour = lhs.Colour + (rhs.Colour - lhs.Colour) * factor;
	Texture = lhs.Texture + (rhs.Texture - lhs.Texture) * factor;
	ClipFlags = 0;
}

extern void		PrintMux( FILE * fh, u64 mux );

//*****************************************************************************
// Creator function for singleton
//*****************************************************************************
template<> bool CSingleton< PSPRenderer >::Create()
{
	DAEDALUS_ASSERT_Q(mpInstance == NULL);

	mpInstance = new PSPRenderer();
	return mpInstance != NULL;
}

//*****************************************************************************
//
//*****************************************************************************
PSPRenderer::PSPRenderer()
:	mN64ToPSPScale( 2.0f, 2.0f )
,	mN64ToPSPTranslate( 0.0f, 0.0f )
,	mTnLModeFlags( 0 )

,	m_dwNumLights(0)
,	m_bZBuffer(false)

,	m_bCullFront(false)
,	m_bCullBack(true)

,	mAlphaThreshold(0)

,	mSmooth( true )
,	mSmoothShade( true )

,	mFogColour(0x00FFFFFF)

,	mProjectionTop(0)
,	mModelViewTop(0)
,	mWorldProjectValid(false)

,	m_dwNumIndices(0)
,	mVtxClipFlagsUnion( 0 )

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
,	m_dwNumTrisRendered( 0 )
,	m_dwNumTrisClipped( 0 )
,	mRecordCombinerStates( false )
#endif
{
	DAEDALUS_ASSERT( IsPointerAligned( &mTnLParams, 16 ), "Oops, params should be 16-byte aligned" );

	for ( u32 t = 0; t < NUM_N64_TEXTURES; t++ )
	{
		mTileTopLeft[t] = v2( 0.0f, 0.0f );
		mTileScale[t] = v2( 1.0f, 1.0f );
	}

	memset( mLights, 0, sizeof(mLights) );

	mTnLParams.Ambient = v4( 1.0f, 1.0f, 1.0f, 1.0f );
	mTnLParams.FogMult = 0.0f;
	mTnLParams.FogOffset = 0.0f;
	mTnLParams.TextureScaleX = 1.0f;
	mTnLParams.TextureScaleY = 1.0f;

	gRDPMux._u64 = 0;

	memset( gWhiteTexture, 0xff, sizeof(gWhiteTexture) );

	u32	texel_idx( 0 );
	const u32	COL_MAGENTA( c32::Magenta.GetColour() );
	const u32	COL_GREEN( c32::Green.GetColour() );
	const u32	COL_BLACK( c32::Black.GetColour() );
	for(u32 y = 0; y < gPlaceholderTextureHeight; ++y)
	{
		for(u32 x = 0; x < gPlaceholderTextureWidth; ++x)
		{
			gPlaceholderTexture[ texel_idx ] = ((x&1) == (y&1)) ? COL_MAGENTA : COL_BLACK;
			gSelectedTexture[ texel_idx ]    = ((x&1) == (y&1)) ? COL_GREEN   : COL_BLACK;

			texel_idx++;
		}
	}

	//
	//	Set up RGB = T0, A = T0
	//
	mCopyBlendStates = new CBlendStates;
	{
		CAlphaRenderSettings *	alpha_settings( new CAlphaRenderSettings( "Copy" ) );
		CRenderSettingsModulate *	colour_settings( new CRenderSettingsModulate( "Copy" ) );

		alpha_settings->AddTermTexel0();
		colour_settings->AddTermTexel0();

		mCopyBlendStates->SetAlphaSettings( alpha_settings );
		mCopyBlendStates->AddColourSettings( colour_settings );
	}


	//
	//	Set up RGB = Diffuse, A = Diffuse
	//
	mFillBlendStates = new CBlendStates;
	{
		CAlphaRenderSettings *	alpha_settings( new CAlphaRenderSettings( "Fill" ) );
		CRenderSettingsModulate *	colour_settings( new CRenderSettingsModulate( "Fill" ) );

		alpha_settings->AddTermConstant( new CBlendConstantExpressionValue( BC_SHADE ) );
		colour_settings->AddTermConstant(  new CBlendConstantExpressionValue( BC_SHADE ) );

		mFillBlendStates->SetAlphaSettings( alpha_settings );
		mFillBlendStates->AddColourSettings( colour_settings );
	}
}

//*****************************************************************************
//
//*****************************************************************************
PSPRenderer::~PSPRenderer()
{
	delete mFillBlendStates;
	delete mCopyBlendStates;
}

//*****************************************************************************
//
//*****************************************************************************
bool PSPRenderer::RestoreRenderStates()
{
	// Initialise the device to our default state

	// We do our own culling
	sceGuDisable(GU_CULL_FACE);

	// But clip our tris please
	sceGuEnable(GU_CLIP_PLANES);
	//sceGuDisable(GU_CLIP_PLANES);

	sceGuScissor(0,0, SCR_WIDTH,SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);

	// We do our own lighting
	sceGuDisable(GU_LIGHTING);

	sceGuAlphaFunc(GU_GEQUAL, 0x04, 0xff );
	sceGuEnable(GU_ALPHA_TEST);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuEnable(GU_BLEND);

	// Default is ZBuffer disabled
	sceGuDepthMask(GL_TRUE);	// GL_TRUE to disable z-writes
	sceGuDepthFunc(GU_GEQUAL);		// GEQUAL?
	sceGuDisable(GU_DEPTH_TEST);

	// Initialise all the renderstate to our defaults.
	sceGuShadeModel(GU_SMOOTH);

	sceGuTexEnvColor( c32::White.GetColour() );
	sceGuTexOffset(0.0f,0.0f);

	//sceGuFog(near,far,mFogColour);
	// Texturing stuff
	sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGB);
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	sceGuTexWrap(GU_REPEAT,GU_REPEAT); 

	sceGuSetMatrix( GU_PROJECTION, reinterpret_cast< const ScePspFMatrix4 * >( &gMatrixIdentity ) );
	sceGuSetMatrix( GU_VIEW, reinterpret_cast< const ScePspFMatrix4 * >( &gMatrixIdentity ) );
	sceGuSetMatrix( GU_MODEL, reinterpret_cast< const ScePspFMatrix4 * >( &gMatrixIdentity ) );

	return true;
}

//*****************************************************************************
//
//*****************************************************************************
// Reset for a new frame
void	PSPRenderer::Reset()
{
	ResetMatrices();

	m_dwNumIndices = 0;
	mVtxClipFlagsUnion = 0;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	m_dwNumTrisRendered = 0;
	m_dwNumTrisClipped = 0;
#endif

	u32 dwScaleX = Memory_VI_GetRegister( VI_X_SCALE_REG ) & 0xFFF;
	u32 dwScaleY = Memory_VI_GetRegister( VI_Y_SCALE_REG ) & 0xFFF;

	float fScaleX = (float)dwScaleX / (1<<10);
	float fScaleY = (float)dwScaleY / (1<<10);

	//DBGConsole_Msg(DEBUG_VI, "VI_X_SCALE_REG set to 0x%08x (%f)", dwValue, 1/fScale);
	//DBGConsole_Msg(DEBUG_VI, "VI_Y_SCALE_REG set to 0x%08x (%f)", dwValue, 1/fScale);

	u32 dwHStartReg = Memory_VI_GetRegister( VI_H_START_REG );
	u32 dwVStartReg = Memory_VI_GetRegister( VI_V_START_REG );

	u32	hstart = dwHStartReg >> 16;
	u32	hend = dwHStartReg & 0xffff;
	//DBGConsole_Msg( 0, "h start/end %x %x", hstart, hend );		// 128 725 - 597

	u32	vstart = dwVStartReg >> 16;
	u32	vend = dwVStartReg & 0xffff;
	//DBGConsole_Msg( 0, "v start/end %x %x", vstart, vend );		// 56 501 - 445

	sViWidth  = (u32)( (hend-hstart)    * fScaleX);
	sViHeight = (u32)(((vend-vstart)/2) * fScaleY);				//

	if ( sViWidth == 0 )
	{
		sViWidth = 320;
	}
	if ( sViHeight == 0 )
	{
		sViHeight = 240;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::BeginScene()
{
	CGraphicsContext::Get()->BeginFrame();

	RestoreRenderStates();

	extern bool			gNeedZBufferUdate;
	gNeedZBufferUdate = true;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	mRecordedCombinerStates.clear();
#endif


	//
	//	We do this each frame as it lets us adapt to changes in the viewport dynamically
	//
	u32		display_width( 0 );
	u32		display_height( 0 );
	u32		frame_width( 480 );
	u32		frame_height( 272 );
	if ( gGlobalPreferences.TVEnable && PSP_TV_CABLE > 0)
	{
		switch ( gGlobalPreferences.ViewportType )
		{
		case VT_UNSCALED_4_3:		// 1:1
			if ( gGlobalPreferences.TVType == TT_WIDESCREEN )
			{
				display_width = 528;
				display_height = 448;
			}
			else
			{
				display_width = 640;
				display_height = 448;
			}
			break;
		case VT_SCALED_4_3:		// Largest 4:3
			if ( gGlobalPreferences.TVType == TT_WIDESCREEN )
			{
				display_width = 542;
				display_height = 460;
			}
			else
			{
				display_width = 658;
				display_height = 460;
			}
			break;
		case VT_FULLSCREEN:		// Fullscreen
			display_width = 720;
			display_height = 460; // 460 seems to be the limit due to renderer conversions
			break;
 		}
 		frame_width = 720;
 		frame_height = 480;
	}
	else
	{
		switch ( gGlobalPreferences.ViewportType )
		{
		case VT_UNSCALED_4_3:		// 1:1
			display_width = 320;
			display_height = 240;
			break;
		case VT_SCALED_4_3:		// Largest 4:3
			display_width = 362;
			display_height = 272;
			break;
		case VT_FULLSCREEN:		// Fullscreen
			display_width = 480;
			display_height = 272;
			break;
 		}
	}
	DAEDALUS_DL_ASSERT( display_width != 0 && display_height != 0, "Unhandled viewport type" );

	s32		display_x( (frame_width - display_width)/2 );
	s32		display_y( (frame_height - display_height)/2 );

	SetPSPViewport( display_x, display_y, display_width, display_height );

	v3 scale( 640/4.0f, 480/4.0f, 511/4.0f );
	v3 trans( 640/4.0f, 480/4.0f, 511/4.0f );

	SetN64Viewport( scale, trans );
}

//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::EndScene()
{
	CGraphicsContext::Get()->EndFrame();

	//
	//	Clear this, to ensure we're force to check for updates to it on the next frame
	//
	for( u32 i = 0; i < NUM_N64_TEXTURES; i++ )
	{
		mpTexture[ i ] = NULL;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void	PSPRenderer::SelectPlaceholderTexture( EPlaceholderTextureType type )
{
	switch( type )
	{
	case PTT_WHITE:			sceGuTexImage(0,gPlaceholderTextureWidth,gPlaceholderTextureHeight,gPlaceholderTextureWidth,gWhiteTexture); break;
	case PTT_SELECTED:		sceGuTexImage(0,gPlaceholderTextureWidth,gPlaceholderTextureHeight,gPlaceholderTextureWidth,gSelectedTexture); break;
	case PTT_MISSING:		sceGuTexImage(0,gPlaceholderTextureWidth,gPlaceholderTextureHeight,gPlaceholderTextureWidth,gPlaceholderTexture); break;
	default:
		DAEDALUS_ERROR( "Unhandled type" );
		break;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::SetPSPViewport( s32 x, s32 y, u32 w, u32 h )
{
	mN64ToPSPScale.x = f32( w ) / f32(sViWidth);
	mN64ToPSPScale.y = f32( h ) / f32(sViHeight);

	mN64ToPSPTranslate.x  = f32( x );
	mN64ToPSPTranslate.y  = f32( y );

	UpdateViewport();
}

//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::SetN64Viewport( const v3 & scale, const v3 & trans )
{
	mVpScale = scale;
	mVpTrans = trans;

	UpdateViewport();
}

//*****************************************************************************
//
//*****************************************************************************
void	PSPRenderer::UpdateViewport()
{
	u32		vx( 2048 );
	u32		vy( 2048 );

	v2		n64_min( mVpTrans.x - mVpScale.x, mVpTrans.y - mVpScale.y );
	v2		n64_max( mVpTrans.x + mVpScale.x, mVpTrans.y + mVpScale.y );

	v2		psp_min( ConvertN64ToPsp( n64_min ) );
	v2		psp_max( ConvertN64ToPsp( n64_max ) );

	s32		vp_x( s32( psp_min.x ) );
	s32		vp_y( s32( psp_min.y ) );
	s32		vp_width( s32( psp_max.x - psp_min.x ) );
	s32		vp_height( s32( psp_max.y - psp_min.y ) );

	sceGuOffset(vx - (vp_width/2),vy - (vp_height/2));
	sceGuViewport(vx + vp_x,vy + vp_y,vp_width,vp_height);
}

//*****************************************************************************
//
//*****************************************************************************
inline f32 round( f32 x )
{
	return (f32)(s32)( x + 0.5f );
}

v2	PSPRenderer::ConvertN64ToPsp( const v2 & n64_coords ) const
{
	v2	psp_coords;

	//
	// We round these value here, so that when we scale up the coords to our screen
	// coords we don't get any gaps.
	//
	psp_coords.x = vfpu_round( vfpu_round( n64_coords.x ) * mN64ToPSPScale.x + mN64ToPSPTranslate.x );
	psp_coords.y = vfpu_round( vfpu_round( n64_coords.y ) * mN64ToPSPScale.y + mN64ToPSPTranslate.y );

	return psp_coords;
}

//*****************************************************************************
//
//*****************************************************************************
RDP_OtherMode	gLastRDPOtherMode;

bool			gLastUseZBuffer = false;
bool			gNeedZBufferUdate = false;


PSPRenderer::SBlendStateEntry	PSPRenderer::LookupBlendState( u64 mux, bool two_cycles )
{
	DAEDALUS_PROFILE( "LookupBlendState" );
#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	mRecordedCombinerStates.insert( mux );
#endif

	u64		key( mux );

	if( two_cycles )
	{
		key |= u64(1)<<63;			// Top 8 bits are never set - use the very top one to differentiate between 1/2 cycles
	}

	BlendStatesMap::const_iterator	it( mBlendStatesMap.find( key ) );
	if( it != mBlendStatesMap.end() )
	{
		return it->second;
	}

	SBlendStateEntry			entry;

	entry.States = NULL;
	entry.OverrideFunction = LookupOverrideBlendModeFunction( mux );

	if( entry.OverrideFunction == NULL )
	{
		CCombinerTree				tree( mux, two_cycles );
		entry.States = tree.GetBlendStates();
#ifndef DAEDALUS_PUBLIC_RELEASE
		printf( "Adding %08x%08x - %d cycles", u32(mux>>32), u32(mux), two_cycles ? 2 : 1 );
		if(entry.States->IsInexact())
		{
			printf( " - Inexact - bodging" );
		}
		printf( "\n" );
#endif
	}

	mBlendStatesMap[ key ] = entry;
	return entry;
}

//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::RenderUsingRenderSettings( const CBlendStates * states, DaedalusVtx * p_vertices, u32 num_vertices, u32 render_flags)
{
	DAEDALUS_PROFILE( "PSPRenderer::RenderUsingRenderSettings" );

	const CAlphaRenderSettings *	alpha_settings( states->GetAlphaSettings() );

	SRenderState	state;

	state.Vertices = p_vertices;
	state.NumVertices = num_vertices;
	state.PrimitiveColour = mPrimitiveColour;
	state.EnvironmentColour = mEnvColour;

	static std::vector< DaedalusVtx >	saved_verts;

	if( states->GetNumStates() > 1 )
	{
		saved_verts.resize( num_vertices );
		memcpy( &saved_verts[0], p_vertices, num_vertices * sizeof( DaedalusVtx ) );
	}


	for( u32 i = 0; i < states->GetNumStates(); ++i )
	{
		const CRenderSettings *		settings( states->GetColourSettings( i ) );

		bool		install_texture0( settings->UsesTexture0() || alpha_settings->UsesTexture0() );
		bool		install_texture1( settings->UsesTexture1() || alpha_settings->UsesTexture1() );

		SRenderStateOut out;

		memset( &out, 0, sizeof( out ) );

		settings->Apply( install_texture0 || install_texture1, state, out );
		alpha_settings->Apply( install_texture0 || install_texture1, state, out );

		// TODO: this nobbles the existing diffuse colour on each pass. Need to use a second buffer...
		if( i > 0 )
		{
			memcpy( p_vertices, &saved_verts[0], num_vertices * sizeof( DaedalusVtx ) );
		}

		if(out.VertexExpressionRGB != NULL)
		{
			out.VertexExpressionRGB->ApplyExpressionRGB( state );
		}
		if(out.VertexExpressionA != NULL)
		{
			out.VertexExpressionA->ApplyExpressionAlpha( state );
		}


		bool	installed_texture( false );

		if(install_texture0 || install_texture1)
		{
			u32	tfx( GU_TFX_MODULATE );
			switch( out.BlendMode )
			{
			case PBM_MODULATE:		tfx = GU_TFX_MODULATE; break;
			case PBM_REPLACE:		tfx = GU_TFX_REPLACE; break;
			case PBM_BLEND:			tfx = GU_TFX_BLEND; break;
			}

			u32 tcc( GU_TCC_RGBA );
			switch( out.BlendAlphaMode )
			{
			case PBAM_RGBA:			tcc = GU_TCC_RGBA; break;
			case PBAM_RGB:			tcc = GU_TCC_RGB; break;
			}

			sceGuTexFunc( tfx, tcc );
			if( tfx == GU_TFX_BLEND )
			{
				sceGuTexEnvColor( out.TextureFactor.GetColour() );
			}

			// NB if install_texture0 and install_texture1 are both set, 0 wins out
			u32		texture_idx( install_texture0 ? 0 : 1 );

			if( mpTexture[texture_idx] != NULL )
			{
				CRefPtr<CNativeTexture> texture;

				if(out.MakeTextureWhite)
				{
					texture = mpTexture[ texture_idx ]->GetRecolouredTexture( c32::White );
				}
				else
				{
					texture = mpTexture[ texture_idx ]->GetTexture();
				}

				if(texture != NULL)
				{
					texture->InstallTexture();
					installed_texture = true;
				}
			}
		}

		// If no texture was specified, or if we couldn't load it, clear it out
		if( !installed_texture )
		{
			sceGuDisable(GU_TEXTURE_2D);
		}

		sceGuDrawArray( GU_TRIANGLES, render_flags, num_vertices, NULL, p_vertices );
	}
}

//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::RenderUsingCurrentBlendMode( DaedalusVtx * p_vertices, u32 num_vertices, ERenderMode mode, bool disable_zbuffer )
{
	DAEDALUS_PROFILE( "PSPRenderer::RenderUsingCurrentBlendMode" );

	if ( disable_zbuffer )
	{
		sceGuDisable(GU_DEPTH_TEST);
		sceGuDepthMask( GL_TRUE );	// GL_TRUE to disable z-writes
	}
	else
	{
		if ( gNeedZBufferUdate ||
			(gRDPOtherMode._u64 != gLastRDPOtherMode._u64) ||
			 (m_bZBuffer != gLastUseZBuffer) )
		{
			// Only update if ZBuffer is enabled
			if (m_bZBuffer)
			{
				if(gRDPOtherMode.z_cmp)
					sceGuEnable(GU_DEPTH_TEST);
				else
					sceGuDisable(GU_DEPTH_TEST);

				sceGuDepthMask( gRDPOtherMode.z_upd ? GL_FALSE : GL_TRUE );	// GL_TRUE to disable z-writes
			}
			else
			{
				sceGuDisable(GU_DEPTH_TEST);
				sceGuDepthMask( GL_TRUE );	// GL_TRUE to disable z-writes
			}

			gNeedZBufferUdate = false;
			gLastUseZBuffer = m_bZBuffer;
		}
	}

	gLastRDPOtherMode._u64 = gRDPOtherMode._u64;

	sceGuShadeModel( mSmooth ? GU_SMOOTH : GU_FLAT );

	//
	// Our filtering starts here.
	// I don't like to do our filtering here...
	// I think is a better idea doing it on the microcode itself
	// Ex : DLParser_XXX_SetOtherModeH and DLParser_RDPSetOtherMode
	// But there has to be a good reason why we do filtering, blender, othermode init, and zbuffer here?
	//
	switch( gGlobalPreferences.ForceTextureFilter )
	{
		case FORCE_DEFAULT_FILTER:
			switch(gRDPOtherMode.text_filt)
			{
				case G_TF_BILERP:
					sceGuTexFilter(GU_LINEAR,GU_LINEAR);
					break;
				default:
					sceGuTexFilter(GU_NEAREST,GU_NEAREST);
					break;
			}
			break;
		case FORCE_POINT_FILTER:
			sceGuTexFilter(GU_NEAREST,GU_NEAREST);
			break;
		case FORCE_LINEAR_FILTER:
			sceGuTexFilter(GU_LINEAR,GU_LINEAR);
			break;
	}




/*

// P or M
#define	G_BL_CLR_IN	0
#define	G_BL_CLR_MEM	1
#define	G_BL_CLR_BL	2
#define	G_BL_CLR_FOG	3

// A inputs
#define	G_BL_A_IN	0
#define	G_BL_A_FOG	1
#define	G_BL_A_SHADE	2
#define	G_BL_0		3

// B inputs
#define	G_BL_1MA	0
#define	G_BL_A_MEM	1
#define	G_BL_1		2
#define	G_BL_0		3

#define	GBL_c1(m1a, m1b, m2a, m2b)	\
	(m1a) << 30 | (m1b) << 26 | (m2a) << 22 | (m2b) << 18
#define	GBL_c2(m1a, m1b, m2a, m2b)	\
	(m1a) << 28 | (m1b) << 24 | (m2a) << 20 | (m2b) << 16

#define	RM_AA_ZB_OPA_SURF(clk)					\
	AA_EN | Z_CMP | Z_UPD | IM_RD | CVG_DST_CLAMP |		\
	ZMODE_OPA | ALPHA_CVG_SEL |				\
	GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)

#define	G_RM_FOG_SHADE_A		GBL_c1(G_BL_CLR_FOG, G_BL_A_SHADE, G_BL_CLR_IN, G_BL_1MA)
#define	G_RM_FOG_PRIM_A			GBL_c1(G_BL_CLR_FOG, G_BL_A_FOG, G_BL_CLR_IN, G_BL_1MA)
#define	G_RM_PASS				GBL_c1(G_BL_CLR_IN, G_BL_0, G_BL_CLR_IN, G_BL_1)
#define	RM_AA_ZB_OPA_SURF(clk)	GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)
*/
	u32 blendmode = u32( gRDPOtherMode._u64 & 0xffff0000 );

	int		blend_op = GU_ADD;
	int		blend_src = GU_SRC_ALPHA;
	int		blend_dst = GU_ONE_MINUS_SRC_ALPHA;
	bool	enable_blend( false );

#define MAKE_BLEND_MODE( a, b )			( (a) | (b) )
#define BLEND_NOOP1				0x00000000		//GBL_c1(G_BL_CLR_IN, G_BL_1MA, G_BL_CLR_IN, G_BL_1MA)
#define BLEND_NOOP2				0x00000000

#define BLEND_FOG_ASHADE1		0xc8000000
#define BLEND_FOG_APRIM1		0xc4000000

#define BLEND_PASS1				0x0c080000		//GBL_c1(G_BL_CLR_IN, G_BL_0, G_BL_CLR_IN, G_BL_1)
#define BLEND_PASS2				0x03020000

#define BLEND_OPA1				0x00440000
#define BLEND_OPA2				0x00110000

#define BLEND_XLU1				0x00400000
#define BLEND_XLU2				0x00100000

#define BLEND_ADD1				0x04400000		//GBL_c##clk(G_BL_CLR_IN, G_BL_A_FOG, G_BL_CLR_MEM, G_BL_1)
#define BLEND_ADD2				0x01100000

#define BLEND_MEM1				0x4c400000		// Mem*0 + Mem*(1-0)?!
#define BLEND_MEM2				0x13100000		// Mem*0 + Mem*(1-0)?!


	if ( gRDPOtherMode.cycle_type == CYCLE_FILL )
	{
		enable_blend = false;
	}
	else
	{
		switch( blendmode )
		{
		case MAKE_BLEND_MODE( BLEND_NOOP1, BLEND_NOOP2 ):
			blend_op = GU_ADD; blend_src = GU_SRC_ALPHA; blend_dst = GU_ONE_MINUS_SRC_ALPHA;
			enable_blend = true;
			DL_PF( "      Blend: NOOP/NOOP" );
			break;
		case MAKE_BLEND_MODE( BLEND_FOG_ASHADE1, BLEND_OPA2 ):
			enable_blend = false;
			DL_PF( "      Blend: FOG_ASHADE/OPA" );
			break;
		case MAKE_BLEND_MODE( BLEND_FOG_ASHADE1, BLEND_XLU2 ):
			blend_op = GU_ADD; blend_src = GU_SRC_ALPHA; blend_dst = GU_ONE_MINUS_SRC_ALPHA;
			enable_blend = true;
			DL_PF( "      Blend: FOG_ASHADE/XLU" );
			break;
		case MAKE_BLEND_MODE( BLEND_XLU1, BLEND_XLU2 ):
			blend_op = GU_ADD; blend_src = GU_SRC_ALPHA; blend_dst = GU_ONE_MINUS_SRC_ALPHA;
			enable_blend = true;
			DL_PF( "      Blend: XLU/XLU" );
			break;
		case MAKE_BLEND_MODE( BLEND_XLU1, BLEND_NOOP2 ):
			blend_op = GU_ADD; blend_src = GU_SRC_ALPHA; blend_dst = GU_ONE_MINUS_SRC_ALPHA;
			enable_blend = true;
			DL_PF( "      Blend: XLU/NOOP" );
			break;
		case MAKE_BLEND_MODE( BLEND_XLU1, BLEND_ADD2 ):
			// XXXX
			blend_op = GU_ADD; blend_src = GU_SRC_COLOR; blend_dst = GU_DST_COLOR;
			enable_blend = true;
			DL_PF( "      Blend: XLU/ADD" );
			break;
		case MAKE_BLEND_MODE( BLEND_OPA1, BLEND_OPA2 ):
			enable_blend = false;
			DL_PF( "      Blend: OPA/OPA" );
			break;
		case MAKE_BLEND_MODE( BLEND_OPA1, BLEND_NOOP2 ):
			enable_blend = false;
			DL_PF( "      Blend: OPA/NOOP" );
			break;
		case MAKE_BLEND_MODE( BLEND_PASS1, BLEND_PASS2 ):
			enable_blend = false;
			DL_PF( "      Blend: PASS/PASS" );
			break;
		case MAKE_BLEND_MODE( BLEND_PASS1, BLEND_XLU2 ):
			blend_op = GU_ADD; blend_src = GU_SRC_ALPHA; blend_dst = GU_ONE_MINUS_SRC_ALPHA;
			enable_blend = true;
			DL_PF( "      Blend: PASS/XLU" );
			break;
		case MAKE_BLEND_MODE( BLEND_PASS1, BLEND_OPA2 ):
			enable_blend = false;
			DL_PF( "      Blend: PASS/OPA" );
			break;
		case MAKE_BLEND_MODE( BLEND_MEM1, BLEND_MEM2 ):
			enable_blend = false;
			DL_PF( "      Blend: MEM/MEM" );
			break;
		default:
			blend_op = GU_ADD; blend_src = GU_SRC_ALPHA; blend_dst = GU_ONE_MINUS_SRC_ALPHA;
			enable_blend = true;
			DL_PF( "      Blend: SRCALPHA/INVSRCALPHA (default: 0x%04x)", gRDPOtherMode.blender );
			break;
		}
	}

	if( enable_blend )
	{
		sceGuBlendFunc( blend_op, blend_src, blend_dst, 0, 0);
		sceGuEnable( GU_BLEND );
	}
	else
	{
		sceGuDisable( GU_BLEND );
	}

	//
	// I can't think why the hand in mario's menu screen is rendered with an opaque rendermode,
	// and no alpha threshold. We set the alpha reference to 1 to ensure that the transparent pixels
	// don't get rendered. (We also do this in Super Smash Bothers to ensure transparent pixels
	// are not rendered. Also fixes other games - Kreationz). I hope this doesn't fuck anything up though. 
	//
	if ( gRDPOtherMode.alpha_compare == 0 )
	{
		if ( gRDPOtherMode.cvg_x_alpha )	// I think this implies that alpha is coming from
		{
			sceGuAlphaFunc(GU_GREATER, 0, 0xff);
			sceGuEnable(GU_ALPHA_TEST);
		}
		else
		{
			sceGuDisable(GU_ALPHA_TEST);
		}
	}
	else
	{
		if( (gRDPOtherMode.alpha_cvg_sel ) && !gRDPOtherMode.cvg_x_alpha ) //We need cvg_sel for SSVN characters to display
		{
			// Use CVG for pixel alpha
			sceGuDisable(GU_ALPHA_TEST);
		}
		else
		{
			// G_AC_THRESHOLD || G_AC_DITHER
			if(	mAlphaThreshold==0 )
			{
				sceGuAlphaFunc(GU_GREATER, 0, 0xff);
			}
			else
			{
				sceGuAlphaFunc(GU_GEQUAL, mAlphaThreshold, 0xff);
			}
			sceGuEnable(GU_ALPHA_TEST);
		}
	}

	SBlendStateEntry		blend_entry;

	switch ( gRDPOtherMode.cycle_type )
	{
		case CYCLE_COPY:		blend_entry.States = mCopyBlendStates; break;
		case CYCLE_FILL:		blend_entry.States = mFillBlendStates; break;
		case CYCLE_1CYCLE:		blend_entry = LookupBlendState( gRDPMux._u64, 1 ); break;
		case CYCLE_2CYCLE:		blend_entry = LookupBlendState( gRDPMux._u64, 2 ); break;
	}

	u32		render_flags( DAEDALUS_VERTEX_FLAGS );

	switch( mode )
	{
	case RM_RENDER_2D:		render_flags |= GU_TRANSFORM_2D; break;
	case RM_RENDER_3D:		render_flags |= GU_TRANSFORM_3D; break;
	}

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	if(IsCombinerStateDisabled( gRDPMux._u64 ))
	{
		// Use the nasty placeholder texture
		sceGuEnable(GU_TEXTURE_2D);
		SelectPlaceholderTexture( PTT_SELECTED );
		sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
		sceGuTexMode(GU_PSM_8888,0,0,GL_TRUE);		// maxmips/a2/swizzle = 0

		sceGuDrawArray( GU_TRIANGLES, render_flags, num_vertices, NULL, p_vertices );
	}
	else
#endif
	if( blend_entry.OverrideFunction != NULL )
	{
		// Local vars for now
		SBlendModeDetails		details;

		details.InstallTexture = false;
		details.EnvColour = mEnvColour;
		details.PrimColour = mPrimitiveColour;
		details.ColourAdjuster.Reset();
		details.RecolourTextureWhite = false;

		blend_entry.OverrideFunction( gRDPOtherMode.cycle_type == CYCLE_2CYCLE ? 2 : 1, details );

		bool	installed_texture( false );

		if( details.InstallTexture )
		{
			if( mpTexture[ 0 ] != NULL )
			{
				CRefPtr<CNativeTexture> texture;

				if(details.RecolourTextureWhite)
				{
					texture = mpTexture[ 0 ]->GetRecolouredTexture( c32::White );
				}
				else
				{
					texture = mpTexture[ 0 ]->GetTexture();
				}

				if(texture != NULL)
				{
					texture->InstallTexture();
					installed_texture = true;
				}
			}
		}

		// If no texture was specified, or if we couldn't load it, clear it out
		if( !installed_texture )
		{
			sceGuDisable( GU_TEXTURE_2D );
		}

		details.ColourAdjuster.Process( p_vertices, num_vertices );

		sceGuDrawArray( GU_TRIANGLES, render_flags, num_vertices, NULL, p_vertices );
	}
	else if( blend_entry.States != NULL )
	{
		bool	inexact( blend_entry.States->IsInexact() );

#ifndef DAEDALUS_PUBLIC_RELEASE
		if( inexact )
		{
			if(mUnhandledCombinderStates.find( gRDPMux._u64 ) == mUnhandledCombinderStates.end())
			{
				char szFilePath[MAX_PATH+1];

				Dump_GetDumpDirectory(szFilePath, g_ROM.settings.GameName.c_str());

				IO::Path::Append(szFilePath, "missing_mux.txt");

				FILE * fh( fopen(szFilePath, mUnhandledCombinderStates.empty() ? "w" : "a") );
				if(fh != NULL)
				{
					PrintMux( fh, gRDPMux._u64 );
					fclose(fh);
				}

				mUnhandledCombinderStates.insert( gRDPMux._u64 );
			}
		}
#endif
		if(inexact && gGlobalPreferences.HighlightInexactBlendModes)
		{
			sceGuEnable( GU_TEXTURE_2D );
			sceGuTexMode( GU_PSM_8888, 0, 0, GL_TRUE );		// maxmips/a2/swizzle = 0

			// Use the nasty placeholder texture
			SelectPlaceholderTexture( PTT_MISSING );
			sceGuTexFunc( GU_TFX_REPLACE, GU_TCC_RGBA );
			sceGuDrawArray( GU_TRIANGLES, render_flags, num_vertices, NULL, p_vertices );
		}
		else
		{
			RenderUsingRenderSettings( blend_entry.States, p_vertices, num_vertices, render_flags );
		}
	}
	else
	{
		// Set default states
		DAEDALUS_ERROR( "Unhandled blend mode" );
		sceGuDisable( GU_TEXTURE_2D );
		sceGuDrawArray( GU_TRIANGLES, render_flags, num_vertices, NULL, p_vertices );
	}
}

//*****************************************************************************
//
//*****************************************************************************
namespace
{
	v2	GetEdgeForCycleMode( u32 cycle_type )
	{
		v2 edge( 0, 0 );

		//
		// In Fill/Copy mode the coordinates are inclusive (i.e. add 1.0f to the w/h)
		//
		switch ( gRDPOtherMode.cycle_type )
		{
			case CYCLE_COPY:			edge.x += 1.0f; edge.y += 1.0f;	break;
			case CYCLE_FILL:			edge.x += 1.0f; edge.y += 1.0f;	break;
			case CYCLE_1CYCLE:											break;
			case CYCLE_2CYCLE:											break;
		}

		return edge;
	}
}

//*****************************************************************************
//
//*****************************************************************************
bool PSPRenderer::TexRect( u32 tile_idx, const v2 & xy0, const v2 & xy1, const v2 & uv0, const v2 & uv1 )
{
	EnableTexturing( tile_idx );

	v2 edge( GetEdgeForCycleMode( gRDPOtherMode.cycle_type ) );
	v2 screen0( ConvertN64ToPsp( xy0 ) );
	v2 screen1( ConvertN64ToPsp( xy1 + edge ) );
	v2 tex_uv0( uv0 - mTileTopLeft[ 0 ] );
	v2 tex_uv1( uv1 - mTileTopLeft[ 0 ] );

	//DL_PF( "      Screen:  %f,%f -> %f,%f", screen0.x, screen0.y, screen1.x, screen1.y );
	//DL_PF( "      Texture: %f,%f -> %f,%f", tex_uv0.x, tex_uv0.y, tex_uv1.x, tex_uv1.y );


	DaedalusVtx trv[ 6 ];

	v3	positions[ 4 ] =
	{
		v3( screen0.x, screen0.y, gTexRectDepth ),
		v3( screen1.x, screen0.y, gTexRectDepth ),
		v3( screen1.x, screen1.y, gTexRectDepth ),
		v3( screen0.x, screen1.y, gTexRectDepth ),
	};
	v2	tex_coords[ 4 ] =
	{
		v2( tex_uv0.x, tex_uv0.y ),
		v2( tex_uv1.x, tex_uv0.y ),
		v2( tex_uv1.x, tex_uv1.y ),
		v2( tex_uv0.x, tex_uv1.y ),
	};

	trv[0] = DaedalusVtx( positions[ 1 ], 0xffffffff, tex_coords[ 1 ] );
	trv[1] = DaedalusVtx( positions[ 0 ], 0xffffffff, tex_coords[ 0 ] );
	trv[2] = DaedalusVtx( positions[ 2 ], 0xffffffff, tex_coords[ 2 ] );

	trv[3] = DaedalusVtx( positions[ 2 ], 0xffffffff, tex_coords[ 2 ] );
	trv[4] = DaedalusVtx( positions[ 0 ], 0xffffffff, tex_coords[ 0 ] );
	trv[5] = DaedalusVtx( positions[ 3 ], 0xffffffff, tex_coords[ 3 ] );

	return RenderTriangleList( trv, 6, true );
}

//*****************************************************************************
//
//*****************************************************************************
bool PSPRenderer::TexRectFlip( u32 tile_idx, const v2 & xy0, const v2 & xy1, const v2 & uv0, const v2 & uv1 )
{
	EnableTexturing( tile_idx );

	v2 edge( GetEdgeForCycleMode( gRDPOtherMode.cycle_type ) );
	v2 screen0( ConvertN64ToPsp( xy0 ) );
	v2 screen1( ConvertN64ToPsp( xy1 + edge ) );
	v2 tex_uv0( uv0 - mTileTopLeft[ 0 ] );
	v2 tex_uv1( uv1 - mTileTopLeft[ 0 ] );

	//DL_PF( "      Screen:  %f,%f -> %f,%f", screen0.x, screen0.y, screen1.x, screen1.y );
	//DL_PF( "      Texture: %f,%f -> %f,%f", tex_uv0.x, tex_uv0.y, tex_uv1.x, tex_uv1.y );

	DaedalusVtx trv[ 6 ];

	v3	positions[ 4 ] =
	{
		v3( screen0.x, screen0.y, gTexRectDepth ),
		v3( screen1.x, screen0.y, gTexRectDepth ),
		v3( screen1.x, screen1.y, gTexRectDepth ),
		v3( screen0.x, screen1.y, gTexRectDepth ),
	};
	v2	tex_coords[ 4 ] =
	{
		v2( tex_uv0.x, tex_uv0.y ),
		v2( tex_uv0.x, tex_uv1.y ),		// In TexRect this is tex_uv1.x, tex_uv0.y
		v2( tex_uv1.x, tex_uv1.y ),
		v2( tex_uv1.x, tex_uv0.y ),		//tex_uv0.x	tex_uv1.y
	};

	trv[0] = DaedalusVtx( positions[ 1 ], 0xffffffff, tex_coords[ 1 ] );
	trv[1] = DaedalusVtx( positions[ 0 ], 0xffffffff, tex_coords[ 0 ] );
	trv[2] = DaedalusVtx( positions[ 2 ], 0xffffffff, tex_coords[ 2 ] );

	trv[3] = DaedalusVtx( positions[ 2 ], 0xffffffff, tex_coords[ 2 ] );
	trv[4] = DaedalusVtx( positions[ 0 ], 0xffffffff, tex_coords[ 0 ] );
	trv[5] = DaedalusVtx( positions[ 3 ], 0xffffffff, tex_coords[ 3 ] );

	return RenderTriangleList( trv, 6, true );
}

//*****************************************************************************
//
//*****************************************************************************
bool PSPRenderer::FillRect( const v2 & xy0, const v2 & xy1, u32 color )
{
	if ( (gRDPOtherMode._u64 & 0xffff0000) == 0x5f500000 )
	{
		// this blend mode is mem*0 + mem*1, so we don't need to render it... Very odd!
		return true;
	}

	// This if for C&C - It might break other stuff (I'm not sure if we should allow alpha or not..)
//	color |= 0xff000000;

	//
	// In Fill/Copy mode the coordinates are inclusive (i.e. add 1.0f to the w/h)
	//
	v2 edge( GetEdgeForCycleMode( gRDPOtherMode.cycle_type ) );
	v2 screen0( ConvertN64ToPsp( xy0 ) );
	v2 screen1( ConvertN64ToPsp( xy1 + edge ) );

	DL_PF( "      Screen:  %f,%f -> %f,%f", screen0.x, screen0.y, screen1.x, screen1.y );

	DaedalusVtx trv[ 6 ];

	v3	positions[ 4 ] =
	{
		v3( screen0.x, screen0.y, gTexRectDepth ),
		v3( screen1.x, screen0.y, gTexRectDepth ),
		v3( screen1.x, screen1.y, gTexRectDepth ),
		v3( screen0.x, screen1.y, gTexRectDepth ),
	};
	v2	tex_coords[ 4 ] =
	{
		v2( 0, 0 ),
		v2( 1, 0 ),
		v2( 1, 1 ),
		v2( 0, 1 ),
	};

	trv[0] = DaedalusVtx( positions[ 1 ], color, tex_coords[ 1 ] );
	trv[1] = DaedalusVtx( positions[ 0 ], color, tex_coords[ 0 ] );
	trv[2] = DaedalusVtx( positions[ 2 ], color, tex_coords[ 2 ] );

	trv[3] = DaedalusVtx( positions[ 2 ], color, tex_coords[ 2 ] );
	trv[4] = DaedalusVtx( positions[ 0 ], color, tex_coords[ 0 ] );
	trv[5] = DaedalusVtx( positions[ 3 ], color, tex_coords[ 3 ] );

	return RenderTriangleList( trv, 6, true );
}

//*****************************************************************************
// Returns true if triangle visible and rendered, false otherwise
//*****************************************************************************
bool PSPRenderer::AddTri(u32 v0, u32 v1, u32 v2)
{
	//DAEDALUS_PROFILE( "PSPRenderer::AddTri" );

	u8	f0( mVtxProjected[v0].ClipFlags );
	u8	f1( mVtxProjected[v1].ClipFlags );
	u8	f2( mVtxProjected[v2].ClipFlags );

	if ( f0 & f1 & f2 & CLIP_TEST_FLAGS )
	{
		DL_PF("   Tri: %d,%d,%d (clipped)", v0, v1, v2);

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
		m_dwNumTrisClipped++;
#endif
		return false;

	}
	else
	{
		DL_PF("   Tri: %d,%d,%d", v0, v1, v2);

		m_swIndexBuffer[ m_dwNumIndices + 0 ] = (u16)v0;
		m_swIndexBuffer[ m_dwNumIndices + 1 ] = (u16)v1;
		m_swIndexBuffer[ m_dwNumIndices + 2 ] = (u16)v2;

		m_dwNumIndices += 3;

		mVtxClipFlagsUnion |= f0;
		mVtxClipFlagsUnion |= f1;
		mVtxClipFlagsUnion |= f2;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
		m_dwNumTrisRendered++;
#endif
		return true;
	}
}

//*****************************************************************************
//
//*****************************************************************************
bool PSPRenderer::TestVerts( u32 v0, u32 vn ) const
{
	u32 flags = mVtxProjected[v0].ClipFlags;

	for ( u32 i = v0+1; i <= vn && i < MAX_VERTS; i++ )
	{
		flags &= mVtxProjected[i].ClipFlags;
	}

	return (flags & CLIP_TEST_FLAGS) == 0;
}

//*****************************************************************************
//
//*****************************************************************************
v4 PSPRenderer::LightVert( const v3 & norm ) const
{
	// Do ambient
	v4	result( mTnLParams.Ambient );

	for ( s32 l = 0; l < m_dwNumLights; l++ )
	{
		f32 fCosT = norm.Dot( mLights[l].Direction );
		if (fCosT > 0)
		{
			result.x += mLights[l].Colour.x * fCosT;
			result.y += mLights[l].Colour.y * fCosT;
			result.z += mLights[l].Colour.z * fCosT;
		//	result.w += mLights[l].Colour.w * fCosT;
		}
	}

	if( result.x > 1.0f ) result.x = 1.0f;
	if( result.y > 1.0f ) result.y = 1.0f;
	if( result.z > 1.0f ) result.z = 1.0f;
	if( result.w > 1.0f ) result.w = 1.0f;

	return result;
}

//*****************************************************************************
//
//	The following clipping code was taken from The Irrlicht Engine.
//	See http://irrlicht.sourceforge.net/ for more information.
//	Copyright (C) 2002-2006 Nikolaus Gebhardt/Alten Thomas
//
//*****************************************************************************
static const v4 NDCPlane[6] =
{
	v4(  0.f,  0.f, -1.f, -1.f ),	// near
	v4(  0.f,  0.f,  1.f, -1.f ),	// far
	v4(  1.f,  0.f,  0.f, -1.f ),	// left
	v4( -1.f,  0.f,  0.f, -1.f ),	// right
	v4(  0.f,  1.f,  0.f, -1.f ),	// bottom
	v4(  0.f, -1.f,  0.f, -1.f )	// top
};

static u32 clipToHyperPlane( DaedalusVtx4 * dest, const DaedalusVtx4 * source, u32 inCount, const v4 &plane )
{
	u32 outCount = 0;
	DaedalusVtx4 * out = dest;

	const DaedalusVtx4 * a;
	const DaedalusVtx4 * b = source;

	f32 bDotPlane;

	bDotPlane = b->ProjectedPos.Dot( plane );

	for( u32 i = 1; i < inCount + 1; ++i)
	{
		const s32 condition = i - inCount;
		const s32 index = (( ( condition >> 31 ) & ( i ^ condition ) ) ^ condition ) << 1;

		a = &source[ index ];

	
		// current point inside
		if ( a->ProjectedPos.Dot( plane ) <= 0.f )
		{
			// last point outside
			if ( bDotPlane > 0.f )
			{
				// intersect line segment with plane
				out->Interpolate( *b, *a, bDotPlane / (b->ProjectedPos - a->ProjectedPos).Dot( plane ) );
				out += 2;
				outCount += 1;
			}
			// copy current to out
			memcpy( out, a, sizeof( DaedalusVtx4 ) * 2);
			b = out;

			out += 2;
			outCount += 1;
		}
		else
		{
			// current point outside

			if ( bDotPlane <= 0.f )
			{
				// previous was inside
				// intersect line segment with plane
				out->Interpolate( *b, *a, bDotPlane / (b->ProjectedPos - a->ProjectedPos).Dot( plane ) );

				out += 2;
				outCount += 1;
			}
			b = a;
		}

		bDotPlane = b->ProjectedPos.Dot( plane );
	}

	return outCount;
}

u32 clip_tri_to_frustum( DaedalusVtx4 * v0, DaedalusVtx4 * v1 )
{
	u32 vOut( 3 );

	vOut = clipToHyperPlane( v1, v0, vOut, NDCPlane[0] ); if( vOut < 3 ) return 0;		// near
	vOut = clipToHyperPlane( v0, v1, vOut, NDCPlane[1] ); if( vOut < 3 ) return 0;		// left
	vOut = clipToHyperPlane( v1, v0, vOut, NDCPlane[2] ); if( vOut < 3 ) return 0;		// right
	vOut = clipToHyperPlane( v0, v1, vOut, NDCPlane[3] ); if( vOut < 3 ) return 0;		// bottom
	vOut = clipToHyperPlane( v1, v0, vOut, NDCPlane[4] ); if( vOut < 3 ) return 0;		// top
	vOut = clipToHyperPlane( v0, v1, vOut, NDCPlane[5] );		// far

	return vOut;
}

u32 clip_tri_to_frustum_vfpu( DaedalusVtx4 * v0, DaedalusVtx4 * v1 )
{
	u32 vOut( 3 );

	vOut = _ClipToHyperPlane( v1, v0, &NDCPlane[0], vOut ); if( vOut < 3 ) return 0;		// near
	vOut = _ClipToHyperPlane( v0, v1, &NDCPlane[1], vOut ); if( vOut < 3 ) return 0;		// left
	vOut = _ClipToHyperPlane( v1, v0, &NDCPlane[2], vOut ); if( vOut < 3 ) return 0;		// right
	vOut = _ClipToHyperPlane( v0, v1, &NDCPlane[3], vOut ); if( vOut < 3 ) return 0;		// bottom
	vOut = _ClipToHyperPlane( v1, v0, &NDCPlane[4], vOut ); if( vOut < 3 ) return 0;		// top
	vOut = _ClipToHyperPlane( v0, v1, &NDCPlane[5], vOut );		// far

	return vOut;
}


//*****************************************************************************
//
//*****************************************************************************
bool PSPRenderer::FlushTris()
{
	DAEDALUS_PROFILE( "FlushTris" );

	if ( m_dwNumIndices == 0 )
	{
		mVtxClipFlagsUnion = 0;
		return true;
	}

	u32				num_vertices;
	DaedalusVtx *	p_vertices;

	if(gGlobalPreferences.SoftwareClipping && (mVtxClipFlagsUnion != 0))
	{
		PrepareTrisClipped( &p_vertices, &num_vertices );
	}
	else
	{
		PrepareTrisUnclipped( &p_vertices, &num_vertices );
	}

	// Hack for Pilotwings 64
	static bool skipNext=false;
	if( gFlushTrisHack )
	{
		if ( (g_DI.Address == g_CI.Address) && gRDPOtherMode.z_cmp+gRDPOtherMode.z_upd > 0 )
		{
			DAEDALUS_ERROR("Warning: using Flushtris to write Zbuffer" );
			m_dwNumIndices = 0;
			mVtxClipFlagsUnion = 0;
			skipNext = true;
			return true;
		}
		else if( skipNext )
		{
			skipNext = false;
			m_dwNumIndices = 0;
			mVtxClipFlagsUnion = 0;
			return true;
		}	
	}
	//
	// Process the software vertex buffer to apply a couple of
	// necessary changes to the texture coords (this is required
	// because some ucodes set the texture after setting the vertices)
	//
	if (mTnLModeFlags & TNL_TEXTURE)
	{
		EnableTexturing( gTextureTile );

		// Bias points in decal mode
		if (IsZModeDecal())
		{
			for ( u32 v = 0; v < num_vertices; v++ )
			{
				p_vertices[v].Position.z += DECAL_Z_OFFSET;
			}
		}
	}

	//
	// Process the software vertex buffer to apply a couple of
	// necessary changes to the texture coords (this is required
	// because some ucodes set the texture after setting the vertices)
	//
	bool	update_tex_coords( (mTnLModeFlags & TNL_TEXTURE) != 0 && (mTnLModeFlags & (TNL_LIGHT|TNL_TEXGEN)) != (TNL_LIGHT|TNL_TEXGEN) );

	v2		offset( 0.0f, 0.0f );
	v2		scale( 1.0f, 1.0f );

	if( update_tex_coords )
	{
		offset = -mTileTopLeft[ 0 ];
		scale = mTileScale[ 0 ];
	}
	sceGuTexOffset( offset.x * scale.x, offset.y * scale.y );
	sceGuTexScale( scale.x, scale.y );


	//
	// If smooth shading is turned off, duplicate the first diffuse color for all three verts
	//
	if ( !mSmoothShade )
	{

	}

	//
	//	If the depth source is prim, use the depth from the prim command
	//

	//
	//	For now do all clipping though ge -
	//
	if(m_bCullFront)
	{
		sceGuFrontFace(GU_CW);
		sceGuEnable(GU_CULL_FACE);
	}
	else if( m_bCullBack )
	{
		sceGuFrontFace(GU_CCW);
		sceGuEnable(GU_CULL_FACE);
	}
	else
	{
		sceGuDisable(GU_CULL_FACE);
	}

	//
	//	Render out our vertices
	//	XXXX Should be using GetWorldProject()?
	//
	const Matrix4x4	&		projection( mProjectionStack[mProjectionTop] );
	const ScePspFMatrix4 *	p_psp_proj( reinterpret_cast< const ScePspFMatrix4 * >( &projection ) );

	sceGuSetMatrix( GU_PROJECTION, p_psp_proj );

	RenderUsingCurrentBlendMode( p_vertices, num_vertices, RM_RENDER_3D, false );

	sceGuDisable(GU_CULL_FACE);

	m_dwNumIndices = 0;
	mVtxClipFlagsUnion = 0;
	return true;
}

//*****************************************************************************
//
//*****************************************************************************
namespace
{
	DaedalusVtx4		temp_a[ 10 ];
	DaedalusVtx4		temp_b[ 10 ];

	const u32			MAX_CLIPPED_VERTS = 1024;	// Probably excessively large...
	DaedalusVtx4		clipped_vertices[MAX_CLIPPED_VERTS];
}

//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::PrepareTrisClipped( DaedalusVtx ** p_p_vertices, u32 * p_num_vertices ) const
{
	DAEDALUS_PROFILE( "PSPRenderer::PrepareTrisClipped" );

	//
	//	At this point all vertices are lit/projected and have both transformed and projected
	//	vertex positions. For the best results we clip against the projected vertex positions,
	//	but use the resulting intersections to interpolate the transformed positions.
	//	The clipping is more efficient in normalised device coordinates, but rendering these
	//	directly prevents the PSP performing perspective correction. We could invert the projection
	//	matrix and use this to back-project the clip planes into world coordinates, but this
	//	suffers from various precision issues. Carrying around both sets of coordinates gives
	//	us the best of both worlds :)
	//
	u32 num_vertices = 0;

	for(u32 i = 0; i+2 < m_dwNumIndices; i+=3)
	{
		u32 idx0 = m_swIndexBuffer[ i + 0 ];
		u32 idx1 = m_swIndexBuffer[ i + 1 ];
		u32 idx2 = m_swIndexBuffer[ i + 2 ];

		if(mVtxProjected[idx0].ClipFlags | mVtxProjected[idx1].ClipFlags | mVtxProjected[idx2].ClipFlags)
		{
			temp_a[ 0 ] = mVtxProjected[ idx0 ];
			temp_a[ 1 ] = mVtxProjected[ idx1 ];
			temp_a[ 2 ] = mVtxProjected[ idx2 ];

			DL_PF( "i%d: %f,%f,%f,%f", i+0, temp_a[0].ProjectedPos.x, temp_a[0].ProjectedPos.y, temp_a[0].ProjectedPos.z, temp_a[0].ProjectedPos.w );
			DL_PF( "i%d: %f,%f,%f,%f", i+1, temp_a[1].ProjectedPos.x, temp_a[1].ProjectedPos.y, temp_a[1].ProjectedPos.z, temp_a[1].ProjectedPos.w );
			DL_PF( "i%d: %f,%f,%f,%f", i+2, temp_a[2].ProjectedPos.x, temp_a[2].ProjectedPos.y, temp_a[2].ProjectedPos.z, temp_a[2].ProjectedPos.w );

			u32 out = clip_tri_to_frustum_vfpu( temp_a, temp_b );
			if( out < 3 )
				continue;

			// Retesselate
			u32 new_num_vertices( num_vertices + (out - 3) * 3 );
			if( new_num_vertices > MAX_CLIPPED_VERTS )
			{
				DAEDALUS_ERROR( "Too many clipped verts: %d", new_num_vertices );
				break;
			}
			for( u32 j = 0; j <= out - 3; ++j )
			{
				clipped_vertices[ num_vertices + 0 ] = temp_a[ 0 ];
				clipped_vertices[ num_vertices + 1 ] = temp_a[ j + 1 ];
				clipped_vertices[ num_vertices + 2 ] = temp_a[ j + 2 ];

				num_vertices += 3;
			}
		}
		else
		{
			if( num_vertices + 3 > MAX_CLIPPED_VERTS )
			{
				DAEDALUS_ERROR( "Too many clipped verts: %d", num_vertices + 3 );
				break;
			}

			clipped_vertices[ num_vertices + 0 ] = mVtxProjected[ idx0 ];
			clipped_vertices[ num_vertices + 1 ] = mVtxProjected[ idx1 ];
			clipped_vertices[ num_vertices + 2 ] = mVtxProjected[ idx2 ];

			num_vertices += 3;
		}
	}

	//
	//	Now the vertices have been clipped we need to write them into
	//	a buffer we obtain this from the display list.
	//  ToDo: Test Allocating vertex buffers to VRAM
	//	Maybe we should allocate all vertex buffers from VRAM?
	//
	DaedalusVtx *	p_vertices( (DaedalusVtx*)sceGuGetMemory(num_vertices*sizeof(DaedalusVtx)) );
	_ConvertVertices( p_vertices, clipped_vertices, num_vertices );
	*p_p_vertices = p_vertices;
	*p_num_vertices = num_vertices;
}

//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::PrepareTrisUnclipped( DaedalusVtx ** p_p_vertices, u32 * p_num_vertices ) const
{
	DAEDALUS_PROFILE( "PSPRenderer::PrepareTrisUnclipped" );
	DAEDALUS_ASSERT( m_dwNumIndices > 0, "The number of indices should have been checked" );

	u32				num_vertices( m_dwNumIndices );
	DaedalusVtx *	p_vertices( (DaedalusVtx*)sceGuGetMemory(num_vertices*sizeof(DaedalusVtx)) );

	//
	//	Previously this code set up an index buffer to avoid processing the
	//	same vertices more than once - we avoid this now as there is apparently
	//	quite a large performance penalty associated with using these on the PSP.
	//
	//	http://forums.ps2dev.org/viewtopic.php?t=4703
	//
	//  ToDo: Test Allocating vertex buffers to VRAM
	//	ToDo: Why Indexed below?
	DAEDALUS_STATIC_ASSERT( MAX_CLIPPED_VERTS > ARRAYSIZE(m_swIndexBuffer) );
	_ConvertVerticesIndexed( p_vertices, mVtxProjected, num_vertices, m_swIndexBuffer );
	*p_p_vertices = p_vertices;
	*p_num_vertices = num_vertices;
}

//*****************************************************************************
// Used for TexRect, TexRectFlip, FillRect
//*****************************************************************************
bool	PSPRenderer::RenderTriangleList( const DaedalusVtx * p_verts, u32 num_verts, bool disable_zbuffer )
{
	//Always passed  as true
	//if ( disable_zbuffer )
	//{
		gNeedZBufferUdate = true;
	//}


	DaedalusVtx*	p_vertices( (DaedalusVtx*)sceGuGetMemory(num_verts*sizeof(DaedalusVtx)) );
	memcpy( p_vertices, p_verts, num_verts*sizeof(DaedalusVtx));

	sceGuSetMatrix( GU_PROJECTION, reinterpret_cast< const ScePspFMatrix4 * >( &gMatrixIdentity ) );
	RenderUsingCurrentBlendMode( p_vertices, num_verts, RM_RENDER_2D, disable_zbuffer );

	return true;
}

//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::SetNewVertexInfo(u32 dwAddress, u32 dwV0, u32 dwNum)
{
	//DAEDALUS_PROFILE( "PSPRenderer::SetNewVertexInfo" );
	SetNewVertexInfoVFPU(dwAddress, dwV0, dwNum);
}

static u32 CalcClipFlags( const v4 & projected )
{
	u32 clip_flags = 0;
	if		(-projected.x > projected.w)	clip_flags |= X_POS;
	else if (+projected.x > projected.w)	clip_flags |= X_NEG;

	if		(-projected.y > projected.w)	clip_flags |= Y_POS;
	else if (+projected.y > projected.w)	clip_flags |= Y_NEG;

	if		(-projected.z > projected.w)	clip_flags |= Z_POS;
	else if (+projected.z > projected.w)	clip_flags |= Z_NEG;

	return clip_flags;
}

void PSPRenderer::TestVFPUVerts( u32 v0, u32 num, const FiddledVtx * verts, const Matrix4x4 & mat_world )
{
	bool	env_map( (mTnLModeFlags & (TNL_LIGHT|TNL_TEXGEN)) == (TNL_LIGHT|TNL_TEXGEN) );

	u32 vend( v0 + num );
	for (u32 i = v0; i < vend; i++)
	{
		const FiddledVtx & vert = verts[i - v0];
		const v4 &	projected( mVtxProjected[i].ProjectedPos );

		if (mTnLModeFlags & TNL_FOG)
		{
			float eyespace_z = projected.z / projected.w;
			float fog_coeff = (eyespace_z * mTnLParams.FogMult) + mTnLParams.FogOffset;

			// Set the alpha
			f32 value = Clamp< f32 >( fog_coeff, 0.0f, 1.0f );

			if( vfpu_abs( value - mVtxProjected[i].Colour.w ) > 0.01f )
			{
				printf( "Fog wrong: %f != %f\n", mVtxProjected[i].Colour.w, value );
			}
		}

		if (mTnLModeFlags & TNL_TEXTURE)
		{
			// Update texture coords n.b. need to divide tu/tv by bogus scale on addition to buffer

			// If the vert is already lit, then there is no normal (and hence we
			// can't generate tex coord)
			float tx, ty;
			if (env_map)
			{
				v3 vecTransformedNormal;		// Used only when TNL_LIGHT set
				v3	model_normal(f32( vert.norm_x ), f32( vert.norm_y ), f32( vert.norm_z ) );

				vecTransformedNormal = mat_world.TransformNormal( model_normal );
				vecTransformedNormal.Normalise();

				const v3 & norm = vecTransformedNormal;

				// Assign the spheremap's texture coordinates
				tx = (0.5f * ( 1.0f + ( norm.x*mat_world.m11 +
										norm.y*mat_world.m21 +
										norm.z*mat_world.m31 ) ));

				ty = (0.5f * ( 1.0f - ( norm.x*mat_world.m12 +
										norm.y*mat_world.m22 +
										norm.z*mat_world.m32 ) ));
			}
			else
			{
				tx = (float)vert.tu * mTnLParams.TextureScaleX;
				ty = (float)vert.tv * mTnLParams.TextureScaleY;
			}

			if( vfpu_abs(tx - mVtxProjected[i].Texture.x ) > 0.0001f ||
				vfpu_abs(ty - mVtxProjected[i].Texture.y ) > 0.0001f )
			{
				printf( "tx/y wrong : %f,%f != %f,%f (%s)\n", mVtxProjected[i].Texture.x, mVtxProjected[i].Texture.y, tx, ty, env_map ? "env" : "scale" );
			}
		}

		//
		//	Initialise the clipping flags (always done on the VFPU, so skip here)
		//
		u32 flags = CalcClipFlags( projected );
		if( flags != mVtxProjected[i].ClipFlags )
		{
			printf( "flags wrong: %02x != %02x\n", mVtxProjected[i].ClipFlags, flags );
		}
	}
}

template< bool FogEnable, int TextureMode >
void PSPRenderer::ProcessVerts( u32 v0, u32 num, const FiddledVtx * verts, const Matrix4x4 & mat_world )
{
	u32 vend( v0 + num );
	for (u32 i = v0; i < vend; i++)
	{
		const FiddledVtx & vert = verts[i - v0];
		const v4 &	projected( mVtxProjected[i].ProjectedPos );

		if (FogEnable)
		{
			float	fog_coeff;
			//if(fabsf(projected.w) > 0.0f)
			{
				float eyespace_z = projected.z / projected.w;
				fog_coeff = (eyespace_z * mTnLParams.FogMult) + mTnLParams.FogOffset;
			}
			//else
			//{
			//	fog_coeff = m_fFogOffset;
			//}

			// Set the alpha
			mVtxProjected[i].Colour.w = Clamp< f32 >( fog_coeff, 0.0f, 1.0f );
		}

		if (TextureMode == 2)
		{
			// Update texture coords n.b. need to divide tu/tv by bogus scale on addition to buffer

			// If the vert is already lit, then there is no normal (and hence we
			// can't generate tex coord)
			v3 vecTransformedNormal;		// Used only when TNL_LIGHT set
			v3	model_normal(f32( vert.norm_x ), f32( vert.norm_y ), f32( vert.norm_z ) );

			vecTransformedNormal = mat_world.TransformNormal( model_normal );
			vecTransformedNormal.Normalise();

			const v3 & norm = vecTransformedNormal;

			// Assign the spheremap's texture coordinates
			mVtxProjected[i].Texture.x = (0.5f * ( 1.0f + ( norm.x*mat_world.m11 +
														    norm.y*mat_world.m21 +
														    norm.z*mat_world.m31 ) ));

			mVtxProjected[i].Texture.y = (0.5f * ( 1.0f - ( norm.x*mat_world.m12 +
														    norm.y*mat_world.m22 +
														    norm.z*mat_world.m32 ) ));
		}
		else if(TextureMode == 1)
		{
			mVtxProjected[i].Texture.x = (float)vert.tu * mTnLParams.TextureScaleX;
			mVtxProjected[i].Texture.y = (float)vert.tv * mTnLParams.TextureScaleY;
			//mVtxProjected[i].t1 = mVtxProjected[i].t0;
		}

		//
		//	Initialise the clipping flags (always done on the VFPU, so skip here)
		//
		mVtxProjected[i].ClipFlags = CalcClipFlags( projected );
	}
}

//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::SetNewVertexInfoVFPU(u32 dwAddress, u32 dwV0, u32 dwNum)
{
	const FiddledVtx * const pVtxBase( (const FiddledVtx*)(g_pu8RamBase + dwAddress) );

	const Matrix4x4 & matWorld( mModelViewStack[mModelViewTop] );
	const Matrix4x4 & matWorldProject( GetWorldProject() );

	switch( mTnLModeFlags & (TNL_TEXTURE|TNL_TEXGEN|TNL_FOG|TNL_LIGHT) )
	{
		// TNL_TEXGEN is ignored when TNL_LIGHT is disabled
	case                                   0: _TransformVerticesWithColour_f0_t0( &matWorld, &matWorldProject, pVtxBase, &mVtxProjected[dwV0], dwNum, &mTnLParams ); break;
	case                         TNL_TEXTURE: _TransformVerticesWithColour_f0_t1( &matWorld, &matWorldProject, pVtxBase, &mVtxProjected[dwV0], dwNum, &mTnLParams ); break;
	case            TNL_TEXGEN              : _TransformVerticesWithColour_f0_t0( &matWorld, &matWorldProject, pVtxBase, &mVtxProjected[dwV0], dwNum, &mTnLParams ); break;
	case            TNL_TEXGEN | TNL_TEXTURE: _TransformVerticesWithColour_f0_t1( &matWorld, &matWorldProject, pVtxBase, &mVtxProjected[dwV0], dwNum, &mTnLParams ); break;
	case  TNL_FOG                           : _TransformVerticesWithColour_f1_t0( &matWorld, &matWorldProject, pVtxBase, &mVtxProjected[dwV0], dwNum, &mTnLParams ); break;
	case  TNL_FOG |              TNL_TEXTURE: _TransformVerticesWithColour_f1_t1( &matWorld, &matWorldProject, pVtxBase, &mVtxProjected[dwV0], dwNum, &mTnLParams ); break;
	case  TNL_FOG | TNL_TEXGEN              : _TransformVerticesWithColour_f1_t0( &matWorld, &matWorldProject, pVtxBase, &mVtxProjected[dwV0], dwNum, &mTnLParams ); break;
	case  TNL_FOG | TNL_TEXGEN | TNL_TEXTURE: _TransformVerticesWithColour_f1_t1( &matWorld, &matWorldProject, pVtxBase, &mVtxProjected[dwV0], dwNum, &mTnLParams ); break;

		// TNL_TEXGEN is ignored when TNL_TEXTURE is disabled
	case TNL_LIGHT                                     : _TransformVerticesWithLighting_f0_t0( &matWorld, &matWorldProject, pVtxBase, &mVtxProjected[dwV0], dwNum, &mTnLParams, mLights, m_dwNumLights ); break;
	case TNL_LIGHT |                        TNL_TEXTURE: _TransformVerticesWithLighting_f0_t1( &matWorld, &matWorldProject, pVtxBase, &mVtxProjected[dwV0], dwNum, &mTnLParams, mLights, m_dwNumLights ); break;
	case TNL_LIGHT |           TNL_TEXGEN              : _TransformVerticesWithLighting_f0_t0( &matWorld, &matWorldProject, pVtxBase, &mVtxProjected[dwV0], dwNum, &mTnLParams, mLights, m_dwNumLights ); break;
	case TNL_LIGHT |           TNL_TEXGEN | TNL_TEXTURE: _TransformVerticesWithLighting_f0_t2( &matWorld, &matWorldProject, pVtxBase, &mVtxProjected[dwV0], dwNum, &mTnLParams, mLights, m_dwNumLights ); break;
	case TNL_LIGHT | TNL_FOG                           : _TransformVerticesWithLighting_f1_t0( &matWorld, &matWorldProject, pVtxBase, &mVtxProjected[dwV0], dwNum, &mTnLParams, mLights, m_dwNumLights ); break;
	case TNL_LIGHT | TNL_FOG |              TNL_TEXTURE: _TransformVerticesWithLighting_f1_t1( &matWorld, &matWorldProject, pVtxBase, &mVtxProjected[dwV0], dwNum, &mTnLParams, mLights, m_dwNumLights ); break;
	case TNL_LIGHT | TNL_FOG | TNL_TEXGEN              : _TransformVerticesWithLighting_f1_t0( &matWorld, &matWorldProject, pVtxBase, &mVtxProjected[dwV0], dwNum, &mTnLParams, mLights, m_dwNumLights ); break;
	case TNL_LIGHT | TNL_FOG | TNL_TEXGEN | TNL_TEXTURE: _TransformVerticesWithLighting_f1_t2( &matWorld, &matWorldProject, pVtxBase, &mVtxProjected[dwV0], dwNum, &mTnLParams, mLights, m_dwNumLights ); break;

	default:
		NODEFAULT;
		break;
	}

	//TestVFPUVerts( dwV0, dwNum, pVtxBase, matWorld );
}

//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::SetNewVertexInfoCPU(u32 dwAddress, u32 dwV0, u32 dwNum)
{
	//DBGConsole_Msg(0, "In SetNewVertexInfo");
	FiddledVtx * pVtxBase = (FiddledVtx*)(g_pu8RamBase + dwAddress);

	const Matrix4x4 & matWorld( mModelViewStack[mModelViewTop] );
	const Matrix4x4 & matWorldProject( GetWorldProject() );

	// Transform and Project + Lighting
	// or
	// Transform and Project with Colour

	u32 i;
	for (i = dwV0; i < dwV0 + dwNum; i++)
	{
		const FiddledVtx & vert = pVtxBase[i - dwV0];

		v4		w( f32( vert.x ), f32( vert.y ), f32( vert.z ), 1.0f );

		mVtxProjected[i].ProjectedPos = matWorldProject.Transform( w );
		mVtxProjected[i].TransformedPos = matWorld.Transform( w );

		DL_PF( "p%d: (%f,%f,%f) -> (%f,%f,%f,%f)", i, w.x, w.y, w.z, mVtxProjected[i].ProjectedPos.x, mVtxProjected[i].ProjectedPos.y, mVtxProjected[i].ProjectedPos.z, mVtxProjected[i].ProjectedPos.w );

		if (mTnLModeFlags & TNL_LIGHT)
		{
			v3	model_normal(f32( vert.norm_x ), f32( vert.norm_y ), f32( vert.norm_z ) );

			v3 vecTransformedNormal;		// Used only when TNL_LIGHT set
			vecTransformedNormal = matWorld.TransformNormal( model_normal );
			vecTransformedNormal.Normalise();

			mVtxProjected[i].Colour = LightVert(vecTransformedNormal);
		}
		else
		{
			mVtxProjected[i].Colour = v4( vert.rgba_r / 255.0f, vert.rgba_g / 255.0f, vert.rgba_b / 255.0f, vert.rgba_a / 255.0f );
		}
	}

	//
	//	Template processing function to hoist conditional checks out of loop
	//
	switch( mTnLModeFlags & (TNL_TEXTURE|TNL_TEXGEN|TNL_FOG|TNL_LIGHT) )
	{
		// TNL_TEXGEN is ignored when TNL_LIGHT is disabled
	case                                              0: ProcessVerts< false, 0 >( dwV0, dwNum, pVtxBase, matWorld );	break;
	case                                    TNL_TEXTURE: ProcessVerts< false, 1 >( dwV0, dwNum, pVtxBase, matWorld );	break;
	case                       TNL_TEXGEN              : ProcessVerts< false, 0 >( dwV0, dwNum, pVtxBase, matWorld );	break;
	case                       TNL_TEXGEN | TNL_TEXTURE: ProcessVerts< false, 1 >( dwV0, dwNum, pVtxBase, matWorld );	break;
	case             TNL_FOG                           : ProcessVerts< true,  0 >( dwV0, dwNum, pVtxBase, matWorld );	break;
	case             TNL_FOG |              TNL_TEXTURE: ProcessVerts< true,  1 >( dwV0, dwNum, pVtxBase, matWorld );	break;
	case             TNL_FOG | TNL_TEXGEN              : ProcessVerts< true,  0 >( dwV0, dwNum, pVtxBase, matWorld );	break;
	case             TNL_FOG | TNL_TEXGEN | TNL_TEXTURE: ProcessVerts< true,  1 >( dwV0, dwNum, pVtxBase, matWorld );	break;

		// TNL_TEXGEN is ignored when TNL_TEXTURE is disabled
	case TNL_LIGHT                                     : ProcessVerts< false, 0 >( dwV0, dwNum, pVtxBase, matWorld );	break;
	case TNL_LIGHT |                        TNL_TEXTURE: ProcessVerts< false, 1 >( dwV0, dwNum, pVtxBase, matWorld );	break;
	case TNL_LIGHT |           TNL_TEXGEN              : ProcessVerts< false, 0 >( dwV0, dwNum, pVtxBase, matWorld );	break;
	case TNL_LIGHT |           TNL_TEXGEN | TNL_TEXTURE: ProcessVerts< false, 2 >( dwV0, dwNum, pVtxBase, matWorld );	break;
	case TNL_LIGHT | TNL_FOG                           : ProcessVerts< true,  0 >( dwV0, dwNum, pVtxBase, matWorld );	break;
	case TNL_LIGHT | TNL_FOG |              TNL_TEXTURE: ProcessVerts< true,  1 >( dwV0, dwNum, pVtxBase, matWorld );	break;
	case TNL_LIGHT | TNL_FOG | TNL_TEXGEN              : ProcessVerts< true,  0 >( dwV0, dwNum, pVtxBase, matWorld );	break;
	case TNL_LIGHT | TNL_FOG | TNL_TEXGEN | TNL_TEXTURE: ProcessVerts< true,  2 >( dwV0, dwNum, pVtxBase, matWorld );	break;

	default:
		NODEFAULT;
		break;
	}
}
//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::ModifyVertexInfo(u32 where, u32 vert, u32 val)
{
	switch ( where )
	{
		case G_MWO_POINT_RGBA:
			{
				DL_PF("      Setting RGBA to 0x%08x", val);
				SetVtxColor( vert, c32( val ) );
			}
			break;

		case G_MWO_POINT_ST:
			{
				s16 tu = s16(val>>16);
				s16 tv = s16(val & 0xFFFF);
				DL_PF( "      Setting tu/tv to %f, %f", tu/32.0f, tv/32.0f );
				SetVtxTextureCoord( vert, tu, tv );
			}
			break;

		case G_MWO_POINT_XYSCREEN:
			{
				u16 x = u16(val>>16);
				u16 y = u16(val & 0xFFFF);
				DL_PF( "      Setting XY Screen to 0x%08x (%d,%d)", val, x, y );
				SetVtxXY( vert, x / 4.0f, y / 4.0f );
			}
			break;

		case G_MWO_POINT_ZSCREEN:
			{
				DL_PF( "      Setting ZScreen to 0x%08x", val );
			}
			break;
	}
}
//*****************************************************************************
// Assumes dwAddress has already been checked!
// Don't inline - it's too big with the transform macros
// DKR seems to use longer vert info
//*****************************************************************************
void PSPRenderer::SetNewVertexInfoDKR(u32 dwAddress, u32 dwV0, u32 dwNum)
{
	s32 nFogR, nFogG, nFogB, nFogA;

	if (mTnLModeFlags & TNL_FOG)
	{
		nFogR = mFogColour.GetR();
		nFogG = mFogColour.GetG();
		nFogB = mFogColour.GetB();
		nFogA = mFogColour.GetA();
	}
	s16 * pVtxBase = (s16*)(g_pu8RamBase + dwAddress);

	const Matrix4x4 & matWorldProject( GetWorldProject() );

	u32 i;
	s32 nOff;

	nOff = 0;
	for (i = dwV0; i < dwV0 + dwNum; i++)
	{
		v4 w;
		u32 dwFlags;

		w.x = (float)pVtxBase[(nOff + 0) ^ 1];
		w.y = (float)pVtxBase[(nOff + 1) ^ 1];
		w.z = (float)pVtxBase[(nOff + 2) ^ 1];
		w.w = 1.0f;

		v4 & projected( mVtxProjected[i].ProjectedPos );
		projected = matWorldProject.Transform( w );	// Convert to w=1

		//float	rhw;
		//if(fabsf(projected.w) > 0.0f)
		//{
		//	rhw = 1.0f / ( projected.w );
		//}
		//else
		//{
		//	rhw = 1.0f;
		//}

		dwFlags = 0;

		// Clip
		mVtxProjected[i].ClipFlags = dwFlags;

		s16 wA = pVtxBase[(nOff + 3) ^ 1];
		s16 wB = pVtxBase[(nOff + 4) ^ 1];

		s8 r = (s8)(wA >> 8);
		s8 g = (s8)(wA);
		s8 b = (s8)(wB >> 8);
		s8 a = (s8)(wB);

		v3 vecTransformedNormal;
		v4 colour;

		if (mTnLModeFlags & TNL_LIGHT)
		{
			v3 mn;		// modelnorm
			const Matrix4x4 & matWorld( mModelViewStack[mModelViewTop] );

			mn.x = (float)r; //norma_x;
			mn.y = (float)g; //norma_y;
			mn.z = (float)b; //norma_z;

			vecTransformedNormal = matWorld.TransformNormal( mn );
			vecTransformedNormal.Normalise();

			colour = LightVert(vecTransformedNormal);
			colour.w = a / 255.0f;
		}
		else
		{
			colour = v4( r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f );
		}

		// Assign true vert colour after lighting/fogging
		mVtxProjected[i].Colour = colour;

		/*if (mTnLModeFlags & TNL_TEXTURE)
		{
			// Update texture coords n.b. need to divide tu/tv by bogus scale on addition to buffer
			v2 & t = m_vecTexture[i];

			// If the vert is already lit, then there is no normal (and hence we
			// can't generate tex coord)
			if ((mTnLModeFlags & (TNL_LIGHT|TNL_TEXGEN)) == (TNL_LIGHT|TNL_TEXGEN))
			{
				const Matrix4x4 & matWV = mModelViewStack[mModelViewTop];
				v3 & norm = vecTransformedNormal;

				// Assign the spheremap's texture coordinates
				t.x = (0.5f * ( 1.0f + ( norm.x*matWV.m00 +
										 norm.y*matWV.m10 +
										 norm.z*matWV.m20 ) ));

				t.y = (0.5f * ( 1.0f - ( norm.x*matWV.m01 +
										 norm.y*matWV.m11 +
										 norm.z*matWV.m21 ) ));
			}
			else
			{
				t.x = (float)vert.t.x * mTnLParams.TextureScaleX;
				t.y = (float)vert.t.y * mTnLParams.TextureScaleY;
			}
		}*/

		nOff += 5;
	}

}

//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::SetVtxColor( u32 vert, c32 color )
{
	if ( vert < MAX_VERTS )
	{
		mVtxProjected[vert].Colour = color.GetColourV4();
	}
}

//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::SetVtxTextureCoord( u32 vert, short tu, short tv )
{
	if ( vert < MAX_VERTS )
	{
		mVtxProjected[vert].Texture.x = (float)tu / 32.0f;
		mVtxProjected[vert].Texture.y = (float)tv / 32.0f;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::SetVtxXY( u32 vert, float x, float y )
{
	if ( vert < MAX_VERTS )
	{
		// XXX Needs reprojection?
		// printf( "SetVtxXY\n" );
		mVtxProjected[vert].TransformedPos.x = x;
		mVtxProjected[vert].TransformedPos.y = y;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::SetLightCol(u32 dwLight, u32 colour)
{
	mLights[dwLight].Colour.x = (float)((colour >> 24)&0xFF) / 255.0f;
	mLights[dwLight].Colour.y = (float)((colour >> 16)&0xFF) / 255.0f;
	mLights[dwLight].Colour.z = (float)((colour >>  8)&0xFF) / 255.0f;
	mLights[dwLight].Colour.w = 1.0f;	// Ignore light alpha
}

//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::SetLightDirection(u32 l, float x, float y, float z)
{
	v3		normal( x, y, z );
	normal.Normalise();

	mLights[l].Direction.x = normal.x;
	mLights[l].Direction.y = normal.y;
	mLights[l].Direction.z = normal.z;
	mLights[l].Padding0 = 0.0f;
}

//*****************************************************************************
// Init matrix stack to identity matrices
//*****************************************************************************
void PSPRenderer::ResetMatrices()
{
	Matrix4x4 mat;

	mat.SetIdentity();

	mProjectionTop = 0;
	mModelViewTop = 0;
	mProjectionStack[0] = mat;
	mModelViewStack[0] = mat;
	mWorldProjectValid = false;
}

//*****************************************************************************
//
//*****************************************************************************
void	PSPRenderer::EnableTexturing( u32 tile_idx )
{
	EnableTexturing( 0, tile_idx );

	// XXXX Not required for texrect etc?
#ifdef RDP_USE_TEXEL1

	if ( gRDPOtherMode.text_lod )
	{
		// LOD is enabled - use the highest detail texture in texel1
		EnableTexturing( 1, tile_idx );
	}
	else
	{
		// LOD is disabled - use two textures
		EnableTexturing( 1, tile_idx+1 );
	}
#endif
}

//*****************************************************************************
//
//*****************************************************************************
void	PSPRenderer::EnableTexturing( u32 index, u32 tile_idx )
{
	DAEDALUS_PROFILE( "PSPRenderer::EnableTexturing" );

	DAEDALUS_ASSERT( tile_idx < 8, "Invalid tile index %d", tile_idx );
	DAEDALUS_ASSERT( index < NUM_N64_TEXTURES, "Invalid texture index %d", index );

	const TextureInfo &		ti( gRDPStateManager.GetTextureDescriptor( tile_idx ) );

	//
	//	Initialise the wrapping/texture offset first, which can be set
	//	independently of the actual texture.
	//
	const RDP_Tile &		rdp_tile( gRDPStateManager.GetTile( tile_idx ) );
	const RDP_TileSize &	tile_size( gRDPStateManager.GetTileSize( tile_idx ) );

	//
	// Initialise the clamping state. When the mask is 0, it forces clamp mode.
	//
	int mode_u = (rdp_tile.clamp_s || rdp_tile.mask_s == 0) ? GU_CLAMP : GU_REPEAT;
	int mode_v = (rdp_tile.clamp_t || rdp_tile.mask_t == 0)	? GU_CLAMP : GU_REPEAT;

	//
	//	In CRDPStateManager::GetTextureDescriptor, we limit the maximum dimension of a
	//	texture to that define by the mask_s/mask_t value.
	//	It this happens, the tile size can be larger than the truncated width/height
	//	as the rom can set clamp_s/clamp_t to wrap up to a certain value, then clamp.
	//	We can't support both wrapping and clamping (without manually repeating a texture...)
	//	so we choose to prefer wrapping.
	//	The castle in the background of the first SSB level is a good example of this behaviour.
	//	It sets up a texture with a mask_s/t of 6/6 (64x64), but sets the tile size to
	//	256*128. clamp_s/t are set, meaning the texture wraps 4x and 2x.
	//
	if( tile_size.GetWidth() > ti.GetWidth() )
	{
		mode_u = GU_REPEAT;
	}
	if( tile_size.GetHeight() > ti.GetHeight() )
	{
		mode_v = GU_REPEAT;
	}

	sceGuTexWrap( mode_u, mode_v );

	// XXXX Double check this
	mTileTopLeft[ index ] = v2( f32( tile_size.left)/4.0f, f32(tile_size.top)/4.0f );

	DL_PF( "     *Performing texture map load:" );
	DL_PF( "     *  Address: 0x%08x, Pitch: %d, Format: %s, Size: %dbpp, %dx%d",
			ti.GetLoadAddress(), ti.GetPitch(),
			ti.GetFormatName(), ti.GetSizeInBits(),
			ti.GetWidth(), ti.GetHeight() );

	if( mpTexture[ index ] != NULL && mpTexture[ index ]->GetTextureInfo() == ti )
		return;


	// Check for 0 width/height textures
	if( ti.GetWidth() == 0 || ti.GetHeight() == 0 )
	{
		DAEDALUS_DL_ERROR( "Loading texture with 0 width/height" );
	}
	else
	{
		CRefPtr<CTexture>	texture( CTextureCache::Get()->GetTexture( &ti ) );

		if( texture != NULL )
		{
			//
			//	Avoid update check and divides if the texture is already installed
			//
			if( texture != mpTexture[ index ] )
			{
				mpTexture[ index ] = texture;

				texture->UpdateIfNecessary();

				const CRefPtr<CNativeTexture> & native_texture( texture->GetTexture() );
				if( native_texture != NULL )
				{
					mTileScale[ index ] = native_texture->GetScale();
				}
			}
		}
	}
}

//*****************************************************************************
//
//*****************************************************************************
void	PSPRenderer::SetCullMode( bool bCullFront, bool bCullBack )
{
	m_bCullFront = bCullFront;
	m_bCullBack = bCullBack;
}

//*****************************************************************************
//
//*****************************************************************************
void	PSPRenderer::SetScissor( s32 x0, s32 y0, s32 x1, s32 y1 )
{
	v2		n64_coords_tl( x0, y0 );
	v2		n64_coords_br( x1, y1 );

	v2		psp_coords_tl( ConvertN64ToPsp( n64_coords_tl ) );
	v2		psp_coords_br( ConvertN64ToPsp( n64_coords_br ) );

	// N.B. Think the arguments are x0,y0,x1,y1, and not x,y,w,h as the docs describe
	sceGuScissor( s32(psp_coords_tl.x), s32(psp_coords_tl.y),
				  s32(psp_coords_br.x), s32(psp_coords_br.y) );
}

//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::SetProjection(const Matrix4x4 & mat, bool bPush, EMatrixLoadStyle load_style)
{
	if (bPush)
	{
		if (mProjectionTop >= (MATRIX_STACK_SIZE-1))
			DBGConsole_Msg(0, "Pushing past proj stack limits! %d/%d", mProjectionTop, MATRIX_STACK_SIZE);
		else
			mProjectionTop++;

		if (load_style == MATRIX_LOAD)
			// Load projection matrix
			mProjectionStack[mProjectionTop] = mat;
		else
			mProjectionStack[mProjectionTop] = mat * mProjectionStack[mProjectionTop-1];

	}
	else
	{
		if (load_style == MATRIX_LOAD)
			// Load projection matrix
			mProjectionStack[mProjectionTop] = mat;
		else
			mProjectionStack[mProjectionTop] = mat * mProjectionStack[mProjectionTop];
	}

	mWorldProjectValid = false;
}

//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::SetWorldView(const Matrix4x4 & mat, bool bPush, EMatrixLoadStyle load_style)
{
	// ModelView
	if (bPush)
	{
		if (mModelViewTop >= (MATRIX_STACK_SIZE-1))
			DBGConsole_Msg(0, "Pushing past modelview stack limits! %d/%d", mModelViewTop, MATRIX_STACK_SIZE);
		else
			mModelViewTop++;

		// We should store the current projection matrix...
		if (load_style == MATRIX_LOAD)
		{
			// Load projection matrix
			mModelViewStack[mModelViewTop] = mat;
		}
		else			// Multiply projection matrix
		{
			mModelViewStack[mModelViewTop] = mat * mModelViewStack[mModelViewTop-1];
		}
	}
	else	// NoPush
	{
		if (load_style == MATRIX_LOAD)
		{
			// Load projection matrix
			mModelViewStack[mModelViewTop] = mat;
		}
		else
		{
			// Multiply projection matrix
			mModelViewStack[mModelViewTop] = mat * mModelViewStack[mModelViewTop];
		}
	}

	mWorldProjectValid = false;
}

//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::PopProjection()
{
	if (mProjectionTop > 0)
		mProjectionTop--;

	mWorldProjectValid = false;
}

//*****************************************************************************
//
//*****************************************************************************
void PSPRenderer::PopWorldView()
{
	if (mModelViewTop > 0)
		mModelViewTop--;

	mWorldProjectValid = false;
}

//*****************************************************************************
//
//*****************************************************************************
const Matrix4x4 & PSPRenderer::GetWorldProject() const
{
	if( !mWorldProjectValid )
	{
		mWorldProject = mModelViewStack[mModelViewTop] * mProjectionStack[mProjectionTop];
		mWorldProjectValid = true;
	}

	return mWorldProject;
}

//*****************************************************************************
//
//*****************************************************************************

void PSPRenderer::PrintActive()
{
#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	if (gDisplayListFile != NULL)
	{
		const Matrix4x4 & mat( GetWorldProject() );
		DL_PF(
			" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\n"
			" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\n"
			" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\n"
			" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\n",
			mat.m[0][0], mat.m[0][1], mat.m[0][2], mat.m[0][3],
			mat.m[1][0], mat.m[1][1], mat.m[1][2], mat.m[1][3],
			mat.m[2][0], mat.m[2][1], mat.m[2][2], mat.m[2][3],
			mat.m[3][0], mat.m[3][1], mat.m[3][2], mat.m[3][3]);
	}
#endif
}

//*****************************************************************************
//
//*****************************************************************************

void PSPRenderer::Draw2DTexture( float imageX, float imageY, float frameX, float frameY, float imageW, float imageH, float frameW, float frameH)
{
	DAEDALUS_PROFILE( "PSPRenderer::Draw2DTexture" );
	TextureVtx *p_verts = (TextureVtx*)sceGuGetMemory(2*sizeof(TextureVtx));

	sceGuDisable(GU_DEPTH_TEST);
	sceGuDepthMask( GL_TRUE );
	sceGuShadeModel( GU_FLAT );

	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	sceGuDisable(GU_ALPHA_TEST);
	sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);

	sceGuEnable(GU_BLEND);
	sceGuTexWrap(GU_CLAMP, GU_CLAMP);

	p_verts[0].pos.x = frameX*mN64ToPSPScale.x + mN64ToPSPTranslate.x; // (Frame X Offset * X Scale Factor) + Screen X Offset
	p_verts[0].pos.y = frameY*mN64ToPSPScale.y + mN64ToPSPTranslate.y; // (Frame Y Offset * Y Scale Factor) + Screen Y Offset
	p_verts[0].pos.z = 0;
	p_verts[0].t0    = v2(imageX, imageY);				   // Source coordinates

	p_verts[1].pos.x = p_verts[0].pos.x + (frameW * mN64ToPSPScale.x); // Translated X Offset + (Image Width  * X Scale Factor)
	p_verts[1].pos.y = p_verts[0].pos.y + (frameH * mN64ToPSPScale.y); // Translated Y Offset + (Image Height * Y Scale Factor)
	p_verts[1].pos.z = 0;
	p_verts[1].t0    = v2(imageW, imageH);				   // Source dimentions

	sceGuDrawArray( GU_SPRITES, GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2, 0, p_verts);
}
