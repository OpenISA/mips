OpenISA ArchC functional model
=====

acsim
-----

    acsim openisa.ac -abi -nw             (create the simulator)
    make                               (compile it)
    openisa.x --load=hello                (compile and run hello world program)

Binary utilities
----------------
To generate binary utilities use:

    acbingen.sh -i<abs-install-path> -a<arch-name> openisa.ac

This will generate the tools source files using the architecture
name <arch-name> , copy them to the
binutils source tree, build and install them into the directory
<abs-install-path> (which -must- be an absolute path).
Use "acbingen.sh -h" to get information about the command-line
options available.


Change history
------------

See [History](HISTORY.md)


Contributing
------------

See [Contributing](CONTRIBUTING.md)


