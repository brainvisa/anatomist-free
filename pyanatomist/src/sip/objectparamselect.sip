
class ObjectParamSelect : QHBox
{
%TypeHeaderCode
#include <anatomist/object/objectparamselect.h>
%End

public:
  // typedef bool (*Filter)( const anatomist::AObject* );
  enum ViewMode
  {
    Initial,
    All,
    TopLevel
  };

  ObjectParamSelect( const set_AObjectPtr &, QWidget* parent /TransferThis/ );
  virtual ~ObjectParamSelect();

  // virtual void addFilter( Filter );
  virtual bool filter( const anatomist::AObject* o ) const;
  void updateLabel( const set_AObjectPtr & );
  const set_AObjectPtr & selectedObjects() const;

signals:
  void selectionStarts();
  void objectsSelected( const set_AObjectPtr & );

public slots:
  virtual void selectObjects();
  virtual void selectObjects( const set_AObjectPtr & obj,
                              const set_AObjectPtr & sel );

protected slots:
  void setObjectsViewMode( int );
};


