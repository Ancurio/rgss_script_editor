#ifndef GOTO_LINE_DIALOG_HXX
#define GOTO_LINE_DIALOG_HXX

#include <QDialog>

#include "line_edit.hxx"

class QLineEdit;

class GotoLineDialog : public QDialog
{
public:
	GotoLineDialog(QWidget *parent = 0);

	int getLine();

private:
	LineEdit *edit;
};

#endif // GOTO_LINE_DIALOG_HXX
