#ifndef uChordTableH
#define uChordTableH

#include <ColorScale.h>
#include <MusicFreq.h>
#include <QList>
#include <QStringList>

class ChordTable {
 public:
  enum { chord, abbrev, notes };
  typedef struct {
    QString cn;  // "1" -> note id
    int nn;      // 0 -> number
  } Note2Number;

  const QList<Note2Number> n2n{
      {"1", 0},   {"3", 4},  {"5", 7}, {"b3", 3}, {"b5", 6},  {"bb7", 8},
      {"b7", 9},  {"#5", 8}, {"6", 9}, {"7", 10}, {"b9", 13}, {"11", 17},
      {"13", 21}, {"4", 5},  {"2", 1}, {"9", 14}, {"#9", 15},
  };
  const QList<QStringList> chordTable = {
      {"Major", "M", "1,3,5"},
      {"Minor", "M", "1,b3,5"},
      {"Diminished", "dim", "1,b3,b5,bb7"},
      {"Half diminished", "M7b5", "1,b3,b5,b7"},
      {"Augmented", "aug", "1,3,#5"},
      {"5th", "5", "1,5"},
      {"7th", "7", "1,3,5,b7"},
      {"Minor 7th", "M7", "1,b3,5,b7"},
      {"Major 7th", "maj7", "1,3,5,7"},
      {"Suspended 4th", "sus4", "1,4,5"},
      {"Suspended 2nd", "sus2", "1,2,5"},
      {"7th suspended 4th", "7sus4", "1,4,5,b7"},
      {"7th suspended 2nd", "7sus2", "1,2,5,b7"},
      {"Added 2nd", "add2", "1,2,3,5"},
      {"Added 9th", "add9", "1,3,5,9"},
      {"Added 4th", "add4", "1,3,4,5"},
      {"6th", "6", "1,3,5,6"},
      {"Minor 6th", "M6", "1,b3,5,6"},
      {"6/9", "6/9", "1,3,5,6,9"},
      {"9th", "9", "1,3,5,b7,9"},
      {"Minor 9th", "M9", "1,b3,5,b7,9"},
      {"Major 9th", "maj9", "1,3,5,7,9"},
      {"11th", "11", "1,3,5,b7,9,11"},
      {"Minor 11th", "M11", "1,b3,5,b7,9,11"},
      {"Major 11th", "maj11", "1,3,5,7,9,11"},
      {"13th", "13", "1,3,5,b7,9,11,13"},
      {"Minor 13th", "M13", "1,b3,5,b7,9,11,13"},
      {"Major 13th", "maj13", "1,3,5,7,9,11,13"},
      {"7th sharp 9th", "7#9", "1,3,5,b7,#9"},
      {"7th flat 9th", "7b9", "1,3,5,b7,b9"},
      {"7th sharp 5th", "7#5", "1,3,#5,b7"},
      {"7th flat 5th", "7b5", "1,3,b5,b7"}};

  QVector<QVector<int>> noteNumber = extractNotes();

  ChordTable() {}

  QVector<QVector<int>> extractNotes() {
    QVector<QVector<int>> nnn;
    for (auto ct : chordTable) {
      QVector<int> nn;
      for (auto nts : ct[notes].split(','))
        for (auto in : n2n)
          if (in.cn == nts) {
            nn << in.nn;
            break;
          }
      nnn << nn;
    }
    return nnn;
  }

  QString getChordName(int i) { return chordTable[i][chord]; }

  int getnChords() { return chordTable.size(); }

  QColor getColor(int i) {
    auto nts = noteNumber[i];
    QColor c1, color;

    for (int j = 0; j < nts.size(); j++) {
      nts[j] = nts[j] % 12;
      c1 = MusicFreq::Freq2Color(MusicFreq::NoteOct2Freq(nts[j], 0));
      if (j == 0)
        color = c1;
      else {
        ColorReff c =
            ColorScaleHSL(color.rgb(), c1.rgb(), 0.7 / j);  // difference
        int r = (c & 0xff0000) >> 16, g = (c & 0x00ff00) >> 8,
            b = (c & 0x0000ff);  // swap red with blue
        color.setRgb(g, b, r);
      }
    }

    return color;
  }
};

#endif
