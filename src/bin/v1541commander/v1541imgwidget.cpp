#include "v1541imgwidget.h"
#include "cbmdosfsmodel.h"

#include <QListView>
#include <QVBoxLayout>

extern "C" {
#include <1541img/d64.h>
#include <1541img/d64reader.h>
#include <1541img/cbmdosfs.h>
}

V1541ImgWidget::V1541ImgWidget() : QWidget(), d64(0), fs(0), model(0)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    dirList = new QListView(this);
    dirList->setMinimumHeight(200);
    dirList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    layout->addWidget(dirList);
    setLayout(layout);
}

V1541ImgWidget::~V1541ImgWidget()
{
    if (fs)
    {
	CbmdosFs_destroy(fs);
    }
    else if (d64)
    {
	D64_destroy(d64);
    }
}

void V1541ImgWidget::open(const QString& filename)
{
    if (fs)
    {
	CbmdosFs_destroy(fs);
	fs = 0;
    }
    if (d64)
    {
	D64_destroy(d64);
	d64 = 0;
    }
    FILE *d64file = fopen(filename.toUtf8().data(), "rb");
    if (d64file)
    {
	d64 = readD64(d64file);
	fclose(d64file);

	if (d64)
	{
	    CbmdosFsOptions opts = CFO_DEFAULT;
	    if (D64_type(d64) == D64_40TRACK) opts.flags = CFF_40TRACK;
	    fs = CbmdosFs_fromImage(d64, opts);
	    if (fs)
	    {
		CbmdosFsModel *newModel = new CbmdosFsModel(fs, this);
		dirList->setModel(newModel);
		if (model) delete model;
		model = newModel;
                dirList->setMinimumWidth(
                        dirList->sizeHintForColumn(0)
                        + 2 * dirList->frameWidth());
                int minItems = model->rowCount();
                if (minItems < 10) minItems = 10;
                if (minItems > 40) minItems = 40;
                dirList->setMinimumHeight(
                        dirList->sizeHintForRow(0) * minItems
                        + 2 * dirList->frameWidth());
	    }
	    setWindowTitle(filename);
	}
    }
}

bool V1541ImgWidget::hasValidImage() const
{
    return fs || d64;
}

