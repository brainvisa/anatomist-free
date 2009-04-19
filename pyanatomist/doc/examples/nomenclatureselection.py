import anatomist.direct.api as anatomist
from soma import aims
import os

a = anatomist.Anatomist()
sh = aims.carto.Paths.shfjShared()
nom = a.loadObject( os.path.join( sh, 'nomenclature', 'hierarchy',
    'sulcal_root_colors.hie' ) )
graph = a.loadObject( 'Rbase.arg' )
w = a.createWindow( '3D' )
w.addObjects( graph, add_graph_nodes=True )

a.execute( 'SelectByNomenclature', names='PREFRONTAL_right', nomenclature=nom )

# to unselect all
# a.execute( 'Select' )
