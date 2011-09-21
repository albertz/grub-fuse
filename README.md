grub-fuse
=========

[**GRUB**](http://www.gnu.org/software/grub/) is the **GRand Unified Bootloader** (see `grub-README`). GRUB has reading support for many filesystems included.

This is a [**FUSE** (Filesystem in Userspace)](http://fuse.sourceforge.net/) driver which uses the GRUB filesystem reading support.

This means that you have basic reading support for ReiserFS, XFS, JFS, ZFS, Btrfs and many others on any system that supports FUSE (e.g. MacOSX, Linux, etc.).

History
-------

The initial work was done by *Vladimir 'phcoder' Serbinenko*, followed by some fixes by *Colin Watson* and some more fixes by me.

Those latest fixes were mostly related to:

* The build system. It didn't really worked for MacOSX. So, I wrote my own simple `compile.py` which works on MacOSX (and can probably easily be ported again to other systems).
* Nested functions. They didn't worked at all for me. I had to remove all their usages. That was quite some annoying work and it is probably not complete (I only tested ReiserFS).
* The FUSE driver itself. It also didn't worked. Mostly because it didn't found the related GRUB device. It tries to mostly work around that now.

Usage
-----

Compile it:

    $ ./compile.py

Run it:

    $ mkdir ~/mnt
    $ ./build/grub-mount /dev/rdisk3s1 ~/mnt
    $ ls ~/mnt

State
-----

In particular, I tested it now on MacOSX 10.7.1 with Fuse4x and a ReiserFS 3 partition on some USB disk.

Performance varies wildly. Directory reading is quite slow (could be improved much by some intelligent caching). File reading is ok with about 5 MB/sec (depending much on the media).

The code is quite hackish! Cleanup by someone experiented with GRUB and/or FUSE is very welcome!

-- Albert Zeyer, <http://www.az2000.de>

