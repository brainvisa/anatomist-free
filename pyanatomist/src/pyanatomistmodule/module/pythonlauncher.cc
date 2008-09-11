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

extern "C"
{
#include <Python.h>
}
#include <pyanatomist/module/pythonlauncher.h>
#include <anatomist/application/settings.h>
#include <aims/def/path.h>
#include <cartobase/stream/fileutil.h>
#include <iostream>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

PythonLauncher::PythonLauncher() : QObject()
{
}


PythonLauncher::~PythonLauncher()
{
}


void PythonLauncher::runModules()
{
  // cout << "PythonLauncher::runModules()\n";

  char		sep = FileUtil::separator();
  string	shared1 = Path::singleton().shfjShared() + sep 
    + "python";
  string	shared2 = Settings::globalPath() + sep + "python_plugins";

  if( Py_IsInitialized() )
    {
      /* cout << "Python is already running." << endl;
      cout << "I don't run anatomist python plugins to avoid conflicts" 
           << endl; */
      return;
    }
  else
    Py_Initialize();

  /* cout << "pythonpath 1: " << shared1 << endl;
     cout << "pythonpath 2: " << shared2 << endl; */

  PyRun_SimpleString( "import sys; sys.argv = [ 'anatomist' ]" );

  PyRun_SimpleString( (char *) ( string( "sys.path.insert( 0, '" ) + shared1 
                                 + "' )" ).c_str() );
  PyRun_SimpleString( (char *) ( string( "sys.path.insert( 1, '" ) + shared2 
                                 + "' )" ).c_str() );
  PyRun_SimpleString( "import anatomist.cpp; anatomist.cpp.Anatomist()" );

  //Py_Finalize();
}


