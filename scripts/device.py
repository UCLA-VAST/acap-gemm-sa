BITS_PER_BYTE = 8

class MetaRequires(type):
  def __init__(cls, name, bases, dct):
    if bases:
      for var, typ in cls.requires.items():
        try:
          x = getattr(cls, var)
        except AttributeError:
          raise TypeError(f'class {name} must define variable {var}')
        if not isinstance(x, typ):
          raise TypeError(f'variable {name}.{var} must have type {typ}')

    super().__init__(name, bases, dct)

class MetaRAM(MetaRequires):
  requires = {
    'BITS': int,
    'WIDTH': int,
  }

class RAM(metaclass=MetaRAM):

  def __init__(self, count):
    self._count = count

  def count(self):
    return self._count

class BRAM_18K(RAM):
  BITS = 18*1024
  WIDTH = 36

class URAM_288K(RAM):
  BITS = 288*1024
  WIDTH = 72

class MetaDevice(MetaRequires):
  requires = {
    'AIE_ROWS': int,
    'AIE_COLS': int,
    'AIE_VLEN': dict,
    'AIE_ALEN': int,
    'AIE_STORE_BITS_PER_CYCLE': int,
    'AIE_MAC_PER_CYCLE': dict,
    'AIE_MEM_BYTES': int,
    'PL_BRAM': RAM,
    'PL_URAM': RAM,
  }

class Device(metaclass=MetaDevice):
  pass

class VC1902(Device):
  AIE_ROWS = 8
  AIE_COLS = 50

  AIE_VLEN = {
     8: 64,
    16: 32,
    32: 16
  }
  AIE_ALEN = 8

  AIE_STORE_BITS_PER_CYCLE = 256
  AIE_MAC_PER_CYCLE = {
     8: 128,
    16:  32,
    32:   8
  }

  AIE_MEM_BYTES = 32*1024

  PL_BRAM = BRAM_18K(1934)
  PL_URAM = URAM_288K(463)

VCK5000 = VC1902
