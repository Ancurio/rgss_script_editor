#ifndef EDITOR_WIDGET_HXX
#define EDITOR_WIDGET_HXX

#include <Qsci/qsciscintilla.h>

class EditorWidget : public QsciScintilla
{
  Q_OBJECT
public:
  EditorWidget(QWidget *parent = 0);

  void centreLine();

signals:
  void archiveDropped(const QString &filename);

private:
  void dragEnterEvent(QDragEnterEvent *);
  void dragMoveEvent(QDragMoveEvent *e);
  void dropEvent(QDropEvent *e);
};

#endif // EDITOR_WIDGET_HXX
