# HSS-over-JTAG
During development it might be useful to run HSS in boot mode 0 and without need to write HSS executable to eNVM. HSS itself is running from L2 scratchpad but there are some initialization steps required for HSS to execute correctly. In bootmodes 1..3 that initialization is performed by `envm-wrapper`. In bootmode 0 `jtag_trampoline` with OpenOCD script can be used to achieve the same results.

## Running HSS-over-JTAG
1. Build HSS binaries.
2. Connect debug adapter to target
3. Execute OpenOCD command: `openocd -f <init script> -c "set HSS_DIR <hart-software-services>/Default" -f <hart-software-services>/Default/run-hss.cfg`
4. HSS will be started


## Details of operation
1. `hss-jtag_trampoline.hex` is loaded into E51 DTIM and E51 ITIM memories
2. Harts are resumed starting from address `0x01800000` (E51 ITIM). OpenOCD hangs on `wait_halt` command.
3. HSS initialization steps are performed: L2 scratchpad configuration, clocks setups
4. `ebreak` instruction is invoked. OpenOCD command `wait_halt` ends and resumes script.
5. `hss-l2scratch.bin` is loaded into L2 scratchpad area.
6. Harts are resumed from address `0x0A000000`.
7. HSS is running
