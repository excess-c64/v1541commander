# V1541Commander

This application allows you to create and modify D64 disk images (for the
Commodore 1541 floppy drive).

For licensing information, see LICENSE.txt and LICENSE-font.txt.

For instructions to build this yourself on your system, see BUILDING.md

For downloads of the latest binaries for Windows or Linux, search
[CSDb](https://csdb.dk/) for "v1541commander".

## Features

* Read and write D64 disk images
* Support for 40- and 42-track images
* Support for BAM formats of SpeedDOS, DolphinDOS and PrologicDOS
* Import and export C64 files either as raw content or in PC64 container
  format (.P00/.U00/...)
* Compress and extract 4-pack and 5-pack ZipCode
* ZipCode files on D64 disk images
* Create and import LyNX archive files

### Not supported

* raw GCR images (.g64)
* GEOS files
* any trackloading mechanisms used in many games and demos

## Commandline

Usage: `v1541commander [-hv] [file ...]`

* `-v`: print program version
* `-h`: print help text
* `[file ...]`: files given on the commandline are opened at startup

## Creating a new D64

Select *New* from the *File* menu to create a new D64 image. The filesystem
options dialog will show. Among other options, it allows you to select extended
BAM formats. The BAM is the block availability map, which is used by CBM DOS to
know which blocks on disk are free and which are used. The default format only
covers 35 tracks, so you probably want to select an extended format if you
create a 40 tracks disk.

Here's what you can configure in the filesystem options:

* 35 tracks: a standard filesystem as used by original Commodore DOS
* 40 tracks: a larger filesystem as used by many Speeder replacement DOS
  versions
* 42 tracks: the maximum size possible, you normally shouldn't use this as
  there is no BAM format for tracks 41 and 42 and many real drives can't
  properly read these tracks.
* directory interleave: how many blocks are skipped between two blocks of the
  directory, the default value is 3, it is recommended to keep this.
* default file interleave: how many blocks are skipped between two blocks of
  a file, the default value is 10, it is recommended to keep this. However, if
  you write files for reading with a non-standard protocol (a software
  speeder), a different value might be better.
* DolphinDOS BAM: write extended BAM in the format used by DolphinDOS
* SpeedDOS BAM: write extended BAM in the format used by SpeedDOS. This can be
  combined with DolphinDOS to write both formats simultaneously.
* PrologicDOS BAM: write extended BAM in the format used by PrologicDOS. This
  can not be combined with the other formats and the resulting disk won't be
  read correctly by the original DOS, because in PrologicDOS, the location of
  the disk name and id is changed.
* allow long directories: with this option set, the directory can contain more
  than 144 files. This is only partially compatible with original DOS: It can
  read such a disk, but it will corrupt the filesystem when trying to write to
  it, so be warned.
* place files on dir track: this is kind of the opposite, allowing files to use
  unused space that's normally reserved to the directory. The same notes about
  compatibility with original DOS apply.
* report 0 blocks free: with this option set, the disk will show "0 blocks
  free." in the directory. The BAM will still stay intact, though.

## Opening a file

Select *Open* from the *File* menu to open a file. The following types are
supported:

* .d64: a disk image. This is opened for editing "in place", but of course you
  can also use "save as" to work on a copy.
* 1!xxx.prg: a member file of a ZipCode fileset. The set is extracted and the
  resulting .d64 is opened as a new image.
* .lnx: a lynx archive. The files contained are extracted and placed on a new
  image.

Note that only ZipCode files are detected from their name. D64 and Lynx files
are detected from their contents.

If you open a .d64 directly or extract one from ZipCode, matching filesystem
options are detected and the filesystem options dialog is shown with these
options preselected. It is *highly* recommended to keep the options. Under
special circumstances, you might want to override some, but be aware they might
not immediately apply.

If ZipCode files are found *inside* a .d64, v1541commander will offer you to
extract these instead of opening the .d64.

If a D64 is somehow readable, but corrupted, you will get the possibility to
"recover" it. In this case, whatever is recovered while trying to read it is
treated as a new image.

*Note*: if you already have a file opened, opening another one will create a
new application window.

## Saving a file

*Save* and *Save As* from the *File* menu work exactly as you would expect.
There's only one thing to know: If your disk is "overfull" or in an otherwise
inconsistent state, you can't save, and you will get a dialog stating the
problems and giving some suggestions how to get to a consistent state.

For a better explanation, see the section *How it works (internally)*.

## Export

The *Export* submenu from the *File* menu gives you several options:

* Zipcode: export as ZipCode compressed fileset. If the filename you choose
  doesn't already start with "1!" to "4!", this will be automatically prepended
  to the names of the individual files. If you don't specify an extension,
  .prg is used.
* Zipcode (D64): export as ZipCode compressed fileset on a new D64 image. A
  new image will open containing properly named ZipCode files.
* LyNX: export files as a Lynx archive. This will disregard .DEL files, as
  Lynx can only store "real" files.

## Changing filesystem options

Select *Filesystem Options* from the *CBM DOS* menu to change the options of
an opened d64 image. Some changes will require to recreate the disk image from
scratch, like e.g. changing the number of tracks. For a description of the
options, see "Creating a new D64".

## Rewriting an image

Select *Rewrite Image* from the *CBM DOS* menu to rewrite the entire image from
scratch. This doesn't save anything to disk, but replaces the image in memory
with a new one. You might want to do that after doing a lot of changes to a
disk -- rewriting all files in the order they appear in the directory will put
them in the best possible layout (think "defragment disk"). It might also fix
issues with corrupted filesystems.

## Adding, deleting and moving files on an image

You can add and delete files from the *CBM DOS* menu or by right-clicking on
the directory view. To move a file up or down in the directory, drag it with
the mouse in the directory view.

## Editing Disk properties

On the top right of the directory view, you can edit some disk properties:

* Name: The disk name, up to 16 PETSCII chars
* ID: The disk ID, up to 5 PETSCII chars. When this is longer than 3 chars, it
  overwrites the "dos signature" ("2A", on PrologicDOS "2P")
* DOS Version: the DOS version byte, defaults to 0x41 or 0x50 on PrologicDOS.
  If you set this to any other non-zero value, the effect is that the original
  drive will refuse to write to the disk.

See also: PETSCII input

## Editing File Properties

Below the disk properties, you can edit the properties of a currently selected
file:

* Name: The file name, up to 16 PETSCII chars. Using a shifted space in a name
  confuses the original DOS, as it serves as an "end mark", so anything after
  it will appear in the directory outside the quotes and not be considered
  part of the name.
* Type: Select the CBM DOS file type here.
* locked and closed: flags of a file.
* record length: Only used when the file type is REL, tells how long one
  "record" of the file content is.
* forced block size: Use this to display a "wrong" block size of the file in
  the directory.
* Content: here you can import and export the file contents. Supported formats
  are plain files and PC64 container files (.P00/.U00/...). The PC64 format
  has the advantage that it also stores the original PETSCII filename as well
  as, for REL files, the record length.

## PETSCII input

Input fields containing PETSCII will use the C64 Pro Mono font by STYLE and
provide extended means of input. Plain and shifted chars should work quite
similar to what you know from a C64. The "Alt" key on a PC keyboard will act
as the "C=" key of a C64 keyboard. This way, you can already produce *most* of
the PETSCII characters.

For those that aren't reachable on the PC keyboard, or if you just don't
remember the correct position, select *PETSCII Input* from the *Windows* menu.
A window will open showing all available PETSCII characters. You can click on
them to input them into a PETSCII input field that currently has the focus.

Be careful with the inverted characters, those are control characters. The
upper row of these will not do what you'd expect in most cases. The lower row
probably works inside filenames, but not in other locations. Do not trust
the directory view when using these control characters, check your result on
a real C64 or in an emulator.

## How it works (internally)

V1541Commander uses the *lib1541img* C library, also freely available from
Excess. This library models 3 layers when editing disk images:

* a "physical layer" represents a raw disk image with tracks and sectors.
* a "virtual filesystem" has all the properties of a CBM-DOS filesystem
  (like disk name, id, dos version, etc) and a collection of CBM-DOS files,
  but no direct link to the physical layer.
* a "concrete filesystem", which takes the responsibility to write the "virtual
  filesystem" to the "physical layer". That's the layer where the filesystem
  options apply, they control aspects of how a filesystem is written to disk.

When working with V1541Commander, you will work on the "virtual filesystem"
most of the time, while the other layers do their work almost unnoticed in the
background. That is, until for some reason, the virtual filesystem just can't
be written to disk correctly, for example because it grew too large. This is
why you might experience a situation where you just can't save: the concrete
filesystem is incapable of writing the current contents of the virtual
filesystem to disk.

For the future, there might be a feature to just save the virtual filesystem.
This won't be a format any other tool recognizes, but it would allow you to
save your work and try to recover later. Currently, this isn't implemented.
So you have to take steps so the virtual filesystem can be written to a disk
again. A simple solution in many cases is to remove files (probably those you
added last). Or you could try and change some filesystem options (e.g., if your
directory got too long, allow long directories).

The filesystem options are an exception to the rule that you always work on the
virtual filesystem, as these tell the concrete filesystem how to behave.

## Log messages

The *lib1541img* C library can "log" all kinds of diagnostic messages.
V1541Commander uses this: When a new "log line" arrives, it is displayed in
the status bar. To see the full log from *lib1541img*, select *lib1541img log*
from the *Windows* menu.
