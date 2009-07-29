/* Copyright (c) 2007 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 * This software is governed by the CeCILL license version 2 under 
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the 
 * terms of the CeCILL license version 2 as circulated by CEA, CNRS
 * and INRIA at the following URL "http://www.cecill.info". 
 * 
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability. 
 * 
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or 
 * data to be ensured and,  more generally, to use and operate it in the 
 * same conditions as regards security. 
 * 
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license version 2 and that you accept its terms.
 */

/****************************************************************************
** Form interface generated from reading ui file 'volrender.ui'
**
** Created by: The User Interface Compiler ($Id: qt/main.cpp   3.3.8   edited Jan 11 14:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef ANATOMIST_OBJECT_VOLRENDERPANEL_H
#define ANATOMIST_OBJECT_VOLRENDERPANEL_H

#include <anatomist/observer/Observer.h>
#include <anatomist/object/Object.h>
#include <aims/qtcompat/qgroupbox.h>
#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QLabel;

class VolRenderPanel : public QWidget, public anatomist::Observer
{
  Q_OBJECT

  public:
    VolRenderPanel( const std::set<anatomist::AObject *> &,
                    QWidget* parent = 0, const char* name = 0,
                    Qt::WFlags fl = Qt::WDestructiveClose );
    ~VolRenderPanel();

    const std::set<anatomist::AObject *> & objects() const;
    virtual void update( const anatomist::Observable* observable, void* arg );
    virtual void unregisterObservable( anatomist::Observable* );
    void updateWindow();
    void updateObjects();

    Q3GroupBox* groupBox1;
    QComboBox* renderingMode;
    QCheckBox* limitSlices;
    QSpinBox* maxSlices;
    QLabel* textLabel1;
    QSpinBox* slabSize;
    QLabel* textLabel2;

  public slots:
    virtual void renderModechanged( const QString & );
    virtual void setSliceLimitEnabled( bool );
    virtual void maxSlicesChanged( int );
    virtual void slabSizeChanged( int );

  protected:
    QVBoxLayout* VolRenderPanelLayout;
    QVBoxLayout* groupBox1Layout;
    QHBoxLayout* layout2;
    QHBoxLayout* layout3;

  protected slots:
    virtual void languageChange();

  private:
    struct Private;
    Private *d;
};

#endif


