///////////////////////////// ColorScale.h ////////////////////////////////////
#ifndef COLOR_SCALE_H
#define COLOR_SCALE_H

#define ColorReff unsigned int	// 0 to 0xFFFFFF and 0xFFFFFFFF for transparencies

// TColor(BGR) 2 ColorRef(RGB)
#define TColor2ColorRef(c)  (ColorReff)( (((c)&0xff0000)>>16) | (((c)&0x00ff00)) | (((c)&0x0000ff)<<16) )
//#define ColorRef2TColor(c)  (TColor  )( (((c)&0xff0000)>>16) | (((c)&0x00ff00)) | (((c)&0x0000ff)<<16) )

// Create a 0xRRGGBB from separate 8 bits colors - no error checking
#define MakeRGB(r,g,b) ( ((r)<<16) | ((g)<<8) | (b))
#define _getRED(c)   ((((c)&0xFF0000)>>16))
#define _getGREEN(c) ((((c)&0x00FF00)>>8))
#define _getBLUE(c)  ((((c)&0x0000FF)))

#define _getREDf(c)   (_getRED(c)/256.)
#define _getGREENf(c) (_getGREEN(c)/256.)
#define _getBLUEf(c)  (_getBLUE(c)/256.)
#define Max(x,y) (((x)>(y))?(x):(y))
#define Min(x,y) (((x)<(y))?(x):(y))

extern ColorReff ColorScaleRGB( ColorReff Col1,  ColorReff Col2,  float Ratio);
extern ColorReff ColorScaleHSL( ColorReff Col1,  ColorReff Col2,  float Ratio);
extern ColorReff ColorScaleRGB3( ColorReff Col1,  ColorReff Col2,  ColorReff Col3,
                                float Ratio1, float Ratio2, float Ratio3);
extern ColorReff ColorScaleHSL3( ColorReff Col1,  ColorReff Col2,  ColorReff Col3,
                                float Ratio1, float Ratio2, float Ratio3);

extern void       RGBtoHLS( ColorReff rgb, double &H, double &L, double &S );
extern ColorReff	HLStoRGB( double H,  double L,  double S );

#endif
