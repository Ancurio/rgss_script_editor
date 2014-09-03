#ifndef SEARCH_BAR_HXX
#define SEARCH_BAR_HXX

#include <QWidget>
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

class QLabel;

class SearchBar : public QWidget
{
  Q_OBJECT

public:
  SearchBar(QWidget *parent = 0);

  void setEditText(const QString &text);
  void setNotFoundFlag();

public slots:
  void onShow();

signals:
  void hidePressed();
  void searchComitted(const QString &text);
  void searchNext();

private slots:
  void onEnterPressed();
  void onTextChanged();

private:
  LineEdit *edit;
  bool text_changed;

  QLabel *not_found;
};

#endif // SEARCH_BAR_HXX
