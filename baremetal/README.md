# Subtrees

`polarfire-soc-bare-metal-library` is a git subtree.

It was created as a subtree using:

    $ git remote add -f polarfire-soc-bare-metal-library https://bitbucket.microchip.com/scm/fpga_pfsoc_es/polarfire-soc-bare-metal-library.git
    $ git subtree add --prefix baremetal/polarfire-soc-bare-metal-library polarfire-soc-bare-metal-library master --squash

To update the bare metal drivers, use:

    $ git fetch polarfire-soc-bare-metal-library master
    $ git subtree pull --prefix baremetal/polarfire-soc-bare-metal-library polarfire-soc-bare-metal-library master --squash

## Notes

The `examples` and `config` subdirectories were removed.
