#!/usr/bin/env python
import sys, os
from PIL import Image, ImageChops

def autocrop(img, bgcolor):
	''' autocrop '''
	if img.mode == "RGBA":
		img_mode = "RGBA"
	elif img.mode != "RGB": 
		img_mode = "RGB"
		img = img.convert("RGB")
        else:
                img_mode = "RGB"
	bg = Image.new(img_mode, img.size, bgcolor)
	diff = ImageChops.difference(img, bg)
	bbox = diff.getbbox()
	return img.crop(bbox)

def main():
	if len(sys.argv) != 3:
		print '%s input output' % sys.argv[0]
		sys.exit(1)
	input, output  = sys.argv[1:]
	img = Image.open(input)
	if img.mode == "RGBA" :
		img2 = autocrop(img, (0, 0, 0, 0))
		img = img2
	img2 = autocrop(img, (255, 255, 255))
	img2.save(output)

main()
