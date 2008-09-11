class QObjectTree : QWidget
{
%TypeHeaderCode
#include <anatomist/control/qObjTree.h>
#include <qwidget.h>
#include <qpixmap.h>
%End

public:

	QObjectTree(QWidget *, const char *);
	virtual ~QObjectTree();

	static void setObjectTypeName(int, std::string &);
	static void setObjectTypeIcon(int, std::string &);
};
