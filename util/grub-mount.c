/* grub-mount.c - FUSE driver for filesystems that GRUB understands */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2008,2009,2010 Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */
#define FUSE_USE_VERSION 26
#include <config.h>
#include <grub/types.h>
#include <grub/emu/misc.h>
#include <grub/emu/getroot.h>
#include <grub/emu/hostdisk.h>
#include <grub/partition.h>
#include <grub/util/misc.h>
#include <grub/misc.h>
#include <grub/device.h>
#include <grub/disk.h>
#include <grub/file.h>
#include <grub/fs.h>
#include <grub/env.h>
#include <grub/term.h>
#include <grub/mm.h>
#include <grub/lib/hexdump.h>
#include <grub/crypto.h>
#include <grub/command.h>
#include <grub/i18n.h>
#include <fuse/fuse.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "progname.h"
#include "argp.h"

static char *root = NULL;
grub_device_t dev = NULL;
grub_fs_t fs = NULL;
static char **images = NULL;
static char *debug_str = NULL;
static char **fuse_args = NULL;
static int fuse_argc = 0;
static int num_disks = 0;

static grub_err_t
execute_command (char *name, int n, char **args)
{
  grub_command_t cmd;

  cmd = grub_command_find (name);
  if (! cmd)
    grub_util_error (_("can\'t find command %s"), name);

  return (cmd->func) (cmd, n, args);
}

/* Translate GRUB error numbers into OS error numbers.  Print any unexpected
   errors.  */
static int
translate_error (void)
{
  int ret;

  switch (grub_errno)
    {
      case GRUB_ERR_NONE:
	ret = 0;
	break;

      case GRUB_ERR_OUT_OF_MEMORY:
	grub_print_error ();
	ret = -ENOMEM;
	break;

      case GRUB_ERR_BAD_FILE_TYPE:
	/* This could also be EISDIR.  Take a guess.  */
	ret = -ENOTDIR;
	break;

      case GRUB_ERR_FILE_NOT_FOUND:
	ret = -ENOENT;
	break;

      case GRUB_ERR_FILE_READ_ERROR:
      case GRUB_ERR_READ_ERROR:
      case GRUB_ERR_IO:
	grub_print_error ();
	ret = -EIO;
	break;

      case GRUB_ERR_SYMLINK_LOOP:
	ret = -ELOOP;
	break;

      default:
	grub_print_error ();
	ret = -EINVAL;
	break;
    }

  /* Any previous errors were handled.  */
  grub_errno = GRUB_ERR_NONE;

  return ret;
}

static struct grub_dirhook_info _fuse_getattr_file_info;
static int _fuse_getattr_file_exists = 0;
static char* _fuse_getattr_filename = NULL;

/* A hook for iterating directories. */
int _fuse_getattr_find_file (const char *cur_filename,
					const struct grub_dirhook_info *info);
int _fuse_getattr_find_file (const char *cur_filename,
			   const struct grub_dirhook_info *info)
{
    if ((info->case_insensitive ? grub_strcasecmp (cur_filename, _fuse_getattr_filename)
		 : grub_strcmp (cur_filename, _fuse_getattr_filename)) == 0)
	{
		_fuse_getattr_file_info = *info;
		_fuse_getattr_file_exists = 1;
		return 1;
	}
    return 0;
}

static int
fuse_getattr (const char *path, struct stat *st)
{
  char *filename, *pathname, *path2;
  const char *pathname_t;
  _fuse_getattr_file_exists = 0;
  
  if (path[0] == '/' && path[1] == 0)
    {
      st->st_dev = 0;
      st->st_ino = 0;
      st->st_mode = 0555 | S_IFDIR;
      st->st_uid = 0;
      st->st_gid = 0;
      st->st_rdev = 0;
      st->st_size = 0;
      st->st_blksize = 512;
      st->st_blocks = (st->st_blksize + 511) >> 9;
      st->st_atime = st->st_mtime = st->st_ctime = 0;
      return 0;
    }

  _fuse_getattr_file_exists = 0;

  pathname_t = grub_strchr (path, ')');
  if (! pathname_t)
    pathname_t = path;
  else
    pathname_t++;
  pathname = xstrdup (pathname_t);
  
  /* Remove trailing '/'. */
  while (*pathname && pathname[grub_strlen (pathname) - 1] == '/')
    pathname[grub_strlen (pathname) - 1] = 0;

  /* Split into path and filename. */
  filename = grub_strrchr (pathname, '/');
  if (! filename)
    {
      path2 = grub_strdup ("/");
      filename = pathname;
    }
  else
    {
      filename++;
      path2 = grub_strdup (pathname);
      path2[filename - pathname] = 0;
    }

  /* It's the whole device. */
	_fuse_getattr_filename = filename;
  (fs->dir) (dev, path2, _fuse_getattr_find_file);

  grub_free (path2);
  if (!_fuse_getattr_file_exists)
    {
      grub_errno = GRUB_ERR_NONE;
      return -ENOENT;
    }
  st->st_dev = 0;
  st->st_ino = 0;
  st->st_mode = _fuse_getattr_file_info.dir ? (0555 | S_IFDIR) : (0444 | S_IFREG);
  st->st_uid = 0;
  st->st_gid = 0;
  st->st_rdev = 0;
  if (!_fuse_getattr_file_info.dir)
    {
      grub_file_t file;
      file = grub_file_open (path);
      if (! file)
	return translate_error ();
      st->st_size = file->size;
      grub_file_close (file);
    }
  else
    st->st_size = 0;
  st->st_blksize = 512;
  st->st_blocks = (st->st_size + 511) >> 9;
  st->st_atime = st->st_mtime = st->st_ctime = _fuse_getattr_file_info.mtimeset
    ? _fuse_getattr_file_info.mtime : 0;
  grub_errno = GRUB_ERR_NONE;
  return 0;
}

static int
fuse_opendir (const char *path, struct fuse_file_info *fi) 
{
  return 0;
}

/* FIXME */
static grub_file_t files[65536];
static int first_fd = 1;

static int 
fuse_open (const char *path, struct fuse_file_info *fi __attribute__ ((unused)))
{
  grub_file_t file;
  file = grub_file_open (path);
  if (! file)
    return translate_error ();
  files[first_fd++] = file;
  fi->fh = first_fd;
  files[first_fd++] = file;
  grub_errno = GRUB_ERR_NONE;
  return 0;
} 

static int 
fuse_read (const char *path, char *buf, size_t sz, off_t off,
	   struct fuse_file_info *fi)
{
  grub_file_t file = files[fi->fh];
  grub_ssize_t size;

  if (off > file->size)
    return -EINVAL;

  file->offset = off;
  
  size = grub_file_read (file, buf, sz);
  if (size < 0)
    return translate_error ();
  else
    {
      grub_errno = GRUB_ERR_NONE;
      return size;
    }
} 

static int 
fuse_release (const char *path, struct fuse_file_info *fi)
{
  grub_file_close (files[fi->fh]);
  files[fi->fh] = NULL;
  grub_errno = GRUB_ERR_NONE;
  return 0;
}

static int 
fuse_readdir (const char *path, void *buf,
	      fuse_fill_dir_t fill, off_t off, struct fuse_file_info *fi)
{
  char *pathname;

  auto int call_fill (const char *filename,
		      const struct grub_dirhook_info *info);
  int call_fill (const char *filename, const struct grub_dirhook_info *info)
  {
    fill (buf, filename, NULL, 0);
    return 0;
  }

  pathname = xstrdup (path);
  
  /* Remove trailing '/'. */
  while (pathname [0] && pathname[1]
	 && pathname[grub_strlen (pathname) - 1] == '/')
    pathname[grub_strlen (pathname) - 1] = 0;

  (fs->dir) (dev, pathname, call_fill);
  free (pathname);
  grub_errno = GRUB_ERR_NONE;
  return 0;
}

struct fuse_operations grub_opers = {
  .getattr = fuse_getattr,
  .open = fuse_open,
  .release = fuse_release,
  .opendir = fuse_opendir,
  .readdir = fuse_readdir,
  .read = fuse_read
};

int dummy_hook(const char* name) {
	printf("huhu: %s\n", name);
}

char* full_dev_name = NULL;

int partition_iterhook(grub_disk_t disk, const grub_partition_t partition) {
	char* part_name = grub_partition_get_name(partition);
	printf("*** part: %s, index: %i\n", part_name, partition->index);
	char buf[1024];
	snprintf(buf, sizeof(buf), "%ss%i", grub_env_get ("root"), partition->index + 1);
	if(strcmp(buf, images[0]) == 0) {
		printf("*** found: matches to %s\n", images[0]);
		full_dev_name = malloc(1024);
		snprintf(full_dev_name, 1024, "%s,%s", grub_env_get ("root"), part_name);		
		return 1;
	}
	return 0;
}

static grub_err_t
fuse_init (void)
{
  int i;

  grub_lvm_fini ();
  grub_mdraid09_fini ();
  grub_mdraid1x_fini ();
  grub_raid_fini ();
  grub_raid_init ();
  grub_mdraid09_init ();
  grub_mdraid1x_init ();
  grub_lvm_init ();
	
	dev = grub_device_open (0);
  if (! dev) {
	  printf("grub_device_open failed\n");
    return grub_errno;
  }

	printf("** opened device\n");
	grub_partition_iterate(dev->disk, partition_iterhook);
	if(!full_dev_name) {
		printf("partition not found\n");
		return grub_errno;		
	}
	
	printf("** opening %s\n", full_dev_name);
	dev = grub_device_open(full_dev_name);
	if (! dev) {
		printf("grub_device_open failed\n");
		return grub_errno;
	}
	
	{

		grub_disk_t disk = dev->disk;
		grub_partition_t part;
		
		if (disk->partition == NULL)
		{
			printf ("no partition map found for %s\n", disk->name);
			//return;
		}
		else
		for (part = disk->partition; part; part = part->parent)
			printf ("part: %s\n", part->partmap->name);
	}

  fs = grub_fs_probe (dev);
  if (! fs)
    {
		printf("grub_fs_probe failed\n");
      grub_device_close (dev);
      return grub_errno;
    }

	printf("yea, fuse_main!\n");
  fuse_main (fuse_argc, fuse_args, &grub_opers, NULL);

  return GRUB_ERR_NONE;
}

static struct argp_option options[] = {  
  {"root",      'r', N_("DEVICE_NAME"), 0, N_("Set root device."),                 2},
  {"debug",     'd', "S",           0, N_("Set debug environment variable."),  2},
  {"verbose",   'v', NULL, OPTION_ARG_OPTIONAL, N_("Print verbose messages."), 2},
  {0, 0, 0, 0, 0, 0}
};

/* Print the version information.  */
static void
print_version (FILE *stream, struct argp_state *state)
{
  fprintf (stream, "%s (%s) %s\n", program_name, PACKAGE_NAME, PACKAGE_VERSION);
}

error_t 
argp_parser (int key, char *arg, struct argp_state *state)
{
  char *p;

  switch (key)
    {
    case 'r':
      root = arg;
      return 0;

    case 'd':
      debug_str = arg;
      return 0;

    case 'v':
      verbosity++;
      return 0;

    case ARGP_KEY_ARG:
      if (arg[0] != '-')
	break;

    default:
      if (!arg)
	return 0;

      fuse_args = xrealloc (fuse_args, (fuse_argc + 1) * sizeof (fuse_args[0]));
      fuse_args[fuse_argc] = xstrdup (arg);
      fuse_argc++;
      return 0;
    }

  if (arg[0] != '/')
    {
      fprintf (stderr, "%s", _("Must use absolute path.\n"));
      argp_usage (state);
    }
  images = xrealloc (images, (num_disks + 1) * sizeof (images[0]));
  images[num_disks] = xstrdup (arg);
  num_disks++;

  return 0;
}

struct argp argp = {
  options, argp_parser, N_("IMAGE1 [IMAGE2 ...] MOUNTPOINT"),
  N_("Debug tool for filesystem driver."), 
  NULL, NULL, NULL
};

static int  process_device (const char *name, int is_floppy)
{
	printf("dev: %s\n", name);
	return 0;
}


int
main (int argc, char *argv[])
{
  char *default_root, *alloc_root;

  set_program_name (argv[0]);
  argp_program_version_hook = print_version;

  grub_util_init_nls ();

  fuse_args = xrealloc (fuse_args, (fuse_argc + 2) * sizeof (fuse_args[0]));
  fuse_args[fuse_argc] = xstrdup (argv[0]);
  fuse_argc++;
  /* Run single-threaded.  */
  fuse_args[fuse_argc] = xstrdup ("-s");
  fuse_argc++;

  argp_parse (&argp, argc, argv, 0, 0, 0);
  
  if (num_disks < 2)
    grub_util_error ("need an image and mountpoint");
  fuse_args = xrealloc (fuse_args, (fuse_argc + 2) * sizeof (fuse_args[0]));
  fuse_args[fuse_argc] = images[num_disks - 1];
  fuse_argc++;
  num_disks--;
  fuse_args[fuse_argc] = NULL;
	
	grub_util_biosdisk_init("");

  /* Initialize all modules. */
  grub_init_all ();

  if (debug_str)
    grub_env_set ("debug", debug_str);

  //default_root = "disk3"; //(num_disks == 1) ? "loop0" : "md0";
  alloc_root = 0;
  if (root)
    {
      if ((*root >= '0') && (*root <= '9'))
        {
          alloc_root = xmalloc (strlen (default_root) + strlen (root) + 2);

          sprintf (alloc_root, "%s,%s", default_root, root);
          root = alloc_root;
        }
    }
  else
    root = default_root;

	char* device_name = images[0];
	char* drive_name = grub_util_get_grub_dev(device_name);
	if(!drive_name) {
		grub_print_error();
		return 1;
	}
	printf ("transformed OS device `%s' into GRUB device `%s'\n",
					device_name, drive_name);
	drive_name = grub_util_biosdisk_get_grub_dev(device_name);
	if(!drive_name) {
		grub_print_error();
		return 1;
	}
	printf ("transformed OS device `%s' into GRUB device `%s'\n",
			device_name, drive_name);
	
	grub_env_set ("root", drive_name);

  if (alloc_root)
    free (alloc_root);

  /* Do it.  */
  fuse_init ();
  if (grub_errno)
    {
      grub_print_error ();
      return 1;
    }

  /* Free resources.  */
  grub_fini_all ();

  return 0;
}
