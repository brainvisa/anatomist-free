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


#ifndef ANATOMIST_COMMANDS_CLOADTRANSFORMATION_H
#define ANATOMIST_COMMANDS_CLOADTRANSFORMATION_H


#include <anatomist/processor/Command.h>
#include <anatomist/processor/context.h>
#include <vector>


namespace anatomist
{

  class Referential;
  class Transformation;


  ///	Load a transformation between two referentials
  class LoadTransformationCommand : public RegularCommand, 
				    public SerializingCommand
  {
  public:
    ///	Load transformation from file
    LoadTransformationCommand( const std::string & filename, Referential* org, 
			       Referential* dst, int res_id = -1, 
			       CommandContext* context 
			       = &CommandContext::defaultContext() );
    /**	Give hard-coded transformation matrix. 
	(4 lines, 3 cols, 1st line=translation) */
    LoadTransformationCommand( const float matrix[4][3], Referential* org, 
			       Referential* dst, int res_id = -1, 
			       CommandContext* context 
			       = &CommandContext::defaultContext() );
    /**	Give hard-coded transformation matrix. 
    (1 line, 1st 3 elementse=translation) */
    LoadTransformationCommand( const std::vector<float> &, Referential* org,
                               Referential* dst, int res_id = -1,
                               CommandContext* context
                                   = &CommandContext::defaultContext() );
    virtual ~LoadTransformationCommand();

    virtual std::string name() const { return( "LoadTransformation" ); }
    virtual void write( Tree & com, Serializer* ser ) const;
    Transformation* trans() const { return( _tra ); }

  protected:
    virtual void doit();

  private:
    Referential		*_org;
    Referential		*_dst;
    Transformation	*_tra;
    int			_id;
    std::string		_filename;
    float		_matrix[4][3];

    friend class StdModule;
    static Command* read( const Tree & com, CommandContext* context );
    static bool initSyntax();
  };

}


#endif
