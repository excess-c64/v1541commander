#include "editoperationcheck.h"

class EditOperationCheck::priv
{
    public:
	priv(const CbmdosFile *newFile, const CbmdosFile *oldFile);
	const CbmdosFile *newFile;
	const CbmdosFile *oldFile;
	bool allowed;
};

EditOperationCheck::priv::priv(const CbmdosFile *newFile,
	const CbmdosFile *oldFile) :
    newFile(newFile),
    oldFile(oldFile),
    allowed(false)
{}

EditOperationCheck::EditOperationCheck(const CbmdosFile *newFile,
	const CbmdosFile *oldFile)
{
    d = new priv(newFile, oldFile);
}

EditOperationCheck::~EditOperationCheck()
{
    delete d;
}

const CbmdosFile *EditOperationCheck::newFile() const
{
    return d->newFile;
}

const CbmdosFile *EditOperationCheck::oldFile() const
{
    return d->oldFile;
}

void EditOperationCheck::setAllowed(bool allowed)
{
    d->allowed = allowed;
}

bool EditOperationCheck::allowed() const
{
    return d->allowed;
}

