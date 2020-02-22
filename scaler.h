#ifndef SCALER_H
#define SCALER_H
#include <QVector>
#include <cfloat>
#include <cmath>
#include <cstdlib>

template <class T>
class Scaler {
 public:
  Scaler() { init(); }
  Scaler(T *v, int n) {
    init();
    for (int i = 0; i < n; i++) update(v[i]);
    for (int i = 0; i < n; i++) v[i] /= max;
  }

  void init() {
    if (typeid(T) == typeid(double)) {
      max = -DBL_MAX;
      min = DBL_MAX;
    } else {
      if (typeid(T) == typeid(float)) {
        max = T(-FLT_MAX);
        min = T(FLT_MAX);
      } else {
        if (typeid(T) == typeid(int)) {
          max = -INT_MAX;
          min = INT_MAX;
        }
      }
    }
  }

  void init(QVector<Scaler> &vs) {
    init();
    for (auto &s : vs) *this *= s;
  }

  inline void update(T v) {
    if (v > max) max = v;
    if (v < min) min = v;

    diff = std::abs(max - min);
    if (diff < eps) diff = 0;
  }

  void update(Scaler<T> &s) {
    update(s.max);
    update(s.min);
  }

  Scaler &operator*=(Scaler &other) {
    update(other);
    return *this;
  }

  T scale_diff(T x) { return diff != 0.0 ? x / diff : x; }   //
  T scale(T x) { return diff != 0 ? (x - min) / diff : x; }  // 0..1
  T scale_1_1(T x) { return 2 * (scale(x) - 0.5); }          // -1..1

  T min, max, diff, eps = 1e-10;
};

#endif  // SCALER_H
