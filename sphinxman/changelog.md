# Changelog

## [Unreleased]

### Changed

- fixed a crash when deleting a graph with selected nodes
- added a fusion from a 5D volume to build a RGB volume. Some NIFTI RGB are stored this way.
- fixed size of FusionRGBAVolume fusion when voxel size is not 1x1x1mm
- measure control: fixed markers display in direct referentials


## [6.6.0] 2026-01-12

There has been several significant changes in this version...

### Added

- `Save` button in 3D views appears when a modified object is displayed. It allows for instance to save a manually labelled graph or a ROI to be saved simply.
- Texture wrapping modes (OpenGL standard modes, like repeat or clamp_to_edge) are supported and can be setup in API, commands, objects header properties, and GUI.
- Important improvements in windows blocks: block grid elements can be resized, moved and reordered, and windows can be dropped at the desired place. A new empty block can be opened (ctrl+N) and will become a "default block": all new windows will open, by default, in this block (after all if we open a block, it's probably because we intend to use it).
- new windows types:
  - Values plots: plots the value of observed objects at the selected position, in various plots types
  - Stats plots: 1D stats along a given axis using several boxplots, errorbars etc
  - Info window: displays the cursor position (modifiable) and objects vertex, value and label at the cursor position
- The new windows types, plus the profile window, can now display data (timecourses) from both volumes and textured meshes.
- Support (in AIMS library, actually) for reading various Freesurfer native formats: surfaces (meshes), curvature, annotations, and .xfm affine transformation files.
- It is now possible to load multiple objects in parallal in the C++ and python APIs.The GUI still uses sequential loading for safety (data formats are not all guananteed to be thread-safe).
- New control for measurements: allow to draw lines by clicking on 3D objects. The length of the line is displayed. Interestingly, this control may also be used to draw filar polygons meshes.
- Better touchscreen support</li>

### Changed

- Change in behavior of palettes with "zero at the center", allowing to have a modifiable low threshold. See the doc...
- Reworked the transformations graph update when adding / changing a transformation. The process is about 3 times faster and avoids triggering lots of unneeded change notifications. The process is still largely suboptimal I think, but it's better than it used to be...
- Ensure thread safety in all code involved in object loading
- Fixed selection issues in the ROI toolbox
- New palette window, smaller, shows only what is useful and pops-up things when needed
- Various bugfixes.

### Internal (programmers)

- The `anatomist` command is no longer the C++ executable, but a python (pyanatomist) wrapper. It starts an IPython engine from the startup, in order to avoid hangups in recent versions of jupyter qtconsole. The former C++ executable is renamed `anatomist-bin`. However for scripts running Anatomist outside of this launcher, opening the shell will generally not worl any longer. We coulnd not find a solution for that.
- Use the newer `QOpenGLWidget` class instead of the old `QGLWidget` (from Qt 1 or 2...). This had consequences in offscreen displays, we think all issues are fixed.
- Switch to Qt offscreen mode instead of Xvfb + VirtualGL for headless Anatomist when possible
- Porting to Qt6. The source code may be compiled for Qt5 or Qt6 as a build configuration option.
- new command: `LoadObjects` to load multiple objects at once.


```{raw} html
:file: ../doc/html/en/changelog.html
```

