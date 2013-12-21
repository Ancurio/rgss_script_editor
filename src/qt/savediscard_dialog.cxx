#include "savediscard_dialog.hxx"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

SaveDiscardDialog::SaveDiscardDialog(QWidget *parent)
    : QDialog(parent)
{
  QVBoxLayout *vbox = new QVBoxLayout();

  QLabel *text = new QLabel(tr("There are unsaved changes"));

  vbox->addWidget(text);
  vbox->addStretch();

  QHBoxLayout *hbox = new QHBoxLayout();

  QPushButton *button;

  button = new QPushButton(tr("Cancel"));
  connect(button, SIGNAL(clicked()), SLOT(onCancel()));
  hbox->addWidget(button);

  button = new QPushButton(tr("Discard"));
  connect(button, SIGNAL(clicked()), SLOT(onDiscard()));
  hbox->addWidget(button);

  button = new QPushButton(tr("Save"));
  connect(button, SIGNAL(clicked()), SLOT(onSave()));
  hbox->addWidget(button);

  vbox->addLayout(hbox);

  setLayout(vbox);
}

void SaveDiscardDialog::onCancel()
{
  QDialog::done(Cancel);
}

void SaveDiscardDialog::onDiscard()
{
  QDialog::done(Discard);
}

void SaveDiscardDialog::onSave()
{
  QDialog::done(Save);
}

void SaveDiscardDialog::reject()
{
  onCancel();
}
