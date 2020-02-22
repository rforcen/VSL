#ifndef AUDIOOUT_H
#define AUDIOOUT_H

#include <QAudio>
#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QIODevice>
#include <QObject>

class AudioOut : public QIODevice {
  Q_OBJECT

 public:
  AudioOut(int sampleRate = 44100, int channels = 2, int bits = 16,
           bool floating_point = false);
  ~AudioOut() {
    if (audio) delete audio;
  }
  void start();
  void stop();
  void switchPlay();
  bool setFormat(int sampleRate, int channels, int bits,
                 bool floating_point = false);

  QAudioDeviceInfo device_info;
  QAudioFormat format;
  QAudioOutput *audio = nullptr;

  bool ok = false, isPlaying = false, modified_format = false;
  int sampleRate, bits, channels;
  bool floating_point = false;
  QAudio::Error err;

 protected:
  qint64 readData(char *data, qint64 maxlen);
  qint64 writeData(const char *data, qint64 len);
  qint64 bytesAvailable() const;
 signals:
  void dataRequest(char *data, qint64 maxlen);  // on write data
  void prepareWave();                           // on start
                                                /* connect signals
                                                 *
                                                    connect(audioOut, &AudioOut::dataRequest, this, &MainWindow::genWave); //
                                              audio support prep, gen                                               connect(audioOut, &AudioOut::prepareWave, this,
                                              &MainWindow::prepWave);

                                                     void genWave(char*data, qint64 maxlen);
                                                       void prepWave();

                                            void MainWindow::genWave(char*data, qint64 maxlen) {
                                                wave->genAM((short*)data, maxlen/2);
                                            }
                                            void MainWindow::prepWave() {
                                                if(wave) delete wave;
                                                wave=new TWave(audioOut->channels, audioOut->sampleRate);
                                            #define phi 1.618
                                            #define nElem(v) (sizeof(v)/sizeof(*v))

                                                double amp=16000, a[]={amp, amp/phi, amp/phi}, f[]={freq, freq*phi, freq/phi};
                                                wave->SetDif(MusicFreq::FreqInOctave(freq,-8)); // bw
                                                wave->Set(a,f,NULL,nElem(a));
                                            }
                                                    */
};

#endif  // AUDIOOUT_H
