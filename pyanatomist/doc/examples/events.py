#%autoindent 0

import anatomist.direct.api as anatomist
from soma import aims

a = anatomist.Anatomist()

# definig a custom event handler in python:
def clickHandler(eventName, params):
    print 'click event: ', eventName
    print 'LinkedCursor event contents:', params.keys()
    pos=params['position']
    print 'pos:', pos
    win=params['window']
    print 'window:', win

# register the function on the cursor notifier of anatomist. It will be called when the user click on a window
a.onCursorNotifier.add(clickHandler)


# definig a custom event in python
class TotoEvent ( anatomist.cpp.OutputEvent ):
  def __init__( self ):
    # we can't make a custom Object yet...
    anatomist.cpp.OutputEvent.__init__( self, 'Toto',
      {}, 1 )
ev=TotoEvent()
ev.send()

# ...
# wen you're done
# you can remove the handler
# a.onCursorNotifier.remove(clickHandler)
