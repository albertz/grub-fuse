//
//  reiserfs_key_cache.h
//  grub-fuse
//
//  Created by Albert Zeyer on 21.09.11.
//  Copyright 2011 Albert Zeyer. All rights reserved.
//
//  code under GPL

// This uses the <cache.h> implementation to remember
// recent current_position and cur_key_offset for the
// "reiserfs.c" grub_reiserfs_read() function.
//
// Without this cache, the initial reading speed when reading
// big files drops down linearly. I.e. it might start with
// about 5 MB/sec and drops to 1 MB/sec and below.
// With this cache, the reading speed stays constant at about
// 5 MB/sec, no matter how huge the file is.

#ifndef __grub_fuse__reiserfs_key_cache_h__
#define __grub_fuse__reiserfs_key_cache_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <grub/types.h>

void _forward_optimal_position(grub_uint32_t dir_id, grub_uint32_t obj_id, grub_off_t initial_pos, grub_off_t* cur_pos, grub_uint64_t* key_offset);
void _save_pos(grub_uint32_t dir_id, grub_uint32_t obj_id, grub_off_t cur_pos, grub_uint64_t key_offset);

#ifdef __cplusplus
}
#endif

#endif
