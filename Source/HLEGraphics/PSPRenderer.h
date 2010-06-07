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

#ifndef __DAEDALUS_D3DRENDER_H__
#define __DAEDALUS_D3DRENDER_H__

#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"
#include "Utility/Singleton.h"
#include "Utility/RefCounted.h"
#include "DaedalusVtx.h"
#include "Graphics/ColourValue.h"
#include "BlendModes.h"

#include <set>
#include <map>


class CTexture;
class CNativeTexture;
class CBlendStates;
struct FiddledVtx;
struct TextureVtx;

ALIGNED_TYPE(struct, DaedalusLight, 16)
{
	v3		Direction;		// w component is ignored. Should be normalised
	f32		Padding0;
	v4		Colour;			// Colour, components in range 0..1
};

DAEDALUS_STATIC_ASSERT( sizeof( DaedalusLight ) == 32 );

ALIGNED_TYPE(struct, TnLParams, 16)
{
	v4			Ambient;
    float		FogMult;
	float		FogOffset;
	float		TextureScaleX;
	float		TextureScaleY;
};
DAEDALUS_STATIC_ASSERT( sizeof( TnLParams ) == 32 );


//*****************************************************************************
//
//*****************************************************************************
class PSPRenderer : public CSingleton< PSPRenderer >
{
protected:
	friend class CSingleton< PSPRenderer >;
	PSPRenderer();

public:
	~PSPRenderer();

	void				BeginScene();
	void				EndScene();

	void				SetVIScales();
	void				Reset();

	// Verious rendering states
	void				SetTextureEnable(bool enable)			{ if( enable ) mTnLModeFlags |= TNL_TEXTURE; else mTnLModeFlags &= ~TNL_TEXTURE; }
	void				SetFogEnable(bool enable)				{ if( enable ) mTnLModeFlags |= TNL_FOG;	 else mTnLModeFlags &= ~TNL_FOG; }
	void				SetLighting(bool enable)				{ if( enable ) mTnLModeFlags |= TNL_LIGHT;	 else mTnLModeFlags &= ~TNL_LIGHT; }
	void				SetTextureGen(bool enable)				{ if( enable ) mTnLModeFlags |= TNL_TEXGEN;  else mTnLModeFlags &= ~TNL_TEXGEN; }

	void				SetPrimitiveColour( c32 colour )		{ mPrimitiveColour = colour; }
	void				SetEnvColour( c32 colour )				{ mEnvColour = colour; }
	void				ZBufferEnable(bool bZBuffer)			{ m_bZBuffer = bZBuffer; }

	inline void			SetNumLights(u32 dwNumLights)			{ m_dwNumLights = dwNumLights; }
	void				SetLightCol(u32 light, u32 colour);
	void				SetLightDirection(u32 l, float x, float y, float z);
	inline void			SetAmbientLight( const v4 & colour )	{ mTnLParams.Ambient = colour; }

	inline void			SetFogMult( float fFogMult )			{ mTnLParams.FogMult = fFogMult; }
	inline void			SetFogOffset( float fFogOffset )		{ mTnLParams.FogOffset = fFogOffset; }
	void				SetFogColour( c32 colour )				{ mFogColour = colour; }

	void				SetCullMode( bool bCullFront, bool bCullBack );
	void				SetSmooth( bool bSmooth )				{ mSmooth = bSmooth; }
	void				SetSmoothShade( bool bSmoothShade )		{ mSmoothShade = bSmoothShade; }
	void				SetAlphaRef(u32 dwAlpha)				{ mAlphaThreshold = dwAlpha; }

	// Texture stuff
	void				SetTextureScale(float fScaleX, float fScaleY)	{ mTnLParams.TextureScaleX = fScaleX; mTnLParams.TextureScaleY = fScaleY; }
	void                Draw2DTexture(float, float, float, float, float, float, float, float);

	// Viewport stuff
	void				SetPSPViewport( s32 x, s32 y, u32 w, u32 h );
	void				SetN64Viewport( const v3 & scale, const v3 & trans );
	void				SetScissor( s32 x0, s32 y0, s32 x1, s32 y1 );

	// Matrix stuff
	enum EMatrixLoadStyle
	{
		MATRIX_LOAD,
		MATRIX_MUL,
	};

	void				ResetMatrices();
	void				PrintActive();
	void				SetProjection(const Matrix4x4 & mat, bool bPush, bool bReplace);
	void				SetWorldView(const Matrix4x4 & mat, bool bPush, bool bReplace);
	void				PopProjection();
	void				PopWorldView();

	// Vertex stuff	
	void				SetNewVertexInfoVFPU_No_Light(u32 address, u32 v0, u32 n);	// For conker..	
	void				SetNewVertexInfoVFPU(u32 address, u32 v0, u32 n);	// Assumes dwAddress has already been checked!	
	void				ModifyVertexInfo(u32 w, u32 vert, u32 val);
	void				SetNewVertexInfoDKR(u32 dwAddress, u32 dwV0, u32 dwNum);	// Assumes dwAddress has already been checked!	
	void				SetVtxColor( u32 vert, c32 color );
	void				SetVtxTextureCoord( u32 vert, short tu, short tv );
	void				SetVtxXY( u32 vert, float x, float y );

	// TextRect stuff
	bool				TexRect( u32 tile_idx, const v2 & xy0, const v2 & xy1, const v2 & uv0, const v2 & uv1 );
	bool				TexRectFlip( u32 tile_idx, const v2 & xy0, const v2 & xy1, const v2 & uv0, const v2 & uv1 );
	bool				FillRect( const v2 & xy0, const v2 & xy1, u32 color );
		
	// Returns true if triangle visible and rendered, false otherwise
	bool				AddTri(u32 v0, u32 v1, u32 v2);

	bool				FlushTris();
	
	bool				TestVerts( u32 v0, u32 vn ) const;				// Returns true if bounding volume is visible, false if culled
	v4					GetTransformedVtxPos( u32 i ) const		{ return mVtxProjected[ i ].TransformedPos; }
	u32					GetVtxFlags( u32 i ) const				{ return mVtxProjected[ i ].ClipFlags; }

	// Rendering stats
#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	u32					GetNumTrisRendered() const				{ return m_dwNumTrisRendered; }
	u32					GetNumTrisClipped() const				{ return m_dwNumTrisClipped; }
#endif

	// Debugging
#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	void					SetRecordCombinerStates( bool enable )	{ mRecordCombinerStates = enable; }					// Sets whether combiner states will be recorded for the subsequent frames
	const std::set<u64> &	GetRecordedCombinerStates() const		{ return mRecordedCombinerStates; }

	bool				IsCombinerStateUnhandled( u64 state ) const	{ return mUnhandledCombinderStates.find( state ) != mUnhandledCombinderStates.end(); }
	bool				IsCombinerStateDisabled( u64 state ) const	{ return mDisabledCombinerStates.find( state ) != mDisabledCombinerStates.end(); }
	void				DisableCombinerState( u64 state )			{ mDisabledCombinerStates.insert( state ); }
	void				EnableCombinerState( u64 state )			{ mDisabledCombinerStates.erase( state ); }

	void				ToggleDisableCombinerState( u64 state )		{ if( IsCombinerStateDisabled( state )) { EnableCombinerState(state); } else { DisableCombinerState( state ); } }

#endif

	struct SBlendStateEntry
	{
		SBlendStateEntry() : OverrideFunction( NULL ), States( NULL ) {}
		OverrideBlendModeFn			OverrideFunction;
		const CBlendStates *		States;
	};

	SBlendStateEntry	LookupBlendState( u64 mux, bool two_cycles );

private:
	void				EnableTexturing( u32 tile_idx );
	void				EnableTexturing( u32 index, u32 tile_idx );

	bool				RestoreRenderStates();

	void				UpdateViewport();

	v2					ConvertN64ToPsp( const v2 & n64_coords ) const;

	enum ERenderMode
	{
		RM_RENDER_2D,
		RM_RENDER_3D,
	};

	void				RenderUsingRenderSettings( const CBlendStates * states, DaedalusVtx * p_vertices, u32 num_vertices, u32 render_flags );
	void				RenderUsingCurrentBlendMode( DaedalusVtx * p_vertices, u32 num_vertices, ERenderMode mode, bool disable_zbuffer );

	void 				TestVFPUVerts( u32 v0, u32 num, const FiddledVtx * verts, const Matrix4x4 & mat_world );

	template< bool FogEnable, int TextureMode >
	void ProcessVerts( u32 v0, u32 num, const FiddledVtx * verts, const Matrix4x4 & mat_world );

	//void				SetNewVertexInfoCPU(u32 dwAddress, u32 dwV0, u32 dwNum);	// Assumes dwAddress has already been checked!	


	void				PrepareTrisClipped( DaedalusVtx ** p_p_vertices, u32 * p_num_vertices ) const;
	void				PrepareTrisUnclipped( DaedalusVtx ** p_p_vertices, u32 * p_num_vertices ) const;

	v4					LightVert( const v3 & norm ) const;

	bool				RenderTriangleList( const DaedalusVtx * p_verts, u32 num_verts, bool disable_zbuffer = false );

	enum EPlaceholderTextureType
	{
		PTT_WHITE = 0,
		PTT_SELECTED,
		PTT_MISSING,
	};

	void				SelectPlaceholderTexture( EPlaceholderTextureType type );

private:
	enum { MAX_VERTS = 80 };		// F3DLP.Rej supports up to 80 verts!

	TnLParams			mTnLParams;

	v2					mN64ToPSPScale;
	v2					mN64ToPSPTranslate;

	v3					mVpScale;
	v3					mVpTrans;

	enum ETnLModeFlags
	{
		TNL_TEXTURE		= 1 << 0,
		TNL_TEXGEN		= 1 << 1,
		TNL_FOG			= 1 << 2,
		TNL_LIGHT		= 1 << 3,
	};

	u32					mTnLModeFlags;

	s32					m_dwNumLights;

	bool				m_bZBuffer;

	bool				m_bCullFront;
	bool				m_bCullBack;
	
	u32					mAlphaThreshold;

	bool				mSmooth;
	bool				mSmoothShade;

	c32					mFogColour;
	c32					mPrimitiveColour;
	c32					mEnvColour;

	DaedalusLight		mLights[8];

	// Texturing
	static const u32 NUM_N64_TEXTURES = 2;

	CRefPtr<CTexture>	mpTexture[ NUM_N64_TEXTURES ];
	v2					mTileTopLeft[ NUM_N64_TEXTURES ];
	v2					mTileScale[ NUM_N64_TEXTURES ];

	static const u32 MATRIX_STACK_SIZE = 20; // XXXX Do not go over 60.

	const Matrix4x4 &	GetWorldProject() const;

	mutable Matrix4x4	mWorldProject;
	Matrix4x4			mProjectionStack[MATRIX_STACK_SIZE];
	Matrix4x4			mModelViewStack[MATRIX_STACK_SIZE];
	u32					mProjectionTop;
	u32					mModelViewTop;
	mutable bool		mWorldProjectValid;
		
	u16					m_swIndexBuffer[500];
	u32					m_dwNumIndices;

	// Processed vertices waiting for output...
	DaedalusVtx4		mVtxProjected[MAX_VERTS];			// Transformed and projected vertices (suitable for clipping etc)
	u32					mVtxClipFlagsUnion;					// Bitwise OR of all the vertex flags added to the current batch. If this is 0, we can trivially accept everything without clipping

	//
	//	BlendMode support
	//
	CBlendStates *		mCopyBlendStates;
	CBlendStates *		mFillBlendStates;
	typedef std::map< u64, SBlendStateEntry > BlendStatesMap;
	BlendStatesMap		mBlendStatesMap;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	//
	// Stats
	//
	u32					m_dwNumTrisRendered;
	u32					m_dwNumTrisClipped;

	// Debugging
	bool				mRecordCombinerStates;
	std::set< u64 >		mRecordedCombinerStates;
	std::set< u64 >		mDisabledCombinerStates;
#endif
	std::set< u64 >		mUnhandledCombinderStates;
};

struct FiddledVtx
{
        s16 y;
        s16 x;

        s16 flag;
        s16 z;

        s16 tv;
        s16 tu;

        union
        {
                struct
                {
                        u8 rgba_a;
                        u8 rgba_b;
                        u8 rgba_g;
                        u8 rgba_r;
                };
                struct
                {
                        s8 norm_a;
                        s8 norm_z;      // b
                        s8 norm_y;      // g
                        s8 norm_x;      // r
                };
        };
};

struct TextureVtx
{
	v2  t0;
	v3  pos;
};

struct FiddledVtxDKR
{
	s16 y;
	s16 x;

	u8 a;
	u8 b;
	s16 z;

	u8 g;
	u8 r;
};

DAEDALUS_STATIC_ASSERT( sizeof(FiddledVtx) == 16 );

#endif // __DAEDALUS_D3DRENDER_H__
