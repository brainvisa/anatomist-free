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

#include <anatomist/sparsematrix/sparsematrix.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/application/settings.h>
#include <aims/sparsematrix/sparseMatrix.h>
#include <aims/io/reader.h>
#include <aims/io/writer.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


struct ASparseMatrix::Private
{
  rc_ptr<SparseMatrix> matrix;
};


namespace
{
  int sparsematrixType()
  {
    static int type = 0;
    if( type == 0 )
    {
      type = AObject::registerObjectType( "SparseMatrix" );
      QObjectTree::setObjectTypeName( type, string( "Sparse Matrix" ) );
      string str = Settings::findResourceFile( "icons/sparsemat.png" );
      QObjectTree::setObjectTypeIcon( type, str );
    }
    return type;
  }
}


ASparseMatrix::ASparseMatrix()
  : AObject(), AttributedAObject(), d( new Private )
{
  _type = sparsematrixType();
}


ASparseMatrix::~ASparseMatrix()
{
  delete d;
}


GenericObject* ASparseMatrix::attributed()
{
  if( d->matrix )
    return &d->matrix->header();
  return 0;
}


const GenericObject* ASparseMatrix::attributed() const
{
  if( d->matrix )
    return &d->matrix->header();
  return 0;
}


const rc_ptr<SparseMatrix> ASparseMatrix::matrix() const
{
  return d->matrix;
}


rc_ptr<SparseMatrix> ASparseMatrix::matrix()
{
  return d->matrix;
}


void ASparseMatrix::setMatrix( rc_ptr<SparseMatrix> matrix )
{
  d->matrix = matrix;
  setChanged();
}


bool ASparseMatrix::reload( const string & filename )
{
  try
  {
    Reader<SparseMatrix> r( filename );
    rc_ptr<SparseMatrix> mat( r.read() );
    setMatrix( mat );
  }
  catch( exception & e )
  {
    cerr << e.what() << endl;
    return false;
  }
  return true;
}


bool ASparseMatrix::save( const string & filename )
{
  if( !d->matrix )
    return false;
  Writer<SparseMatrix> w( filename );
  w.write( *d->matrix );
  return true;
}


AObject* ASparseMatrix::clone( bool shallow )
{
  ASparseMatrix *amat = new ASparseMatrix;
  if( shallow )
    amat->setMatrix( d->matrix );
  else if( d->matrix )
    amat->setMatrix( rc_ptr<SparseMatrix>( new SparseMatrix( *d->matrix ) ) );
  return amat;
}


