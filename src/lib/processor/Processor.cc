/* Copyright (c) 1995-2005 CEA
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


//--- header files ------------------------------------------------------------

#include <anatomist/processor/Processor.h>
#include <anatomist/processor/Command.h>
#include <anatomist/processor/errormessage.h>
#include <assert.h>
#include <qapplication.h>
#include <anatomist/processor/context.h>
#include <anatomist/processor/Registry.h>
#include <graph/tree/tree.h>
#include <cartobase/object/pythonreader.h>
#include <cartobase/stream/sstream.h>

#include <typeinfo>

using namespace anatomist;
using namespace carto;
using namespace std;

//--- global variables --------------------------------------------------------

Processor* anatomist::theProcessor = 0;


//--- methods -----------------------------------------------------------------

Processor::Processor() : _idle(true)
{
	assert(theProcessor == 0);
	theProcessor = this;
}


Processor::~Processor()
{
	while (_done.size() > 0)
	{
		delete _done.top();
		_done.pop();
	}

	while (_undone.size() > 0)
	{
		delete _undone.top();
		_undone.pop();
	}

	theProcessor = 0;
}


void
Processor::execute(Command* c)
{
	_todo.push(c);
	if (_idle)
	{
		while (!_todo.empty())
		{
			_idle = false;
			Command* q = _todo.front();
			_todo.pop();
                        try
                          {
                            q->execute();
                          }
                        catch( exception & e )
                          {
                            ErrorMessage::message
                              ( string( qApp->translate
                                        ( "ErrorMessage", 
                                          e.what() ).utf8().data() ), 
                                ErrorMessage::Error );
                          }
			this->setChanged();
			Memo memo(Memo::EXECUTE, q);
			this->notifyObservers(&memo);
			_done.push(q);
			_idle = true;
		}
	}
}


Command *Processor::execute( const string & cname, const string & params, 
                             CommandContext * cc )
{
  /*
  std::cout << "execute: " << cname << std::endl;
  std::cout << "args: " << params << std::endl;
  std::cout << "context: " << cc << std::endl;
  */

  Tree	t( true, cname );
  if( !params.empty() )
    {
      istringstream	ss( params.c_str() );
      PythonReader pr( Registry::instance()->syntax() );
      pr.attach( ss );
      pr.readDictionary( t );
      /* cout << "props: " << t.carto::AttributedObject::size() << std::endl;
      Object to, ido;
      ido = t.getProperty( "res_pointer" );
      to = t.getProperty( "type" );
      std::cout << "prop res_pointer:" << t.hasProperty( "res_pointer" ) 
                << " :  " << ido->type() << std::endl;
      std::cout << "prop type:" << t.hasProperty( "type" ) 
                << " :  " << to->type() << std::endl;
      std::string	type;
      int		id;
      std::cout << "type: " << t.getProperty( "type", type ) << " : " << type << std::endl;
      std::cout << "res_pointer: " << t.getProperty( "res_pointer", type ) << " : " << id << std::endl;
      std::cout << to->getString() << std::endl;
      std::cout << ido->getScalar() << std::endl;
      std::cout << typeid( *to ).name() << std::endl;
      std::cout << typeid( *ido ).name() << std::endl;
      std::cout << dynamic_cast<carto::TypedObject<std::string> *>( to.get() ) << std::endl;
      std::cout << dynamic_cast<carto::TypedObject<int> *>( ido.get() ) << std::endl;
      std::cout << dynamic_cast<carto::ValueObject<std::string> *>( to.get() ) << std::endl;
      std::cout << dynamic_cast<carto::ValueObject<int> *>( ido.get() ) << std::endl;
      std::cout << std::endl;
      std::cout << SUPERSUBCLASS( carto::TypedObject<int>, carto::ValueObject<int> ) << std::endl;
      std::cout << SUPERSUBCLASS( carto::GenericObject, carto::ValueObject<int> ) << std::endl;
      std::cout << SUPERSUBCLASS( anatomist::Command, anatomist::RegularCommand ) << std::endl;
      carto::ValueObject<int> toto;
      carto::GenericObject *toto2 = &toto;
      std::cout << dynamic_cast<carto::TypedObject<int> *>( &toto ) << std::endl;
      std::cout << dynamic_cast<carto::TypedObject<int> *>( toto2 ) << std::endl;
      */
    }
  //cout << "synt: " << t.getSyntax() << std::endl;

  if( !cc )
    cc = &CommandContext::defaultContext();
  Command	*c = Registry::instance()->create( t, cc );
  //cout << "command: " << c << endl;
  if( c )
    execute( c );
  return c;
}


bool
Processor::undo()
{
	if (_done.empty())
	{
		return false;
	}
	Command* c = _done.top();
	_done.pop();
	_undone.push(c);
	c->undo();
	this->setChanged();
	Memo memo(Memo::UNDO, 0);
	this->notifyObservers(&memo);
	return true;
}


bool
Processor::redo()
{
	if (_undone.empty())
	{
		return false;
	}
	Command* c = _undone.top();
	_undone.pop();
	_done.push(c);
	c->redo();
	this->setChanged();
	Memo memo(Memo::REDO, 0);
	this->notifyObservers(&memo);
	return true;
}
