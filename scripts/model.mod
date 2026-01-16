################################################################################
# SETS
################################################################################

################################################################################
# PARAMETERS
################################################################################

# dimensions
param m integer > 0;
param k integer > 0;
param n integer > 0;

param max_r integer > 0;
param max_c integer > 0;

param fixed_rc binary default 0;
param r integer > 0;
param c integer > 0;

param fixed_pl binary default 0;
param pm integer > 0;
param pk integer > 0;
param pn integer > 0;

param fixed_aie binary default 0;
param am integer > 0;
param ak integer > 0;
param an integer > 0;

param enable_split binary default 0;

# memory
param double_buffer integer = 2;
param bits_per_byte integer = 8;
param parts_a integer > 0;
param parts_b integer > 0;
param parts_c integer > 0;

param bitwidth_dram integer > 0;
param bitwidth_plio integer > 0;
param bitwidth_word integer > 0;
param bitwidth_data integer > 0;
param bitwidth_aie  integer > 0;
param bytes_data integer = bitwidth_data / bits_per_byte;
param aie_pl_clock_ratio integer > 0;

param fixed_bram binary default 0;
param bram_a binary default 0;
param bram_b binary default 0;
param bram_c binary default 0;

param bram_bits integer > 0;
param bram_width integer > 0;
param bram_count integer > 0;
param bram_depth integer = bram_bits / bram_width;
param bram_units_per_word integer = ceil(bitwidth_word / bram_width);
param bram_total_bytes integer = bram_bits * bram_count / bits_per_byte;
param bram_util > 0.0, <= 1.0;

param uram_bits integer > 0;
param uram_width integer > 0;
param uram_count integer > 0;
param uram_depth integer = uram_bits / uram_width;
param uram_units_per_word integer = ceil(bitwidth_word / uram_width);
param uram_total_bytes integer = uram_bits * uram_count / bits_per_byte;
param uram_util > 0.0, <= 1.0;

# aie
param aie_store_bits_per_cycle integer > 0;
param aie_mac_per_cycle integer > 0;
param aie_vlen integer > 0;
param aie_alen integer > 0;
param aie_mem_bytes integer > 0;

# efficiency
param comp_eff > 0.0, <= 1.0;
param mem_eff > 0.0, <= 1.0;

################################################################################
# VARIABLES
################################################################################

# dimensions
var Rows integer >= 1; #, <= max_r;
var Cols integer >= 1; #, <= max_c;
var PhysRows integer >= 1;
var PhysCols integer >= 1;
var SplitRows integer >= 1;

var PadM integer >= m;
var PadK integer >= k;
var PadN integer >= n;
var PM integer >= 1;
var PK integer >= 1;
var PN integer >= 1;
var AM integer >= 1;
var AK integer >= 1;
var AN integer >= 1;
var BM1 integer >= 1;
var BK1 integer >= 1;
var BN1 integer >= 1;
var BM2 integer >= 1;
var BK2 integer >= 1;
var BN2 integer >= 1;
var BM3 integer >= 1;
var BK3 integer >= 1;
var BN3 integer >= 1;

# memory
var HostBytes integer >= 1;
var AIEBytes integer >= 1;
var PLBytes integer >= 1;

var PLBRAMUnitsPerPartA integer >= 1;
var PLBRAMUnitsPerPartB integer >= 1;
var PLBRAMUnitsPerPartC integer >= 1;
var PLBRAMUnitsA integer >= 1;
var PLBRAMUnitsB integer >= 1;
var PLBRAMUnitsC integer >= 1;
var PLBRAMUnits integer >= 0;

var PLURAMUnitsPerPartA integer >= 1;
var PLURAMUnitsPerPartB integer >= 1;
var PLURAMUnitsPerPartC integer >= 1;
var PLURAMUnitsA integer >= 1;
var PLURAMUnitsB integer >= 1;
var PLURAMUnitsC integer >= 1;
var PLURAMUnits integer >= 0;

var AOnBRAM binary;
var BOnBRAM binary;
var COnBRAM binary;

# aie cycles
var AIEBufferReadCycles integer >= 1;
var AIECompCycles integer >= 1;
var AIEFlushCycles integer >= 1;
var AIECycles integer >= 1;
var AIECyclesPL integer >= 1;

# pl cycles
var PLBufferReadCycles integer >= 1;
var PLBufferReadACycles integer >= 1;
var PLBufferReadBCycles integer >= 1;
var PLBufferWriteCycles integer >= 1;
var SABufferSendCycles integer >= 1;
var SACompCycles integer >= 1;
var PLBufferReuse integer >= 1;
var PLBufferCompCycles integer >= 1;
var PLBufferCycles integer >= 1;
var PLCycles integer >= 1;

# helpers
var Int0 integer >= 0; var Int1 integer >= 0; var Int2 integer >= 0;
var Cont0 >= 0; var Cont1 >= 0; var Cont2 >= 0;
var Cont3 >= 0; var Cont4 >= 0; var Cont5 >= 0;
var Cont6 >= 0;
var Ceil0 integer >= 0; var Ceil1 integer >= 0; var Ceil2 integer >= 0;
var Ceil3 integer >= 0; var Ceil4 integer >= 0; var Ceil5 integer >= 0;
var Ceil6 integer >= 0;

################################################################################
# DEFINITIONS
################################################################################

# memory
subject to HostBytesDef:
  HostBytes = (PadM*PadK + PadK*PadN + PadM*PadN) * bytes_data; 

subject to PLBytesDef:
  PLBytes = (PM*PK + PK*PN + PM*PN) * double_buffer * bytes_data;

subject to AIEBytesDef:
  AIEBytes = (double_buffer*(AM*AK + AK*AN + 2*AM*AN) + AM*AN) * bytes_data;

subject to PLBRAMUnitsPerPartADef:
  Cont0 = PM*PK*bitwidth_data / Rows / bitwidth_word / bram_depth
  and Ceil0 >= Cont0
  and Ceil0 <= Cont0 + 1 - 1e-6
  and PLBRAMUnitsPerPartA = Ceil0 * bram_units_per_word;

subject to PLBRAMUnitsPerPartBDef:
  Cont1 = PK*PN*bitwidth_data / Cols / bitwidth_word / bram_depth
  and Ceil1 >= Cont1
  and Ceil1 <= Cont1 + 1 - 1e-6
  and PLBRAMUnitsPerPartB = Ceil1 * bram_units_per_word;

subject to PLBRAMUnitsPerPartCDef:
  Cont2 = PM*PN*bitwidth_data / Cols / bitwidth_word / bram_depth
  and Ceil2 >= Cont2
  and Ceil2 <= Cont2 + 1 - 1e-6
  and PLBRAMUnitsPerPartC = Ceil2 * bram_units_per_word;

subject to PLBRAMUnitsADef:
  PLBRAMUnitsA = PLBRAMUnitsPerPartA * Rows * double_buffer;

subject to PLBRAMUnitsBDef:
  PLBRAMUnitsB = PLBRAMUnitsPerPartB * Cols * double_buffer;

subject to PLBRAMUnitsCDef:
  PLBRAMUnitsC = PLBRAMUnitsPerPartC * Cols * double_buffer;

subject to PLBRAMUnitsDef:
  PLBRAMUnits = AOnBRAM*PLBRAMUnitsA + BOnBRAM*PLBRAMUnitsB + COnBRAM*PLBRAMUnitsC;

subject to PLURAMUnitsPerPartADef:
  Cont3 = PM*PK*bitwidth_data / Rows / bitwidth_word / uram_depth
  and Ceil3 >= Cont3
  and Ceil3 <= Cont3 + 1 - 1e-6
  and PLURAMUnitsPerPartA = Ceil3 * uram_units_per_word;

subject to PLURAMUnitsPerPartBDef:
  Cont4 = PK*PN*bitwidth_data / Cols / bitwidth_word / uram_depth
  and Ceil4 >= Cont4
  and Ceil4 <= Cont4 + 1 - 1e-6
  and PLURAMUnitsPerPartB = Ceil4 * uram_units_per_word;

subject to PLURAMUnitsPerPartCDef:
  Cont5 = PM*PN*bitwidth_data / Cols / bitwidth_word / uram_depth
  and Ceil5 >= Cont5
  and Ceil5 <= Cont5 + 1 - 1e-6
  and PLURAMUnitsPerPartC = Ceil5 * uram_units_per_word;

subject to PLURAMUnitsADef:
  PLURAMUnitsA = PLURAMUnitsPerPartA * Rows * double_buffer;

subject to PLURAMUnitsBDef:
  PLURAMUnitsB = PLURAMUnitsPerPartB * Cols * double_buffer;

subject to PLURAMUnitsCDef:
  PLURAMUnitsC = PLURAMUnitsPerPartC * Cols * double_buffer;

subject to PLURAMUnitsDef:
  PLURAMUnits = (1-AOnBRAM)*PLURAMUnitsA + (1-BOnBRAM)*PLURAMUnitsB + (1-COnBRAM)*PLURAMUnitsC;

# aie cycles
subject to AIEBufferReadCyclesDef:
  AIEBufferReadCycles = max(AM*AK, AK*AN) * bitwidth_data / bitwidth_aie;

subject to AIECompCyclesDef:
  Cont6 = AM*AK*AN / aie_mac_per_cycle / comp_eff
  and Ceil6 >= Cont6
  and Ceil6 <= Cont6 + 1 - 1e-6
  and AIECompCycles = Ceil6;

subject to AIEFlushCyclesDef:
  AIEFlushCycles = AM*AN * bitwidth_data / aie_store_bits_per_cycle;

subject to AIECyclesDef:
  AIECycles = max(AIEBufferReadCycles, AIECompCycles + AIEFlushCycles);

subject to AIECyclesPLDef:
  Int2 * aie_pl_clock_ratio >= AIECycles / aie_pl_clock_ratio
  and (Int2 - 1) * aie_pl_clock_ratio < AIECycles / aie_pl_clock_ratio
  and AIECyclesPL = Int2 * aie_pl_clock_ratio;

# pl cycles
subject to PLBufferReadCyclesDef:
  PLBufferReadCycles = max(PLBufferReadACycles, PLBufferReadBCycles);

subject to PLBufferReadCyclesADef:
  PLBufferReadACycles = PM*PK * bitwidth_data / bitwidth_dram / parts_a / mem_eff;

subject to PLBufferReadCyclesBDef:
  PLBufferReadBCycles = PK*PN * bitwidth_data / bitwidth_dram / parts_b / mem_eff;

subject to PLBufferWriteCyclesDef:
  PLBufferWriteCycles = PM*PN * bitwidth_data / bitwidth_dram / parts_c / mem_eff;

subject to SABufferSendCyclesDef:
  SABufferSendCycles = max(AM*AK, AK*AN) * bitwidth_data / bitwidth_plio;

subject to SACompCyclesDef:
  SACompCycles = max(SABufferSendCycles, AIECyclesPL);

subject to PLReuseDef:
  PLBufferReuse = BM2 * BK2 * BN2;

subject to PLBufferCompCyclesDef:
  PLBufferCompCycles = SACompCycles * PLBufferReuse;

subject to PLBufferCyclesDef:
  PLBufferCycles = max(PLBufferReadCycles, PLBufferCompCycles);
  #PLBufferCycles = max(PLBufferReadCycles, PLBufferCompCycles, PLBufferWriteCycles);

subject to PLCyclesDef:
  PLCycles = PLBufferReadCycles + PLBufferCycles*BM1*BK1*BN1 + PLBufferWriteCycles;

################################################################################
# CONSTRAINTS
################################################################################

# dimensions
subject to FixedRC:
  fixed_rc * (PhysRows - r) = 0 and fixed_rc * (PhysCols - c) = 0;

subject to FixedPL:
  fixed_pl * (PM - pm) = 0 and fixed_pl * (PK - pk) = 0 and fixed_pl * (PN - pn) = 0;

subject to FixedAIE:
  fixed_aie * (AM - am) = 0 and fixed_aie * (AK - ak) = 0 and fixed_aie * (AN - an) = 0;

subject to EnableSplit:
  SplitRows = 1 + enable_split * (SplitRows - 1);

subject to PhysRowsDef:
  PhysRows = Rows / SplitRows;

subject to PhysColsDef:
  PhysCols = Cols * SplitRows;

subject to FoldableSA:
  PhysRows <= max_r and PhysCols <= max_c;

subject to EvenArray:
  Rows = 2*Int0 and Cols = 2*Int1;

subject to SizeOrder:
  2*m > PadM >= PM >= AM and 2*k > PadK >= PK >= AK and 2*n > PadN >= PN >= AN;

subject to PDividesPad:
  PadM = PM*BM1 and PadK = PK*BK1 and PadN = PN*BN1;

subject to ADividesP:
  PM = Rows*AM*BM2 and PK = AK*BK2 and PN = Cols*AN*BN2;

subject to VlenDividesA:
  AM = aie_vlen*BM3 and AK = aie_vlen*BK3 and AN = aie_vlen*BN3;

# memory
subject to FixedBRAM:
  fixed_bram * (AOnBRAM - bram_a) = 0 and fixed_bram * (BOnBRAM - bram_b) = 0 and fixed_bram * (COnBRAM - bram_c) = 0;

subject to AIEBytesLimit:
  AIEBytes <= aie_mem_bytes;

subject to PLBRAMUnitsLimit:
  PLBRAMUnits <= bram_count * bram_util;

subject to PLURAMUnitsLimit:
  PLURAMUnits <= uram_count * uram_util;

## pl
#subject to PLOverlap:
#  max(PLBufferReadCycles, PLBufferWriteCycles) <= PLBufferCompCycles;

# aie
subject to AIEFlush:
  PK / AK >= Rows;

################################################################################
# OBJECTIVE FUNCTION
################################################################################

minimize Latency:
  PLCycles;

#minimize MinimizeHostBytes:
#  HostBytes;
#
#maximize MaximizeAIEBytes:
#  AIEBytes;
#
#maximize PreferBRAM:
#  AOnBRAM + BOnBRAM + COnBRAM;
