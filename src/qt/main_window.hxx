#ifndef RGSS_MAIN_WINDOW_HXX
#define RGSS_MAIN_WINDOW_HXX

#include <QtGui/QMainWindow>
#include <QtGui/QSplitter>
#include <QtGui/QListWidget>

#include <Qsci/qsciscintilla.h>

#include "ruby_data.hxx"

class QSplitter;
class QListWidget;
class QsciScintilla;

class RGSS_MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  RGSS_MainWindow(QWidget* parent = NULL, Qt::WindowFlags = 0);

 public slots:
  void setScriptArchive(QString const& file);
  void saveScriptArchive();
  void saveScriptArchiveAs(QString const& file);

  void setCurrentIndex(int idx);

 private:
  QString file_;
  ScriptList scripts_;
  int current_row_;

  QSplitter splitter_;
  QListWidget script_list_;
  QsciScintilla script_editor_;
};

#endif
