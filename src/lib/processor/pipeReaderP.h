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


#ifndef ANATOMIST_PROCESSOR_PIPEREADERP_H
#define ANATOMIST_PROCESSOR_PIPEREADERP_H


#include <qobject.h>

//#ifdef _WIN32
#if !defined( ANA_THREADED_PIPEREADER ) && !defined( CARTO_NO_THREAD )
#define ANA_THREADED_PIPEREADER
#endif
//#endif

namespace anatomist
{
  class APipeReader;
}


class APipeReader_Bridge : public QObject
{
  Q_OBJECT

public:
  APipeReader_Bridge( anatomist::APipeReader* pr )
    : QObject(), preader( pr ) {}
  virtual ~APipeReader_Bridge();

public slots:
  void readSocket( int );

private:
  anatomist::APipeReader	*preader;
};

#ifdef ANA_THREADED_PIPEREADER

namespace anatomist{
namespace internal{
class CommandReader_Bridge : public QObject
{
public:
  CommandReader_Bridge() : QObject() {}
  virtual ~CommandReader_Bridge();

  virtual bool event( QEvent* e );
  static CommandReader_Bridge* _executor();
};

}}
#endif

#endif
