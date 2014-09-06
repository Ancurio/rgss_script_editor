#ifndef LINE_EDIT_HXX
#define LINE_EDIT_HXX

#include <QLineEdit>
#include <QKeyEvent>

class LineEdit : public QLineEdit
{
  Q_OBJECT

public:
  LineEdit(QWidget *parent = 0)
    : QLineEdit(parent)
  {}

signals:
  void escapePressed();

private:
  void keyPressEvent(QKeyEvent *ke)
  {
    if (ke->key() == Qt::Key_Escape)
    {
      escapePressed();
      ke->accept();
      return;
    }

    QLineEdit::keyPressEvent(ke);
  }
};

#endif // LINE_EDIT_HXX
