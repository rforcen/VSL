//---------------------------------------------------------------------------
#ifndef MusicFreqH
#define MusicFreqH

#include <QColor>
#include <QString>

class MusicFreq {
  static constexpr double MUSICAL_INC = 1.0594630943593,  // 2^(1/12)
      LOG_MUSICAL_INC = 0.0577622650466,
               baseC0 = 261.62556530061,  // 440 * MUSICAL_INC^(-9)
      LOG_baseC0 = 5.5669143414923, LOG2 = 0.6931471805599;

 public:
  static QColor Freq2Color(double freq);
  static double FreqInOctave(double f, int oct);
  static double FreqInOctaves(double f, int oct0, int oct1);
  static QString NoteString(int i);
  static QString NoteString(double freq);
  static double NoteOct2Freq(int note, int oct);
  static int Freq2NoteOct(double freq, int &note, int &oct, QString &NoteOct,
                          double &err);
  static int Freq2NoteOct(double freq, int &note, int &oct, QString &NoteOct);
  static QString Freq2NoteOct(double freq);
  static int Freq2Oct(double freq);
  static int Freq2Note(double freq);
  static double ErrInNote(double freq);
  static bool FreqInOctRange(double freq, int octDown, int octUp);
  static double Freq2Element(double freq);
  static QString Freq2StrNote(double freq);
  static void Frm2OctaveRange(double *HzFrm, int nform, int oct = 0);
  static double NoteFit(double hz);
  static QStringList note_str() {
    return {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  }
  static QString NoteStr2ColorName(QString note);
  static QString Freq2ColorName(double freq);
};
//---------------------------------------------------------------------------
#endif
