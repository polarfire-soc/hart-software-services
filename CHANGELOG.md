# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## 2024.02

### Added

 * HSS: crypto: User Crypto support added - supports AES (ECB, CBC, OFB, CFB and CTR modes)

 * HSS: tinycli: Added simple ECC stats to tinyCLI.

 * HSS: boot: Restructured early initialization as a state machine, and offloaded DDR training to U54_1 for faster boot-up.

### Changed

 * HSS: scrub: improved scrubbing of LIM, U54 I$/VMA TLBs. Disabled scrubbing by default as it
   is incompatible with non-coherent PCIe speed-ups enabled by default in MPFS Linux.

 * HSS: tinycli: Simplified early pre-boot CLI to use regular tinyCLI service.

 * HSS: sbi: Tidy-up of ECALL returns

 * HSS: boot: Simplified boot logo to save time.

 * XML: Updated XML for mpfs-video-kit and mpfs-icicle-kit-es

### Fixed

 * HSS: init: reset reason type is incorrect

 * Docs: fixed web link to software flow documentation

 * HSS: build: moved build assets to $(BINDIR) directory (from "Default" to "build") and fix make clean on Windows

 * HSS: init: Detect if MPU config prevents USBDMSC

 * QSPI: build issue with QSPI-enabled `def_config` files resolved

## 2023.09

### Added

 * HSS: opensbi: added unique SBI impid to allow supervisor mode software differentiate HSS from OpenSBI

 * HSS: boards: added PolarBerry support

 * HSS: uart: allow extra serial port configuration (via Kconfig)

### Changed

 * HSS: boards: Update to Aries m100pfsevp board support

 * HSS: slight refactoring of uart code

### Fixed

 * HSS: post-boot detection

 * HSS: ymodem: increased timeout for more reliable behaviour

 * HSS: ancilliary data: re-register harts after downloading to correctly pass ancilliary data


## 2023.06

### Added

 * HSS: Auto Update: added support to mpfs-video-kit (not supported on icicle-kit-es)

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

   - Using mpfs-hal to switch MSSIOs for SD/eMMC switchover

    - `SDIO_REGISTER` renamed to `FABRIC_SD_EMMC_DEMUX_SELECT`

 * HSS: QSPI: add support for booting from Micron MT25Q

 * HSS: eMMC: fixes for HS200 / HS400

 * HSS: remoteproc: ecall support for remoteproc - the ability to start and stop a
   remote AMP context using the remoteproc framework in Linux.

    - Payload Generator: add `skip-autoboot` flag to YAML to skip automatically
      loading and booting a payload on a per-context basis

    - IHC: various related ecall fixes

 * HSS: reboot: add context-based reboot support (via SRST)

    - Kconfig: add `ALLOW_COLDREBOOT` option to enable cold reboot

    - Payload Generator: add `allow-reboot` flag to YAML to control entitlement to
      warm/cold reboot

 * HSS: ddr: add Kconfig option to skip training DDR

 * HSS: ddr: early output status of DDR training progress

 * Payload Generator: add ability to override context boot image name in YAML


### Changed

 * HSS: save space by enabling LTO and whole program optimization

   - Note this means disabling strong stack protection.

   - Removed function names from console output to save space.

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

   - This requires python3 tkinter bindings on Linux. For Ubuntu/Debian, use

    $ sudo apt install python3-tk

### Changed

 * HSS: DDR: Configured for High-Memory DDR (0x10'0000'0000) instead of Low-Memory (0x8000'0000)

   - **WARNING:** Pre-existing code will break if not re-linked for the new DDR address

 * HSS: L2: Move decompressed HSS from L2LIM to L2-Scratchpad


### Fixed

 * Payload Generator: updated to ignore zero-byte sections

 * Payload Generator: enhance error checks, and ensure string concatenation always safe for names
