The HealthMon service is a service that automatically checks an array of monitors for various out of bounds 
or exceptional value conditions on every superloop of the HSS.

It relies on the following weakly-bound data structures

    const struct HealthMonitor monitors[];
    struct HealthMonitor_Status monitor_status[];
    const size_t monitors_array_size;

The intention is that the board subdirectory selected will provide an implementation for these variables. If none is
provided, weakly bound versions will ensure the system links, but no health monitoring will be performed.

`monitors` is specific to each board/design, and `monitor_status` and `monitors_array_size` are derived from monitors.
As an example of use, please see `boards/mpfs-icicle-kit-es/healthmon_monitors.c`
