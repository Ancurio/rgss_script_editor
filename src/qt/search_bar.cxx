#include "search_bar.hxx"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>


SearchBar::SearchBar(QWidget *parent)
  : QWidget(parent),
    text_changed(false)
{
  QBoxLayout *layout = new QHBoxLayout(this);

  edit = new LineEdit();
  connect(edit, SIGNAL(returnPressed()), SLOT(onEnterPressed()));
  connect(edit, SIGNAL(textChanged(QString)), SLOT(onTextChanged()));

  QPushButton *hide = new QPushButton(tr("Hide"));
  connect(hide, SIGNAL(clicked()), SIGNAL(hidePressed()));
  connect(edit, SIGNAL(escapePressed()), SIGNAL(hidePressed()));

  not_found = new QLabel(tr("Not found"));
  not_found->setStyleSheet("color: red");
  not_found->hide();

  layout->addWidget(new QLabel(tr("Find:")));
  layout->addWidget(edit);
  layout->addWidget(not_found);
  layout->addStretch();
  layout->addWidget(hide);
  layout->setContentsMargins(QMargins());

  setLayout(layout);
}

void SearchBar::setEditText(const QString &text)
{
  edit->setText(text);
}

void SearchBar::setNotFoundFlag()
{
  not_found->show();
}

void SearchBar::onShow()
{
  show();
  edit->selectAll();
  edit->setFocus();
}

void SearchBar::onEnterPressed()
{
  if (edit->text().isEmpty())
    return;

  if (text_changed)
  {
    text_changed = false;
    searchComitted(edit->text());
  }
  else
  {
    searchNext();
  }
}

void SearchBar::onTextChanged()
{
  text_changed = true;
  not_found->hide();
}
