# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## 2026.04

### Added

 * HSS: feat(mss-i2c): add non-blocking poll API

    -  `MSS_I2C_poll()` advances the I2C state machine one step if SI is set and
       returns current `master_status`

    -  `MSS_I2C_poll_complete()` returns true once the transfer leaves `IN_PROGRESS`
       state

 * HSS: feat(boot): allow null AMP payload (for harts that need no initial image)

 * Payload Generator: feat(hss-payload-gen): add null payload support

 * HSS: feat(mpfs-beaglev-fire): reduce TinyCLI auto-boot timeout to 1s

 * HSS: feat(bsp): improve BeagleV-Fire boot logo detail

 * HSS: feat(suspend): add `SBI_EXT_SUSP` system suspend/resume support

    -  waits for secondary harts to reach Idle, enters DDR self-refresh, spins on
       `EVENT_SYSTEM_SUSPEND_RESUME` trigger, exits self-refresh, and returns harts
       to Linux via `IPI_MSG_GOTO`

    -  U54 watchdog monitoring suppressed during suspend

### Changed

 * HSS: refactor(envm-wrapper): derive struct offsets from `hss_types.h` via `asm_offsets.c`,
   add decompression bounds checks and UART error spin-loops on failure

 * HSS: refactor(sgdma): add chain length limit and DDR address validation before accepting
   SGDMA IPI transfers

 * HSS: refactor(gpt): improve GPT partition table validation

 * HSS: refactor(uart): remove unused UART service

 * HSS: refactor(beu): on uncorrectable double-bit ECC error, trigger the hart's
   watchdog to force a reboot rather than continuing with potentially corrupt state

    -  implemented via a weak `beu_handle_double_fault()` that BSP code can override

    -  also use explicit state assignment instead of fragile `state++`

 * HSS: refactor(build): switch to xPack GNU RISC-V Embedded GCC toolchain

    -  SoftConsole-bundled GCC 8.3 support deprecated and will be removed in a
       future release

    -  xPack toolchains are significantly newer with bug fixes and encoding
       improvements

### Fixed

 * HSS: fix(mss-util): `sleep_ms()` was passing the millisecond argument directly as a
   raw tick delta

    -  multiply by `ONE_MILLISEC` (derived from `LIBERO_SETTING_MSS_RTC_TOGGLE_CLK`)
       so the delay is correct

 * HSS: fix(sgmii): duplicate clock/reset configuration of MAC0 resolved

 * HSS: fix(boot, opensbi): validate U-mode-supplied pointers in boot IPI handler, IHC
   ecall, and IPC ecall paths to prevent arbitrary M-mode writes or OOB accesses

 * HSS: fix(boot): improve checks for rproc buffer and hart ID range
   add NULL/DDR validation of extended buffer before dereferencing

 * HSS: fix(qspi): add bounds checks on block offset and count before `uint32_t` cast
   guard logical-to-physical map lookups against out-of-range block numbers

 * HSS: fix(storage): NULL writeBlock pointer check and fix `size_t` truncation issues

 * HSS: fix(ymodem): fix `size_t` overflow in file-size calculation and ensure filename
   is null-terminated

 * HSS: fix(hss-state-machine): maxState was recording prevState instead of the
   executing state, giving incorrect slowest-state diagnostics

 * HSS: fix(hss-clock): `HSS_Timer_IsElapsed()` could misfire on tick-count wrap

 * HSS: fix(ssmb): add memory barrier before CLINT MSIP write

 * HSS: fix(healthmon): validate `checkName[]` array index before use

 * Payload Generator: fix(payload-gen): avoid binary corruption on Windows due to line-ending
   translation in stdio

 * HSS: fix(ddr): correct number of DDR training states

 * HSS: fix(random): random offset was never incremented between reads

 * HSS: fix(hss-init): shift operand must be unsigned to avoid undefined behaviour

 * HSS: fix(tinycli): monitor timestamps not updated for monitor slots `> 0`

 * HSS: fix(uncompress): decompression failure was silently ignored

 * HSS: fix(state machine): add bounds check on `currentState` index before dispatch

 * HSS: fix(hss-registry): add missing `%s` format argument to debug print

 * HSS: fix(memset): size argument was element count rather than byte count

 * HSS: fix(crt.s): hartid bounds check must precede stack pointer calculation

 * HSS: fix(SBI): prevent PLIC reset during Linux boot

 * HSS: fix(build): guard against parallel make dependency races

 * HSS: fix(build): extract and display correct version string

 * HSS: fix(disco-kit): correct linker script region-name typo

 * HSS: fix(platform): correctly re-enable M-mode software interrupts before
   warmboot wait

    -  use `csr_set(CSR_MSTATUS, MSTATUS_MIE)` instead of
       `csr_write(CSR_MSTATUS, MIP_MSIP)` which was zeroing MPP, MPIE and other fields

 * HSS: fix(boot): remove incorrect "to DDR" references in boot messages

    -  target address may be L2-scratchpad or other memory, not necessarily DDR

 * HSS: fix(sbi): report SBI version 2.0 to allow Linux to probe `SBI_EXT_SUSP`

    -  all SBI 2.0 feature extensions are optional so existing clients are unaffected

 * HSS: upgrade(mpfs-hal): updating to mpfs-hal v2.3.110

    -  fixes display of unit for DDR DRV settings

    -  fixes write latency retry causing failure to boot

    -  fixes typos in comments and other minor fixes

 * HSS: fix(scrub): fix typo in scrub service

 * HSS: fix(tinycli): post-boot RESUME command warning

## 2025.07

### Added

 * HSS: feat(IHC): Add support for Mi-V IHC v2 IP (available from Libero catalog),
                   deprecate support for IHC v1.

 * HSS: feat(BSP): add boards directory for `mpfs-icicle-kit` with production silicon

 * HSS: feat(dtb): add minimal device trees for qspi NAND and NOR flash (for use with U-Boot)

 * HSS: feat(ihc): add support for the the Mi-V IHC IP driver v2

    - Starting from the 2025.07 release and onwards, the Icicle Kit reference
      design will use the Mi-V IHC IP available in the Libero catalog.

      The Mi-V IHC IP version 2 is not backwards compatible with the Mi-V IHC
      subsystem used in the Icicle Kit reference design 2025.03 or earlier. For
      this reason, the AMP support in the v2025.07 release will not be backwards
      compatible with previous releases.

     - Update the compatible string in the Icicle Kit device tree source and
       blob to the latest 2025.07 reference design, which includes the new
       MiV-IHC IP v2.

 * HSS: feat(boot): allow null AMP payload (for harts that need no initial image)

 * Payload Generator: feat(hss-payload-gen): add null payload support

 * HSS: feat(mpfs-beaglev-fire): reduce TinyCLI auto-boot timeout to 1 s

 * HSS: feat(bsp): improve BeagleV-Fire boot logo detail

### Changed

 * HSS: chore(README): update README for Windows build (python details)

 * HSS: refactor(design-version-info): make functions weak

### Fixed

 * HSS: fix(USBDMSC): ensure USB OTG is re-initialized correctly when block is reset.

 * HSS: fix(SBI): fix mismatch in domain memory region order

 * HSS: fix(SBI): prevent PLIC reset during Linux boot

 * HSS: fix(UART): Fix issue with `UART_SURRENDER` state transitions

 * HSS: fix(SC-SPI): Fix data corruption on System Controller SPI

 * HSS: fix(build): guard against parallel make dependency races

 * HSS: fix(build): extract and display correct version string

 * HSS: fix(disco-kit): correct linker script region-name typo


## 2025.03

### Added

 * HSS: feat(crypto): Add user crypto support

 * Payload Generator: feat(hss-payload-gen): Update README

 * Payload Generator: feat(hss-payload-gen): Add verify to code signing

### Changed

 * HSS: trivial: update copyright to 2025

 * HSS: chore(polarberry): tidy comment, remove weak bind

 * HSS: chore(README): update command to program Video Kit

 * HSS: refactor(build): include `DIE` and `PACKAGE` arguments

### Fixed

 * Payload Generator: fix(hss-payload-gen): Order of YAML issue

 * HSS: fix(mpfs-beaglev-fire): ensure payload gets initiaized

 * HSS: fix(mpfs-beaglev-fire): Add SD/emmc demux


## 2024.09

### Added

 Not applicable for this release.

### Changed

 * HSS: healthmon: Ensure that health monitoring monitor arrays are board/design specific

 * HSS: README: update command to program Video Kit

 * HSS: build: include `DIE` and `PACKAGE` arguments for `mpfs-video-kit`


### Fixed

 * HSS: ymodem: Ensuring watchdog doesn't fire if `YMODEM` is running.

 * HSS: boot: Fixing some build errors when `CONFIG_SERVICE_BOOT` is not enabled.

 * HSS: build: ensure -fwhole-program optimizations are not enabled when strong stack protection is enabled.


## 2024.06

### Added

 * HSS: hal: updating to mpfs-hal v2.3.102

 * HSS: lockdown: adding lockdown service (allows custom restrictions of HSS after booting)

    - This provides an opportunity/single place in the code to modify behavior
    once boot has completed and the user wishes to restrict what the E51/HSS can
    subsequently do using PMPs/restricted set of state machines to service.

 * HSS: info: update print design version info at startup

 * HSS: trigger: add event triggers to allow state machines synchronize with each other

 * HSS: boot: provide minimal common DTB for U-Boot purposes

### Changed

 * HSS: boot: don't delay for DDR training if not needed for a Hart

 * HSS: envm-wrapper: determine HSS UART from BSP rather than hardcoding

 * HSS: sbi: removed unnecessary optimization (O0)

 * HSS: usbdmsc: refactored to make USBDMSC a proper service

### Fixed

 * HSS: build: fixed minor issues related to build directory change

 * HSS: healthmon: throttle messages to prevent output flood


## 2024.02.1

### Added

 * HSS: boards: add support for BeagleV Fire BSP

 * HSS: boards: add support for PolarFire SoC Discovery Kit (`mpfs-disco-kit`)

 * HSS: info: print design version info at startup

 * HSS: MMC: make MMC/SD speed configurable per BSP

 * HSS: gpio-ui: Add GPIO/LED UI service for platforms without serial console

### Changed

 * HSS: init: turn on all FICs by default

### Fixed

 * HSS: usbdmsc: fix bug in USBDMSC startup

 * HSS: ihc: avoid writes through NULL pointer

 * HSS: scrub: ensure all RAMs can be turned off

 * HSS: spi: fix issues when booting from System Controller SPI Flash


## 2024.02

### Added

 * HSS: crypto: User Crypto support added - supports AES (ECB, CBC, OFB, CFB and CTR modes)

 * HSS: tinycli: Added simple ECC stats to tinyCLI.

 * HSS: boot: Restructured early initialization as a state machine, and offloaded DDR training to `U54_1` for faster boot-up.

### Changed

 * HSS: scrub: improved scrubbing of LIM, U54 I$/VMA TLBs. Disabled scrubbing by default as it
   is incompatible with non-coherent PCIe speed-ups enabled by default in MPFS Linux.

 * HSS: tinycli: Simplified early pre-boot CLI to use regular tinyCLI service.

 * HSS: sbi: Tidy-up of ECALL returns

 * HSS: boot: Simplified boot logo to save time.

 * XML: Updated XML for `mpfs-video-kit` and `mpfs-icicle-kit-es`

### Fixed

 * HSS: init: reset reason type is incorrect

 * Docs: fixed web link to software flow documentation

 * HSS: build: moved build assets to `$(BINDIR)` directory (from "Default" to "build") and fix make clean on Windows

 * HSS: init: Detect if MPU config prevents USBDMSC

 * QSPI: build issue with QSPI-enabled `def_config` files resolved


## 2023.09

### Added

 * HSS: opensbi: added unique SBI impid to allow supervisor mode software differentiate HSS from OpenSBI

 * HSS: boards: added PolarBerry support

 * HSS: uart: allow extra serial port configuration (via `Kconfig`)

### Changed

 * HSS: boards: Update to Aries m100pfsevp board support

 * HSS: slight refactoring of uart code

### Fixed

 * HSS: post-boot detection

 * HSS: ymodem: increased timeout for more reliable behaviour

 * HSS: ancilliary data: re-register harts after downloading to correctly pass ancilliary data


## 2023.06

;### Added

 * HSS: Auto Update: added support to `mpfs-video-kit` (not supported on `mpfs-icicle-kit-es`)

 * HSS: healthmon: added generic health monitoring service

### Changed

 * HSS: fixed various compiler warnings

 * HSS: envm-wrapper: improved boot mode selection

 * HSS: mpfs-hal: updated to v2.2.104

 * HSS: opensbi: updated to v1.2 (SBI v1.0)

 * HSS: sys-services: Updated driver to v2.1.100

### Fixed

 * HSS: profiling: refactored, fixed build issue, added user tool, and added documentation on use

 * HSS: mmc: add delay before sending status command, to fix SD failure issue

 * Docs: updated broken link in contribution docs


## 2023.02

### Added

 * HSS: moved default payload.bin destination in DDR to section now reserved in dtbs

 * HSS: mpfs-hal: updated mpfs-hal to v2.2.100

 * HSS: watchdog: support for rebooting individual AMP contexts

 * HSS: reboot: configurable support for cold reboot on fault

 * HSS: sbi: fake mvendorid/marchid/mimpid (to allow Linux driver quirks)

 * HSS: tinycli: allow selection of eMMC, SDCARD, or SDCARD to eMMC as boot medium

 * HSS: beu: errors now logged to HSS console when detected

### Changed

 * HSS: memtest: use common routine to interrupt memory test

 * HSS: uart: add hook to control UART assigned per hart

 * HSS: tinycli: refactor to use command command dispatcher

 * HSS: build: give more meaningful errors if files missing for builds

### Fixed

 * HSS: reboot: back-to-back reboots would eventually fill queues

 * HSS: envm-wrapper: improvements to avoid cache bugs on startup

 * HSS: scrub: fix bug in calculating end of range


## 2022.09

### Added

 * HSS: OpenSBI: updated to OpenSBI v1.0 (SBI v0.3)

 * HSS: tiny-cli: Added "debug opensbi" command, along with U54 state tracking

 * HSS: mpfs-hal: updated to mpfs-hal v2.0.101

    -  Using mpfs-hal to switch MSSIOs for SD/eMMC switchover

    -  `SDIO_REGISTER` renamed to `FABRIC_SD_EMMC_DEMUX_SELECT`

 * HSS: QSPI: add support for booting from Micron MT25Q

 * HSS: eMMC: fixes for HS200 / HS400

 * HSS: remoteproc: ecall support for remoteproc - the ability to start and stop a
   remote AMP context using the remoteproc framework in Linux.

    -  Payload Generator: add `skip-autoboot` flag to YAML to skip automatically
       loading and booting a payload on a per-context basis

    -  IHC: various related ecall fixes

 * HSS: reboot: add context-based reboot support (via SRST)

    -  Kconfig: add `ALLOW_COLDREBOOT` option to enable cold reboot

    -  Payload Generator: add `allow-reboot` flag to YAML to control entitlement to
       warm/cold reboot

 * HSS: ddr: add Kconfig option to skip training DDR

 * HSS: ddr: early output status of DDR training progress

 * Payload Generator: add ability to override context boot image name in YAML

### Changed

 * HSS: save space by enabling LTO and whole program optimization

    -  Note this means disabling strong stack protection.

    -  Removed function names from console output to save space.

### Fixed

 * Payload Generator: secure-boot: update from deprecated OpenSSL APIs

 * HSS: platform: fixed potential NULL lookup of fdt


## 2022.03

### Added

 * HSS: QSPI: Add support for booting from Winbond W25N01GV flash

### Changed

 * HSS: resets: bringing all peripherals out of reset

 * HSS: scrub: add throttling support to memory scrubber

### Fixed

 * HSS: board/custom-board-design: update to build with latest HSS code base

 * HSS: boot: Resolve out-of-bounds issue


## 2022.02

### Added

 * HSS: secure-boot: image signing for payload.bin

 * Payload Generator: secure-boot: support for image signing payload.bin

 * HSS: m100pfsevp: Aries Board Support Package added

 * HSS: Added support for Kconfiglib guiconfig tool

    -  This requires python3 tkinter bindings on Linux. For Ubuntu/Debian, use

     $ sudo apt install python3-tk

### Changed

 * HSS: DDR: Configured for High-Memory DDR (0x10'0000'0000) instead of Low-Memory (0x8000'0000)

    -  **WARNING:** Pre-existing code will break if not re-linked for the new DDR address

 * HSS: L2: Move decompressed HSS from L2LIM to L2-Scratchpad


### Fixed

 * Payload Generator: updated to ignore zero-byte sections

 * Payload Generator: enhance error checks, and ensure string concatenation always safe for names
