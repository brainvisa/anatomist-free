==================
Anatomist glossary
==================

.. glossary::

  Browser
    Anatomist browser windows information about objects, which can be browsed in a tree view.

  Geometric fusion
    Creation of a new volume by merging two volumes. The value of voxels are obtained by mutltiplying rgb channels of source volumes.

  Graph
    Structured object. A graph has global attributes and is composed of nodes, which also have attributes (eg. position, name, label, colour...). The nodes can be linked by relations (eg. sulci graph). There are several types of graphs, according to the nodes content.

  ROI graph
    Each node is a region of interest.

  Mesh graph
    Each node is a mesh.

  Header
    The header of a file stores attributes for an image. For example, the GIS format is composed of a .ima file which contain the image and a .dim file which is the header. It contains information about the image as for example the image size, the voxel size, ... This file can be opened with a text editor or with a browser window in Anatomist.

  Histogram (of a volume)
    distribution of voxels values intensity in a volume.

  Interpolation
    Computing method to get a voxel value in a new image from several voxel values. There are several methods: closest neighbour, bilinear, trilinear...

  Label image
    A label is associated to each voxel and it enables to make groups of voxels with the same label. For example, a binary image is a label image with two labels: 0 and 1.

  Linear fusion
    Linear combination of each voxel of each source volume, with an adjustable mixing rate (*voxel1*fm1 + voxel2*fm2* with *fm1* and *fm2* representing the mixing rate).

  Mesh
    3D object. Representation of an object by a surface composed of polygons (triangles).

  Neurological convention
    Display and/or storage convention of volumes where the right side of the brain is diplayed at the right side of the image.

  Normalisation
    Normalizing an image is resizing it according to a template. Useful to compare different images.

  Object
    In Anatomist, an object is any data structure that can be loaded an displayed. An object can be an image, a graph, a mesh, a texture...

  Palette
    colors applied to an image voxels values. Also named colormap.

  Radiological (convention)
    Display and/or storage convention where the right side of the brain is diplayed at the left side of the image. It's an historical convention linked to radiologists habits.

  Referential
    Coordinates system.

  Resolution of voxels
    Dimension of voxels along x, y and z axis in mm.

  RGB channels
    Coding a colour by three values corresponding to Red, Green and Blue. Each colour has 256 possible values, so for each voxel there are 256x256x256 possible colours. Here are several examples of colour codes:

    * black (0, 0, 0)
    * blue (0, 0, 255)
    * white (255, 255, 255)
    * grey (128, 128, 128)

  Segmentation
    Definition of areas of interest in a volume or an image accoding some criterions. For example, if we want a white matter mask, we have to segment the brain according to grey levels.

  SPM Talairach
    It is one of the available referentials in Anatomist. It represents the Talairach referential used in SPM. This is the referential of images normalized with SPM.

  Talairach AC/PC
    Referential in Anatomist based on the anterior commissure position.

  Texture
    Data used to put information on a mesh. There are 1D, 2D or 3D textures according to the number of information pieces by mesh element.

  4D Volume
    Volume with 4 dimensions: x, y, z and t. For example, one 3D volume for each time unit. When a 4D volume is loaded in Anatomist and put in a window, the window gets 2 cursors, one for the 3D volume and the other for the 4th dimension. This 4th dimension can be the time or another dimension, for example the directions in a diffusion image.

