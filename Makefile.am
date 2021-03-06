AUTOMAKE_OPTIONS = subdir-objects

DEPDIR = .deps-util
SUBDIRS = grub-core/gnulib . grub-core po docs util/bash-completion.d

include $(top_srcdir)/conf/Makefile.common
include $(top_srcdir)/conf/Makefile.extra-dist

AM_CFLAGS = $(HOST_CFLAGS)
AM_LDFLAGS = $(HOST_LDFLAGS)
AM_CPPFLAGS = $(HOST_CPPFLAGS) $(CPPFLAGS_DEFAULT)
AM_CCASFLAGS = $(HOST_CCASFLAGS) $(CCASFLAGS_DEFAULT)

ACLOCAL_AMFLAGS = -I m4

CFLAGS_PROGRAM += $(CFLAGS_GNULIB)
LDFLAGS_PROGRAM += $(LDFLAGS_GNULIB)
CPPFLAGS_PROGRAM += $(CPPFLAGS_GNULIB)
CCASFLAGS_PROGRAM += $(CCASFLAGS_GNULIB)

include $(srcdir)/Makefile.util.am

# XXX Use Automake's LEX & YACC support
grub_script.tab.h: $(top_srcdir)/grub-core/script/parser.y
	$(YACC) -d -p grub_script_yy -b grub_script $(top_srcdir)/grub-core/script/parser.y
grub_script.tab.c: grub_script.tab.h
CLEANFILES += grub_script.tab.c grub_script.tab.h

# For the lexer.
grub_script.yy.h: $(top_srcdir)/grub-core/script/yylex.l
	$(LEX) -o grub_script.yy.c --header-file=grub_script.yy.h $(top_srcdir)/grub-core/script/yylex.l
grub_script.yy.c: grub_script.yy.h
CLEANFILES += grub_script.yy.c grub_script.yy.h

# For libgrub.a
libgrub.pp: grub_script.tab.h grub_script.yy.h $(libgrubmods_a_SOURCES) $(libgrubkern_a_SOURCES)
	$(CPP) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(libgrubmods_a_CPPFLAGS) $(libgrubkern_a_CPPFLAGS) $(CPPFLAGS) \
	  -D'GRUB_MOD_INIT(x)=@MARKER@x@' $^ > $@ || (rm -f $@; exit 1)
CLEANFILES += libgrub.pp

libgrub_a_init.lst: libgrub.pp
	cat $< | grep '@MARKER@' | sed 's/@MARKER@\(.*\)@/\1/g' | sort -u > $@ || (rm -f $@; exit 1)
CLEANFILES += libgrub_a_init.lst

libgrub_a_init.c: libgrub_a_init.lst $(top_srcdir)/geninit.sh
	sh $(top_srcdir)/geninit.sh `cat $<` > $@ || (rm -f $@; exit 1)
CLEANFILES += libgrub_a_init.c

# For grub-fstest
grub_fstest.pp: $(grub_fstest_SOURCES)
	$(CPP) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(grub_fstest_CPPFLAGS) $(CPPFLAGS) \
	  -D'GRUB_MOD_INIT(x)=@MARKER@x@' $^ > $@ || (rm -f $@; exit 1)
CLEANFILES += grub_fstest.pp

grub_fstest_init.lst: libgrub.pp grub_fstest.pp
	cat $^ | grep '@MARKER@' | sed 's/@MARKER@\(.*\)@/\1/g' | sort -u > $@ || (rm -f $@; exit 1)
CLEANFILES += grub_fstest_init.lst

grub_fstest_init.c: grub_fstest_init.lst $(top_srcdir)/geninit.sh
	sh $(top_srcdir)/geninit.sh `cat $<` > $@ || (rm -f $@; exit 1)
CLEANFILES += grub_fstest_init.c

if COND_GRUB_MKFONT
if COND_HAVE_FONT_SOURCE
grubdata_DATA = unicode.pf2 ascii.pf2 euro.pf2 ascii.h widthspec.h
endif
endif

unicode.pf2: $(FONT_SOURCE) grub-mkfont
	$(builddir)/grub-mkfont -o $@ $(FONT_SOURCE)
CLEANFILES += unicode.pf2

# Arrows and lines are needed to draw the menu, so always include them
UNICODE_ARROWS=0x2190-0x2193
UNICODE_LINES=0x2501-0x251B

ascii.pf2: $(FONT_SOURCE) grub-mkfont
	$(builddir)/grub-mkfont -o $@ $(FONT_SOURCE) -r 0x0-0x7f,$(UNICODE_ARROWS),$(UNICODE_LINES)
CLEANFILES += ascii.pf2

euro.pf2: $(FONT_SOURCE) grub-mkfont
	$(builddir)/grub-mkfont -o $@ $(FONT_SOURCE) -r 0x0-0x4ff,0x1e00-0x1fff,$(UNICODE_ARROWS),$(UNICODE_LINES)
CLEANFILES += euro.pf2

ascii.bitmaps: $(FONT_SOURCE) grub-mkfont
	$(builddir)/grub-mkfont --ascii-bitmaps -o $@ $(FONT_SOURCE)
CLEANFILES += ascii.bitmaps

ascii.h: ascii.bitmaps grub-bin2h
	$(builddir)/grub-bin2h ascii_bitmaps < $< > $@
CLEANFILES += ascii.h $(top_builddir)/grub-core/include/ascii.h

widthspec.bin: $(FONT_SOURCE) grub-mkfont
	$(builddir)/grub-mkfont --width-spec -o $@ $(FONT_SOURCE)
CLEANFILES += widthspec.bin

widthspec.h: widthspec.bin grub-bin2h
	$(builddir)/grub-bin2h widthspec < $< > $@
CLEANFILES += widthspec.h

# Install config.h into platformdir
platform_HEADERS = config.h

pkglib_DATA += grub-mkconfig_lib
pkglib_DATA += update-grub_lib


if COND_i386_coreboot
BOOTTARGET=coreboot
QEMU32=qemu-system-i386
endif

if COND_i386_multiboot
BOOTTARGET=cd
QEMU32=qemu-system-i386
endif

if COND_i386_ieee1275
BOOTTARGET=cd
QEMU32=qemu-system-i386
endif

if COND_i386_qemu
BOOTTARGET=qemu
QEMU32=qemu-system-i386
endif

if COND_i386_pc
BOOTTARGET=cd
QEMU32=qemu-system-i386
endif

if COND_i386_efi
QEMU32=qemu-system-i386
BOOTTARGET=cd
endif

if COND_x86_64_efi
QEMU32=qemu-system-x86_64
BOOTTARGET=cd
endif

linux.init.x86_64: $(srcdir)/grub-core/tests/boot/linux.init-x86_64.S
	$(TARGET_CC) -o $@ $< -m64 -nostdlib -nostdinc -DSUCCESSFUL_BOOT_STRING=\"$(SUCCESSFUL_BOOT_STRING)\"

linux.init.i386: $(srcdir)/grub-core/tests/boot/linux.init-i386.S
	$(TARGET_CC) -o $@ $< -m32 -nostdlib -nostdinc -DSUCCESSFUL_BOOT_STRING=\"$(SUCCESSFUL_BOOT_STRING)\"

multiboot.elf: $(srcdir)/grub-core/tests/boot/kernel-i386.S
	$(TARGET_CC) -o $@ $< -DSUCCESSFUL_BOOT_STRING=\"$(SUCCESSFUL_BOOT_STRING)\" -ffreestanding -nostdlib -nostdinc -DTARGET_MULTIBOOT=1 -Wl,-N -Wl,-Ttext,0x100000 -m32 -I$(srcdir)/include

kfreebsd.elf: $(srcdir)/grub-core/tests/boot/kernel-i386.S
	$(TARGET_CC) -o $@ $< -DSUCCESSFUL_BOOT_STRING=\"$(SUCCESSFUL_BOOT_STRING)\" -ffreestanding -nostdlib -nostdinc -Wl,-N -Wl,-Ttext,0x100000 -m32 -I$(srcdir)/include

kfreebsd.aout: kfreebsd.elf
	$(OBJCOPY) -O a.out-i386-linux $< $@ -R .note.gnu.build-id

pc-chainloader.elf: $(srcdir)/grub-core/tests/boot/kernel-8086.S
	$(TARGET_CC) -o $@ $< -DTARGET_CHAINLOADER=1 -DSUCCESSFUL_BOOT_STRING=\"$(SUCCESSFUL_BOOT_STRING)\" -ffreestanding -nostdlib -nostdinc -Wl,-N -Wl,-Ttext,0x7c00 -m32

pc-chainloader.bin: pc-chainloader.elf
	$(OBJCOPY) -O binary --strip-unneeded -R .note -R .comment -R .note.gnu.build-id -R .reginfo -R .rel.dyn $< $@;

ntldr.elf: $(srcdir)/grub-core/tests/boot/kernel-8086.S
	$(TARGET_CC) -o $@ $< -DTARGET_NTLDR=1 -DSUCCESSFUL_BOOT_STRING=\"$(SUCCESSFUL_BOOT_STRING)\" -ffreestanding -nostdlib -nostdinc -Wl,-N -Wl,-Ttext,0 -m32

ntldr.bin: ntldr.elf
	$(OBJCOPY) -O binary --strip-unneeded -R .note -R .comment -R .note.gnu.build-id -R .reginfo -R .rel.dyn $< $@;

multiboot2.elf: $(srcdir)/grub-core/tests/boot/kernel-i386.S
	$(TARGET_CC) -o $@ $< -DSUCCESSFUL_BOOT_STRING=\"$(SUCCESSFUL_BOOT_STRING)\" -ffreestanding -nostdlib -nostdinc -Wl,-N -Wl,-Ttext,0x100000 -m32 -I$(srcdir)/include -DTARGET_MULTIBOOT2=1

kfreebsd.init.x86_64: $(srcdir)/grub-core/tests/boot/kfreebsd.init-x86_64.S
	$(TARGET_CC) -o $@ $< -m64 -nostdlib -nostdinc -DSUCCESSFUL_BOOT_STRING=\"$(SUCCESSFUL_BOOT_STRING)\" && freebsd-brandelf -t FreeBSD $@

kfreebsd.init.i386: $(srcdir)/grub-core/tests/boot/kfreebsd.init-i386.S
	$(TARGET_CC) -o $@ $< -m32 -nostdlib -nostdinc -DSUCCESSFUL_BOOT_STRING=\"$(SUCCESSFUL_BOOT_STRING)\" && freebsd-brandelf -t FreeBSD $@

knetbsd.init.i386: $(srcdir)/grub-core/tests/boot/kbsd.init-i386.S
	$(TARGET_CC) -o $@ $< -m32 -nostdlib -nostdinc -DTARGET_NETBSD=1 -DSUCCESSFUL_BOOT_STRING=\"$(SUCCESSFUL_BOOT_STRING)\"

kopenbsd.init.i386: $(srcdir)/grub-core/tests/boot/kbsd.init-i386.S
	$(TARGET_CC) -o $@ $< -m32 -nostdlib -nostdinc -DTARGET_OPENBSD=1 -DSUCCESSFUL_BOOT_STRING=\"$(SUCCESSFUL_BOOT_STRING)\"

knetbsd.init.x86_64: $(srcdir)/grub-core/tests/boot/kbsd.init-x86_64.S
	$(TARGET_CC) -o $@ $< -m64 -DTARGET_NETBSD=1 -nostdlib -nostdinc -DSUCCESSFUL_BOOT_STRING=\"$(SUCCESSFUL_BOOT_STRING)\"

kopenbsd.init.x86_64: $(srcdir)/grub-core/tests/boot/kbsd.init-x86_64.S
	$(TARGET_CC) -o $@ $< -m64 -DTARGET_OPENBSD=1 -nostdlib -nostdinc -DSUCCESSFUL_BOOT_STRING=\"$(SUCCESSFUL_BOOT_STRING)\"

linux-initramfs.i386: linux.init.i386 Makefile
	TDIR=`mktemp -d "$${TMPDIR:-/tmp}/tmp.XXXXXXXXXX"` && cp $< $$TDIR/init && (cd $$TDIR && echo ./init | cpio --quiet --dereference -o -H newc) | gzip > $@ && rm -rf $$TDIR

linux-initramfs.x86_64: linux.init.x86_64 Makefile
	TDIR=`mktemp -d "$${TMPDIR:-/tmp}/tmp.XXXXXXXXXX"` && cp $< $$TDIR/init && (cd $$TDIR && echo ./init | cpio --quiet --dereference -o -H newc) | gzip > $@ && rm -rf $$TDIR

kfreebsd-mfsroot.i386.img: kfreebsd.init.i386 Makefile
	TDIR=`mktemp -d "$${TMPDIR:-/tmp}/tmp.XXXXXXXXXX"` && mkdir $$TDIR/dev && mkdir $$TDIR/sbin && cp $< $$TDIR/sbin/init && makefs -t ffs -s 30m -f 1000 -o minfree=0,version=1 $@ $$TDIR && rm -rf $$TDIR

knetbsd.image.i386: knetbsd.init.i386 $(srcdir)/grub-core/tests/boot/kbsd.spec.txt
	TDIR=`mktemp -d "$${TMPDIR:-/tmp}/tmp.XXXXXXXXXX"` && mkdir $$TDIR/dev && mkdir $$TDIR/sbin && cp $< $$TDIR/sbin/init && makefs -F $(srcdir)/grub-core/tests/boot/kbsd.spec.txt -t ffs -s 64k -f 10 -o minfree=0,version=1 $@ $$TDIR && rm -rf $$TDIR

kopenbsd.image.i386: kopenbsd.init.i386 $(srcdir)/grub-core/tests/boot/kopenbsdlabel.txt
	TDIR=`mktemp -d "$${TMPDIR:-/tmp}/tmp.XXXXXXXXXX"` && mkdir $$TDIR/dev && mkdir $$TDIR/sbin && cp $< $$TDIR/sbin/init && makefs -F $(srcdir)/grub-core/tests/boot/kbsd.spec.txt -t ffs -s 128k -f 10 -o minfree=0,version=1 $@ $$TDIR && bsdlabel -f -R $@ $(srcdir)/grub-core/tests/boot/kopenbsdlabel.txt && rm -rf $$TDIR || rm -f $@

kopenbsd.image.x86_64: kopenbsd.init.x86_64 $(srcdir)/grub-core/tests/boot/kopenbsdlabel.txt
	TDIR=`mktemp -d "$${TMPDIR:-/tmp}/tmp.XXXXXXXXXX"` && mkdir $$TDIR/dev && mkdir $$TDIR/sbin && cp $< $$TDIR/sbin/init && makefs -F $(srcdir)/grub-core/tests/boot/kbsd.spec.txt -t ffs -s 128k -f 10 -o minfree=0,version=1 $@ $$TDIR && bsdlabel -f -R $@ $(srcdir)/grub-core/tests/boot/kopenbsdlabel.txt && rm -rf $$TDIR || rm -f $@

knetbsd.miniroot-image.i386.img: knetbsd.image.i386 $(GRUB_PAYLOADS_DIR)/knetbsd.miniroot.i386
	$(OBJCOPY) --add-section=miniroot=$< $(GRUB_PAYLOADS_DIR)/knetbsd.miniroot.i386 $@

kfreebsd-mfsroot.x86_64.img: kfreebsd.init.x86_64 Makefile
	TDIR=`mktemp -d "$${TMPDIR:-/tmp}/tmp.XXXXXXXXXX"` && mkdir $$TDIR/dev && mkdir $$TDIR/sbin && cp $< $$TDIR/sbin/init && makefs -t ffs -s 30m -f 1000 -o minfree=0,version=1 $@ $$TDIR && rm -rf $$TDIR

knetbsd.image.x86_64: knetbsd.init.x86_64 $(srcdir)/grub-core/tests/boot/kbsd.spec.txt
	TDIR=`mktemp -d "$${TMPDIR:-/tmp}/tmp.XXXXXXXXXX"` && mkdir $$TDIR/dev && mkdir $$TDIR/sbin && cp $< $$TDIR/sbin/init && makefs -F $(srcdir)/grub-core/tests/boot/kbsd.spec.txt -t ffs -s 64k -f 10 -o minfree=0,version=1 $@ $$TDIR && rm -rf $$TDIR

knetbsd.miniroot-image.x86_64.img: knetbsd.image.x86_64 $(GRUB_PAYLOADS_DIR)/knetbsd.miniroot.x86_64
	$(OBJCOPY) --add-section=miniroot=$< $(GRUB_PAYLOADS_DIR)/knetbsd.miniroot.x86_64 $@

CLEANFILES += linux.init.i386 kfreebsd.init.i386 linux.init.x86_64 linux-initramfs.i386 linux-initramfs.x86_64

kfreebsd-mfsroot.i386.gz: kfreebsd-mfsroot.i386.img
	gzip < $< > $@

bootcheck-kfreebsd-i386: kfreebsd-mfsroot.i386.gz $(GRUB_PAYLOADS_DIR)/kfreebsd.i386 $(GRUB_PAYLOADS_DIR)/kfreebsd_env.i386 $(srcdir)/grub-core/tests/boot/kfreebsd.cfg grub-shell
	timeout -s KILL $(BOOTCHECK_TIMEOUT) ./grub-shell --qemu-opts="$(GRUB_QEMU_OPTS)" --boot=$(BOOTTARGET) --qemu=$(QEMU32) --files=/mfsroot.gz=kfreebsd-mfsroot.i386.gz --files=/kfreebsd=$(GRUB_PAYLOADS_DIR)/kfreebsd.i386 --files=/kfreebsd_env=$(GRUB_PAYLOADS_DIR)/kfreebsd_env.i386 $(srcdir)/grub-core/tests/boot/kfreebsd.cfg | grep $(SUCCESSFUL_BOOT_STRING) > /dev/null

kfreebsd-mfsroot.x86_64.gz: kfreebsd-mfsroot.x86_64.img
	gzip < $< > $@

bootcheck-kfreebsd-x86_64: kfreebsd-mfsroot.x86_64.gz $(GRUB_PAYLOADS_DIR)/kfreebsd.x86_64 $(GRUB_PAYLOADS_DIR)/kfreebsd_env.x86_64 $(srcdir)/grub-core/tests/boot/kfreebsd.cfg grub-shell
	timeout -s KILL $(BOOTCHECK_TIMEOUT) ./grub-shell --qemu-opts="$(GRUB_QEMU_OPTS)" --boot=$(BOOTTARGET) --qemu=qemu-system-x86_64 --files=/mfsroot.gz=kfreebsd-mfsroot.x86_64.gz --files=/kfreebsd=$(GRUB_PAYLOADS_DIR)/kfreebsd.x86_64 --files=/kfreebsd_env=$(GRUB_PAYLOADS_DIR)/kfreebsd_env.x86_64 $(srcdir)/grub-core/tests/boot/kfreebsd.cfg | grep $(SUCCESSFUL_BOOT_STRING) > /dev/null

knetbsd.miniroot-image.i386.gz: knetbsd.miniroot-image.i386.img
	gzip < $< > $@

bootcheck-knetbsd-i386: knetbsd.miniroot-image.i386.gz $(GRUB_PAYLOADS_DIR)/knetbsd.i386 $(srcdir)/grub-core/tests/boot/knetbsd.cfg grub-shell
	timeout -s KILL $(BOOTCHECK_TIMEOUT) ./grub-shell --qemu-opts="$(GRUB_QEMU_OPTS)" --boot=$(BOOTTARGET) --qemu=$(QEMU32) --files=/miniroot.gz=knetbsd.miniroot-image.i386.gz --files=/knetbsd=$(GRUB_PAYLOADS_DIR)/knetbsd.i386 $(srcdir)/grub-core/tests/boot/knetbsd.cfg | grep $(SUCCESSFUL_BOOT_STRING) > /dev/null

bootcheck-kopenbsd-i386: kopenbsd.image.i386 $(GRUB_PAYLOADS_DIR)/kopenbsd.i386 $(srcdir)/grub-core/tests/boot/kopenbsd.cfg grub-shell
	timeout -s KILL $(BOOTCHECK_TIMEOUT) ./grub-shell --qemu-opts="$(GRUB_QEMU_OPTS)" --boot=$(BOOTTARGET) --qemu=$(QEMU32) --files=/ramdisk=kopenbsd.image.i386 --files=/kopenbsd=$(GRUB_PAYLOADS_DIR)/kopenbsd.i386 $(srcdir)/grub-core/tests/boot/kopenbsd.cfg | grep $(SUCCESSFUL_BOOT_STRING) > /dev/null

bootcheck-kopenbsd-x86_64: kopenbsd.image.x86_64 $(GRUB_PAYLOADS_DIR)/kopenbsd.x86_64 $(srcdir)/grub-core/tests/boot/kopenbsd.cfg grub-shell
	timeout -s KILL $(BOOTCHECK_TIMEOUT) ./grub-shell --qemu-opts="$(GRUB_QEMU_OPTS)" --boot=$(BOOTTARGET) --qemu=qemu-system-x86_64 --files=/ramdisk=kopenbsd.image.x86_64 --files=/kopenbsd=$(GRUB_PAYLOADS_DIR)/kopenbsd.x86_64 $(srcdir)/grub-core/tests/boot/kopenbsd.cfg | grep $(SUCCESSFUL_BOOT_STRING) > /dev/null

knetbsd.miniroot-image.x86_64.gz: knetbsd.miniroot-image.x86_64.img
	gzip < $< > $@

bootcheck-knetbsd-x86_64: knetbsd.miniroot-image.x86_64.gz $(GRUB_PAYLOADS_DIR)/knetbsd.x86_64 $(srcdir)/grub-core/tests/boot/knetbsd.cfg grub-shell
	timeout -s KILL $(BOOTCHECK_TIMEOUT) ./grub-shell --qemu-opts="$(GRUB_QEMU_OPTS)" --boot=$(BOOTTARGET) --qemu=qemu-system-x86_64 --files=/miniroot.gz=knetbsd.miniroot-image.x86_64.gz --files=/knetbsd=$(GRUB_PAYLOADS_DIR)/knetbsd.x86_64 $(srcdir)/grub-core/tests/boot/knetbsd.cfg | grep $(SUCCESSFUL_BOOT_STRING) > /dev/null

bootcheck-linux-i386: linux-initramfs.i386 $(GRUB_PAYLOADS_DIR)/linux.i386 $(srcdir)/grub-core/tests/boot/linux.cfg grub-shell
	timeout -s KILL $(BOOTCHECK_TIMEOUT) ./grub-shell --qemu-opts="$(GRUB_QEMU_OPTS)" --boot=$(BOOTTARGET) --qemu=$(QEMU32) --files=/initrd=linux-initramfs.i386 --files=/linux=$(GRUB_PAYLOADS_DIR)/linux.i386 $(srcdir)/grub-core/tests/boot/linux.cfg | grep $(SUCCESSFUL_BOOT_STRING) > /dev/null

bootcheck-linux-x86_64: linux-initramfs.x86_64 $(GRUB_PAYLOADS_DIR)/linux.x86_64 $(srcdir)/grub-core/tests/boot/linux.cfg grub-shell
	timeout -s KILL $(BOOTCHECK_TIMEOUT) ./grub-shell --qemu-opts="$(GRUB_QEMU_OPTS)" --boot=$(BOOTTARGET) --qemu=qemu-system-x86_64 --files=/initrd=linux-initramfs.x86_64 --files=/linux=$(GRUB_PAYLOADS_DIR)/linux.x86_64 $(srcdir)/grub-core/tests/boot/linux.cfg | grep $(SUCCESSFUL_BOOT_STRING) > /dev/null

bootcheck-linux16-i386: linux-initramfs.i386 $(GRUB_PAYLOADS_DIR)/linux.i386 $(srcdir)/grub-core/tests/boot/linux.cfg grub-shell
	timeout -s KILL $(BOOTCHECK_TIMEOUT) ./grub-shell --qemu-opts="$(GRUB_QEMU_OPTS)" --boot=$(BOOTTARGET) --qemu=$(QEMU32) --files=/initrd=linux-initramfs.i386 --files=/linux=$(GRUB_PAYLOADS_DIR)/linux.i386 $(srcdir)/grub-core/tests/boot/linux16.cfg | grep $(SUCCESSFUL_BOOT_STRING) > /dev/null

bootcheck-linux16-x86_64: linux-initramfs.x86_64 $(GRUB_PAYLOADS_DIR)/linux.x86_64 $(srcdir)/grub-core/tests/boot/linux.cfg grub-shell
	timeout -s KILL $(BOOTCHECK_TIMEOUT) ./grub-shell --qemu-opts="$(GRUB_QEMU_OPTS)" --boot=$(BOOTTARGET) --qemu=qemu-system-x86_64 --files=/initrd=linux-initramfs.x86_64 --files=/linux=$(GRUB_PAYLOADS_DIR)/linux.x86_64 $(srcdir)/grub-core/tests/boot/linux16.cfg | grep $(SUCCESSFUL_BOOT_STRING) > /dev/null

bootcheck-multiboot: multiboot.elf $(srcdir)/grub-core/tests/boot/multiboot.cfg grub-shell
	timeout -s KILL $(BOOTCHECK_TIMEOUT) ./grub-shell --qemu-opts="$(GRUB_QEMU_OPTS)" --boot=$(BOOTTARGET) --qemu=$(QEMU32) --files=/multiboot.elf=multiboot.elf $(srcdir)/grub-core/tests/boot/multiboot.cfg | grep $(SUCCESSFUL_BOOT_STRING) > /dev/null

bootcheck-multiboot2: multiboot2.elf $(srcdir)/grub-core/tests/boot/multiboot2.cfg grub-shell
	timeout -s KILL $(BOOTCHECK_TIMEOUT) ./grub-shell --qemu-opts="$(GRUB_QEMU_OPTS)" --boot=$(BOOTTARGET) --qemu=$(QEMU32) --files=/multiboot2.elf=multiboot2.elf $(srcdir)/grub-core/tests/boot/multiboot2.cfg | grep $(SUCCESSFUL_BOOT_STRING) > /dev/null

bootcheck-kfreebsd-aout: kfreebsd.aout $(srcdir)/grub-core/tests/boot/kfreebsd-aout.cfg grub-shell
	timeout -s KILL $(BOOTCHECK_TIMEOUT) ./grub-shell --qemu-opts="$(GRUB_QEMU_OPTS)" --boot=$(BOOTTARGET) --qemu=$(QEMU32) --files=/kfreebsd.aout=kfreebsd.aout $(srcdir)/grub-core/tests/boot/kfreebsd-aout.cfg | grep $(SUCCESSFUL_BOOT_STRING) > /dev/null

bootcheck-pc-chainloader: pc-chainloader.bin $(srcdir)/grub-core/tests/boot/pc-chainloader.cfg grub-shell
	timeout -s KILL $(BOOTCHECK_TIMEOUT) ./grub-shell --qemu-opts="$(GRUB_QEMU_OPTS)" --boot=$(BOOTTARGET) --qemu=$(QEMU32) --files=/pc-chainloader.bin=pc-chainloader.bin $(srcdir)/grub-core/tests/boot/pc-chainloader.cfg | grep $(SUCCESSFUL_BOOT_STRING) > /dev/null

bootcheck-ntldr: ntldr.bin $(srcdir)/grub-core/tests/boot/ntldr.cfg grub-shell
	timeout -s KILL $(BOOTCHECK_TIMEOUT) ./grub-shell --qemu-opts="$(GRUB_QEMU_OPTS)" --boot=$(BOOTTARGET) --qemu=$(QEMU32) --files=/ntldr.bin=ntldr.bin $(srcdir)/grub-core/tests/boot/ntldr.cfg | grep $(SUCCESSFUL_BOOT_STRING) > /dev/null

BOOTCHECKS= 

if COND_i386_pc
#pc chainloader by definition is only for i386-pc
BOOTCHECKS += bootcheck-pc-chainloader
#ntldr and bootmgr require BIOS.
BOOTCHECKS += bootcheck-ntldr
#legacy protocol makes early BIOS calls.
BOOTCHECKS += bootcheck-linux16-i386 bootcheck-linux16-x86_64
# Crashes early on non-BIOS
BOOTCHECKS += bootcheck-knetbsd-i386
endif

if !COND_i386_coreboot
# Crashes because memory at 0-0x1000 is occupied
BOOTCHECKS += bootcheck-knetbsd-x86_64

# Likewise and require ACPI.
if !COND_i386_multiboot
if !COND_i386_qemu
BOOTCHECKS += bootcheck-kfreebsd-x86_64
BOOTCHECKS += bootcheck-kfreebsd-i386
endif
endif
endif

BOOTCHECKS += bootcheck-kfreebsd-aout

BOOTCHECKS += bootcheck-kopenbsd-i386 bootcheck-kopenbsd-x86_64

BOOTCHECKS += bootcheck-multiboot bootcheck-multiboot2

BOOTCHECKS += bootcheck-linux-i386 bootcheck-linux-x86_64


.PHONY: bootcheck-linux-i386 bootcheck-linux-x86_64 \
       bootcheck-kfreebsd-i386 bootcheck-kfreebsd-x86_64 \
       bootcheck-knetbsd-i386 bootcheck-knetbsd-x86_64

# Randomly generated
SUCCESSFUL_BOOT_STRING=3e49994fd5d82b7c9298d672d774080d
# tianocore cd access is very slow
BOOTCHECK_TIMEOUT=180

bootcheck: $(BOOTCHECKS)
