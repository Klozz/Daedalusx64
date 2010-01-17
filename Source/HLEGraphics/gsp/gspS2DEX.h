#ifndef GSP_S2DEX_H
#define GSP_S2DEX_H


//*****************************************************************************
// GBI2 S2DEX
//*****************************************************************************

#define	G_GBI2_OBJ_RECTANGLE		0x01
#define	G_GBI2_OBJ_SPRITE			0x02
#define	G_GBI2_SELECT_DL			0x04
#define	G_GBI2_OBJ_LOADTXTR			0x05
#define	G_GBI2_OBJ_LDTX_SPRITE		0x06
#define	G_GBI2_OBJ_LDTX_RECT		0x07
#define	G_GBI2_OBJ_LDTX_RECT_R		0x08
#define	G_GBI2_BG_1CYC				0x09
#define	G_GBI2_BG_COPY				0x0a
#define	G_GBI2_OBJ_RENDERMODE		0x0b
#define	G_GBI2_OBJ_RECTANGLE_R		0xda
#define	G_GBI2_OBJ_MOVEMEM			0xdc
#define	G_GBI2_RDPHALF_0			0xe4

//*****************************************************************************
// GBI1 S2DEX
//*****************************************************************************

#define	G_GBI1_BG_1CYC				0x01
#define	G_GBI1_BG_COPY				0x02
#define	G_GBI1_OBJ_RECTANGLE		0x03
#define	G_GBI1_OBJ_SPRITE			0x04
#define	G_GBI1_OBJ_MOVEMEM			0x05
#define	G_GBI1_SELECT_DL			0xb0
#define	G_GBI1_OBJ_RENDERMODE		0xb1
#define	G_GBI1_OBJ_RECTANGLE_R		0xb2
#define	G_GBI1_OBJ_LOADTXTR			0xc1
#define	G_GBI1_OBJ_LDTX_SPRITE		0xc2
#define	G_GBI1_OBJ_LDTX_RECT		0xc3
#define	G_GBI1_OBJ_LDTX_RECT_R		0xc4
#define	G_GBI1_RDPHALF_0			0xe4

//*****************************************************************************
// Needed by S2DEX
//*****************************************************************************

#define	S2DEX_OBJLT_TXTRBLOCK	0x00001033
#define	S2DEX_OBJLT_TXTRTILE	0x00fc1034
#define	S2DEX_OBJLT_TLUT		0x00000030
#define	S2DEX_BGLT_LOADBLOCK	0x0033
#define	S2DEX_BGLT_LOADTILE		0xfff4


//*****************************************************************************


void DLParser_S2DEX_BgCopy			( MicroCodeCommand command );
void DLParser_S2DEX_SelectDl		( MicroCodeCommand command );
void DLParser_S2DEX_ObjSprite		( MicroCodeCommand command );
void DLParser_S2DEX_ObjRectangle	( MicroCodeCommand command );
void DLParser_S2DEX_ObjRendermode	( MicroCodeCommand command );
void DLParser_S2DEX_ObjLoadTxtr		( MicroCodeCommand command );
void DLParser_S2DEX_ObjLdtxSprite	( MicroCodeCommand command );
void DLParser_S2DEX_ObjLdtxRect		( MicroCodeCommand command );
void DLParser_S2DEX_ObjLdtxRectR	( MicroCodeCommand command );
void DLParser_S2DEX_RDPHalf_0		( MicroCodeCommand command );
void DLParser_S2DEX_ObjMoveMem		( MicroCodeCommand command );
void DLParser_S2DEX_Bg1cyc			( MicroCodeCommand command );
void DLParser_S2DEX_ObjRectangleR	( MicroCodeCommand command );

struct uObjBg
{
        u16 imageW;
        u16 imageX;
        u16 frameW;
        s16 frameX;
        u16 imageH;
        u16 imageY;
        u16 frameH;
        s16 frameY;

        u32 imagePtr;
        u8  imageSiz;
        u8  imageFmt;
        u16 imageLoad;
        u16 imageFlip;
        u16 imagePal;

        u16 tmemH;
        u16 tmemW;
        u16 tmemLoadTH;
        u16 tmemLoadSH;
        u16 tmemSize;
        u16 tmemSizeW;
};

#endif /* GSP_S2DEX_H */
