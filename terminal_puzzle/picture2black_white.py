import sys
import skimage
from skimage import data, io, filters, color
from skimage.transform import rescale, resize, downscale_local_mean
from matplotlib import pyplot as plt
import numpy as np


def png2bit(path_to_img, path_to_bit, rescale_factor=1.0):
  """
    Converts and optionally downscales a square image for the terminal puzzle.
    Image should be square.

    Args:
      path_to_img     (str):    Path to the square image that is converted
      path_to_bit     (str):    Path to the file where the converted image is saved as black(1) and white(0) text
      rescale_factor  (float):  Downscaling factor
  """

  img = io.imread(path_to_img)
  img_gray = color.rgb2gray(img)
  img_gray = rescale(img_gray, rescale_factor, anti_aliasing=True)
  threshold = filters.threshold_otsu(img_gray)
  binary = ~(img_gray > threshold)
  np.savetxt(path_to_bit, binary, fmt="%i", delimiter='')


if __name__ == '__main__':
  png2bit(sys.argv[1], sys.argv[2], float(sys.argv[3]) if len(sys.argv) > 3 else 1.0)
