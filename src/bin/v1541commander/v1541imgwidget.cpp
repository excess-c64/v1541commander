#include "v1541imgwidget.h"

extern "C" {
#include <1541img/d64.h>
#include <1541img/d64reader.h>
#include <1541img/cbmdosfs.h>
}

V1541ImgWidget::V1541ImgWidget() : QWidget(), d64(0), fs(0)
{
    setMinimumSize(16*16, 16*25);
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
		d64 = 0;
	    }
	    setWindowTitle(filename);
	}
    }
}

bool V1541ImgWidget::hasValidImage() const
{
    return fs || d64;
}

