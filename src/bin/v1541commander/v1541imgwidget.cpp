#include "v1541imgwidget.h"
#include "v1541commander.h"
#include "cbmdosfsmodel.h"
#include "cbmdosfsoptionsdialog.h"
#include "cbmdosfsstatuswidget.h"
#include "cbmdosfswidget.h"
#include "cbmdosfilewidget.h"
#include "utils.h"

#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QListView>
#include <QMessageBox>
#include <QShortcut>
#include <QVBoxLayout>

#include <1541img/d64.h>
#include <1541img/d64reader.h>
#include <1541img/d64writer.h>
#include <1541img/cbmdosfile.h>
#include <1541img/cbmdosfs.h>
#include <1541img/cbmdosvfs.h>
#include <1541img/cbmdosvfsreader.h>
#include <1541img/filedata.h>
#include <1541img/hostfilereader.h>
#include <1541img/hostfilewriter.h>
#include <1541img/lynx.h>
#include <1541img/zc45compressor.h>
#include <1541img/zc45extractor.h>
#include <1541img/zcfileset.h>

class V1541ImgWidget::priv
{
    public:
	priv();
	CbmdosFs *fs;
	CbmdosFsModel model;
	QListView dirList;
	CbmdosFsWidget fsprop;
	CbmdosFileWidget file;
        CbmdosFsStatusWidget fsstat;

	bool canSaveImage(V1541ImgWidget *w);
};

V1541ImgWidget::priv::priv() :
    fs(0),
    model(),
    dirList(),
    fsprop(),
    file(),
    fsstat()
{}

bool V1541ImgWidget::priv::canSaveImage(V1541ImgWidget *w)
{
    if (!fs) return false;
    CbmdosFsStatus status = CbmdosFs_status(fs);
    if (status & CbmdosFsStatus::CFS_DISKFULL)
    {
	QMessageBox::critical(w->window(), tr("Unable to save"),
		tr("<p>The virtual disk is full. This means not all files you "
		    "added could be entirely written. Therefore you can't "
		    "save the disk in this state.</p>"
		    "<p>You can either remove some files or try to change the "
		    "filesystem options (e.g. allow to store files on the "
		    "directory track or switch to a larger number of tracks) "
		    "and rewrite the disk, then try again.</p>"));
	return false;
    }
    if (status & CbmdosFsStatus::CFS_DIRFULL)
    {
	QMessageBox::critical(w->window(), tr("Unable to save"),
		tr("<p>The directory on the virtual disk is full. This means "
		    "not all files you added could be written to the "
		    "directory. Therefore you can't save the disk in this "
		    "state.</p>"
		    "<p>You can either remove some files or you can set the "
		    "filesystem option to allow long directories, rewrite the "
		    "disk, then try again.</p>"));
	return false;
    }
    if (status & CbmdosFsStatus::CFS_BROKEN)
    {
	QMessageBox::critical(w->window(), tr("Unable to save"),
		tr("<p>The filesystem on the virtual disk is broken for "
		    "unknown reasons. You can try to change some filesystem "
		    "options and rewrite the disk."));
	return false;
    }
    if (status & CbmdosFsStatus::CFS_INVALIDBAM)
    {
	if (QMessageBox::question(w->window(), tr("Save with invalid BAM?"),
		    tr("<p>The BAM on the virtual disk is currently invalid. "
			"You can fix this by rewriting the disk first, but "
                        "this could also destroy data on the disk not "
                        "recognized by this tool.</p>"
			"<p>Do you want to save the disk in the current "
			"state?</p>"),
		    QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
	{
	    return true;
	}
	else
	{
	    return false;
	}
    }
    return true;
}

V1541ImgWidget::V1541ImgWidget(QWidget *parent) : QWidget(parent)
{
    d = new priv();
    QHBoxLayout *layout = new QHBoxLayout(this);
    QVBoxLayout *propLayout = new QVBoxLayout();
    d->dirList.setMinimumHeight(200);
    d->dirList.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->dirList.setModel(&d->model);
    d->dirList.setDragEnabled(true);
    d->dirList.setAcceptDrops(true);
    d->dirList.setDragDropMode(QAbstractItemView::InternalMove);
    d->dirList.addAction(&cmdr.newFileAction());
    d->dirList.addAction(&cmdr.deleteFileAction());
    d->dirList.setContextMenuPolicy(Qt::ActionsContextMenu);
    layout->addWidget(&d->dirList);
    propLayout->addWidget(&d->fsprop);
    propLayout->addWidget(&d->file);
    propLayout->addStretch();
    layout->addLayout(propLayout);
    connect(d->dirList.selectionModel(), &QItemSelectionModel::currentChanged,
	    this, &V1541ImgWidget::selected);
    connect(&d->model,
	    SIGNAL(selectedIndexChanged(const QModelIndex &,
		    QItemSelectionModel::SelectionFlags)),
	    d->dirList.selectionModel(),
	    SLOT(setCurrentIndex(const QModelIndex &,
		    QItemSelectionModel::SelectionFlags)));
    connect(&d->model, &CbmdosFsModel::modified,
	    this, &V1541ImgWidget::modelModified);
    QShortcut *f3 = new QShortcut(QKeySequence(Qt::Key_F3), this);
    connect(f3, SIGNAL(activated()), &d->dirList, SLOT(setFocus()));
    QShortcut *sup = new QShortcut(QKeySequence(Qt::SHIFT+Qt::Key_Up), this);
    connect(sup, &QShortcut::activated, this, [this](){
	    if (!d->fs) return;
	    QModelIndexList selected =
		    d->dirList.selectionModel()->selectedIndexes();
	    if (selected.count() == 0) return;
	    QModelIndex current = selected.first();
	    if (current.row() < 2) return;
	    CbmdosVfs *vfs = CbmdosFs_vfs(d->fs);
	    if ((unsigned)current.row() > CbmdosVfs_fileCount(vfs)) return;
	    CbmdosVfs_move(vfs, current.row()-2, current.row()-1);
	    });
    QShortcut *sdn = new QShortcut(QKeySequence(Qt::SHIFT+Qt::Key_Down), this);
    connect(sdn, &QShortcut::activated, this, [this](){
	    if (!d->fs) return;
	    QModelIndexList selected =
		    d->dirList.selectionModel()->selectedIndexes();
	    if (selected.count() == 0) return;
	    QModelIndex current = selected.first();
	    if (current.row() < 1) return;
	    CbmdosVfs *vfs = CbmdosFs_vfs(d->fs);
	    if ((unsigned)current.row() >= CbmdosVfs_fileCount(vfs)) return;
	    CbmdosVfs_move(vfs, current.row(), current.row()-1);
	    });
    d->dirList.setToolTip(tr("The directory listing\n"
		"use F3 to activate\n"
		"change selection with cursor up/down\n"
		"move selected file with Shift + cursor up/down"));
    connect(&cmdr, &V1541Commander::autoMapToLcChanged,
	    this, [this](bool autoMapToLc){
		if (d->fs)
		{
		    CbmdosVfs_setAutoMapToLc(CbmdosFs_vfs(d->fs),
			    autoMapToLc, true);
		}
	    });
    QShortcut *sdel = new QShortcut(
            QKeySequence(Qt::SHIFT+Qt::Key_Delete), this);
    connect(sdel, &QShortcut::activated, this, [this](){deleteFile(true);});
}

V1541ImgWidget::~V1541ImgWidget()
{
    CbmdosFs *fs = d->fs;
    delete d;
    CbmdosFs_destroy(fs);
}

QWidget *V1541ImgWidget::statusWidget() const
{
    return &d->fsstat;
}

void V1541ImgWidget::selected(const QModelIndex &current,
	const QModelIndex &previous)
{
    (void) previous;

    CbmdosFile *file = 0;
    if (current.row() > 0 && d->fs)
    {
	CbmdosVfs *vfs = CbmdosFs_vfs(d->fs);
	if (current.row() <= (int)CbmdosVfs_fileCount(vfs))
	{
	    file = CbmdosVfs_file(vfs, current.row()-1);
	}
    }
    d->file.setFile(file);
    emit selectionChanged();
}

void V1541ImgWidget::modelModified()
{
    d->fsstat.setStatus(CbmdosFs_status(d->fs));
    emit modified();
}

void V1541ImgWidget::keyPressEvent(QKeyEvent *event)
{
    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
    if (mods == Qt::NoModifier)
    {
	int key = event->key();
	if (key == Qt::Key_Up || key == Qt::Key_Down)
	{
	    event->accept();
	    if (!d->fs) return;
	    QItemSelectionModel *ism = d->dirList.selectionModel();
	    QModelIndexList selected = ism->selectedIndexes();
	    if (selected.count() == 0) return;
	    QModelIndex current = selected.first();
	    if (key == Qt::Key_Up)
	    {
		if (current.row() < 2) return;
		QModelIndex prev = d->model.index(current.row() - 1, 0);
		ism->setCurrentIndex(prev,
			QItemSelectionModel::ClearAndSelect);
	    }
	    else
	    {
		if (current.row() >= d->model.rowCount() - 2) return;
		QModelIndex next = d->model.index(current.row() + 1, 0);
		ism->setCurrentIndex(next,
			QItemSelectionModel::ClearAndSelect);
	    }
	}
    }
    return QWidget::keyPressEvent(event);
}

void V1541ImgWidget::newImage()
{
    CbmdosFs_destroy(d->fs);
    d->fs = 0;
    CbmdosFsOptions opts = CFO_DEFAULT;
    CbmdosFsOptionsDialog optDlg(&opts, parentWidget());
    optDlg.setWindowTitle(QString(tr("Options for new image")));
    if (optDlg.exec() == QDialog::Accepted)
    {
	d->fs = CbmdosFs_create(opts);
	if (cmdr.autoMapToLc())
	{
	    CbmdosVfs_setAutoMapToLc(CbmdosFs_vfs(d->fs), true, true);
	}
	d->model.setFs(d->fs);
	d->fsprop.setFs(d->fs);
	d->dirList.setMinimumWidth(
		d->dirList.sizeHintForColumn(0)
		+ 2 * d->dirList.frameWidth());
	d->dirList.setMinimumHeight(
		d->dirList.sizeHintForRow(0) * 10
		+ 2 * d->dirList.frameWidth());
	d->dirList.setFocus();
        d->fsstat.setStatus(CbmdosFs_status(d->fs));
    }
}

void V1541ImgWidget::open(const QString& filename)
{
    CbmdosFs_destroy(d->fs);
    d->fs = 0;

    D64 *d64 = 0;
    bool extracted = false;

    QFileInfo fileInfo(filename);
    if (fileInfo.baseName().indexOf('!') == 1 &&
	    !(fileInfo.suffix().toUpper() == "D64"))
    {
	ZcFileSet *fileset = ZcFileSet_fromFile(qfname(filename));
	if (fileset)
	{
	    d64 = extractZc45(fileset);
	    ZcFileSet_destroy(fileset);
	}
	extracted = true;
    }
    else
    {
	FILE *file = qfopen(filename, "rb");
	if (file)
	{
	    FileData *data = readHostFile(file);
	    fclose(file);
	    if (isLynx(data))
	    {
		CbmdosVfs *vfs = CbmdosVfs_create();
		if (extractLynx(vfs, data) >= 0)
		{
		    openVfs(vfs);
		    emit modified();
		}
		FileData_destroy(data);
		return;
	    }
	    ZcFileSet *fileset = ZcFileSet_tryFromFileData(data);
	    if (fileset)
	    {
		if (QMessageBox::question(window(),
			    tr("Extract Zipcode?"),
			    tr("This image seems to contain a set of Zipcode "
				"files. Do you want to extract them instead?"),
			    QMessageBox::Yes|QMessageBox::No)
			== QMessageBox::Yes)
		{
		    d64 = extractZc45(fileset);
		    extracted = true;
		}
		ZcFileSet_destroy(fileset);
	    }
	    if (!extracted)
	    {
		d64 = readD64FromFileData(data);
	    }
	    FileData_destroy(data);
	}
    }

    if (d64)
    {
	CbmdosFsOptions opts = CFO_DEFAULT;
	int proberc = probeCbmdosFsOptions(&opts, d64);
	if (proberc < 0)
	{
	    opts.flags = (CbmdosFsFlags)
		(opts.flags | CbmdosFsFlags::CFF_RECOVER);
	    proberc = probeCbmdosFsOptions(&opts, d64);
	}
	if (proberc == 0)
	{
	    CbmdosFsOptionsDialog optDlg(&opts, parentWidget(), false);
	    optDlg.setWindowTitle(QString(tr("Options for "))
		    .append(filename));
	    if (D64_type(d64) == D64Type::D64_STANDARD)
	    {
		optDlg.disable42Tracks();
		optDlg.disable40Tracks();
	    }
	    else if (D64_type(d64) == D64Type::D64_40TRACK)
	    {
		optDlg.disable42Tracks();
	    }
	    if (opts.flags & CFF_42TRACK)
	    {
		optDlg.disable35Tracks();
		optDlg.disable40Tracks();
	    }
	    else if (opts.flags & CFF_40TRACK)
	    {
		optDlg.disable35Tracks();
	    }
	    if ((D64_type(d64) != D64Type::D64_STANDARD)
		    && !(opts.flags & (CFF_40TRACK|CFF_42TRACK)))
	    {
		opts.flags = (CbmdosFsFlags)
		    (opts.flags | CFF_40TRACK);
		optDlg.reset();
	    }
	    optDlg.disableZeroFree();
	    optDlg.exec();
	    d->fs = CbmdosFs_fromImage(d64, opts);
	    if (d->fs)
	    {
		if (cmdr.autoMapToLc())
		{
		    CbmdosVfs_setAutoMapToLc(CbmdosFs_vfs(d->fs), true, true);
		}
		d->model.setFs(d->fs);
		d->fsprop.setFs(d->fs);
		d->dirList.setMinimumWidth(
			d->dirList.sizeHintForColumn(0)
			+ 2 * d->dirList.frameWidth());
		int minItems = d->model.rowCount();
		if (minItems < 10) minItems = 10;
		if (minItems > 40) minItems = 40;
		d->dirList.setMinimumHeight(
			d->dirList.sizeHintForRow(0) * minItems
			+ 2 * d->dirList.frameWidth());
		if (opts.flags & CFF_RECOVER)
		{
		    CbmdosFs_rewrite(d->fs);
		    emit modified();
		}
		if (extracted) emit modified();
		d->dirList.setFocus();
                CbmdosFsStatus status = CbmdosFs_status(d->fs);
                d->fsstat.setStatus(status);
                if (status == CbmdosFsStatus::CFS_INVALIDBAM
                        && QMessageBox::question(window(),
                            tr("Invalid BAM -- save as new file?"),
                            tr("<p>This disk's BAM seems invalid. This could "
                                "be a sign that the disk contains things "
                                "not supported by this tool, like e.g. data "
                                "for trackloaders or C128 boot sectors.</p>"
                                "<p>It's therefore recommended to work on "
                                "a copy. Do you want to treat this as a new "
                                "image now?"),
                            QMessageBox::Yes|QMessageBox::No)
                        == QMessageBox::Yes)
                {
                    emit modified();
                }
	    }
	}
	if (!d->fs)
	{
	    D64_destroy(d64);
	}
    }
}

void V1541ImgWidget::openVfs(CbmdosVfs *vfs)
{
    CbmdosFs_destroy(d->fs);
    d->fs = CbmdosFs_fromVfs(vfs, CFO_DEFAULT);
    if (d->fs)
    {
	if (cmdr.autoMapToLc())
	{
	    CbmdosVfs_setAutoMapToLc(CbmdosFs_vfs(d->fs), true, true);
	}
	d->model.setFs(d->fs);
	d->fsprop.setFs(d->fs);
	d->dirList.setMinimumWidth(
		d->dirList.sizeHintForColumn(0)
		+ 2 * d->dirList.frameWidth());
	int minItems = d->model.rowCount();
	if (minItems < 10) minItems = 10;
	if (minItems > 40) minItems = 40;
	d->dirList.setMinimumHeight(
		d->dirList.sizeHintForRow(0) * minItems
		+ 2 * d->dirList.frameWidth());
	d->dirList.setFocus();
        d->fsstat.setStatus(CbmdosFs_status(d->fs));
    }
    else
    {
	CbmdosVfs_destroy(vfs);
    }
}

void V1541ImgWidget::save(const QString& filename)
{
    if (!hasValidImage()) return;
    if (!d->canSaveImage(this)) return;
    FILE *d64file = qfopen(filename, "wb");
    if (d64file)
    {
	if (writeD64(d64file, CbmdosFs_image(d->fs)) < 0)
	{
	    QMessageBox::critical(this, tr("Error writing file"),
		    tr("The selected file couldn't be written."));
	}
	else
	{
	    emit saved();
	}
	fclose(d64file);
    }
    else
    {
	QMessageBox::critical(this, tr("Error opening file"),
		tr("The selected file couldn't be opened for writing."));
    }
}

void V1541ImgWidget::exportZipcode(const QString& filename)
{
    if (!hasValidImage()) return;
    if (!d->canSaveImage(this)) return;
    ZcFileSet *fileset = compressZc45(CbmdosFs_image(d->fs));
    if (fileset)
    {
	if (ZcFileSet_save(fileset, qfname(filename)) < 0)
	{
	    QMessageBox::critical(this, tr("Error saving zipcode files"),
		    tr("The compressed files couldn't be written."));
	}
	ZcFileSet_destroy(fileset);
    }
    else
    {
	QMessageBox::critical(this, tr("Error compressing to Zipcode"),
		tr("compressing the image as Zipcode failed."));
    }
}

CbmdosVfs *V1541ImgWidget::exportZipcodeVfs()
{
    if (!hasValidImage()) return 0;
    if (!d->canSaveImage(this)) return 0;
    ZcFileSet *fileset = compressZc45(CbmdosFs_image(d->fs));
    if (fileset)
    {
	CbmdosVfs *vfs = CbmdosVfs_create();
	if (ZcFileSet_saveVfs(fileset, vfs) < 0)
	{
	    QMessageBox::critical(this, tr("Error writing zipcode files"),
		    tr("The compressed files couldn't be written."));
	    ZcFileSet_destroy(fileset);
	    CbmdosVfs_destroy(vfs);
	    return 0;
	}
	ZcFileSet_destroy(fileset);
	return vfs;
    }
    else
    {
	QMessageBox::critical(this, tr("Error compressing to Zipcode"),
		tr("compressing the image as Zipcode failed."));
	return 0;
    }
}

void V1541ImgWidget::exportLynx(const QString& filename)
{
    if (!hasValidImage()) return;
    FileData *lynx = archiveLynx(CbmdosFs_vfs(d->fs));
    if (lynx)
    {
	FILE *lynxFile = qfopen(filename, "wb");
	if (lynxFile)
	{
	    if (writeHostFile(lynx, lynxFile) < 0)
	    {
		QMessageBox::critical(this, tr("Error saving LyNX file"),
			tr("The LyNX archive couldn't be written."));
	    }
	    fclose(lynxFile);
	}
	else
	{
	    QMessageBox::critical(this, tr("Error opening LyNX file"),
		    tr("Couldn't open the selected file for writing."));
	}
	FileData_destroy(lynx);
    }
    else
    {
	QMessageBox::critical(this, tr("Error archiving as LyNX"),
		tr("Archiving as LyNX failed."));
    }
}

void V1541ImgWidget::fsOptions()
{
    if (!hasValidImage()) return;
    CbmdosFsOptions opts = CbmdosFs_options(d->fs);
    CbmdosFsOptionsDialog optDlg(&opts, parentWidget());
    if (optDlg.exec() == QDialog::Accepted)
    {
	CbmdosFs_setOptions(d->fs, opts);
    }
}

void V1541ImgWidget::rewriteImage()
{
    if (!hasValidImage()) return;
    QMessageBox::StandardButton reply = QMessageBox::question(this,
	    tr("Rewrite the image?"), tr("When rewriting an image from "
		"scratch, you might lose data that isn't stored in standard "
		"CBM DOS format on the disk, like for example raw sectors in "
		"demos or games. Are you sure you want to proceed?"),
	    QMessageBox::Ok|QMessageBox::Cancel);
    if (reply == QMessageBox::Ok)
    {
	CbmdosFs_rewrite(d->fs);
        d->fsstat.setStatus(CbmdosFs_status(d->fs));
	emit modified();
    }
}

void V1541ImgWidget::mapToLc()
{
    if (!hasValidImage()) return;
    QMessageBox::StandardButton reply = QMessageBox::question(this,
	    tr("Map UC gfx to LC?"), tr("This will possibly modify the disk "
		"name, id and all the file names. Are you sure you want to "
		"proceed?"),
	    QMessageBox::Ok|QMessageBox::Cancel);
    if (reply == QMessageBox::Ok)
    {
	CbmdosVfs_mapUpperGfxToLower(CbmdosFs_vfs(d->fs), true);
	emit modified();
    }
}

void V1541ImgWidget::newFile()
{
    if (!hasValidImage()) return;
    CbmdosFile *newFile = CbmdosFile_create();
    if (cmdr.autoMapToLc())
    {
	CbmdosFile_setAutoMapToLc(newFile, true);
    }
    const QModelIndex &index = d->dirList.selectionModel()->currentIndex();
    d->model.addFile(index, newFile);
}

void V1541ImgWidget::deleteFile(bool skipConfirmation)
{
    if (!hasValidImage() || !hasValidSelection()) return;
    if (skipConfirmation || QMessageBox::question(window(),
                tr("Delete this file?"),
                tr("A deleted file cannot be restored. "
	        "Are you sure you want to delete this file now?"),
	        QMessageBox::Ok|QMessageBox::Cancel) == QMessageBox::Ok)
    {
	const QModelIndex &index = d->dirList.selectionModel()->currentIndex();
	d->model.deleteAt(index);
    }
}

bool V1541ImgWidget::hasValidImage() const
{
    return d->fs;
}

bool V1541ImgWidget::hasValidSelection() const
{
    const QModelIndex &index = d->dirList.selectionModel()->currentIndex();
    if (!index.isValid()) return false;
    return (index.row() > 0 && index.row() < d->model.rowCount() - 1);
}
