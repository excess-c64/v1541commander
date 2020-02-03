#include "skippablequestion.h"

#include <QCheckBox>
#include <QMessageBox>

class SkippableQuestion::priv
{
    public:
	priv();
	QCheckBox skipCheckBox;
};

SkippableQuestion::priv::priv() :
    skipCheckBox(tr("Don't ask this question again"))
{}

SkippableQuestion::SkippableQuestion(const QString &title,
	const QString &caption, QWidget *parent) :
    QMessageBox(QMessageBox::Question, title, caption,
            QMessageBox::Yes|QMessageBox::No, parent)
{
    d = new priv();
    setCheckBox(&d->skipCheckBox);
}

SkippableQuestion::~SkippableQuestion()
{
    delete d;
}

bool SkippableQuestion::skip() const
{
    return d->skipCheckBox.isChecked();
}

