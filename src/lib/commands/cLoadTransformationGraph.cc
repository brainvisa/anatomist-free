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

#include <anatomist/commands/cLoadTransformationGraph.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/application/Anatomist.h>
#include <aims/io/reader.h>
#include <aims/data/pheader.h>
#include <aims/transformation/transformationgraph3d.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


LoadTransformationGraphCommand::LoadTransformationGraphCommand(
  const string & filename )
  : RegularCommand(), _filename( filename )
{
}


LoadTransformationGraphCommand::LoadTransformationGraphCommand(
  const Object & desc )
  : RegularCommand(), _description( desc )
{
}


LoadTransformationGraphCommand::~LoadTransformationGraphCommand()
{
}


bool LoadTransformationGraphCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "LoadTransformationGraph" ];

  s[ "filename"    ] = Semantic( "string", false );

  Registry::instance()->add( "LoadTransformationGraph", &read, ss );
  return( true );
}


void LoadTransformationGraphCommand::doit()
{
  cout << "LoadTransformationGraph\n";

  rc_ptr<TransformationGraph3d> graph;

  if( !_filename.empty() )
  {
    aims::Reader<TransformationGraph3d> r( _filename );
    graph.reset( r.read() );
  }
  else if( _description )
  {
    graph.reset( new TransformationGraph3d );
    string gdir;
    try
    {
      gdir = _description->getProperty( "directory" )->getString();
    }
    catch( ... )
    {
    }

    graph->loadTransformationsGraph( _description, gdir );
  }
//   cout << "graph: " << graph->order() << " refs, " << graph->edgesSize()
//        << " trans\n";

  // load only affine, and convert them
  graph->loadAffineTransformations();

  // referentials
  Graph::iterator iv, ev = graph->end();
  const set<Referential *> & refs = theAnatomist->getReferentials();
  set<Referential *>::const_iterator ir, er = refs.end();
  list<string> ref_toadd;
  bool found;
  map<string, Referential *> ref_ids;
  string rid, rid2;

  for( iv=graph->begin(); iv!=ev; ++iv )
  {
    rid = graph->referential( *iv );
    found = false;
    for( ir=refs.begin(); ir!=er; ++ir )
    {
      if( (*ir)->uuid() == rid )
        found = true;
      else
        try
        {
          rid2 = (*ir)->header().getProperty( "name" )->getString();
          if( rid == rid2 )
            found = true;
        }
        catch( ... )
        {
        }
      if( found )
      {
        ref_ids[ rid ] = *ir;
        break;
      }
    }
    if( !found )
      ref_toadd.push_back( rid );
  }
  list<string>::const_iterator ira, era = ref_toadd.end();
  for( ira=ref_toadd.begin(); ira!=era; ++ira )
  {
    rid = *ira;
    Referential *ref = new Referential;
    if( rid.length() == 36 && rid[8] == '-' && rid[13] == '-' && rid[18] == '-'
        && rid[23] == '-' )
      ref->header().setProperty( "uuid", rid );
    else
      ref->header().setProperty( "name", rid );
    ref_ids[ rid ] = ref;
  }

  // affine transformations
  Graph::ESet::iterator ie, ee = graph->edges().end();
  set<pair<Referential *, Referential *> > done;

  for( ie=graph->edges().begin(); ie!=ee; ++ie )
  {
    Edge *e = *ie;
    if( e->hasProperty( "deduced" )
        && bool( e->getProperty( "deduced" )->getScalar() ) )
      continue;  // ignore deduced transforms
    rc_ptr<Transformation3d> tr = graph->transformation( e );
    if( !tr )
      continue;
    AffineTransformation3d *taff
      = dynamic_cast<AffineTransformation3d *>( tr.get() );
    if( !taff )
    {
      // cout << "not an affine transform\n";
      continue;
    }
    string source = graph->referential( *e->begin() );
    string dest = graph->referential( *e->rbegin() );
    // cout << "source: " << source << "\ndest: " << dest << endl;
    Referential *r1 = ref_ids[ source ];
    Referential *r2 = ref_ids[ dest ];
    if( done.find( make_pair( r2, r1 ) ) != done.end() )
    {
      // cout << "its inverse is already registered.\n";
      continue;
    }
    Transformation *t = new Transformation( r1, r2 );
    t->motion() = *taff;
    t->registerTrans();
    // cout << "register: (" << r1 << ", " << r2 << ")\n";
    done.insert( make_pair( r1, r2 ) );
  }
}


Command* LoadTransformationGraphCommand::read( const Tree & com,
                                               CommandContext* context )
{
  string filename;

  if( !com.getProperty( "filename", filename ) )
  {
    cerr << "LoadTransformationGraphCommand: no filename specified\n";
    return 0;
  }

  if( !filename.empty() )
    return new LoadTransformationGraphCommand( filename );

  return 0;
}


void LoadTransformationGraphCommand::write( Tree & com, Serializer* ser ) const
{
  Tree *t = new Tree( true, name() );

  t->setProperty( "filename", _filename );

  com.insert( t );
}

