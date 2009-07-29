/* Copyright (c) 2009 CEA
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

#include <anatomist/commands/cDeleteAll.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/window/Window.h>
#include <anatomist/reference/Referential.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <qwidget.h>

using namespace anatomist;
using namespace carto;
using namespace std;

//-----------------------------------------------------------------------------

DeleteAllCommand::DeleteAllCommand()
  : RegularCommand()
{
}


DeleteAllCommand::~DeleteAllCommand()
{
}


bool DeleteAllCommand::initSyntax()
{
  SyntaxSet     ss;
  ss[ "DeleteAll" ];
  Registry::instance()->add( "DeleteAll", &read, ss );
  return true;
}


void
DeleteAllCommand::doit()
{
  set<AWindow *> wins = theAnatomist->getWindows();
  set<AWindow *>::iterator iw, ew = wins.end();
  set<QWidget *> wids;
  for( iw=wins.begin(); iw!=ew; ++iw )
  {
    QWidget *qw = dynamic_cast<QWidget *>( *iw );
    if( qw && qw->parent() != 0 )
    {
      QWidget *qw2 = dynamic_cast<QWidget *>( qw->parent() );
      if( qw2 )
        wids.insert( qw2 );
    }
    (*iw)->tryDelete();
  }
  set<QWidget *>::iterator iqw, eqw=wids.end();
  for( iqw=wids.begin(); iqw!=eqw; ++iqw )
    delete *iqw;

  set<AObject *> objs;
  set<AObject *>::iterator io, eo=objs.end();
  size_t cnt;
  do
  {
    cnt = 0;
    objs = theAnatomist->getObjects();
    for( io=objs.begin(); io!=eo; ++io )
    {
      if( (*io)->CanBeDestroyed() )
        cnt += theAnatomist->destroyObject( *io );
    }
  }
  while( !objs.empty() && cnt != 0 );

  set<Referential *> refs = theAnatomist->getReferentials();
  set<Referential *>::iterator ir, er = refs.end();
  for( ir=refs.begin(); ir!=er; ++ir )
  {
    if( *ir!=theAnatomist->centralReferential() )
      delete *ir;
  }

  theAnatomist->UpdateInterface();
  theAnatomist->Refresh();
}


Command * DeleteAllCommand::read( const Tree &, CommandContext* )
{
  return new DeleteAllCommand;
}


void DeleteAllCommand::write( Tree & com, Serializer* ) const
{
  Tree  *t = new Tree( true, name() );
  com.insert( t );
}
