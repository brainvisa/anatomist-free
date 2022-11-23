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

#include <anatomist/commands/cSaveTransformationGraph.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/transfSet.h>
#include <anatomist/application/Anatomist.h>
#include <aims/io/writer.h>
#include <aims/data/pheader.h>
#include <aims/transformation/transformationgraph3d.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


SaveTransformationGraphCommand::SaveTransformationGraphCommand(
  const string & filename )
  : RegularCommand(), _filename( filename )
{
}


SaveTransformationGraphCommand::~SaveTransformationGraphCommand()
{
}


bool SaveTransformationGraphCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "SaveTransformationGraph" ];

  s[ "filename"    ] = Semantic( "string", false );

  Registry::instance()->add( "SaveTransformationGraph", &read, ss );
  return true;
}


void SaveTransformationGraphCommand::doit()
{
  cout << "SaveTransformationGraph\n";

  TransformationGraph3d graph;
  ATransformSet *tset = ATransformSet::instance();

  const set<Transformation *> & trans = tset->allTransformations();
  set<Transformation *>::const_iterator it, et = trans.end();

  for( it=trans.begin(); it!=et; ++it )
  {
    const Transformation *tr = *it;
    if( tr->isGenerated() )
      continue;
    const Referential *src = tr->source();
    const Referential *dst = tr->destination();
    string suuid = src->uuid().toString();
    string duuid = dst->uuid().toString();
    rc_ptr<Transformation3d> t( new AffineTransformation3d( tr->motion() ) );
    graph.registerTransformation( suuid, duuid, t );
  }

  if( !_filename.empty() )
  {
    aims::Writer<TransformationGraph3d> w( _filename );
    Object options = Object::value( Dictionary() );
    options->setProperty( "embed_affines", true );

    w.setOptions( options );
    w.write( graph );
  }
}


Command* SaveTransformationGraphCommand::read( const Tree & com,
                                               CommandContext* context )
{
  string filename;

  if( !com.getProperty( "filename", filename ) )
  {
    cerr << "SaveTransformationGraphCommand: no filename specified\n";
    return 0;
  }

  if( !filename.empty() )
    return new SaveTransformationGraphCommand( filename );

  return 0;
}


void SaveTransformationGraphCommand::write( Tree & com, Serializer* ser ) const
{
  Tree *t = new Tree( true, name() );

  t->setProperty( "filename", _filename );

  com.insert( t );
}

