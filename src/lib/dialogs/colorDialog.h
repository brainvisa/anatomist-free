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


#ifndef ANAQT_DIALOGS_COLORDIALOG_H
#define ANAQT_DIALOGS_COLORDIALOG_H


#include <qdialog.h>
#include <qcolor.h>


class QAColorWidget;


class QAColorDialog : public QDialog
{
  Q_OBJECT

public:
  QAColorDialog( QColor, QWidget * parent = 0, const char * name = 0, 
		 bool modal = true, bool allowAlpha = false, 
		 bool allowNeutralAlpha = false, int initalpha = 255, 
		 bool neutralph = false );
  virtual ~QAColorDialog();

  static QColor getColor( QColor, QWidget * parent = 0, 
			  const char * name = 0, int* alpha = 0, 
			  bool *neutraph = 0 );
  void relook( const QColor & col, int initalpha = 255, 
	       bool allowalpha = false, bool neutralpha = false, 
	       bool allownalph = false );

  QColor	color() const;
  int alpha() const;
  bool neutralAlpha() const;

signals:
  void colorChanged();

protected:
  static QAColorDialog	*_dialog;

  QAColorWidget		*_widget;

private:
};



#endif
