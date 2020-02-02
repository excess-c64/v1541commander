#ifndef V1541C_EDITOPERATIONCHECK_H
#define V1541C_EDITOPERATIONCHECK_H

struct CbmdosFile;

class EditOperationCheck
{
    private:
	class priv;
	priv *d;

    public:
	EditOperationCheck(const CbmdosFile *newFile,
		const CbmdosFile *oldFile = nullptr);
	~EditOperationCheck();

	const CbmdosFile *newFile() const;
	const CbmdosFile *oldFile() const;

	void setAllowed(bool allowed);
	bool allowed() const;
};

#endif
