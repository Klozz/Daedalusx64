#ifndef GSP_SPRITE2D_H
#define GSP_SPRITE2D_H

//
//	Sprite2D
//

void DLParser_GBI1_Sprite2DBase( MicroCodeCommand command );
void DLParser_GBI1_Sprite2DScaleFlip( MicroCodeCommand command );
void DLParser_GBI1_Sprite2DDraw( MicroCodeCommand command );

typedef struct {
  u32 SourceImagePointer;
  u32 TlutPointer;

  u16 SubImageWidth;
  u16 Stride;

  u8  SourceImageBitSize;
  u8  SourceImageType;
  u16 SubImageHeight;

  u16 SourceImageOffsetT;
  u16 SourceImageOffsetS;

  char  dummy[4];
} SpriteStruct;

typedef struct{
        u16 px;
        u16 py;
        float scaleX;
        float scaleY;
        u8  flipX;
        u8  flipY;
        SpriteStruct *spritePtr;
} Sprite2DInfo;

#endif /* GSP_SPRITE2D_H */
