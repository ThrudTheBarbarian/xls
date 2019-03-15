connect -url tcp:127.0.0.1:3121
source /media/psf/raid/simon/src/verilog/xls/xls.sdk/design_1_wrapper_hw_platform_0/ps7_init.tcl
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Platform Cable USB II 0000174c743a01"} -index 0
loadhw -hw /media/psf/raid/simon/src/verilog/xls/xls.sdk/design_1_wrapper_hw_platform_0/system.hdf -mem-ranges [list {0x40000000 0xbfffffff}]
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Platform Cable USB II 0000174c743a01"} -index 0
stop
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "ARM*#0" && jtag_cable_name =~ "Platform Cable USB II 0000174c743a01"} -index 0
rst -processor
targets -set -nocase -filter {name =~ "ARM*#0" && jtag_cable_name =~ "Platform Cable USB II 0000174c743a01"} -index 0
dow /media/psf/raid/simon/src/verilog/xls/xls.sdk/testDisplay/Debug/testDisplay.elf
configparams force-mem-access 0
bpadd -addr &main
