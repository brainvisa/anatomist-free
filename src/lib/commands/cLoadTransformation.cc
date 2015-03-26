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

#include <anatomist/commands/cLoadTransformation.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/context.h>
#include <aims/io/reader.h>
#include <aims/data/pheader.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


LoadTransformationCommand::LoadTransformationCommand( const string & filename, 
						      Referential* org, 
						      Referential* dst, 
						      int id, 
						      CommandContext* context )
  : RegularCommand(), SerializingCommand( context ), _org( org ), _dst( dst ), 
    _id( id ), _filename( filename )
{
}


LoadTransformationCommand::LoadTransformationCommand( const float 
						      matrix[4][3], 
						      Referential* org, 
						      Referential* dst, 
						      int id, 
						      CommandContext* context )
  : RegularCommand(), SerializingCommand( context ), _org( org ), _dst( dst ), 
    _id( id )
{
  unsigned	i, j;
  for( i=0; i<4; ++i )
    for( j=0; j<3; ++j )
      _matrix[i][j] = matrix[i][j];
}


LoadTransformationCommand::LoadTransformationCommand( const vector<float> &
    matrix,
    Referential* org,
    Referential* dst,
    int id,
    CommandContext* context )
  : RegularCommand(), SerializingCommand( context ), _org( org ), _dst( dst ),
                   _id( id )
{
  unsigned	i, j, k = 0, n = matrix.size();
  for( i=0; i<4; ++i )
    for( j=0; j<3; ++j )
    {
      if( k >= n )
        _matrix[i][j] = 0;
      else
        _matrix[i][j] = matrix[k++];
    }
}


LoadTransformationCommand::~LoadTransformationCommand()
{
}


bool LoadTransformationCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "LoadTransformation" ];

  s[ "origin"      ] = Semantic( "int", false );
  s[ "destination" ] = Semantic( "int", false );
  s[ "filename"    ] = Semantic( "string", false );
  s[ "matrix"      ] = Semantic( "float_vector", false );
  s[ "res_pointer" ] = Semantic( "int", true );

  Registry::instance()->add( "LoadTransformation", &read, ss );
  return( true );
}


void LoadTransformationCommand::doit()
{
  // cout << "LoadTransformation\n";

  Motion m;
  Referential *created1 = 0, *created2 = 0;

  if( !_filename.empty() )
  {
    aims::Reader<Motion> rd( _filename );
    try
    {
      rd.read( m );
    }
    catch( exception & e )
    {
      cerr << e.what() << endl;
      return;
    }
    string usrc, udst;
    PythonHeader  *ph = m.header();
    if( ph )
    {
      ph->getProperty( "source_referential", usrc );
      ph->getProperty( "destination_referential", udst );
    }
    // cout << "source_referential: " << usrc << endl;
    // cout << "destination_referential: " << udst << endl;
    if( usrc.empty() && !_org )
    {
      cerr << "transformation with no origin and none specified" << endl;
      return;
    }
    if( udst.empty() && !_dst )
    {
      cerr << "transformation with no origin and none specified" << endl;
      return;
    }

    Referential *r;

    if( !usrc.empty() )
    {
      r = Referential::referentialOfUUID( usrc );
      if( r )
      {
        if( _org && _org != r )
        {
          cerr << "origin and source UUID mismatch." << endl;
          if( _org->uuid().isNull() )
          {
            cerr << "setting UUID to existing referential" << endl;
            _org->header().setProperty( "uuid", usrc );
          }
          else
          {
            cout << "overriding transformation source referential" << endl;
            ph->setProperty( "source_referential", _org->uuid().toString() );
          }
        }
        else if( !_org )
          _org = r;
      }
      else // no ref of UUID rsrc
      {
        cerr << "Referential of UUID " << usrc << " not found." << endl;
        if( _org )
        {
          if( _org->uuid().isNull() )
          {
            cout << "setting UUID to existing referential" << endl;
            _org->header().setProperty( "uuid", usrc );
          }
          else
          {
            cout << "overriding transformation source referential" << endl;
            ph->setProperty( "source_referential", _org->uuid().toString() );
          }
        }
        else
        {
          cout << "creating new source referential" << endl;
          _org = new Referential;
          _org->header().setProperty( "uuid", usrc );
          created1 = _org;
        }
      }
    }

    if( !udst.empty() )
    {
      r = Referential::referentialOfUUID( udst );
      if( r )
      {
        if( _dst && _dst != r )
        {
          cerr << "destination and destination UUID mismatch." << endl;
          if( _dst->uuid().isNull() )
          {
            cerr << "setting UUID to existing referential" << endl;
            _dst->header().setProperty( "uuid", udst );
          }
          else
          {
            cout << "overriding transformation destination referential"
                 << endl;
            ph->setProperty( "destination_referential",
                            _dst->uuid().toString() );
          }
        }
        else if( !_dst )
          _dst = r;
      }
      else // no ref of UUID rdst
      {
        cerr << "Referential of UUID " << udst << " not found." << endl;
        if( _dst )
        {
          if( _dst->uuid().isNull() )
          {
            cout << "setting UUID to existing referential" << endl;
            _dst->header().setProperty( "uuid", udst );
          }
          else
          {
            cout << "overriding transformation destination referential"
                << endl;
            ph->setProperty( "destination_referential",
                            _dst->uuid().toString() );
          }
        }
        else
        {
          cout << "creating new destination referential" << endl;
          _dst = new Referential;
          _dst->header().setProperty( "uuid", udst );
          created2 = _dst;
        }
      }
    }
  }

  if( !_org || !_dst )
  {
    cerr << "LoadTransformation: missing origin / destination information."
          << endl;
    delete created1;
    delete created2;
    return;
  }

  _tra = theAnatomist->getTransformation( _org, _dst );
  if( _tra )
    {
      cout << "Load transformation : overriding existing one\n";
      if( _tra->isGenerated() )
        {
          cerr << "cannot override an implicit transformation. Check for " 
               << "other paths in the transformation graph." << endl;
          return;
        }
      _tra->unregisterTrans();
    }
  else
    _tra = new Transformation( _org, _dst );

  if( !_filename.empty() )	// rotation file
    {
      _tra->motion() = m;
    }
  else	// hard-coded matrix
    {
      cout << "set matrix\n";
      _tra->setMatrixT( _matrix );
      _tra->setGenerated( false );
    }
  _tra->registerTrans();

  //	Create inverse transformation
  Transformation* inv = theAnatomist->getTransformation( _dst, _org );
  if( inv )
    {
      //cout << "LoadTransformation : inverse trans overriding existing one\n";
      *inv = *_tra;
    }
  else
    inv = new Transformation( _dst, _org, *_tra );
  inv->invert();
  inv->setGenerated( true );

  if( context() && context()->unserial && _id >= 0 )
    context()->unserial->registerPointer( _tra, _id, "Transformation" );
  // we should maybe also register a pointer for the inverse transformation ?

  // refresh is now automatic (TransformObserver system)
  /* cout << "transf " << _tra << ": translation: " << _tra->Translation(0) 
       << ", " << _tra->Translation(1) << ", " << _tra->Translation(2) << endl;
  */
}


Command* LoadTransformationCommand::read( const Tree & com, 
					  CommandContext* context )
{
  int			id;
  void			*ptr;
  int			rid;
  Referential		*org = 0, *dst = 0;
  vector<float>		matvec;
  string		filename;

  if( com.getProperty( "origin", id ) )
  {
    ptr = context->unserial->pointer( id, "Referential" );
    if( !ptr )
      {
        cerr << "Referential id " << id << " not found\n";
        return 0;
      }
    org = (Referential *) ptr;
  }

  if( com.getProperty( "destination", id ) )
  {
    ptr = context->unserial->pointer( id, "Referential" );
    if( !ptr )
      {
        cerr << "Referential id " << id << " not found\n";
        return 0;
      }
    dst = (Referential *) ptr;
  }

  com.getProperty( "res_pointer", rid );
  if( !com.getProperty( "filename", filename ) 
      && !com.getProperty( "matrix", matvec ) )
    {
      cerr << "LoadTransformationCommand: no filename nor matrix specified\n";
      return( 0 );
    }

  if( !filename.empty() )
    return new LoadTransformationCommand( filename, org, dst, rid, context );
  else
    {
      if( matvec.size() != 12 )
	{
	  cerr << "Wrong transformation matrix size (" << matvec.size() 
	       << ", should be 12)\n";
	  return 0;
	}

      return new LoadTransformationCommand( matvec, org, dst, rid, context );
    }
}


void LoadTransformationCommand::write( Tree & com, Serializer* ser ) const
{
  Tree		*t = new Tree( true, name() );

  t->setProperty( "origin", ser->serialize( _org ) );
  t->setProperty( "destination", ser->serialize( _dst ) );
  t->setProperty( "res_pointer", ser->serialize( _tra ) );
  if( !_filename.empty() )
    t->setProperty( "filename", _filename );
  else
    {
      vector<float>	matr;
      unsigned		i, j;

      for( i=0; i<4; ++i )
	for( j=0; j<3; ++j )
	  matr.push_back( _matrix[i][j] );
      t->setProperty( "matrix", matr );
    }

  com.insert( t );
}

