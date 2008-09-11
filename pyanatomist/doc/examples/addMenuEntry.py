import anatomist.direct.api as anatomist
from soma import aims
import os

a = anatomist.Anatomist()

class MyCallback(anatomist.cpp.ObjectMenuCallback):
  def __init__(self):
    anatomist.cpp.ObjectMenuCallback.__init__(self)

  def doit(self, objects):
    print 'plop!!'

# Store python callbacks
callbacks_list = []


# Add plop Menu to an object
def addMenuEntryToOptionTree(object):
  import sip
  m = anatomist.cpp.ObjectMenu(object.optionTree())
  mycallback = MyCallback()
  callbacks_list.append(mycallback)
  m.insertItem([], 'plop!', mycallback)
  m.insertItem(['bloups'], 'plop!', mycallback)
  t = m.releaseTree()
  sip.transferto(t, None)


# Create a dummy AGraph and add plop menu entry
g = aims.Graph('dummy')
ag = a.toAObject( g )
addMenuEntryToOptionTree(ag)


if __name__ == '__main__' :
  import qt
  if qt.QApplication.startingUp():
    qt.qApp.exec_loop()
