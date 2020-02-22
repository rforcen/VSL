//---------------------------------------------------------------------------
//  MusicFreq.cpp   27/08/2001    RFN
//	 Frequency <--> Musical notes conversor.
//	 class TMusicFreq
//---------------------------------------------------------------------------

#include "MusicFreq.h"
#include <float.h>
#include <math.h>
#include "ColorScale.h"

//---------------------------------------------------------------------------
// Freq2Color, convert freq to color
//---------------------------------------------------------------------------
QColor MusicFreq::Freq2Color(double freq) {
  int oct = Freq2Oct(freq);  // get note and freq. err
  double f0 = NoteOct2Freq(0, oct), fz = NoteOct2Freq(0, oct + 1),
         ratio = (freq - f0) / (fz - f0);

  // ratio between RED, VIOLET
  ColorReff c =
      ColorScaleHSL(MakeRGB(0xff, 0, 0), MakeRGB(0xff, 0, 0xff), ratio);
  int r = (c & 0xff0000) >> 16, g = (c & 0x00ff00) >> 8,
      b = (c & 0x0000ff);  // swap red with blue
  return QColor::fromRgb(r, g, b);
}
// FreqInOctave. convert 'f' to octave 'o'
double MusicFreq::FreqInOctave(double f, int o) {
  double fb, ft;
  f = fabs(f);
  fb = NoteOct2Freq(0, o);
  ft = NoteOct2Freq(11, o);  // rage freq in octave
  if (f > fb)
    for (; f && f > fb && !(f >= fb && f <= ft); f /= 2)
      ;  // in octave
  else
    for (; f && f < ft && !(f >= fb && f <= ft); f *= 2)
      ;
  return f;
}  // FreqInOctave. convert 'f' to octave range  'o0..o1'
double MusicFreq::FreqInOctaves(double f, int oct0, int oct1) {
  double fb, ft;
  f = fabs(f);
  fb = NoteOct2Freq(0, oct0);
  ft = NoteOct2Freq(11, oct1);  // rage freq in octave
  if (f > fb)
    for (; f && f > fb && !(f >= fb && f <= ft); f /= 2)
      ;  // in octave
  else
    for (; f && f < ft && !(f >= fb && f <= ft); f *= 2)
      ;
  return f;
}

QString MusicFreq::NoteString(int i) {
  if (i >= 0 && i <= 12)
    return note_str()[i];
  else
    return "";
}
QString MusicFreq::NoteString(double freq) {
  int i = Freq2Note(freq);
  if (i >= 0 && i <= 12)
    return note_str()[i];
  else
    return "";
}

//---------------------------------------------------------------------------
// NoteOct2Freq, convert Note/oct to freq
//---------------------------------------------------------------------------
double MusicFreq::NoteOct2Freq(int note, int oct) {
  return (baseC0 * pow(MUSICAL_INC, note + 12. * oct));
}

// QString note of a freq
QString MusicFreq::Freq2StrNote(double freq) {
  return NoteString(Freq2Note(freq));
}
//---------------------------------------------------------------------------
//	Freq2NoteOct, convert Freq to note/oct and error
// 0,  1  2   3  4  5   6  7   8  9  10 11
// C, C#, D, D#, E, F, F#, G, G#, A, A#, B
//---------------------------------------------------------------------------
QString MusicFreq::Freq2NoteOct(double freq) {
  int n, o;
  QString no;
  Freq2NoteOct(freq, n, o, no);
  return NoteString(n) + "/" + QString::number(o);
}

double MusicFreq::NoteFit(double hz) {
  int n, o;
  QString no;
  double err;
  Freq2NoteOct(hz, n, o, no, err);
  return NoteOct2Freq(n, o);
}

int MusicFreq::Freq2NoteOct(double freq, int &note, int &oct,
                            QString &NoteOct) {
  int n, o;
  QString no;
  double err;
  Freq2NoteOct(freq, note, oct, NoteOct, err);
  n = note;
  o = oct;
  no = NoteOct;
  if (++n > 11) {
    n = 0;
    o++;
  }  // freq of next note
  if (NoteOct2Freq(n, o) - freq < err) {
    note = n;
    oct = o;
    NoteOct = NoteString(n);
  }  // next note
  return oct * 12 + ((oct >= 0) ? note : -note);
}

int MusicFreq::Freq2NoteOct(double freq, int &note, int &oct, QString &NoteOct,
                            double &err) {
  if (freq <= 0) {
    note = oct = 0;
    err = 0;
    return 0;
  }
  // oct = floor( log2(freq/baseC0) )
  // note = baseC0 * MUSICAl_INC ^(note+12*oct)
  double lfB = log(freq) - LOG_baseC0;
  oct = (int)floor(lfB / LOG2);
  note = (int)(lfB / LOG_MUSICAL_INC - oct * 12.);
  // QString
  if (NoteOct != NULL)
    NoteOct = QString(note_str()[note]) +
              ((oct < 0) ? QString(oct) : "+" + QString(oct));
  // error
  err = freq - NoteOct2Freq(note, oct);
  return oct * 12 + ((oct >= 0) ? note : -note);
}

// Hz 2 octave
int MusicFreq::Freq2Oct(double freq) {
  if (freq <= 0) return -999;
  return (int)floor((log(freq) - LOG_baseC0) / LOG2);
}

// Hz 2 note
int MusicFreq::Freq2Note(double freq) {
  if (freq <= 0) return 0;
  return (int)((log(freq) - LOG_baseC0) / LOG_MUSICAL_INC -
               Freq2Oct(freq) * 12.);
}

// Error in note calc
double MusicFreq::ErrInNote(double freq) {
  return freq - NoteOct2Freq(Freq2Note(freq), Freq2Oct(freq));
}

// limit of freq in octave range
bool MusicFreq::FreqInOctRange(double freq, int octDown, int octUp) {
  if (freq < 0 || freq > DBL_MAX / 2) return false;
  int o = Freq2Oct(freq);
  return (o >= octDown) && (o <= octUp);
}

// convert a freq. to a element freq in octave -4
double MusicFreq::Freq2Element(double freq) {
  double fb = NoteOct2Freq(11, -4);  // B(-4) upper limit
  if (freq == 0) freq = 0.01;
  if (freq > fb)
    for (; freq > fb; freq /= 2.)
      ;
  else
    for (; freq < fb / 2; freq *= 2.)
      ;
  return freq;
}

// convert Hzfrm freq to 'oct'
void MusicFreq::Frm2OctaveRange(double *HzFrm, int nform, int oct) {
  for (int i = 0; i < nform; i++) HzFrm[i] = FreqInOctave(HzFrm[i], oct);
}
// note in string fmt. i.e. C, B# to color name '#rrggbb'
QString MusicFreq::NoteStr2ColorName(QString note) {
  int nnote = note_str().indexOf(note.trimmed());
  double freq = NoteOct2Freq(nnote, 0);
  QColor col = Freq2Color(freq);
  return col.name();
}
QString MusicFreq::Freq2ColorName(double freq) {
  QColor col = Freq2Color(freq);
  return col.name();
}
