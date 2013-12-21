#ifndef RGSS_MAIN_WINDOW_HXX
#define RGSS_MAIN_WINDOW_HXX

#include <QtGui/QMainWindow>
#include <QtGui/QSplitter>
#include <QtGui/QListWidget>
#include <QtGui/QWidget>
#include <QtGui/QLineEdit>
#include <QtGui/QStackedWidget>
#include <QtCore/QHash>
#include <QtCore/QFileInfo>

#include <Qsci/qsciscintilla.h>

#include "ruby_data.hxx"
#include "editor_widget.hxx"

class RGSS_MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  RGSS_MainWindow(const QString &path_to_load,
                  QWidget* parent = NULL, Qt::WindowFlags = 0);

 public slots:
  /* Menu functions */
  void onOpenArchive();
  void onSaveArchive();
  bool onSaveArchiveAs();
  void onImportScripts();
  void onExportScripts();
  void onCloseArchive();

 private slots:
  void onScriptIndexChange(int idx);
  void onScriptNameEdited(QString const& name);
  void onScriptEditorModified();
  void onArchiveDropped(const QString &);

 private:
  void loadScriptArchive(QString const& file, bool show_errors = true);
  bool saveScriptArchiveAs(QString const& file);
  void closeScriptArchive();
  void setDataModified(bool);
  void enableEditing(bool v);
  void updateWindowTitle();
  void setupLoadedArchive();

  void closeEvent(QCloseEvent *);

  EditorWidget *getEditorForScript(const ScriptArchive::Script &script);

  /* Stores text changed in editors back
   * into the respective script struct */
  void storeChangedScripts();

  /* Asks the user if changes should be
   * saved or discarded (or the action canceled).
   * true: go ahead with action, false: cancel action */
  bool verifySaveDiscard(const QString &actionTitle);

  /* Setup common settings */
  static void setupEditor(QsciScintilla &editor);

  /* Path of currently opened archive */
  QString open_path;

  /* An archive can be open even without
   * a valid path (eg. after import */
  bool archive_opened;

  /* Last folder from which a valid archive
   * was opened / saved */
  QString last_valid_folder;
  QString last_valid_folder_impexp;

  ScriptArchive archive_;
  int current_row_;
  bool data_modified_;

  QSplitter splitter_;
  QWidget left_side_;
  QListWidget script_list_;
  QLineEdit script_name_editor_;

  QStackedWidget editor_stack;

  /* Is only shown when no archive is opened */
  EditorWidget dummy_editor;

  QHash<int, EditorWidget*> editor_hash;

  QList<EditorWidget*> recycled_editors;
};

#endif
