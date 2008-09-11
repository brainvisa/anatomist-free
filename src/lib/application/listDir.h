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


#ifndef ANATOMIST_APPLICATION_LISTDIR_H
#define ANATOMIST_APPLICATION_LISTDIR_H

#include <qdir.h>
#include <string>
#include <list>


namespace anatomist
{
#if QT_VERSION >= 0x040000
  typedef enum QDir::Filter FileFilterSpec;
  typedef enum QDir::SortFlag FileSortSpec;
#else
  typedef enum QDir::FilterSpec FileFilterSpec;
  typedef enum QDir::SortSpec FileSortSpec;
#endif
  /**	A utility function to scan a directory for files or directories, 
	which will hide non-portable things. Uses Qt's QDir - see the doc...
	\param dir directory to scan
	\param filter files filter 
	\param sortSpec sorting orger specifications, using QDir::SortSpec
	\param filterSpec attributes to filter files with, bitwise OR of 
	\a anatomist::FileAttrib which are just a typedef from 
	QDir::FilterSpec */
  std::list<std::string> 
  listDirectory( const std::string & dir, const std::string & filter = "", 
		 int sortSpec = QDir::Name | QDir::IgnoreCase, 
		 int filterSpec = QDir::All );
}


#endif
