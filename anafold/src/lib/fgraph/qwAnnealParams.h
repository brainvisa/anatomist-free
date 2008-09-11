/*
 *  Copyright (C) 1998-2003 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#ifndef ANAFOLD_FGRAPH_QWANNEALPARAMS_H
#define ANAFOLD_FGRAPH_QWANNEALPARAMS_H


#include <anatomist/observer/Observer.h>
#include <qwidget.h>

namespace sigraph
{
  class AnnealConfigurator;
}

namespace anatomist
{
  class AFGraph;
}


///	Control window for annealing parameters
class QAnnealParams : public QWidget, public anatomist::Observer
{
  Q_OBJECT

public:
  enum State
    {
      STOPPED, RUNNING, PAUSED
    };

  QAnnealParams( QWidget* parent, const char* name, 
		 anatomist::AFGraph* fusion );
  virtual ~QAnnealParams();
  virtual void update( const anatomist::Observable* obs, void* arg = 0 );

signals:

public slots:
  void loadConfig();
  void saveConfig();
  void start();
  void stop();
  void selectTranslationFile();
  void selectPlotFile();


protected slots:
  void annealStepFinished();
  void setThreaded( bool );

protected:
  void annealThread();
  void updateInterface();
  void updateConfig();
  void updateBoxes();

  sigraph::AnnealConfigurator	*_conf;
  anatomist::AFGraph		*_fusion;

private:
  struct PrivateData;
  class AnnealPThread;
  friend class AnnealPThread;
  PrivateData	*pdat;
};


#endif

