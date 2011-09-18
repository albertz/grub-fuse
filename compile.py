#!/usr/bin/python

import os, os.path, sys

os.chdir(os.path.dirname(__file__))
try: os.mkdir("build")
except: pass

import re
from fnmatch import fnmatch
from pipes import quote
from pprint import pprint

CFLAGS = "-g -Iinclude -Igrub-core/gnulib " + \
	"-Ibuild -DLIBDIR=\\\"/usr/local/lib\\\" " + \
	"-DHAVE_CONFIG_H=1 -DGRUB_UTIL=1 " + \
	"-DGRUB_FILE=__FILE__ " + \
	"-D_FILE_OFFSET_BITS=64 " + \
	"-fnested-functions " + \
	"-DAPPLE_CC "
LDFLAGS = "-lfuse4x"

configh = open("build/config.h", "w")
configh.write("""
#define PACKAGE_NAME "grub"
#define PACKAGE_VERSION "1.0.0"
#define HAVE_WORKING_O_NOFOLLOW 1
#define HAVE_DECL_FWRITE_UNLOCKED 0
#define HAVE_DECL_FPUTS_UNLOCKED 0
#define NESTED_FUNC_ATTR
#define __getopt_argv_const const
#define _GL_UNUSED
#if defined(__i386__)
#define SIZEOF_LONG 4
#define SIZEOF_VOID_P 4
#elif defined(__x86_64__)
#define SIZEOF_LONG 8
#define SIZEOF_VOID_P 8
#else
#error "unknown arch"
#endif
#define GRUB_TARGET_SIZEOF_VOID_P SIZEOF_VOID_P
""")
for funcn in ["strcmp","strlen","strchr","strrchr","strdup","strtoull"]:
	configh.write("#define grub_" + funcn + " " + funcn + "\n")
configh.close()

open("build/config-util.h", "w").write("")

re_grubmodinit = re.compile("GRUB_MOD_INIT\((.+)\)")
ofiles = []
grubmods = []
def compile(fn):
	basen,ext = os.path.splitext(fn)
	ofile = "build/" + basen.replace("/","_") + ".o"
	if ofile in ofiles: return # hm, whatever...
	cmd = "gcc -c " + CFLAGS + " " + quote(fn) + " -o " + quote(ofile)
	print cmd
	assert os.system(cmd) == 0
	ofiles.append(ofile)
	m = re_grubmodinit.search(open(fn).read())
	if m: grubmods.append(m.group(1))
	
re_base_start = re.compile("^([a-z]+) = {")
re_base_end = re.compile("^};")
re_entry_stmnt = re.compile("^ *([a-z_0-9]+);$")
re_entry = re.compile("^ *([a-z_0-9]+) = (.*);$")

content = {} # type (e.g. 'program') -> list -> dict
curentry = None

for l in open("Makefile.util.def"):
	l = l.strip("\n")
	if l.strip() == "": continue
	if l.strip()[0:1] == "#": continue
	m = re_base_start.match(l)
	if m:
		typename = m.group(1)
		curentry = {}
		if typename not in content: content[typename] = []
		content[typename].append(curentry)
		continue
	m = re_base_end.match(l)
	if m:
		curentry = None
		continue
	if curentry is not None:
		if re_entry_stmnt.match(l): continue
		m = re_entry.match(l)
		assert m, "no match in " + repr(l)
		k,v = m.groups()
		if k not in curentry:
			curentry[k] = []
		curentry[k].append(v)

libs = {}
progs = {}

for l in content["library"]:
	libs[l["name"][0]] = l
for l in content["program"]:
	progs[l["name"][0]] = l

# ------------

def read_gnulib_makefile():
	re_vardef = re.compile("^([a-zA-Z_]+) *= *(.*)$")
	re_variadd = re.compile("^([a-zA-Z_]+) *\+= *(.*)$")
	re_filerules = re.compile("^([a-zA-Z_.\-\+/]+) *: *(.*)$")
	
	vars = {}
	fileiter = iter(open("grub-core/gnulib/Makefile.am"))
	for l in fileiter:
		while True:
			l = l.strip("\n")
			if l[-1:] == "\\":
				l = l[:-1]
				l += next(fileiter)
			else: break
		if l.strip() == "": continue
		if l.strip()[0:1] == "#": continue
		m = re_vardef.match(l)
		if m:
			k,v = m.groups()
			assert not k in vars
			vars[k] = v
			continue
		m = re_variadd.match(l)
		if m:
			k,v = m.groups()
			assert k in vars
			vars[k] += " " + v
			continue
		m = re_filerules.match(l)
		if m:
			# ignore
			continue
		if l[0:1] == "\t": # file rule part
			continue
		assert False, l + " not matched"
	
	return vars["libgnu_a_SOURCES"].split()

# ------------

prog = progs["grub-mount"]
lddeps = []
curtarget = prog
while True:
	for f in curtarget["common"]:
		compile(f)
	for d in curtarget.get("ldadd", []):
		if fnmatch(d, "lib*.a"):
			lddeps.append(d)
	if not lddeps: break
	curtarget = libs[lddeps.pop(0)]

for f in read_gnulib_makefile():
	if not fnmatch(f, "*.c"): continue
	compile("grub-core/gnulib/" + f)

assert os.system("sh geninit.sh " + " ".join(map(quote, grubmods)) + " >build/grubinit.c") == 0
compile("build/grubinit.c")

# additional stuff
compile("grub-core/gnulib/mempcpy.c")
compile("grub-core/gnulib/strchrnul.c")
compile("grub-core/gnulib/getopt.c")
compile("grub-core/gnulib/getopt1.c")
compile("grub-core/gnulib/rawmemchr.c")
compile("grub-core/gnulib/basename-lgpl.c")

cmd = "gcc " + LDFLAGS + " " + " ".join(map(quote, ofiles)) + " -o build/grub-mount"
print cmd
assert os.system(cmd) == 0
