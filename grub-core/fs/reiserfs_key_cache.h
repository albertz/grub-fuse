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
