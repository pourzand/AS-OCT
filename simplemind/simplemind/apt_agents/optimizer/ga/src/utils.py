import math, logging
GA_UTILS_LOG = logging.getLogger("opt.ga.utils")
## Originally: 
#   return (('%%0%sX' % math.ceil(len(str(input))/4)) % int("".join((str(i) for i in reversed(str(input)))),2))[::-1]
def binlist_to_hexstr(bin_input):
    """Returns a hexdecimal string from a given list of 0s and 1s.
    Below is an example:
        Command: binlist_to_hexstr([1,1,0,0,1,0,1,1,0])
        Expected ouput: 3D0
    """
    if bin_input is None:
        GA_UTILS_LOG.warning("Warning: Conversion binary was None.")
        return None
    bin_input = "".join((str(i) for i in reversed(bin_input)))
    return (('%%0%sX' % math.ceil(len(str(bin_input))/4)) % int(bin_input,2))[::-1]
