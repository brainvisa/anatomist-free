import anatomist.direct.api as anatomist
from soma import aims
r = aims.Reader()
g = r.read( 'Rbase.arg' )

a = anatomist.Anatomist()

ag = a.toAObject( g )
for x in g.vertices():
  x[ 'toto' ] = 12.3

g.vertices().list()[10]['toto'] = 24.3
g.vertices().list()[12]['toto'] = 48

ag.setColorMode( ag.PropertyMap )
ag.setColorProperty( 'toto' )
ag.notifyObservers()

w = a.createWindow( '3D' )
w.addObjects( ag, add_graph_nodes=True )

def main():
  import qt
  qt.qApp.exec_loop()

if __name__ == '__main__' : main()
