create_pblock pblock_vnoc0
create_pblock pblock_vnoc1
create_pblock pblock_vnoc2
create_pblock pblock_vnoc3

set_property PARENT pblock_dynamic_region [get_pblocks pblock_vnoc*]
set_property CONTAIN_ROUTING true [get_pblocks pblock_vnoc*]

resize_pblock [get_pblocks pblock_vnoc0] -add {NOC_NMU512_X0Y0:NOC_NMU512_X0Y4}
add_cells_to_pblock -clear_locs [get_pblocks pblock_vnoc0] [get_cells top_i/ulp/axi_noc_kernel0/inst/S03_AXI_nmu/*/NOC_NMU512_INST]

resize_pblock [get_pblocks pblock_vnoc1] -add {NOC_NMU512_X1Y0:NOC_NMU512_X1Y6}
add_cells_to_pblock -clear_locs [get_pblocks pblock_vnoc1] [get_cells top_i/ulp/axi_noc_kernel0/inst/S00_AXI_nmu/*/NOC_NMU512_INST]

resize_pblock [get_pblocks pblock_vnoc2] -add {NOC_NMU512_X2Y0:NOC_NMU512_X2Y6}
add_cells_to_pblock -clear_locs [get_pblocks pblock_vnoc2] [get_cells top_i/ulp/axi_noc_kernel0/inst/S01_AXI_nmu/*/NOC_NMU512_INST]
add_cells_to_pblock -clear_locs [get_pblocks pblock_vnoc1] [get_cells top_i/ulp/axi_noc_kernel0/inst/S04_AXI_nmu/*/NOC_NMU512_INST]

resize_pblock [get_pblocks pblock_vnoc3] -add {NOC_NMU512_X3Y0:NOC_NMU512_X3Y6}
add_cells_to_pblock -clear_locs [get_pblocks pblock_vnoc3] [get_cells top_i/ulp/axi_noc_kernel0/inst/S02_AXI_nmu/*/NOC_NMU512_INST]
add_cells_to_pblock -clear_locs [get_pblocks pblock_vnoc3] [get_cells top_i/ulp/axi_noc_kernel0/inst/S05_AXI_nmu/*/NOC_NMU512_INST]

