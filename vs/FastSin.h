#ifndef FastSinH
#define FastSinH
//------------------------------------------------------------------------------
// TFastSin
// calc y(n) = sin(n*W+B)
// W, B in RAD
// usage:
//     TFastSin fs(amp, Hz, samp, phase);
//     fs.calc();
//
class FastSin {
  double y0, y1, y2, p;
  double Zfade, xfade;

 public:
  double cW, cB, cA, x;
  double n;
  bool isInit;

  FastSin(void);
  FastSin(double a, double w, double b);
  FastSin(double amp, double Hz, double Samp, double phase);
  void init(double a, double w, double b);
  void init(double amp, double Hz, double Samp, double phase);
  void deinit() { isInit = false; }
  void reset(void);
  double calc(void);
  double calcSin(double x);
  void SetAmp(double amp);
  double Freq2Inc(double freq, double samp);
  void FillBuffer(short int* sbuff, int sb);
  double initFade(double samp, double sec);
  double Fade(void);
};
#endif
