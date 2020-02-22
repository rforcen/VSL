#ifndef WAVEGRAPH_H
#define WAVEGRAPH_H

#include <common.h>
#include <math.h>
#include <QMouseEvent>
#include <QPainter>
#include <QWidget>

#include <climits>

class WaveGraph : public QWidget {
  Q_OBJECT
  const double divisor = 1000;

 public:
  explicit WaveGraph(QWidget *parent = nullptr);

  void set_ranges(int zoom, int pan) {
    this->zoom = zoom ? zoom / divisor : 1;
    this->pan = pan / divisor;
    update();
  }

  void get_ranges(int &zoom, int &pan) {
    zoom = int(this->zoom * divisor);
    pan = int(this->pan * divisor);
  }

 signals:
  void moving();

 private:  // mouse virtual events
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

  void paintEvent(QPaintEvent *) override;

 private:
  void draw_data(), draw_labels();

  void limitszp() {  // set zoom, pan to limits
    if (pan < 0) pan = 0;
    if (pan > 1) pan = 1;
    if (zoom < 0) zoom = 1 / divisor;
    if (zoom > 1) zoom = 1;
  }

  QPoint m_lastPos;

  int w, h, nchan;
  double zoom = 1, pan = 0;
  QPainter *painter;
  QVector<double> x_ms;
};

#endif  // WAVEGRAPH_H
