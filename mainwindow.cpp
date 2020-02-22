#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), ui_rnd(new Ui_RandomWindow) {
  ui->setupUi(this);

  c = new Common;  // must be init in main window creator

  ui->centralwidget->hide();

  init_symbol_buttons();

  // random wave parameters form
  ui_rnd->setupUi(rnd_wind = new QMainWindow(this));

  loadSettings();
  highlighter = new Highlighter(ui->editor->document());

  // rise up combos to tool bar
  //  ui->toolBar->addWidget(ui->progress);
  ui->chb_decompile->setVisible(false);
  ui->toolBar->addWidget(ui->chb_decompile);


  connect(c, &Common::progress, this, &MainWindow::on_progress);
  connect(c, &Common::finished, this, &MainWindow::on_finished);

  set_dir_files_2_list();
  fill_func_list();

  prepare_wave();
  ui->decompiled->setHidden(true);
  ui->toolBar->setIconSize(QSize(32, 32));

  // init waveTable
  for (int r = 0; r < ui->waveTable->rowCount(); r++) {
    QTableWidgetItem *item = ui->waveTable->item(r, 0);
    if (!item) {
      item = new QTableWidgetItem();
      ui->waveTable->setItem(r, 0, item);
    }
  }
}

MainWindow::~MainWindow() {
  stop();
  saveSettings();
  delete ui;
}

// aux

void MainWindow::set_dir_files_2_list() {
  if (!file_VSL.isEmpty()) {
    auto files =
        QDir(QFileInfo(file_VSL).path()).entryList({"*.vsl"}, QDir::Files);
    ui->listFiles->clear();
    ui->listFiles->addItems(files);
    setWindowTitle("VSL - " + QFileInfo(file_VSL).fileName());
  }
}

void MainWindow::fill_func_list() { ui->func_list->addItems(c->func_list); }

void MainWindow::insert_text(QString txt) {
  QTextCursor txt_curs = ui->editor->textCursor();
  txt_curs.insertText(txt);
  if (txt.size() > 1) {
    txt_curs.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor);
    ui->editor->setTextCursor(txt_curs);
  }
}

void MainWindow::init_symbol_buttons() {
  for (auto btn : ui->symbols->findChildren<QPushButton *>()) {
    auto txt = btn->text();
    connect(btn, &QPushButton::clicked, [=]() { insert_text(txt); });
  }
}

bool MainWindow::prepare_wave() {
  bool ok = true;

  if (ui->editor->document()->isModified())
    ok = c->generate_wave(ui->editor->toPlainText());

  if (ok)
    c->init_wave();
  else {
    on_progress("");
    ui->statusbar->showMessage(c->compiler.error_msg());
  }

  ui->editor->document()->setModified(false);
  on_editor_cursorPositionChanged();
  return ok && c->compiler.get_n_channels() > 0;
}

void MainWindow::stop() {
  c->audioOut->stop();
  ui->actionplay->setChecked(false);
  ui->statusbar->clearMessage();
}

void MainWindow::open_vsl(QString fn, bool refresh_list = true) {
  file_VSL = fn;
  QFile fh(file_VSL);
  if (fh.open(QIODevice::ReadOnly)) {
    QTextStream in(&fh);
    c->expression = in.readAll();
    ui->editor->setText(c->expression);
    ui->editor->document()->setModified(true);
    fh.close();
    if (refresh_list) set_dir_files_2_list();
    setWindowTitle("VSL - " + QFileInfo(file_VSL).fileName());
    prepare_wave();
  }
}

void MainWindow::save_VSL() {
  if (ui->editor->document()->isModified()) {
    if (file_VSL.isEmpty())
      file_VSL = QFileDialog::getSaveFileName(this, "Save VSL file", file_VSL,
                                              "VSL File (*.vsl)");
    if (!file_VSL.isEmpty()) {
      QFile fh(file_VSL);
      if (fh.open(QIODevice::WriteOnly)) {
        QTextStream out(&fh);
        out << ui->editor->toPlainText();
        fh.close();
        setWindowTitle("VSL - " + QFileInfo(file_VSL).fileName());
        set_dir_files_2_list();
        on_editor_cursorPositionChanged();
      }
    }
  }
}

// settings

void MainWindow::loadSettings() {
  settings = new QSettings("voicesync", "UWG");

  restoreState(settings->value("windowState").toByteArray());
  restoreGeometry(settings->value("geometry").toByteArray());

  file_WAV = settings->value("fileWAV").toString();
  file_VSL = settings->value("fileWDF").toString();
  file_ETG = settings->value("fileETG").toString();

  ui->editor->setText(settings->value("editor").toString());
  ui->editor->document()->setModified(true);
  rnd_wind->restoreGeometry(settings->value("rand_geometry").toByteArray());
  setWindowTitle("VSL - " + QFileInfo(file_VSL).fileName());
}

void MainWindow::saveSettings() {
  settings->setValue("geometry", saveGeometry());
  settings->setValue("windowState", saveState());

  settings->setValue("fileWAV", file_WAV);
  settings->setValue("fileWDF", file_VSL);
  settings->setValue("fileETG", file_ETG);

  settings->setValue("editor", ui->editor->toPlainText());
  settings->setValue("rand_geometry", rnd_wind->saveGeometry());

  delete settings;
}

// events...
void MainWindow::on_actionnew_triggered() {
  stop();
  save_VSL();

  file_VSL=QFileInfo(file_VSL).path()+QDir::separator();
  ui->editor->clear();
  c->clear();

  setWindowTitle("VSL");

  ui->wave_graph_widget->update();
}

void MainWindow::on_actionplay_triggered() {
  save_VSL();

  if (c->audioOut->isPlaying)
    c->audioOut->stop();
  else {
    if (prepare_wave()) {
      c->audioOut->start();
    } else
      ui->actionplay->toggle();
  }
}

void MainWindow::on_actionexport_triggered() {
  stop();

  file_WAV = QFileDialog::getSaveFileName(this, "Export WAV file", file_WAV,
                                          "WAV File (*.wav)");
  if (!file_WAV.isEmpty()) {
    prepare_wave();
    ui->statusbar->showMessage(
        QString("file %1 generated %2")
            .arg(file_WAV)
            .arg(c->export_wav(file_WAV) ? "ok" : "with errors"));
  }
}

void MainWindow::on_actionpaste_ss_triggered() {
  stop();
  save_VSL();

  ui->editor->setText(c->paste_from_clip());
  setWindowTitle("VSL - clip");
  prepare_wave();
}

void MainWindow::on_actionrandom_triggered() {
  stop();
  save_VSL();

  rnd_wind->show();
  ui->editor->setText(c->random_expression(
      ui_rnd->n_channels->value(), ui_rnd->n_waves->value(),
      ui_rnd->hz_min->value(), ui_rnd->hz_max->value()));
  setWindowTitle("VSL -  rand");
  prepare_wave();
}

void MainWindow::on_actionsave_VSL_triggered() {
  stop();

  file_VSL = QFileDialog::getSaveFileName(this, "Save VSL file", file_VSL,
                                          "VSL File (*.vsl)");
  if (!file_VSL.isEmpty()) {
    save_VSL();
    ui->statusbar->showMessage(QString("file %1 saved").arg(file_VSL));
  }
}

void MainWindow::on_actionopenVSL_triggered() {
  stop();
  save_VSL();

  file_VSL = QFileDialog::getOpenFileName(this, "Open VSL file", file_VSL,
                                          "VSL File (*.vsl)");
  if (!file_VSL.isEmpty()) open_vsl(file_VSL);
}

void MainWindow::on_func_list_activated(const QString &func) {
  insert_text(func + "()");
}

void MainWindow::on_editor_cursorPositionChanged() {
  QTextCursor cursor = ui->editor->textCursor();
  int y = cursor.blockNumber() + 1;
  int x = cursor.columnNumber() + 1;
  ui->dock_editor->setWindowTitle(
      QString("editor [ %1, %2 ] %3 - %4")
          .arg(y, 3)
          .arg(x, 3)
          .arg(ui->editor->document()->isModified() ? '*' : ' ')
          .arg(QFileInfo(file_VSL).absoluteFilePath()));
}

void MainWindow::on_editor_textChanged() {
  emit ui->editor->cursorPositionChanged();
}

void MainWindow::on_actionopenetg_triggered() {
  stop();
  save_VSL();

  file_ETG = QFileDialog::getOpenFileName(this, "Open ETG file", file_ETG,
                                          "ETG File (*.ETG)");
  if (!file_ETG.isEmpty()) {
    QString s = c->read_etg(file_ETG);
    if (!s.isEmpty()) {
      ui->editor->setText(s);
      prepare_wave();
    }
  }
}

void MainWindow::on_replace_button_clicked() {
  ui->editor->setText(ui->editor->toPlainText().replace(
      ui->replace_org->text(), ui->replace_with->text()));
  ui->editor->document()->setModified(true);
  on_editor_cursorPositionChanged();
}

void MainWindow::on_progress(QString msg) {
  ui->progressBar->setValue(c->get_iprogress());
  ui->progress->setText(msg);
}

void MainWindow::on_finished() {
  ui->wave_graph_widget->update();

  if (c->compiler.is_ok()) {
    ui->statusbar->showMessage(
        QString("syntax: %1 | channels: %2 | sample_rate: "
                "%3 | bits_sample: %4 %7 | seconds: %5 | "
                "volume: %8% | # samples: %6 | lap: %9 ms | samps/sec: %10")
            .arg(c->compiler.error_msg())
            .arg(c->compiler.get_n_channels())
            .arg(c->sample_rate)
            .arg(c->bits_sample)
            .arg(c->secs)
            .arg(c->get_n_samples())
            .arg(c->floating_point ? "float" : "")
            .arg(c->volume * 100)
            .arg(c->lap)
            .arg(1000. * c->get_n_samples() / c->lap, 0, 'g', 2));
    QVector<double> table_values = {1. * c->compiler.get_n_channels(),
                                    1. * c->sample_rate,
                                    1. * c->bits_sample,
                                    c->secs,
                                    c->get_n_samples(),
                                    c->volume * 100,
                                    1.0 * c->lap,
                                    1000. * c->get_n_samples() / c->lap};
    for (int r = 0; r < ui->waveTable->rowCount(); r++)
      ui->waveTable->item(r, 0)->setText(QString("%1").arg(table_values[r]));
    QString ds = QString::QString::fromStdWString(c->compiler.get_decompiled());
    ui->decompiled->setPlainText(ds);
  } else
    ui->statusbar->showMessage(c->compiler.error_msg());
}

void MainWindow::on_listFiles_clicked(const QModelIndex &index) {
  stop();
  auto currentRow = index.row();
  if (currentRow >= 0) {
    save_VSL();
    QString file_name = ui->listFiles->item(currentRow)->text();
    ui->dock_editor->setWindowTitle(QFileInfo(file_VSL).path());
    open_vsl(QFileInfo(file_VSL).path() + QDir::separator() + file_name, false);
  }
}

void MainWindow::on_chb_decompile_clicked(bool checked) {
  ui->decompiled->setHidden(!checked);
}
