#include "v1541imgwidget.h"
#include "petsciistr.h"

#include <QLabel>
#include <QFont>
#include <QVBoxLayout>

extern "C" {
#include <1541img/d64.h>
#include <1541img/d64reader.h>
#include <1541img/cbmdosfs.h>
#include <1541img/cbmdosvfs.h>
#include <1541img/cbmdosfile.h>
#include <1541img/filedata.h>
}

V1541ImgWidget::V1541ImgWidget() : QWidget(), d64(0), fs(0)
{
    QVBoxLayout *layout = new QVBoxLayout();
    label = new QLabel();
    label->setFont(QFont("C64 Pro Mono"));
    layout->addWidget(label);
    setLayout(layout);
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
		const CbmdosVfs *vfs = CbmdosFs_rvfs(fs);
		QString dirstr("0 ");
		uint8_t linebuf[27];
		PetsciiStr dirheader((char *)linebuf, 24);
		CbmdosVfs_getDirHeader(vfs, linebuf);
		dirstr.append(dirheader.toString(0, 1));

		PetsciiStr dirline((char *)linebuf, 27);
		for (unsigned fn = 0; fn < CbmdosVfs_fileCount(vfs); ++fn)
		{
		    const CbmdosFile *file = CbmdosVfs_rfile(vfs, fn);
		    CbmdosFile_getDirLine(file, linebuf);
		    dirstr.append("\n");
		    dirstr.append(dirline.toString());
		}

		dirstr.append("\n");
		PetsciiStr freeline((char *)linebuf, 16);
		CbmdosFs_getFreeBlocksLine(fs, linebuf);
		dirstr.append(freeline.toString());
		label->setText(dirstr);
	    }
	    setWindowTitle(filename);
	}
    }
}

bool V1541ImgWidget::hasValidImage() const
{
    return fs || d64;
}

