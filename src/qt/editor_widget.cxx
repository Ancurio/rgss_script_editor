#include "editor_widget.hxx"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QUrl>
#include <QFileInfo>
#include <QDebug>
EditorWidget::EditorWidget(QWidget *parent)
  : QsciScintilla(parent)
{
  setAcceptDrops(true);
}

void EditorWidget::dragEnterEvent(QDragEnterEvent *e)
{
  if (!e->mimeData()->hasUrls()) {
    e->ignore();
    return;
  }

  QString filename = e->mimeData()->urls().first().toLocalFile();
  QString suffix = QFileInfo(filename).suffix();

  if (suffix != "rxdata" && suffix != "rxdata2") {
    e->ignore();
    return;
  }

  e->acceptProposedAction();
}

void EditorWidget::dragMoveEvent(QDragMoveEvent *e)
{
  e->acceptProposedAction();
}

void EditorWidget::dropEvent(QDropEvent *e)
{
  QString filename = e->mimeData()->urls().first().toLocalFile();

  archiveDropped(filename);
}
