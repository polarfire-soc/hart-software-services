# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

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
