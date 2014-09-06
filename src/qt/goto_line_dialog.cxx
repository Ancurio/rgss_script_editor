#include "goto_line_dialog.hxx"

#include <QHBoxLayout>
#include <QLabel>
#include <QIntValidator>

GotoLineDialog::GotoLineDialog(QWidget *parent)
    : QDialog(parent)
{
	/* The window will be too small for a title */
	setWindowTitle(QString(" "));

	edit = new LineEdit(this);
	edit->setValidator(new QIntValidator(this));
	edit->setFixedWidth(50);

	QBoxLayout *box = new QHBoxLayout();
	box->addWidget(new QLabel(tr("Line:")));
	box->addWidget(edit);

	setLayout(box);

	connect(edit, SIGNAL(returnPressed()), SLOT(accept()));
	connect(edit, SIGNAL(escapePressed()), SLOT(reject()));

	resize(minimumSize());
}

int GotoLineDialog::getLine()
{
	return edit->text().toInt();
}
