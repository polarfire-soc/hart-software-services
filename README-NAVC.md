## How to apply configuration file to HSS bootloader

- - - -

1. Set configuration for definition

        $ cd $(HSS source)
        $ cp boards/mpfs-navc/def_config .config # When navc firmware should be compiled

2. Choose build options if some features are needed to be changed

        $ make BOARD=mpfs-navc menuconfig

3. Build

        $ make BOARD=mpfs-navc -j 1

- - - -

## MSS Configuration Files
To modify hardware configuration, open NAVC_MSS.cfg file with MSS configuration tool.
And, save changed values, and click generate button. 
As a result, new below files are updated automatically.

<pre>
mpfs-navc/mss_config/
├── NAVC_MSS.cfg
├── NAVC_MSS.cxz
├── NAVC_MSS_Report.html
└── NAVC_MSS_mss_cfg.xml
</pre>

- - - -
