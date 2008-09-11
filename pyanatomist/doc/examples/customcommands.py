# custom commands example using Anatomist commands factory

import anatomist.direct.api as anatomist
from soma import aims
import sip

a = anatomist.Anatomist()

cx = anatomist.cpp.CommandContext.defaultContext()

# custom command
i = cx.freeID()
c = a.execute( 'CreateWindow', type = 'Coronal', res_pointer = i )

win = c.createdWindow()

# otherwise, we can retreive the new window using the context
# (which will work if the exact command has no python binding)
print cx.id( win )
print cx.object( i )
win2 = cx.object( i )

print win2 is win

r = aims.Reader()
vol = r.read( 'irm.ima' )
avol = a.toAObject( vol )

# keywords / real objects test
a.execute( 'AddObject', objects = [ avol ], windows = [ win ] )

