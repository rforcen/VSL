//------------------------------------------------------------------------------
// TFastSin
// calc y(n) = sin(n*W+B)
// W, B in RAD
// usage TFastSin fs(a,w,b)
//
#include "FastSin.h"
#include <math.h>

FastSin::FastSin(void) { isInit = false; }
FastSin::FastSin(double a, double w, double b) { init(a, w, b); }

FastSin::FastSin(double amp, double Hz, double Samp, double phase) {
  init(amp, Hz, Samp, phase);
}

void FastSin::init(double amp, double Hz, double Samp,
                   double phase) {  // init and save for reset
  isInit = true;
  cA = amp;
  cW = Freq2Inc(Hz, Samp);
  cB = phase;
  y0 = sin(-2 * cW + cB);
  y1 = sin(-cW + cB);
  p = 2.0 * cos(cW);
  n = -1;
  x = 0;
}

void FastSin::init(double a, double w, double b) {  // init and save for reset
  isInit = true;
  cA = a;
  cW = w;
  cB = b;
  y0 = sin(-2 * w + b);
  y1 = sin(-w + b);
  p = 2.0 * cos(w);
  n = -1;
  x = 0;
}

void FastSin::reset(void) {  // start n=0;
  init(cA, cW, cB);
}

// set amplitude.
void FastSin::SetAmp(double amp) { cA = amp; }

// sin(wt+phase + x)
double FastSin::calcSin(double px) {
  n++;
  x = n * cW;
  return cA * sin(cW * n + cB + px);
}

double FastSin::calc(void) {
  n++;
  x = n * cW;
  y2 = p * y1 - y0;
  y0 = y1;
  y1 = y2;
  return cA * y2;  // mutl by amp.
}

double FastSin::Freq2Inc(double freq, double samp) {
  return freq * 2. * M_PI / samp;
}

// fill a short int mono buffer
void FastSin::FillBuffer(short int* sbuff, int sb) {
  for (int i = 0; i < sb; i++) {  //
    sbuff[i] = (short)calc();
  }
}

double FastSin::initFade(double samp, double sec) {
  return xfade = Zfade = pow(10, -5 / (samp * sec));
}

double FastSin::Fade(void) {
  xfade *= Zfade;
  return xfade;
}
