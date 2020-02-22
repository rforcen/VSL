// ---------------------------------------------------------------------------

#ifndef ColorIndexH
#define ColorIndexH

#include <ColorScale.h>
#include <QOpenGLWidget>

// ---------------------------------------------------------------------------
class ColorIndex {
 public:
  int* colorIndex;
  int nColors, colorFrom, colorTo;
  float color4f[3];  // rgb in float 0..1

  ColorIndex() { createColorIndex(512, 0x0000ff, 0xff0000); }

  ColorIndex(int nColors, int colorFrom, int colorTo) {
    createColorIndex(nColors, colorFrom, colorTo);
  }

  void createColorIndex(int nColors, int colorFrom, int colorTo) {
    this->nColors = nColors;
    this->colorFrom = colorFrom;
    this->colorTo = colorTo;
    colorIndex = new int[nColors];
    for (int i = 0; i < nColors; i++)
      colorIndex[i] = ColorScaleHSL(colorFrom, colorTo, (float)i / nColors);
  }

  int getColor(float r) { return colorIndex[(int)(nColors * r) % nColors]; }

  float* getColorRGBA(float r) {  // alpha = 1
    int col = getColor(r);
    float* cv = new float[4];
    cv[0] = getRedf(col), cv[1] = getGreenf(col);
    cv[2] = getBluef(col), cv[3] = 1.0f;
    return cv;
  }

  static int getRed(int color) {
    color &= 0x00ffffff;
    return (color >> 16) & 0xff;
  }

  static int getGreen(int color) {
    color &= 0x00ffffff;
    return (color >> 8) & 0xff;
  }

  static int getBlue(int color) {
    color &= 0x00ffffff;
    return (color)&0xff;
  }

  static float getRedf(int color) { return getRed(color) / 255.f; }

  static float getGreenf(int color) { return getGreen(color) / 255.f; }

  static float getBluef(int color) { return getBlue(color) / 255.f; }

  void glColor(int color) {
    glColor4f(getRedf(color), getGreenf(color), getBluef(color), 1);
  }
};
#endif
