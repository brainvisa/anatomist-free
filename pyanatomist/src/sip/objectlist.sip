namespace anatomist
{

class ObjectList : anatomist::MObject
{
%TypeHeaderCode
#include <anatomist/object/Object.h>
#include <anatomist/mobject/ObjectList.h>
#include <pyaims/vector/vector.h>
%End

%ConvertToSubClassCode
	if (dynamic_cast<anatomist::ObjectList *>(sipCpp))
		sipClass = sipClass_anatomist_ObjectList;
	else
		sipClass = 0;
%End

public:
	ObjectList ();
	virtual ~ObjectList ();
	virtual size_t size() const;
	virtual anatomist::AIterator begin();
	virtual anatomist::AIterator end();
	virtual void insert (anatomist::AObject * /Transfer/);
};

};

