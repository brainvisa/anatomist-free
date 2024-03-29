/* This software and supporting documentation are distributed by
 *     Institut Federatif de Recherche 49
 *     CEA/NeuroSpin, Batiment 145,
 *     91191 Gif-sur-Yvette cedex
 *     France
 *
 * This software is governed by the CeCILL-B license under
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the
 * terms of the CeCILL-B license as circulated by CEA, CNRS
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
 * knowledge of the CeCILL-B license and that you accept its terms.
 */


#ifndef ANAQT_BROWSER_STRINGEDIT_H
#define ANAQT_BROWSER_STRINGEDIT_H

#include <anatomist/application/Anatomist.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <string>



///	A "cancelable" line edit: hitting 'esc' sends the "cancel()" signal
class QCancelLineEdit : public QLineEdit
{
  Q_OBJECT

public:
  QCancelLineEdit( QWidget* parent = 0, const char* name = 0 ) 
    : QLineEdit( parent ) { setObjectName(name); }
  QCancelLineEdit( const QString & txt, QWidget* parent = 0, 
                   const char* name = 0 ) 
    : QLineEdit( txt, parent ) { setObjectName(name); }

signals:
  void cancel();

protected:
  void keyPressEvent( QKeyEvent* kev );
};


///	Qt string attribute editor
class QStringEdit : public QDialog
{
  Q_OBJECT

public:
  QStringEdit( const std::string & text, int x, int y, int w, int h, 
               QWidget* parent = theAnatomist->getQWidgetAncestor(), 
               const char* name = 0, Qt::WindowFlags f = Qt::WindowFlags() );
  ~QStringEdit();

  std::string text() const
    { return( _le->text().toStdString() ); }
  QLineEdit	*lineEdit() { return( _le ); }

protected:
  QCancelLineEdit	*_le;

private:
};


#endif
