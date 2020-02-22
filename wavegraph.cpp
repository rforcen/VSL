#include "wavegraph.h"

WaveGraph::WaveGraph(QWidget *parent) : QWidget(parent) {}

void WaveGraph::paintEvent(QPaintEvent *) {
  nchan = c->get_n_channels();
  draw_data();
}

void WaveGraph::draw_data() {
  painter = new QPainter(this);
  painter->fillRect(0, 0, w, h, Qt::black);  // clg

  if (c->data_completed && nchan && c->compiler.is_ok()) {
    limitszp();

    w = painter->device()->width();
    h = painter->device()->height();

    x_ms = QVector<double>(w);

    painter->fillRect(0, 0, w, h, Qt::black);  // clg

    for (int ch = 0, hch = h / (nchan + 1); ch < nchan; ch++) {
      int y0c = h * (1 + ch * 2) / (2 * nchan); // chan y axis

      Scaler<double> scaler;
      double xa = 0, ya = 0;
      bool ft = true;
      int ix = 0, nlines = 0;

      painter->setPen(Qt::darkCyan);
      for (double ns = 0, ns_tot = c->get_n_samples() * zoom,
                  t = c->pi2 * c->secs * pan, t_inc = c->time_inc(), x = 0,
                  x_inc = w / ns_tot, mf = ns_tot < w ? 1 : 8;
           ns < ns_tot && t < c->pi2 * c->secs;
           t += t_inc * mf, x += x_inc * mf, ns += mf) {
        // scale to 0..1
        double v = c->buff->get_value(int(ch + t / t_inc)) / c->scaled_amp;
        double y = y0c - hch * (c->scaler.scale(v) - 0.5);

        if (ns_tot > w * 50) {
          if (int(x) == ix) {  // draw only w lines from ymin->ymax
            scaler.update(y);
          } else {
            painter->drawLine(ix, int(scaler.max), ix, int(scaler.min));

            ix = int(x);
            scaler.init();

            nlines++;
          }
        } else {
          if (ft)
            ft = false;
          else
            painter->drawLine(int(xa), int(ya), int(x), int(y));
          xa = x;
          ya = y;
        }

        x_ms[int(x) % w] = t / c->pi2;
      }

      painter->setPen(Qt::green);
      painter->drawLine(0, y0c, w, y0c);  // axis

      // chan id
      painter->setPen(Qt::red);
      QFontMetrics fm(painter->font());
      auto s = QString("%1").arg(ch + 1);

      int swd = fm.width(s);
      int shi = fm.height() / 4;

      painter->drawText(QPointF(swd, y0c + shi), s);
      painter->drawText(QPointF(w - swd * 2, y0c + shi), s);
    }
    draw_labels();
  }
  delete painter;
}

void WaveGraph::draw_labels() {
  int lt = 3, ht = h - lt, ntics = 10;
  painter->setPen(Qt::green);
  painter->drawLine(0, ht, w, ht);  // axis

  // labels
  painter->setPen(Qt::yellow);
  painter->setFont(QFont("Courier New", 8));
  QFontMetrics fm(painter->font());

  for (int i = 1, wt = w / ntics; i < ntics; i++) {
    // vert. dot-dash lines
    QPen pant = painter->pen();
    painter->setPen(
        QPen(Qt::darkYellow, 1, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawLine(i * wt, 0, i * wt, h);
    painter->setPen(pant);

    painter->drawLine(i * wt, ht + lt, i * wt, ht - lt);  // tick
    auto s = x_ms[i * wt] != 0. ? QString("%1%2")
                                      .arg(divisor * x_ms[i * wt], 0, 'f', 0)
                                      .arg(i != ntics - 1 ? "" : " (ms)")
                                : "";

    int pixelsWide = fm.width(s);
    int pixelsHigh = fm.height();
    painter->drawText(QPointF(i * wt - pixelsWide / 2, ht - pixelsHigh), s);
  }
}

void WaveGraph::mouseDoubleClickEvent(QMouseEvent *) {
  zoom = 1;
  pan = 0;
  update();
}

void WaveGraph::mousePressEvent(QMouseEvent *event) {
  m_lastPos = event->pos();
}

void WaveGraph::mouseMoveEvent(QMouseEvent *event) {
  QPoint p = event->pos() - m_lastPos;

  if (event->buttons() & Qt::LeftButton) {
    pan -= p.rx() / divisor;
    update();
  } else if (event->buttons() & Qt::RightButton) {
    zoom += 10. * p.ry() / divisor;
    update();
  }
  m_lastPos = event->pos();
  emit moving();
}

void WaveGraph::wheelEvent(QWheelEvent *event) {
  QPoint numDegrees = event->angleDelta() / 8;

  if (!numDegrees.isNull()) {
    QPoint numSteps = numDegrees / 15;
    zoom -= numSteps.ry() / 1000.;
    update();
    emit moving();
  }
}
