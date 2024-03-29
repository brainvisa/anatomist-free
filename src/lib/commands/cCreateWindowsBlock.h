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


#ifndef ANA_COMMANDS_CCREATEWINDOWSBLOCK_H
#define ANA_COMMANDS_CCREATEWINDOWSBLOCK_H


#include <anatomist/processor/Command.h>
#include <cartobase/object/object.h>
#include <qwidget.h>

namespace anatomist
{

  ///	Windows block creation
  class CreateWindowsBlockCommand
    : public RegularCommand, public SerializingCommand
  {
  public:
    CreateWindowsBlockCommand( int id = -1,
                               CommandContext* context = 0,
                               const std::vector<int> & geom
                                = (std::vector<int>) 0,
                               int cols = 0, int rows = 0,
                               int default_block = true );
    virtual ~CreateWindowsBlockCommand();

    virtual std::string name() const { return( "CreateWindowsBlock" ); }
    virtual void write( Tree & com, Serializer* ser ) const;
    QWidget *block() const { return( _block ); }

  protected:
    virtual void doit();
    virtual void undoit();

  private:
    friend class StdModule;
    static Command* read( const Tree & com, CommandContext* context );
    static bool initSyntax();

    int			_id;
    std::vector<int>	_geom;
    QWidget		*_block;
    int			_cols;
    int                 _rows;
    int                 _default_block;
  };

}


#endif
