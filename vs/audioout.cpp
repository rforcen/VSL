#include "audioout.h"

AudioOut::AudioOut(int sampleRate, int channels, int bits,
                   bool floating_point) {
  setFormat(sampleRate, channels, bits, floating_point);
}

bool AudioOut::setFormat(int sampleRate, int channels, int bits,
                         bool floating_point) {
  this->sampleRate = sampleRate;
  this->bits = bits;
  this->channels = channels;
  this->floating_point = floating_point;

  // default output device
  device_info = QAudioDeviceInfo::defaultOutputDevice();
  if (device_info.isNull()) {
    audio = nullptr;
    printf("error: no audio default device\n");
  } else {
    // set up  format
    format.setSampleRate(sampleRate);
    format.setChannelCount(channels);
    format.setSampleSize(bits);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);
    if (floating_point)
      format.setSampleType(QAudioFormat::Float);
    else
      format.setSampleType(QAudioFormat::SignedInt);

    modified_format = !device_info.isFormatSupported(format);

    if (modified_format) {
      format = device_info.nearestFormat(format);  // reassign format values
      this->sampleRate = format.sampleRate();
      this->bits = format.bytesPerFrame();
      this->channels = format.channelCount();
    }

    audio = new QAudioOutput(device_info,
                             format);  // new QAudioOutput(format, this);
    if (audio) audio->setVolume(1);
  }
  return ok = (audio != nullptr);
}

qint64 AudioOut::readData(char *data, qint64 maxlen) {
  emit dataRequest(data, maxlen);
  return maxlen;
}
qint64 AudioOut::writeData(const char *data, qint64 len) {
  (void)len;
  (void)data;
  return 0;
}
qint64 AudioOut::bytesAvailable() const { return QIODevice::bytesAvailable(); }
void AudioOut::start() {
  if (ok) {
    ok = open(QIODevice::ReadOnly);
    if (!ok) printf("error opening audio out device\n");

    emit prepareWave();
    audio->start(this);

    isPlaying = ok = ((err = audio->error()) == QAudio::NoError);

    if (!ok)
      printf("error starting audio device, error: %s\n",
             errorString().toLatin1().data());
  }
}
void AudioOut::stop() {
  if (ok) {
    close();
    audio->stop();
    isPlaying = false;
  }
}
void AudioOut::switchPlay() {  // start/stop toggle
  if (ok) {
    if (isPlaying)
      stop();
    else
      start();
  }
}
