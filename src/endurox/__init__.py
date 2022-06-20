"""Python3 bindings for writing Enduro/X clients and servers"""

import sys
import ctypes

flags = sys.getdlopenflags()

# Need as Enduro/X XA drivers are dynamically loaded, and
# they need to see Enduro/X runtime.
sys.setdlopenflags(flags | ctypes.RTLD_GLOBAL)

from .endurox import *

# change module name... for importeds symbols
sys.setdlopenflags(flags)

__all__ = ['endurox']
