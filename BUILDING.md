# Building V1541Commander from source

## Dependencies

The following libraries are needed for building:

* Qt5 (recent versions preferred)
* lib1541img (see below)

To get the most recent source, you will need `git`.

## Getting and installing lib1541img

To get the source from git, type the following

    git clone https://github.com/excess-c64/lib1541img
    cd lib1541img
    git submodule update --init

If you want a specific version, e.g. 0.9, do the following

    git clone https://github.com/excess-c64/lib1541img
    cd lib1541img
    git checkout v0.9
    git submodule update --init

To build and install lib1541img, the following commands should be enough:

    make -j4
    make install

This will install to `/usr/local`, so for `make install`, you will need root
privileges.

The build system will understand standard variables like `prefix` and
`DESTDIR`.

## Getting and installing V1541Commander

To get the source from git, type the following

    git clone https://github.com/excess-c64/v1541commander
    cd v1541commander
    git submodule update --init

If you want a specific version, e.g. 1.0, do the following

    git clone https://github.com/excess-c64/v1541commander
    cd v1541commander
    git checkout v1.0
    git submodule update --init

To build and install v1541commander, the following commands should be enough:

    make -j4
    make install

This will install to `/usr/local`, so for `make install`, you will need root
privileges.

The build system will understand standard variables like `prefix` and
`DESTDIR`.

## Notes on non-GNU systems like FreeBSD

The build system needs GNU make to work, so make sure you have it installed.
On systems where the default `make` isn't GNU make, GNU make is typically
called `gmake`. Therefore, replace all invocations of `make` with `gmake`.

## Notes on Windows

To build the software yourself for Windows, it's recommended to either use a
cross-toolchain like [MXE](https://mxe.cc/) or install
[MSYS2](https://www.msys2.org/) on your Windows machine. With MSYS2, building
should work like described above. You will have to place all the required
DLLs (Qt, its dependencies, and lib1541img) in the application directory for
it to work standalone.

Static linking is possible as well, for an idea where to start, see the
`build-static-mxe-win32.sh` script I use to create a static build with MXE.

