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


//--- header files ------------------------------------------------------------

#include <anatomist/processor/Reader.h>
#include <anatomist/misc/error.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/Processor.h>
#include <graph/tree/treader.h>
#include <graph/tree/tree.h>
#include <map>
#include <assert.h>


using namespace anatomist;
using namespace carto;
using namespace std;

#ifdef ANA_THREADED_PIPEREADER

#include <cartobase/thread/thread.h>
#include <qevent.h>
#include <qapplication.h>

namespace
{
  class CommandExecutorEvent : public QCustomEvent
  {
  public:
    CommandExecutorEvent( Tree* cmd, MutexRcPtr<CommandContext> contxt ) 
      : QCustomEvent( QEvent::User + 1111 ), command( cmd ), context( contxt )
    {}
    virtual ~CommandExecutorEvent() {}

    Tree	*command;
    MutexRcPtr<CommandContext> context;
  };


}
  
  
anatomist::internal::CommandReader_Bridge::~CommandReader_Bridge()
{
}

anatomist::internal::CommandReader_Bridge* anatomist::internal::CommandReader_Bridge::_executor()
{
  static anatomist::internal::CommandReader_Bridge	*crb = new anatomist::internal::CommandReader_Bridge;
  return crb;
}

  bool anatomist::internal::CommandReader_Bridge::event( QEvent* e )
  {
    // cout << "CommandReader_Bridge::event\n" << flush;
    if( e->type() !=  QEvent::User + 1111 )
      return false;
    CommandExecutorEvent	*cee 
      = dynamic_cast<CommandExecutorEvent *>( e );
    if( !e )
      return false;

    cee->context.lock();
    // cout << "exec event locked\n" << flush;
    Tree	*ctype = cee->command, *com;
    Command	*c;

    if( cee->command->getSyntax() == "EXECUTE" )
      {
        if( ctype->size() != 1 )
          cerr << "invalid number of command: " << ctype->size() 
               << ", should be 1\n";
        else
          {
            com = (Tree *) *ctype->begin();
            // cout << "exec (event) " << com->getSyntax() << endl;
            c = Registry::instance()->create( *com, cee->context.get() );
            // THEN GIVE IT TO THE Processor FOR EXECUTION
            if( c )
              theProcessor->execute( c );
            // cout << "execution done\n" << flush;
          }
      }
    else
      cerr << "command mode " << ctype->getSyntax() << " not handled\n";
    cee->context.unlock();
    cee->context.reset( 0 ); // detach context
    delete ctype;
    return true;
  }

#endif


//--- methods -----------------------------------------------------------------

CommandReader::CommandReader( const string& filename, 
			      MutexRcPtr<CommandContext> & context )
  : _history( new ifstream( filename.c_str() ) ), _ownStream( true ), 
    _context( context ), _askedToClose( false ), _removePipe( false )
{
  if (!*_history)
    {
      AWarning("Unable to open history file. Won't execute...");
    }
}


CommandReader::CommandReader( MutexRcPtr<CommandContext> & context )
  : _history( 0 ), _ownStream( false ), 
    _context( context ), 
    _askedToClose( false ), _removePipe( false )
{
}


CommandReader::~CommandReader()
{
  //cout << "CommandReader::~CommandReader()\n";
  close();
}


void
CommandReader::read()
{
  // run finite state machine to read the stream
  while (!_history->eof())
    readOne();
}


void CommandReader::readOne()
{
  // cout << "CommandReader::readOne\n" << flush;

  assert( _history );

  // debug
  /*SyntaxSet	ss( Registry::instance()->syntax() );
  SyntaxSet::iterator	is, es = ss.end();
  for( is=ss.begin(); is!=es; ++is )
    cout << "syntax: " << is->first << endl; */

  TreeReader	tr( Registry::instance()->syntax() );
  tr.attach( *_history );

  Tree		*ctype = new Tree( true, "command" );
  Tree		*com;
  Command	*c;

  // cout << "read tree...\n" << flush;
  tr >> *ctype;
  // cout << "tree read OK\n" << flush;

  string	synt = ctype->getSyntax();

#ifdef ANA_THREADED_PIPEREADER
  if( ! Thread::currentIsMainThread() )
    {
      // cout << "not in main thread\n" << flush;
      // _context.lock(); // should not be needed: ctype is only local for now
      if( synt == "EXECUTE" && ctype->size() == 1 )
        {
          com = (Tree *) *ctype->begin();
          if( com->getSyntax() == "ClosePipe" )
            {
              _askedToClose = true;
              int	rmv = 0;
              com->getProperty( "remove_file", rmv );
              _removePipe = rmv;
            }
        }
      // cout << "post event\n" << flush;
      if( !_askedToClose )
        qApp->postEvent( anatomist::internal::CommandReader_Bridge::_executor(), 
                         new CommandExecutorEvent( ctype, _context ) );
      // _context.unlock();
    }
  else
#endif
    {
      if( synt == "EXECUTE" )
        {
          if( ctype->size() != 1 )
            cerr << "invalid number of command: " << ctype->size() 
                 << ", should be 1\n";
          else
            {
              com = (Tree *) *ctype->begin();
              // cout << "exec " << com->getSyntax() << endl;
              c = Registry::instance()->create( *com, _context.get() );
              // THEN GIVE IT TO THE Processor FOR EXECUTION
              if( c )
                theProcessor->execute( c );
              //	Test for closing command... (hack from Denis...)
              if( com->getSyntax() == "ClosePipe" )
                {
                  _askedToClose = true;
                  int	rmv = 0;
                  com->getProperty( "remove_file", rmv );
                  _removePipe = rmv;
                }
            }
        }
      else
        cerr << "command mode " << synt << " not handled\n";
    }
#ifndef ANA_THREADED_PIPEREADER
  delete ctype;
#endif
}


void CommandReader::attach( istream & s )
{
  close();
  _history = &s;
  _ownStream = false;
}


void CommandReader::close()
{
  if( _history )
    {
      if( _ownStream )
	delete _history;
      _history = 0;
    }
}
