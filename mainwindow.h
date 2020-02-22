#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QComboBox>
#include <QFileDialog>
#include <QSettings>
#include <QTextStream>

#include <VSL_compiler.h>
#include <common.h>
#include <highlighter.h>

#include "ui_random_window.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

 private slots:
  void on_actionnew_triggered();
  void on_actionplay_triggered();
  void on_actionexport_triggered();
  void on_actionpaste_ss_triggered();
  void on_actionrandom_triggered();
  void on_actionsave_VSL_triggered();
  void on_actionopenVSL_triggered();
  void on_func_list_activated(const QString &arg1);
  void on_editor_cursorPositionChanged();
  void on_editor_textChanged();
  void on_actionopenetg_triggered();
  void on_replace_button_clicked();
  void on_progress(QString msg);
  void on_finished();
  void on_listFiles_clicked(const QModelIndex &index);

  void on_chb_decompile_clicked(bool checked);

private:
  void init_symbol_buttons();
  void stop();
  void set_dir_files_2_list();
  void open_vsl(QString fn, bool refresh_list);
  void fill_func_list();
  void insert_text(QString);
  void save_VSL();

  Highlighter *highlighter;
  VSL_compiler mcc;

 private:
  Ui::MainWindow *ui;

  QSettings *settings = nullptr;
  QString file_WAV, file_VSL, file_ETG;
  bool editing = false;

  void loadSettings(), saveSettings();
  bool prepare_wave();

  Ui_RandomWindow *ui_rnd = nullptr;
  QMainWindow *rnd_wind = nullptr;
};
#endif  // MAINWINDOW_H
