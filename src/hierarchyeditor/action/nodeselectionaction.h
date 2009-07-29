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


#ifndef GRAPH_EDITOR_ACTION_H
#define GRAPH_EDITOR_ACTION_H

#include <anatomist/controler/action.h>
#include <graph/graph/graph.h>
#include <graph/tree/tree.h>
#include <anatomist/object/Object.h>


namespace anatomist
{
	class AWindow ;
	class AGraph ;
	class AGraphObject ;
	class AObject ;
	class Hierarchy;

	class NodeSelectionAction : public Action
	{

		public:
			NodeSelectionAction() ;
			virtual ~NodeSelectionAction() ;


		// Action inputs
			virtual std::string name() const;
			AObject * get_o();
			static Action* creator();
			bool removeVertex( Vertex * V );
			bool addVertex( Vertex * V );
			void remove( int x, int y, int globalX, int globalY  );
			void add( int x, int y, int globalX, int globalY  );
			void select( int x, int y, int globalX, int globalY  );
			void setHie( Hierarchy * g );
			void setBrowser( AWindow * b );
			Hierarchy *  getHie( );
			AWindow *  getBrowser( );

			bool graph_set;

		private:
			int count_fold;
			AWindow* action_browser;
			Hierarchy * action_hie;
			//Hierarchy* Amain_hie;


			//std::set < Vertex * > list_vertex;
			std::map < Vertex * , Tree * > list_vertex;  //First : original graph node; Second : copy.


	};
}

#endif
