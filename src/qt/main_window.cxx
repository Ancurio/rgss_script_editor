#include "main_window.hxx"

#include "savediscard_dialog.hxx"
#include "editor_widget.hxx"

#include <QtGui/QAction>
#include <QtGui/QKeySequence>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QFileDialog>
#include <QtGui/QCloseEvent>
#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>
#include <QtCore/QSettings>
#include <QtCore/QRegExp>

#include <Qsci/qscilexerruby.h>

#include <algorithm>
#include <iostream>

#define SETTINGS_ORG "rgss_script_editor"
#define SETTINGS_APP "rgss_script_editor"

RGSS_MainWindow::RGSS_MainWindow(const QString &path_to_load,
                                 QWidget* const parent, Qt::WindowFlags const flags)
    : QMainWindow(parent, flags)
    , data_modified_(false)
{
  /* Read settings */
  QSettings settings(SETTINGS_ORG, SETTINGS_APP);
  QSize window_size = (settings.value("window_size", QSize(800, 600)).toSize());
  QString last_open_path = settings.value("open_path").toString();
  last_valid_folder = settings.value("last_valid_folder", QDir::homePath()).toString();
  last_valid_folder_impexp =
      settings.value("last_valid_folder_impexp", QDir::homePath()).toString();

  resize(window_size);

  /* Setup UI */
  QMenuBar* const menu_bar = new QMenuBar(this);
  {
    QMenu* const file = new QMenu(tr("File"), menu_bar);

    QAction* const open = new QAction(tr("Open"), menu_bar);
    open->setShortcut(QKeySequence(QKeySequence::Open));
    connect(open, SIGNAL(triggered()), SLOT(onOpenArchive()));
    file->addAction(open);

    QAction* const save = new QAction(tr("Save"), menu_bar);
    save->setShortcut(QKeySequence(QKeySequence::Save));
    connect(save, SIGNAL(triggered()), SLOT(onSaveArchive()));
    file->addAction(save);

    QAction* const save_as = new QAction(tr("Save As"), menu_bar);
    save_as->setShortcut(QKeySequence(QKeySequence::SaveAs));
    connect(save_as, SIGNAL(triggered()), SLOT(onSaveArchiveAs()));
    file->addAction(save_as);

    file->addSeparator();

    QAction* const imp_scripts = new QAction(tr("Import Scripts"), menu_bar);
    connect(imp_scripts, SIGNAL(triggered()), SLOT(onImportScripts()));
    file->addAction(imp_scripts);

    QAction* const exp_scripts = new QAction(tr("Export Scripts"), menu_bar);
    connect(exp_scripts, SIGNAL(triggered()), SLOT(onExportScripts()));
    file->addAction(exp_scripts);

    file->addSeparator();

    QAction* const close = new QAction(tr("Close"), menu_bar);
    close->setShortcut(QKeySequence(QKeySequence::Close));
    connect(close, SIGNAL(triggered()), SLOT(onCloseArchive()));
    file->addAction(close);

    edit_menu_.setTitle(tr("Edit"));

    QAction* const insert = new QAction(tr("Insert"), menu_bar);
    connect(insert, SIGNAL(triggered()), SLOT(onInsertScript()));
    edit_menu_.addAction(insert);

    delete_action_ = new QAction(tr("Delete"), menu_bar);
    connect(delete_action_, SIGNAL(triggered()), SLOT(onDeleteScript()));
    delete_action_->setShortcut(QKeySequence(QKeySequence::Delete));
    edit_menu_.addAction(delete_action_);

    menu_bar->addMenu(file);
    menu_bar->addMenu(&edit_menu_);
  }
  setMenuBar(menu_bar);

  splitter_.setOrientation(Qt::Horizontal);
  setCentralWidget(&splitter_);

  script_list_.setModel(&archive_);

  left_side_.setLayout(new QVBoxLayout(&left_side_));
  left_side_.layout()->addWidget(&script_list_);
  left_side_.layout()->addWidget(&script_name_editor_);
  left_side_.setContextMenuPolicy(Qt::CustomContextMenu);
  connect(&left_side_, SIGNAL(customContextMenuRequested(QPoint)), SLOT(onShowContextMenu(QPoint)));

  editor_stack.addWidget(&dummy_editor);

  splitter_.addWidget(&left_side_);
  splitter_.addWidget(&editor_stack);

  /* Only the editor widget should expand on resize */
  splitter_.setStretchFactor(0, 0);
  splitter_.setStretchFactor(1, 1);

  const int left_size_width = 200;
  QList<int> sizes;
  sizes.push_back(left_size_width);
  sizes.push_back(window_size.width() - left_size_width);
  splitter_.setSizes(sizes);

  setupEditor(dummy_editor);
  connect(&dummy_editor, SIGNAL(archiveDropped(QString)), SLOT(onArchiveDropped(QString)));

  connect(script_list_.selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(onScriptIndexChange(QModelIndex,QModelIndex)));
  connect(&script_name_editor_, SIGNAL(textEdited(QString)), SLOT(onScriptNameEdited(QString)));

  connect(&archive_, SIGNAL(scriptCountChanged(int)), SLOT(onScriptCountChanged(int)));

  enableEditing(false);

  if (!path_to_load.isEmpty())
    loadScriptArchive(path_to_load);
  else if (!last_open_path.isEmpty())
    loadScriptArchive(last_open_path, false);

  updateWindowTitle();
}

void RGSS_MainWindow::setupEditor(QsciScintilla &editor)
{
  // other setting
  editor.setUtf8(true);
  editor.setEolMode(QsciScintilla::EolWindows);

  // indent
  editor.setIndentationWidth(2);
  editor.setAutoIndent(true);

  // lexer and font
  QFont font;
  font.setStyleHint(QFont::Monospace);
  font.setFamily(font.defaultFamily());
  editor.setFont(font);
  editor.setMarginsFont(font);
  QsciLexer* lexer = new QsciLexerRuby(&editor);
  lexer->setDefaultFont(font);
  editor.setLexer(lexer);

  // line number
  QFontMetrics fontmetrics(font);
  editor.setMarginWidth(0, fontmetrics.width("00000") + 6);
  editor.setMarginLineNumbers(0, true);

  // auto complete
  editor.setAutoCompletionThreshold(3);
  editor.setAutoCompletionSource(QsciScintilla::AcsAll);
}

EditorWidget *RGSS_MainWindow::getEditorForScript(Script *script)
{
  /* If we already have an editor associated
   * with this script, just return that */
  if (editor_hash.contains(script))
    return editor_hash.value(script);

  /* Otherwise, create (or recycle) a new one */
  EditorWidget *editor;

  /* Check recycled list first */
  if (!recycled_editors.isEmpty()) {
    editor = recycled_editors.back();
    recycled_editors.pop_back();
  } else {
    editor = new EditorWidget();
    setupEditor(*editor);
    editor_stack.addWidget(editor);
    connect(editor, SIGNAL(archiveDropped(QString)), SLOT(onArchiveDropped(QString)));
  }

  /* Associate editor with script */
  editor->setText(script->data);
  editor_hash.insert(script, editor);

  /* Listen for text changes */
  connect(editor, SIGNAL(textChanged()), SLOT(onScriptEditorModified()));

  return editor;
}

QModelIndex RGSS_MainWindow::getCurrentIndex()
{
  return script_list_.selectionModel()->currentIndex();
}

void RGSS_MainWindow::storeChangedScripts()
{
  QHash<Script*, EditorWidget*>::const_iterator iter;
  for (iter = editor_hash.constBegin(); iter != editor_hash.constEnd(); ++iter) {
    QsciScintilla *editor = iter.value();

    if (!editor->isModified())
      continue;

    iter.key()->data = editor->text();
    editor->setModified(false);
  }
}

bool RGSS_MainWindow::verifySaveDiscard(const QString &actionTitle)
{
  if (data_modified_) {
    SaveDiscardDialog dialog(this);
    dialog.setWindowTitle(actionTitle);

    switch (dialog.exec()) {
      default:
      case SaveDiscardDialog::Cancel :
        return false;

      case SaveDiscardDialog::Save :
        return saveScriptArchiveAs(open_path);

      case SaveDiscardDialog::Discard :
        break;
    }
  }

  return true;
}

void RGSS_MainWindow::onScriptEditorModified()
{
  setDataModified(true);
}

void RGSS_MainWindow::setDataModified(bool m)
{
  if (data_modified_ == m)
    return;

  data_modified_ = m;

  updateWindowTitle();
}

void RGSS_MainWindow::onArchiveDropped(const QString &filename)
{
  if (!verifySaveDiscard(tr("Open Archive")))
    return;

  if (archive_opened)
    closeScriptArchive();

  loadScriptArchive(filename);

  updateWindowTitle();
}

void RGSS_MainWindow::onShowContextMenu(const QPoint &p)
{
  edit_menu_.exec(left_side_.mapToGlobal(p));
}

void RGSS_MainWindow::onInsertScript()
{
  int row = getCurrentIndex().row();

  archive_.insertRow(row);
  script_list_.setCurrentIndex(archive_.index(row));

  setDataModified(true);
}

void RGSS_MainWindow::onDeleteScript()
{
  QModelIndex current_index = getCurrentIndex();

  if (!current_index.isValid())
    return;

  int row = current_index.row();

  Script *script = archive_.indexToScript(getCurrentIndex());
  EditorWidget *editor = editor_hash.value(script);
  editor_hash.remove(script);
  recycled_editors.append(editor);

  archive_.removeRow(row);

  setDataModified(true);
}

void RGSS_MainWindow::enableEditing(bool v) {
  script_name_editor_.setEnabled(v);
  script_list_.setEnabled(v);
  edit_menu_.setEnabled(v);
}

void RGSS_MainWindow::updateWindowTitle() {
  QString title;
  QRegExp reg(".*/([^/]*)/Data/Scripts.*");

  if (reg.indexIn(open_path) != -1)
    title = reg.cap(1);
  else
    title = open_path;

  if (title.isEmpty() && archive_opened)
    title = "(Untitled)";

  if (data_modified_)
    title.prepend("*");

  setWindowTitle(title);
}

void RGSS_MainWindow::setupLoadedArchive()
{
  archive_opened = true;

  enableEditing(true);
  updateWindowTitle();
  script_list_.setCurrentIndex(archive_.index(0));
}

void RGSS_MainWindow::onScriptCountChanged(int newCount)
{
  delete_action_->setEnabled(newCount > 0);
}

void RGSS_MainWindow::closeEvent(QCloseEvent *ce)
{
  if (!verifySaveDiscard(tr("Exit Editor")))
    ce->ignore();
  else
    ce->accept();

  /* Write settings */
  QSettings settings(SETTINGS_ORG, SETTINGS_APP);
  settings.setValue("window_size", size());
  settings.setValue("open_path", open_path);
  settings.setValue("last_valid_folder", last_valid_folder);
  settings.setValue("last_valid_folder_impexp", last_valid_folder_impexp);
}

static const char *fileFilter =
    QT_TRANSLATE_NOOP("RGSS_MainWindow",
    "Script Archive (Scripts.rxdata Scripts.rvdata Scripts.rvdata2);;"
    "All files (*)");

void RGSS_MainWindow::onOpenArchive() {

  if (!verifySaveDiscard(tr("Open Archive")))
    return;

  QString const f = QFileDialog::getOpenFileName(
      this, tr("Select script archive to open..."), last_valid_folder, tr(fileFilter));

  if(f.isNull()) { return; } // check cancel

  if (archive_opened)
    closeScriptArchive();

  loadScriptArchive(f);
}

bool RGSS_MainWindow::onSaveArchiveAs() {
  QString const f = QFileDialog::getSaveFileName(
      this, tr("Select saving file..."), last_valid_folder, tr(fileFilter));

  if(f.isNull()) { return false; } // check cancel

  return saveScriptArchiveAs(f);
}

void RGSS_MainWindow::onImportScripts()
{
  if (!verifySaveDiscard("Import Scripts"))
    return;

  const QString src_folder = QFileDialog::getExistingDirectory(
        this, tr("Select import folder..."), last_valid_folder_impexp);

  if (src_folder.isEmpty())
    return;

  /* Open index */
  QFile indFile(src_folder + "/index");
  if (!indFile.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, "Importing error.", "Cannot open index file");
    return;
  }

  QTextStream indStream(&indFile);
  int scIdx = 0;

  ScriptList scripts;

  while (!indStream.atEnd())
  {
    QString scName = indStream.readLine();

    QString scFilename = QString("%1").arg(scIdx, 3, 10, QLatin1Char('0'));
    QFile scFile(src_folder + "/" + scFilename);

    if (!scFile.open(QFile::ReadOnly)) {
      QMessageBox::critical(this, "File reading error.",
                            QString("Cannot open script \"%1\" (%2)").arg(scName, scFilename));
      return;
    }

    QByteArray scData = scFile.readAll();
    scFile.close();

    Script script;
    script.magic = 0;
    script.name = scName;
    script.data = scData;

    scripts.append(script);

    ++scIdx;
  }

  closeScriptArchive();

  archive_.setScriptList(scripts);

  open_path = QString();
  setupLoadedArchive();
  setDataModified(true);

  last_valid_folder_impexp = QFileInfo(src_folder).absolutePath();
}

void RGSS_MainWindow::onExportScripts()
{
  const QString dest_folder = QFileDialog::getExistingDirectory(
        this, tr("Select export folder..."), last_valid_folder_impexp);

  if (dest_folder.isEmpty())
    return;

  /* Write index */
  QFile indFile(dest_folder + "/index");
  if (!indFile.open(QFile::WriteOnly)) {
    QMessageBox::critical(this, "Importing error.", "Cannot open index file");
    return;
  }

  storeChangedScripts();

  QTextStream indStream(&indFile);
  ScriptList scripts = archive_.scriptList();

  for (int i = 0; i < scripts.count(); ++i)
  {
    const Script &sc = scripts[i];

    indStream << sc.name << "\n";

    QFile scFile(dest_folder + "/" + QString("%1").arg(i, 3, 10, QLatin1Char('0')));
    if (!scFile.open(QFile::WriteOnly)) {

      break;
    }

    scFile.write(sc.data.toUtf8());

    scFile.close();
  }

  indFile.close();

  last_valid_folder_impexp = QFileInfo(dest_folder).absolutePath();
}

void RGSS_MainWindow::onCloseArchive()
{
  if (!verifySaveDiscard(tr("Close Archive")))
    return;

  closeScriptArchive();
  updateWindowTitle();
}

void RGSS_MainWindow::closeScriptArchive() {

  /* Recycle editor widgets for later use */
  QHash<Script*, EditorWidget*>::const_iterator iter;
  for (iter = editor_hash.constBegin(); iter != editor_hash.constEnd(); ++iter) {
    EditorWidget *editor = iter.value();

    disconnect(editor, SIGNAL(textChanged()), this, SLOT(onScriptEditorModified()));
    recycled_editors.append(iter.value());
  }

  editor_hash.clear();

  editor_stack.setCurrentWidget(&dummy_editor);

  open_path = QString();
  archive_.clear();

  enableEditing(false);

  archive_opened = false;

  setDataModified(false);
}

void RGSS_MainWindow::onScriptIndexChange(QModelIndex current, QModelIndex)
{
  Script *script = archive_.indexToScript(current);

  script_name_editor_.setEnabled(script);

  if (!script) {
    script_name_editor_.clear();
    editor_stack.setCurrentWidget(&dummy_editor);
    return;
  }

  editor_stack.setCurrentWidget(getEditorForScript(script));
  script_name_editor_.setText(script->name);
}

void RGSS_MainWindow::onScriptNameEdited(QString const& name) {
  archive_.setData(getCurrentIndex(), name, Qt::DisplayRole);
  setDataModified(true);
}

void RGSS_MainWindow::loadScriptArchive(QString const& file, bool show_errors) {
  QFile archiveFile(file);
  if (!archiveFile.open(QFile::ReadOnly)) {
    if (show_errors)
      QMessageBox::critical(this, "File reading error.", "Cannot open file: " + file);
    return;
  }

  try {
    archive_.read(archiveFile);
    archiveFile.close();
  }
  catch (const QByteArray &error) {
    if (show_errors)
      QMessageBox::critical(this, "File reading error.", "Cannot read: " + file + "\n" + error);
    return;
  }

  QFileInfo finfo(file);
  open_path = finfo.absoluteFilePath();
  last_valid_folder = finfo.absolutePath();

  setupLoadedArchive();
  setDataModified(false);
}

void RGSS_MainWindow::onSaveArchive() {
  if (open_path.isEmpty()) {
    onSaveArchiveAs();
    return;
  }

  saveScriptArchiveAs(open_path);
}

bool RGSS_MainWindow::saveScriptArchiveAs(QString const& file) {
  /* Determine marshal format */
  QFileInfo finfo(file);
  QString fileExt = finfo.suffix();
  Script::Format format;

  if (fileExt == "rxdata")
    format = Script::XP;
  else if (fileExt == "rvdata")
    format = Script::XP;
  else if (fileExt == "rvdata2")
    format = Script::VXAce;
  else {
    QMessageBox::critical(this, "File saving error.", "Unrecognized file extension: " + fileExt);
    return false;
  }

  QFile archiveFile(file);
  if (!archiveFile.open(QFile::WriteOnly)) {
    QMessageBox::critical(this, "File saving error.", "Cannot open for writing: " + file);
    return false;
  }

  /* Store any modifications into the archive */
  storeChangedScripts();

  try {
    archive_.write(archiveFile, format);
    archiveFile.close();
  }
  catch (const QByteArray &) {
    QMessageBox::critical(this, "File saving error.", "Cannot save: " + file);
    return false;
  }

  /* Update filename */
  open_path = finfo.absoluteFilePath();
  last_valid_folder = finfo.absolutePath();

  setDataModified(false);

  return true;
}
