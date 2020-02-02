#include "skippablequestion.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QVBoxLayout>
#include <QWindow>

class SkippableQuestion::priv
{
    public:
	priv(const QString &caption);
	QVBoxLayout mainLayout;
	QHBoxLayout questionLayout;
	QLabel icon;
	QLabel caption;
	QCheckBox skipCheckBox;
	QDialogButtonBox buttons;
};

SkippableQuestion::priv::priv(const QString &caption) :
    mainLayout(),
    questionLayout(),
    icon(),
    caption(caption),
    skipCheckBox(tr("Don't ask this question again")),
    buttons(QDialogButtonBox::Yes|QDialogButtonBox::No)
{}

SkippableQuestion::SkippableQuestion(const QString &title,
	const QString &caption, QWidget *parent)
    : QDialog(parent)
{
    d = new priv(caption);
    setWindowTitle(title);
    int iconSize = style()->pixelMetric(
	    QStyle::PM_MessageBoxIconSize, 0, this);
    d->icon.setPixmap(style()->standardIcon(
		QStyle::SP_MessageBoxQuestion, 0, this)
	    .pixmap(qobject_cast<QWindow *>(window()),
		QSize(iconSize, iconSize)));
    d->caption.setWordWrap(true);
    d->questionLayout.addWidget(&d->icon);
    d->questionLayout.addWidget(&d->caption);
    d->mainLayout.addLayout(&d->questionLayout);
    d->mainLayout.addWidget(&d->skipCheckBox);
    d->mainLayout.addWidget(&d->buttons);
    setLayout(&d->mainLayout);

    connect(&d->buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(&d->buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

SkippableQuestion::~SkippableQuestion()
{
    delete d;
}

bool SkippableQuestion::skip() const
{
    return d->skipCheckBox.isChecked();
}

