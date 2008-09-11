
#ifndef ANAFOLD_FGRAPH_QWFFOLDCTRL_H
#define ANAFOLD_FGRAPH_QWFFOLDCTRL_H


#include <anatomist/observer/Observer.h>
#include <aims/qtcompat/qbuttongroup.h>

class QCheckBox;
class QPushButton;
class QLineEdit;

namespace anatomist
{
  class AFGraph;
}

///	Control window for fold graph fusions
class QFFoldCtrl : public QWidget, public anatomist::Observer
{
  Q_OBJECT

public:
  QFFoldCtrl( QWidget* parent, const char* name, anatomist::AFGraph* fusion );
  virtual ~QFFoldCtrl();

  virtual void update( const anatomist::Observable* observable, 
		       void* arg = 0 );

signals:

public slots:
  void btnClick( int btn );
  void relPotBtnClicked();
  void weightButtonClicked();
  void midBtnClicked();
  void pot0BtnClicked();
  void setNoPotColor();
  void setPot0Color();
  void updateModelWeights();

protected:
  anatomist::AFGraph	*_fusion;
  Q3ButtonGroup		*_groupBox;
  QCheckBox		*_rpBtn;
  QCheckBox		*_wtBtn;
  QCheckBox		*_midBtn;
  QCheckBox		*_p0Btn;
  QPushButton		*_p0col;
  QPushButton		*_nopotCol;
  QLineEdit		*_modWeightEdit;
  bool			_updating;

private:
};


#endif

