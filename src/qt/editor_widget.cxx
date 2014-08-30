#include "editor_widget.hxx"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QUrl>
#include <QFileInfo>
#include <QPalette>

EditorWidget::EditorWidget(QWidget *parent)
  : QsciScintilla(parent)
{
  setAcceptDrops(true);

  // Set cursor color to text color
  const QColor &textColor = palette().text().color();
  setCaretForegroundColor(textColor);
}

void EditorWidget::dragEnterEvent(QDragEnterEvent *e)
{
  if (!e->mimeData()->hasUrls()) {
    e->ignore();
    return;
  }

  QList<QUrl> urls = e->mimeData()->urls();

  if (urls.isEmpty()) {
	  e->ignore();
      return;
  }

  QString filename = urls.first().toLocalFile();
  QString suffix = QFileInfo(filename).suffix();

  if (suffix != "rxdata" && suffix != "rvdata" && suffix != "rvdata2") {
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
