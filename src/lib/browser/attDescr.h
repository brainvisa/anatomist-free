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


#ifndef ANAQT_BROWSER_ATTDESR_H
#define ANAQT_BROWSER_ATTDESR_H


#include <cartobase/object/attributed.h>
#include <aims/qtcompat/qlistview.h>

class Tree;
class QObjectBrowserWidget;


namespace anatomist
{

  /**	Descripteur d'objet attribu�
	Ecrit dans un arbre.
  */
  class AttDescr
  {
  public:
    typedef void (*Helper)( QObjectBrowserWidget*, 
			    const carto::GenericObject &, 
			    const std::string &, std::string & output );
    typedef void (*ListHelper)( QObjectBrowserWidget*, 
				const carto::GenericObject &, 
				const std::string &, Q3ListViewItem* parent, 
				const AttDescr* ad, bool regist );
    typedef std::map<std::string, Helper>	HelperSet;
    typedef std::map<std::string, ListHelper>	ListHelperSet;

    AttDescr();
    virtual ~AttDescr();

    void describeAttributes( QObjectBrowserWidget* br, Q3ListViewItem* parent, 
			     const carto::GenericObject *ao, 
			     bool regist = true, 
                             bool checkexisting = true ) const;

    std::string printAttribute( QObjectBrowserWidget* br, 
				const carto::GenericObject* ao, 
				const std::string & semantic, 
				const std::string & type ) const;
    void printAttribute( QObjectBrowserWidget* br, 
			 const carto::GenericObject* ao, 
			 const std::string & semantic, 
			 const std::string & type, Q3ListViewItem* parent, 
			 bool regist = true, bool checkexisting = true ) const;
    std::string objectName( const carto::GenericObject* ao ) const;

    void setSyntax( const carto::SyntaxSet & s );
    void addSyntax( const carto::SyntaxSet & s );
    void setHelpers( const HelperSet & help );
    void addHelpers( const HelperSet & help );
    void setListHelpers( const ListHelperSet & help );
    void addListHelpers( const ListHelperSet & help );
    const carto::SyntaxSet & syntaxSet() const { return( syntax ); }
    const HelperSet & helperSet() const { return( helpers ); }
    const ListHelperSet & listHelperSet() const { return( listHelpers ); }

    void describeTree( QObjectBrowserWidget* br, const Tree* tr, 
		       Q3ListViewItem* parent, bool regist = true ) const;
    ///	ne recr� pas l'arbre de base
    void describeTreeInside( QObjectBrowserWidget* br, const Tree* tr, 
			     Q3ListViewItem* parent, bool regist ) const;
    ///	ne recr� pas l'arbre de base
    void describeUnregisteredTreeInside( QObjectBrowserWidget* br, 
					 const Tree* tr, 
					 Q3ListViewItem* parent ) const;
    static void printError( Q3ListViewItem* parent, 
			    const std::string & semantic );

    static void treeListHelper( QObjectBrowserWidget* br, 
				const carto::GenericObject &, 
				const std::string &, 
				Q3ListViewItem* parent, const AttDescr* gvw, 
				bool regist = true );
    virtual void initHelpers();

    static AttDescr* descr() { return( _theAttDescr ); }
    static QPixmap rgbPixmap( const QColor & col );

  protected:
    ///	Helpers
    static void printInt( QObjectBrowserWidget*, 
			  const carto::GenericObject &, 
			  const std::string &, std::string & output );
    static void printFloat( QObjectBrowserWidget*, 
			    const carto::GenericObject &, 
			    const std::string &, std::string & output );
    static void printDouble( QObjectBrowserWidget*, 
			     const carto::GenericObject &, 
			     const std::string &, std::string & output );
    static void printString( QObjectBrowserWidget*, 
			     const carto::GenericObject &, 
			     const std::string &, std::string & output );
    static void printFloatVector( QObjectBrowserWidget*, 
				  const carto::GenericObject &, 
				  const std::string &, std::string & output );
    static void printDoubleVector( QObjectBrowserWidget*, 
				   const carto::GenericObject &, 
				   const std::string &, std::string & output );
    static void printIntVector( QObjectBrowserWidget*, 
				const carto::GenericObject &, 
				const std::string &, std::string & output );
    static void printStringVector( QObjectBrowserWidget*, 
				   const carto::GenericObject &, 
				   const std::string &, std::string & output );
    static void printStringVectorPtr( QObjectBrowserWidget*, 
				      const carto::GenericObject &, 
				      const std::string &, 
				      std::string & output );

    static AttDescr	*_theAttDescr;

    carto::SyntaxSet syntax;
    HelperSet helpers;
    ListHelperSet listHelpers;
  };

}

#endif
