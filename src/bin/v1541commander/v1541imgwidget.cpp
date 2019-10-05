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
		uint8_t namelen;
		const char *rawname = CbmdosVfs_name(vfs, &namelen);
		QString dirstr("0 ");
		dirstr.append(0xe222);
		if (namelen > 0)
		{
		    PetsciiStr name(rawname, namelen);
		    dirstr.append(name.toString(0, 1));
		}
		if (namelen < 16)
		{
			dirstr.append(QString(16 - namelen, 0xe2a0));
		}
		dirstr.append(0xe222);
		dirstr.append(0xe2a0);
		const char *rawid = CbmdosVfs_id(vfs, &namelen);
		if (namelen > 0)
		{
		    PetsciiStr id(rawid, namelen);
		    dirstr.append(id.toString(0, 1));
		}
		if (namelen < 5)
		{
		    if (namelen < 3)
		    {
			dirstr.append(QString(3 - namelen, 0xe2a0));
		    }
		    if (namelen < 4)
		    {
			dirstr.append(0xe232);
		    }
		    dirstr.append(0xe241);
		}
		for (unsigned fn = 0; fn < CbmdosVfs_fileCount(vfs); ++fn)
		{

		    const CbmdosFile *file = CbmdosVfs_rfile(vfs, fn);
		    uint16_t blocks = CbmdosFile_forcedBlocks(file);
		    if (blocks == 0xffff)
		    {
			const FileData *dat = CbmdosFile_rdata(file);
			if (!dat || CbmdosFile_type(file) == CFT_DEL)
			{
			    blocks = 0;
			}
			else
			{
			    size_t size = FileData_size(dat);
			    blocks = size / 254;
			    if (size % 254) ++blocks;
			}
		    }
		    dirstr.append(QString("\n%1\"").arg(blocks, -5, 10));
		    rawname = CbmdosFile_name(file, &namelen);
		    if (namelen > 0)
		    {
			PetsciiStr name(rawname, namelen);
			dirstr.append(name.toString());
		    }
		    if (namelen < 16)
		    {
			dirstr.append(QString(16 - namelen, 0xe0a0));
		    }
		    dirstr.append("\" ");
		    dirstr.append(CbmdosFileType_name(CbmdosFile_type(file)));
		}
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

