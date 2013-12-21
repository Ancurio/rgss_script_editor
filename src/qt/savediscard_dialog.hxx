#ifndef SAVEDISCARD_DIALOG_HXX
#define SAVEDISCARD_DIALOG_HXX

#include <QDialog>

class SaveDiscardDialog : public QDialog
{
  Q_OBJECT
public:
  SaveDiscardDialog(QWidget *parent = 0);

  enum {
    Cancel,
    Discard,
    Save
  };

private slots:
  void onCancel();
  void onDiscard();
  void onSave();
private:
  void reject();
};

#endif // SAVEDISCARD_DIALOG_HXX
