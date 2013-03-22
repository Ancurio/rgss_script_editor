#include "main_window.hxx"

#include <QtGui/QAction>
#include <QtGui/QKeySequence>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
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
    QMenu* const file = new QMenu("File", menu_bar);
    QAction* const save = new QAction("Save", menu_bar);
    save->setShortcut(QKeySequence(QKeySequence::Save));
    connect(save, SIGNAL(triggered()), SLOT(saveScriptArchive()));
    file->addAction(save);
    QMenu* const edit = new QMenu("Edit", menu_bar);

    menu_bar->addMenu(file);
    menu_bar->addMenu(edit);
  }
  setMenuBar(menu_bar);

  splitter_.setOrientation(Qt::Horizontal);
  setCentralWidget(&splitter_);

  splitter_.addWidget(&script_list_);
  splitter_.addWidget(&script_editor_);

  QList<int> sizes;
  // 1 : 3 = list : editor
  sizes.push_back(window_size.width() / 4 * 1);
  sizes.push_back(window_size.width() / 4 * 3);
  splitter_.setSizes(sizes);

  script_editor_.setUtf8(true);
  script_editor_.setLexer(new QsciLexerRuby(&script_editor_));

  connect(&script_list_, SIGNAL(currentRowChanged(int)), SLOT(setCurrentIndex(int)));
}

void RGSS_MainWindow::setCurrentIndex(int idx) {
  assert(size_t(idx) < scripts_.size());
  assert(size_t(current_row_) < scripts_.size());

  std::string const script = script_editor_.text().toStdString();
  if(not parseScript(script)) {
    QString const syntax_err = "Syntax error in "
        + QString::fromStdString(scripts_[current_row_].name);
    QMessageBox::warning(this, "Syntax error", syntax_err);
  }

  if(script_editor_.isModified()) {
    scripts_[current_row_].data = script;
    script_editor_.setModified(false);
  }
  script_editor_.setText(QString::fromStdString(scripts_[idx].data));

  std::swap(idx, current_row_);
}

void RGSS_MainWindow::setScriptArchive(QString const& file) {
  if(not QFileInfo(file).exists()) { return; }

  ScriptList tmp;
  if(not loadScripts(file.toStdString(), tmp)) { return; }

  scripts_.swap(tmp);
  file_ = QFileInfo(file).absoluteFilePath();

  script_list_.clear();
  for(ScriptList::const_iterator i = scripts_.begin(); i < scripts_.end(); ++i) {
    script_list_.addItem(QString::fromStdString(i->name));
  }

  int const current_row = script_list_.currentRow();
  int const next_row = std::max(std::min<int>(current_row, scripts_.size() - 1), 0);
  current_row_ = next_row;
  script_list_.setCurrentRow(next_row);
}

void RGSS_MainWindow::saveScriptArchive() {
  saveScriptArchiveAs(file_);
}

void RGSS_MainWindow::saveScriptArchiveAs(QString const& file) {
  bool result = dumpScripts(file.toStdString(), scripts_);
  if(result and QFileInfo(file).exists()) {
    // update file name
    file_ = QFileInfo(file).absoluteFilePath();
  } else {
    QMessageBox::critical(this, "File saving error.", "Cannot save: " + file);
  }
}
