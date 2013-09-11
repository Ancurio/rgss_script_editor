#include "main_window.hxx"

#include <QtGui/QAction>
#include <QtGui/QKeySequence>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QFileDialog>
#include <QtCore/QFileInfo>

#include <Qsci/qscilexerruby.h>

#include <algorithm>
#include <iostream>

RGSS_MainWindow::RGSS_MainWindow(QWidget* const parent, Qt::WindowFlags const flags)
    : QMainWindow(parent, flags)
    , current_row_(0)
{
  QSize const window_size(800, 600);
  resize(window_size);

  QMenuBar* const menu_bar = new QMenuBar(this);
  {
    QMenu* const file = new QMenu(tr("File"), menu_bar);

    QAction* const open = new QAction(tr("Open"), menu_bar);
    open->setShortcut(QKeySequence(QKeySequence::Open));
    connect(open, SIGNAL(triggered()), SLOT(openScriptArchive()));
    file->addAction(open);

    QAction* const save = new QAction(tr("Save"), menu_bar);
    save->setShortcut(QKeySequence(QKeySequence::Save));
    connect(save, SIGNAL(triggered()), SLOT(saveScriptArchive()));
    file->addAction(save);

    QAction* const save_as = new QAction(tr("Save As"), menu_bar);
    save_as->setShortcut(QKeySequence(QKeySequence::SaveAs));
    connect(save_as, SIGNAL(triggered()), SLOT(saveScriptArchiveAs()));
    file->addAction(save_as);

    QAction* const close = new QAction(tr("Close"), menu_bar);
    close->setShortcut(QKeySequence(QKeySequence::Close));
    connect(close, SIGNAL(triggered()), SLOT(closeScriptArchive()));
    file->addAction(close);

    QMenu* const edit = new QMenu("Edit", menu_bar);

    menu_bar->addMenu(file);
    menu_bar->addMenu(edit);
  }
  setMenuBar(menu_bar);

  splitter_.setOrientation(Qt::Horizontal);
  setCentralWidget(&splitter_);

  left_side_.setLayout(new QVBoxLayout(&left_side_));
  left_side_.layout()->addWidget(&script_list_);
  left_side_.layout()->addWidget(&script_name_editor_);

  splitter_.addWidget(&left_side_);
  splitter_.addWidget(&script_editor_);

  QList<int> sizes;
  // 1 : 3 = list : editor
  sizes.push_back(window_size.width() / 4 * 1);
  sizes.push_back(window_size.width() / 4 * 3);
  splitter_.setSizes(sizes);

  // other setting
  script_editor_.setUtf8(true);
  script_editor_.setEolMode(QsciScintilla::EolWindows);

  // indent
  script_editor_.setIndentationWidth(2);
  script_editor_.setAutoIndent(true);

  // lexer and font
  QFont font;
  font.setStyleHint(QFont::Monospace);
  font.setFamily(font.defaultFamily());
  script_editor_.setFont(font);
  script_editor_.setMarginsFont(font);
  QsciLexer* lexer = new QsciLexerRuby(&script_editor_);
  lexer->setDefaultFont(font);
  script_editor_.setLexer(lexer);

  // line number
  QFontMetrics fontmetrics(font);
  script_editor_.setMarginWidth(0, fontmetrics.width("00000") + 6);
  script_editor_.setMarginLineNumbers(0, true);

  // auto complete
  script_editor_.setAutoCompletionThreshold(3);
  script_editor_.setAutoCompletionSource(QsciScintilla::AcsAll);

  connect(&script_list_, SIGNAL(currentRowChanged(int)), SLOT(setCurrentIndex(int)));
  connect(&script_name_editor_, SIGNAL(textEdited(QString)), SLOT(scriptNameEdited(QString)));

  enableEditing(false);
}

void RGSS_MainWindow::enableEditing(bool v) {
  script_editor_.setEnabled(v);
  script_name_editor_.setEnabled(v);
  script_list_.setEnabled(v);
}

bool RGSS_MainWindow::scriptArchiveOpened() const {
  return not file_.isEmpty();
}

static const char *fileFilter =
    QT_TRANSLATE_NOOP("RGSS_MainWindow",
    "Script Archive (Scripts.rxdata Scripts.rvdata Scripts.rvdata2);;"
    "All files (*)");

void RGSS_MainWindow::openScriptArchive() {

  QString const f = QFileDialog::getOpenFileName(
      this, tr("Select script archive to open..."), QDir::homePath(), tr(fileFilter));

  if(f.isNull()) { return; } // check cancel
  setScriptArchive(f);
}

void RGSS_MainWindow::saveScriptArchiveAs() {
  QString const f = QFileDialog::getSaveFileName(
      this, tr("Select saving file..."), QFileInfo(file_).dir().path(), tr(fileFilter));
  if(f.isNull()) { return; } // check cancel
  saveScriptArchiveAs(f);
}

void RGSS_MainWindow::closeScriptArchive() {
  file_ = QString();
  archive_.scripts.clear();
  setCurrentIndex(-1);

  script_list_.clear();

  enableEditing(false);
}

void RGSS_MainWindow::setCurrentIndex(int idx) {
  if (idx == -1) {
    script_editor_.clear();
    script_name_editor_.clear();
    current_row_ = -1;
    return;
  }

  if(not scriptArchiveOpened()) { return; }

  Q_ASSERT(ScriptList::size_type(idx) < archive_.scripts.size());
  Q_ASSERT(ScriptList::size_type(current_row_) < archive_.scripts.size());

  std::string const script = script_editor_.text().toStdString();
  if(not parseScript(script)) {
    QString const syntax_err = "Syntax error in " + archive_.scripts[current_row_].name;
    QMessageBox::warning(this, "Syntax error", syntax_err);
    return;
  }

  if(script_editor_.isModified() && current_row_ != -1) {
    archive_.scripts[current_row_].data = script_editor_.text();
    script_editor_.setModified(false);
  }

  script_editor_.setText(archive_.scripts[idx].data);
  script_name_editor_.setText(archive_.scripts[idx].name);

  std::swap(idx, current_row_);
}

void RGSS_MainWindow::scriptNameEdited(QString const& name) {
  Q_ASSERT(current_row_ == script_list_.currentRow());
  script_list_.currentItem()->setText(name);
  archive_.scripts[current_row_].name = name;
}

void RGSS_MainWindow::setScriptArchive(QString const& file) {
  QFile archiveFile(file);
  if (!archiveFile.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, "File reading error.", "Cannot open file: " + file);
    return;
  }

  try {
    archive_.read(archiveFile);
    archiveFile.close();
  }
  catch (const QByteArray &error) {
    QMessageBox::critical(this, "File reading error.", "Cannot read: " + file + "\n" + error);
    return;
  }

  file_ = QFileInfo(file).absoluteFilePath();

  int const next_row = std::max(std::min<int>(current_row_, archive_.scripts.size() - 1), 0);

  script_list_.clear();
  for(ScriptList::const_iterator i = archive_.scripts.begin(); i < archive_.scripts.end(); ++i) {
    script_list_.addItem(i->name);
  }

  script_list_.setCurrentRow(next_row);
  script_editor_.setModified(false);

  enableEditing(true);
}

void RGSS_MainWindow::saveScriptArchive() {
  if(script_editor_.isModified()) {
    archive_.scripts[current_row_].data = script_editor_.text();
    script_editor_.setModified(false);
  }

  saveScriptArchiveAs(file_);
}

void RGSS_MainWindow::saveScriptArchiveAs(QString const& file) {
  /* Determine marshal format */
  QFileInfo finfo(file);
  QString fileExt = finfo.suffix();
  ScriptArchive::Format format;

  if (fileExt == "rxdata")
    format = ScriptArchive::XP;
  else if (fileExt == "rvdata")
    format = ScriptArchive::XP;
  else if (fileExt == "rvdata2")
    format = ScriptArchive::VXAce;
  else {
    QMessageBox::critical(this, "File saving error.", "Unrecognized file extension: " + fileExt);
    return;
  }

  QFile archiveFile(file);
  if (!archiveFile.open(QFile::WriteOnly)) {
    QMessageBox::critical(this, "File saving error.", "Cannot open for writing: " + file);
    return;
  }

  try {
    archive_.write(archiveFile, format);
    archiveFile.close();
  }
  catch (const QByteArray &) {
    QMessageBox::critical(this, "File saving error.", "Cannot save: " + file);
    return;
  }

  /* Update filename */
  file_ = QFileInfo(file).absoluteFilePath();
}
