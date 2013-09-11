#ifndef RGSS_MAIN_WINDOW_HXX
#define RGSS_MAIN_WINDOW_HXX

#include <QtGui/QMainWindow>
#include <QtGui/QSplitter>
#include <QtGui/QListWidget>
#include <QtGui/QWidget>
#include <QtGui/QLineEdit>

#include <Qsci/qsciscintilla.h>

#include "ruby_data.hxx"

class RGSS_MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  RGSS_MainWindow(QWidget* parent = NULL, Qt::WindowFlags = 0);

 public slots:
  void setScriptArchive(QString const& file);
  void saveScriptArchive();
  void saveScriptArchiveAs(QString const& file);

  void setCurrentIndex(int idx);

  void scriptNameEdited(QString const& name);

  void openScriptArchive();
  void saveScriptArchiveAs();
  void closeScriptArchive();

  bool scriptArchiveOpened() const;

 private:
  void enableEditing(bool v);

  QString file_;
  ScriptArchive archive_;
  int current_row_;

  QSplitter splitter_;
  QWidget left_side_;
  QListWidget script_list_;
  QLineEdit script_name_editor_;
  QsciScintilla script_editor_;
};

#endif
