#ifndef COMMON_H
#define COMMON_H

#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QDataStream>
#include <QFileDialog>
#include <QModelIndex>
#include <QTime>
#include <QVector>

#include <MusicFreq.h>
#include <Thread.h>
#include <Timer.h>
#include <VSL_compiler.h>
#include <audioout.h>
#include <math.h>
#include <riff.h>
#include <scaler.h>
#include <unistd.h>
#include <cfloat>

class Common : public QObject {  // data depot
  Q_OBJECT

 signals:
  void progress(QString msg);
  void finished();
 public slots:
  void finish_gen() {
    processing = false;
    main_th.join();
  }

 public:
  Thread main_th = Thread(1);

  typedef enum { t_int8, t_int16, t_int32, t_float } DataType;

  static constexpr double pi2 = M_PI * 2.;

  const QStringList header = {"amp", "pitch(hz)", "phase"},
                    typeName = {"amp", "freq", "phase"};
  static const int MaxFreq = 384000;

  int sample_rate = 44100, n_channels = 2, bits_sample = 16, nSamples,
      bytes_per_sample;

  DataType data_type;
  bool floating_point = false;
  double scaled_amp;
  double secs = 1, volume = 1;
  double t = 0, t_inc = 0;  // wave generation time lap

  bool data_completed = false;
  static bool processing;

  double progress_value = 0;

  Scaler<double> scaler;

  VSL_compiler compiler;
  QString expression;

  Timer timer;
  long lap = 0;

  class MultiBuffer {
    DataType data_type;

   protected:
    void *ptr = nullptr;

   public:
    MultiBuffer() { ptr = nullptr; }

    inline int8_t &_int8_t_ptr(int i) const {
      return static_cast<int8_t *>(ptr)[i];
    }
    inline int16_t &_int16_t_ptr(int i) const {
      return static_cast<int16_t *>(ptr)[i];
    }
    inline int &_int32_t_ptr(int i) const { return static_cast<int *>(ptr)[i]; }
    inline float &_float_ptr(int i) const {
      return static_cast<float *>(ptr)[i];
    }

    inline void set_value(int i, float d) {
      switch (data_type) {
        case t_int8:
          _int8_t_ptr(i) = int8_t(d);
          break;
        case t_int16:
          _int16_t_ptr(i) = int16_t(d);
          break;
        case t_int32:
          _int32_t_ptr(i) = int32_t(d);
          break;
        case t_float:
          _float_ptr(i) = float(d);
          break;
      }
    }

    inline float get_value(int i) const {
      switch (data_type) {
        case t_int8:
          return float(_int8_t_ptr(i));
        case t_int16:
          return float(_int16_t_ptr(i));
        case t_int32:
          return float(_int32_t_ptr(i));
        case t_float:
          return float(_float_ptr(i));
      }
    }

    int get_data_size() {
      switch (data_type) {
        case t_int8:
          return sizeof(int8_t);
        case t_int16:
          return sizeof(int16_t);
        case t_int32:
          return sizeof(int32_t);
        case t_float:
          return sizeof(float);
      }
    }

    void set_data(void *data) { this->ptr = data; }
    void *get_data() { return ptr; }
    void set_type(DataType dt) { this->data_type = dt; }
  };

  class AudioBuffer : public MultiBuffer {
   public:
    explicit AudioBuffer() {}
    AudioBuffer(int n) : n(n) { set_data(new int8_t[n]); }
    AudioBuffer(const AudioBuffer &) = default;
    AudioBuffer(AudioBuffer &&) = default;
    AudioBuffer &operator=(const AudioBuffer &) = default;
    AudioBuffer &operator=(AudioBuffer &&) = default;

    ~AudioBuffer() {
      if (n) {
        delete static_cast<int8_t *>(ptr);
        ptr = nullptr;
      }
    }

    void zero() { memset(ptr, 0, size_t(n)); }
    int size() { return n; }
    int n_samples() { return n / get_data_size(); }
    double get_progress() { return double(offset) / n; }
    void *get_data_offset() {
      return static_cast<int8_t *>(get_data()) + offset;
    }
    int get_remain() { return n - offset; }
    int get_remain(int bc) { return bc - (n - offset); }
    void set_offset_overrun(int bc) { offset = bc - (n - offset); }
    void inc(int i) {
      offset += i;
      if (offset > n) offset = 0;
    }
    bool overrun(int bc) { return offset + bc > n; }
    void traverse() {
      float v;
      for (int i = 0; i < n_samples(); i++) v = get_value(i);
    }

    int offset = 0, n = 0;
  };

  class WaveItem {
   public:
    WaveItem() : amp(0), hz(0), phase(0) {}
    WaveItem(double amp, double hz, double phase = 0)
        : amp(amp), hz(hz), phase(phase) {}
    WaveItem(QVector<double> &vd) : amp(vd[0]), hz(vd[1]), phase(vd[2]) {}

    double amp, hz, phase;  // 0..1, hz, 0..2pi

    QString to_string(int index, double *scale) {
      switch (index) {
        case 0:
          return QString::number(amp * scale[0], 'f', 0);
        case 1:
          return QString::number(hz * scale[1], 'f', 2);
        case 2:
          return QString::number(phase * scale[2], 'f', 1);
      }
      return QString();
    }

    double get(int index) {
      switch (index) {
        case 0:
          return amp;
        case 1:
          return hz;
        case 2:
          return phase;
      }
      return 0;
    }

    void put(double val, int index) {
      switch (index) {
        case 0:
          amp = val;
          break;
        case 1:
          hz = val;
          break;
        case 2:
          phase = val;
          break;
      }
    }

    void random(int min, int max) {
      amp = double(rand()) / INT_MAX;
      hz = min + (max - min) * double(rand()) / INT_MAX;
      phase = pi2 * double(rand()) / INT_MAX;
    }

    QVector<double> qv() { return QVector<double>{amp, hz, phase}; }

   private:
    friend QDataStream &operator<<(QDataStream &out, const WaveItem &v) {
      out << v.amp << v.hz << v.phase;
      return out;
    }
    friend QDataStream &operator>>(QDataStream &in, WaveItem &v) {
      in >> v.amp >> v.hz >> v.phase;
      return in;
    }
  };

  Common(QObject *parent = nullptr) : QObject(parent) {
    qsrand(uint(QTime::currentTime().msec()));  // random seed

    connect(this, &Common::finished, this, &Common::finish_gen);
    // audio
    init_wave();
  }

  // audio
  AudioOut *audioOut = nullptr;
  AudioBuffer *buff = nullptr;

  // wave format values
  QList<int> _sample_rate_values = {8000,  11025, 16000, 22050,  32000, 44100,
                                    48000, 88200, 96000, 178200, 384000},
             _bits_sample_values = {8, 16, 32};

  QStringList sample_rate_values() {
    QStringList sl;
    for (auto &v : _sample_rate_values) sl << QString::number(v);
    return sl;
  }

  QStringList func_list{
      "sin",  "cos",  "tan",  "exp", "log",    "log10",       "int",  "sqrt",
      "asin", "acos", "atan", "abs", "wave",   "tone",        "note", "sec",
      "osc",  "saw",  "saw1", "lap", "hz2oct", "magneticring"};

  void clear() {
    compiler.clear();
    expression.clear();
  }

  int get_n_channels() { return compiler.get_n_channels(); }

  QStringList bits_sample_values() { return {"8", "16", "32", "float"}; }

  QStringList n_channels_values() { return {"1", "2", "4"}; }

  DataType get_data_type() {
    DataType dt = t_float;

    if (!floating_point) {
      switch (bits_sample) {
        case 8:
          dt = t_int8;
          break;
        case 16:
          dt = t_int16;
          break;
        case 32:
          dt = t_int32;
          break;
      }
    }
    return dt;
  }

  void set_progress(double progress) { this->progress_value = progress; }
  double get_progress() { return progress_value; }
  int get_iprogress() { return int(progress_value); }

  double max_amp() {
    return floating_point ? 1.0 : double(1l << (bits_sample - 1));
  }

  double time_inc() { return pi2 / sample_rate; }

  double get_n_samples() { return secs * sample_rate; }     // per channel
  int iget_n_samples() { return int(secs * sample_rate); }  // per channel

  int byte_count() {
    return int(n_channels * secs * sample_rate * bits_sample / 8.);
  }

  double sample_2_t(double sample) { return sample * pi2 / sample_rate; }

  void set_wave_format() {
    n_channels = compiler.get_n_channels();
    sample_rate = compiler.get_sample_rate();
    bits_sample = compiler.get_bits_sample();
    floating_point = compiler.get_floating_point();
    secs = compiler.get_seconds();
    volume = compiler.get_volume();

    data_type = get_data_type();
    bytes_per_sample = n_channels * (bits_sample / 8);
    scaled_amp = max_amp();
    t_inc = time_inc();
  }

  double get_scaled_diff() { return scaler.diff; }

  bool generate_wave(QString expression) {  // set, compile & scale

    if (processing) finish_gen();
    processing = true;
    data_completed = false;

    if (compiler.compile(expression.toStdWString())) {
      this->expression = expression;

      set_wave_format();
      fill_audio_buffer();
    }

    return compiler.is_ok();
  }

  void fill_audio_buffer() {
    if (buff) delete buff;
    buff = new AudioBuffer(byte_count());
    buff->set_type(data_type);

    main_th.run_nojoin([=]() { generate(); });
  }

  void update() {  // audio
    if (audioOut) emit audioOut->prepareWave();
  }

  void switch_play() {
    if (audioOut->isPlaying)
      audioOut->stop();
    else {
      init_wave();
      audioOut->start();
    }
  }

  void init_wave() {
    if (audioOut) delete audioOut;

    audioOut =
        new AudioOut(sample_rate, n_channels, bits_sample, floating_point);

    // audio support prep, gen
    connect(audioOut, &AudioOut::prepareWave, [=]() { wave_reset(); });
    connect(audioOut, &AudioOut::dataRequest,
            [=](char *data, qint64 byte_count) {
              get_data(data, int(byte_count));
            });
  }

  void wave_reset() {  // triggered from audioOut
    t = 0;             // reset timing
    t_inc = pi2 / sample_rate;
  }

  void inline inc_t() {
    t += t_inc;
    if (t > secs * pi2) t = 0;
    // next multi channel sample
  }

  void get_data(char *data, int n_bytes) {
    memset(data, 0, size_t(n_bytes));
    if (buff->size()) {
      if (buff->size() < n_bytes) {
        int bs = buff->size(), n = n_bytes / bs, r = n_bytes % bs;
        for (int i = 0; i < n; i++)
          memcpy(data + i * bs, buff->get_data(), size_t(bs));
        memcpy(data + n * bs, buff->get_data(), size_t(r));
      } else {
        if (buff->overrun(n_bytes)) {
          memcpy(data, buff->get_data_offset(), size_t(buff->get_remain()));
          memcpy(data + buff->get_remain(), buff->get_data(),
                 size_t(buff->get_remain(n_bytes)));
          buff->set_offset_overrun(n_bytes);
        } else {
          memcpy(data, buff->get_data_offset(), size_t(n_bytes));
          buff->inc(n_bytes);
        }
      }

      set_progress(100. * buff->get_progress());
      emit progress(
          QString("%1 ms").arg(1000. * secs * buff->get_progress(), 8, 'f', 0));
    }
  }

  void generate() {  // scale & fill audio buffer buff w/evaluation
    buff->zero();

    if (compiler.is_ok()) {
      int cnt = 0;

      // compiler list for mt support
      QVector<VSL_compiler> compilers(Thread::getnthreads());
      for (auto &c : compilers)
        c.compile(expression.toStdWString());
      QVector<Scaler<double>> scalers(Thread::getnthreads());
      int n_values = int(n_channels * secs * sample_rate);
      QVector<float> snd(n_values);

      // calc scaler
      compiler.execute(0, 0);

      timer.start();
      Thread(iget_n_samples())
          .run([=, &compilers, &scalers, &snd, &cnt](int th, int from, int to) {
            double t = t_inc * from;  // thread timing
            for (int samp = from, isamp = from * n_channels;
                 samp < to && processing;
                 samp++, t = (t > pi2 * secs) ? 0 : t + t_inc) {
              for (int ch = 0; ch < n_channels; ch++, isamp++) {
                double y = compilers[th].execute(t, ch);
                scalers[th].update(y);
                snd[isamp] = float(y);

                if (++cnt % 50000 == 0) {
                  set_progress(100. * cnt / n_values);
                  emit progress(
                      QString("%1 %").arg(100. * cnt / n_values, 3, 'f', 0));
                }
              }
            }
          });

      scaler.init(scalers);  // update scaler w/scalers
      scaled_amp = scaler.scale_diff(volume * max_amp());
      float fsa = float(scaled_amp);

      // fill audio buffer w/scaled values buff[i]=scaled_amp * snd[i]
      Thread(n_values).run(
          [=, &snd](int i) { buff->set_value(i, fsa * snd[i]); });

      data_completed = true;
      lap = timer.lap();

      if (processing) {
        emit progress("100 %");
        set_progress(100);
        emit finished();
      }
      processing = false;
    }
  }

  bool export_wav(QString file_name) {
    WaveFile wf;  // do all open, write, close
    if (wf.OpenForWrite(file_name.toLatin1().data(), UINT_32(sample_rate),
                        UINT16(bits_sample),
                        UINT16(n_channels)) != DDC_SUCCESS) {
      return false;
    }

    if (wf.WriteData(static_cast<UINT8 *>(buff->get_data()),
                     UINT_32(buff->size())) == DDC_SUCCESS) {
      if (wf.Close() != DDC_SUCCESS) return false;
    } else {
      wf.Close();
      return false;
    }

    return true;
  }

  QString random_expression(int nch = 2, int n = 5, double f_min = 200,
                            double f_max = 500) {
    expression = "// random wave\n\n";

    for (int c = 0; c < nch; c++) {
      for (int i = 0; i < n; i++) {
        expression +=
            QString("{%1, %2} %3")
                .arg(100. * rand() / RAND_MAX, 0, 'f', 0)
                .arg(f_min + abs(f_max - f_min) * rand() / RAND_MAX, 5, 'f', 0)
                .arg(i < n - 1 ? " + " : ";\n");
      }
    }

    return expression;
  }

  QString default_expression() { return expression; }

  QString paste_from_clip() {  // spread sheet oriented paste
    QString s = QApplication::clipboard()->text();

    if (!s.isEmpty()) {
      bool ok;
      expression.clear();

      auto lines = s.split(QRegExp("[\n\r]"), QString::SkipEmptyParts);
      int ch = 0, wc = 0;
      for (int il = 1; il < lines.size(); il++) {  // skip header
        auto slahp = lines[il].split('\t', QString::SkipEmptyParts);
        if (slahp.size() == 3) {
          double amp = slahp[0].toDouble(&ok), hz = slahp[1].toDouble(&ok),
                 phase = slahp[2].toDouble(&ok);
          expression += QString("%4{%1, %2, %3}")
                            .arg(amp)
                            .arg(hz)
                            .arg(phase)
                            .arg(wc ? "+" : "");
          wc++;
        } else {
          ch++;
          wc = 0;
          expression += ";\n";
        }
      }
      expression += ";\n";
    }
    return expression;
  }

  QString read_etg(QString file_ETG) {
    QString s_res;
    QFile file(file_ETG);
    if (file.open(QIODevice::ReadOnly)) {
      int i, maxRows = 50;
      double f;
      char line[1024], s[40];
      file.readLine(line, sizeof(line));
      sscanf(line, "%d\n", &i);
      file.readLine(line, sizeof(line));
      sscanf(line, "%lf\n", &f);
      file.readLine(line, sizeof(line));
      sscanf(line, "%d\n", &i);
      file.readLine(line, sizeof(line));
      sscanf(line, "%d\n", &i);

      QVector<QPair<QString, QString>> tv;
      for (int i = 0; i < maxRows; i++) {  // vars
        file.readLine(line, sizeof(line));
        sscanf(line, "%s\t%lf\n", s, &f);
        QString si = "", sv = "";
        if (*(s + 1)) {
          si = QString(s + 1);
          sv = QString::number(f);
          tv << QPair<QString, QString>(si, sv);
        }
      }
      if (!tv.isEmpty()) {
        s_res = "const ";
        for (int i = 0; i < tv.size(); i++)
          s_res += tv[i].first + "=" + tv[i].second +
                   (i < tv.size() - 1 ? ", " : ";\n\n");
      }

      // exps
      for (int i = 0; i < 2; i++) {
        QString s = "";
        char line[2048];
        while (!file.atEnd()) {
          if (file.readLine(line, sizeof(line))) {
            char *p = strchr(line, ';');
            s += QString(line).trimmed();
            if (p) break;
          }
        }
        s_res += s + "\n\n";
      }

      file.close();
    }
    return s_res;
  }
};

extern Common *c;  // init a QObject in the main window creator!!, otherwise
                   // audio will not work w/static compiling
#endif             // COMMON_H
