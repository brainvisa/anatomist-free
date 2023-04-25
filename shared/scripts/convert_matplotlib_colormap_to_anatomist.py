#!/usr/bin/env python3

import os.path
import sys

import matplotlib
import numpy
from soma import aims


def convert_colormap(cm, cm_name, centered=False, display_properties={}):
    if centered and hasattr(cm, '_segmentdata'):
        cm = cm._resample(cm.N - 1)

    vol = aims.Volume(cm.N, 1, 1, dtype='RGBA')

    if hasattr(cm, '_segmentdata'):
        # Make sure that the colormap is completely opaque
        if 'alpha' in cm._segmentdata:
            assert all(numpy.asarray(cm._segmentdata['alpha'])[:, 1:2] == 1.0)
        # Every (anchor, value) double is repeated twice, to force the
        # interpolator to use linear interpolation (as in Matplotlib). By
        # default Anatomist uses second-order spline interpolation.
        palette_gradients = '#'.join(
            (';'.join(f'{anchor};{value};{anchor};{value}' for anchor, value
                      in numpy.asarray(cm._segmentdata[primary])[:, :2]))
            for primary in ('red', 'green', 'blue')
        ) + '#0.5;1'
        vol.header().update({
            'palette_gradients': palette_gradients,
            'palette_gradients_mode': 'RGBA',
        })

    arr = cm(numpy.arange(cm.N), bytes=True)
    # assert numpy.all(arr[:, 3] == 255)
    arr_v = numpy.empty((cm.N,), dtype=[('v', 'u1', (4,))])
    arr_v['v'][:] = arr
    vol[...] = arr_v[:, numpy.newaxis, numpy.newaxis, numpy.newaxis]

    final_display_properties = display_properties.copy()
    if centered:
        final_display_properties['centered'] = 1
        fusion_pos = cm.N // 2
    else:
        fusion_pos = 0
    fusion_value = cm(fusion_pos, bytes=True)
    if fusion_value == (0, 0, 0, 255):
        final_display_properties['fusion'] = 'B'
    elif fusion_value == (255, 255, 255, 255):
        final_display_properties['fusion'] = 'W'
    elif fusion_value[3] == 0:
        final_display_properties['fusion'] = 'T'
    vol.header()['display_properties'] = final_display_properties


    aims.write(vol, os.path.join(os.path.expanduser('~/.anatomist/rgb/'),
                                 cm_name))

#convert_colormap(nilearn_cmaps['red_transparent'], 'red_transparent',
#                 display_properties={'sequential': 1})
convert_colormap(matplotlib.colormaps['viridis'], 'viridis',
                 display_properties={'uniform': 1, 'sequential': 1})
convert_colormap(matplotlib.colormaps['plasma'], 'plasma',
                 display_properties={'uniform': 1, 'sequential': 1})
convert_colormap(matplotlib.colormaps['inferno'], 'inferno',
                 display_properties={'uniform': 1, 'sequential': 1})
convert_colormap(matplotlib.colormaps['magma'], 'magma',
                 display_properties={'uniform': 1, 'sequential': 1})
convert_colormap(matplotlib.colormaps['cividis'], 'cividis',
                 display_properties={'uniform': 1, 'sequential': 1})

convert_colormap(matplotlib.colormaps['Purples'], 'Purples',
                 display_properties={'sequential': 1})
convert_colormap(matplotlib.colormaps['Blues'], 'Blues',
                 display_properties={'sequential': 1})
convert_colormap(matplotlib.colormaps['Greens'], 'Greens',
                 display_properties={'sequential': 1})
convert_colormap(matplotlib.colormaps['Oranges'], 'Oranges',
                 display_properties={'sequential': 1})
convert_colormap(matplotlib.colormaps['Reds'], 'Reds',
                 display_properties={'sequential': 1})


convert_colormap(matplotlib.colormaps['bwr'], 'bwr', centered=True)
convert_colormap(matplotlib.colormaps['coolwarm'], 'coolwarm', centered=True)
convert_colormap(matplotlib.colormaps['RdBu'], 'RdBu', centered=True)
convert_colormap(matplotlib.colormaps['RdGy'], 'RdGy', centered=True)


convert_colormap(matplotlib.colormaps['twilight'],
                 'twilight', centered=True,
                 display_properties={'cyclic': 1})
convert_colormap(matplotlib.colormaps['twilight_shifted'],
                 'twilight_shifted', centered=True,
                 display_properties={'cyclic': 1})

convert_colormap(matplotlib.colormaps['nipy_spectral'], 'nipy_spectral')
convert_colormap(matplotlib.colormaps['gist_ncar'], 'gist_ncar')
convert_colormap(matplotlib.colormaps['gist_rainbow'], 'gist_rainbow')


from nilearn.plotting.cm import _cmap_d as nilearn_cmaps

convert_colormap(nilearn_cmaps['cold_hot'], 'cold_hot', centered=True)
convert_colormap(nilearn_cmaps['cold_white_hot'], 'cold_white_hot',
                 centered=True)

# Weird colormaps with a discontinuity at the centre
#convert_colormap(nilearn_cmaps['cyan_copper'], 'cyan_copper')
#convert_colormap(nilearn_cmaps['cyan_orange'], 'cyan_orange')
#convert_colormap(nilearn_cmaps['blue_red'], 'blue_red')
