
numtypes = [ 'unsigned char', 'short', 'unsigned short', 'int',
              'unsigned', 'float', 'double' ]
basetypes = numtypes + [ 'AimsRGB', 'AimsRGBA' ]
matrix = []
for z in [ map( lambda y: (x,y), basetypes ) for x in basetypes ]:
  matrix += z

todo = { 'rcptr' : [ 'anatomist::EventHandler', 'anatomist::Unserializer',
                     'anatomist::GLItem', 'anatomist::AObject',
                     'anatomist::AWindow', 'anatomist::APalette',
                     'anatomist::ObjectMenu', ],
         'asurface' : [ '3', '2' ],
         'control' : [ 'Void' ],
         'vector' : [ 'anatomist::AObject *' ],
         'list' : [ 'carto::rc_ptr<anatomist::APalette>',
                    'anatomist::RefGLItem' ],
         'set' : [ 'anatomist::AObject *', 'anatomist::AWindow *',
                   'anatomist::Referential *', 'anatomist::Transformation *' ],
	 'map' : [ ( 'unsigned', 'std::set<anatomist::AObject *>' ),
                   ( 'unsigned', 'std::set<anatomist::AWindow *>' ),
                   ('int', 'QPixmap'),
                   ('std::string', 'carto::rc_ptr<anatomist::ObjectMenu>') ],
         'anatomist' : [ 'Void' ],
         'sharedptr' : [ 'anatomist::AObject', 'anatomist::AWindow' ],
        }

