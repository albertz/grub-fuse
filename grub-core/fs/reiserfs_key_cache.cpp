#include "reiserfs_key_cache.h"
#include <cache.h>

struct CacheKey {
	grub_uint32_t dir_id;
	grub_uint32_t obj_id;
	grub_off_t initial_pos;
	bool operator==(const CacheKey& other) const {
		return
		dir_id == other.dir_id &&
		obj_id == other.obj_id &&
		initial_pos == other.initial_pos;
	}
	bool operator<(const CacheKey& other) const {
		if(dir_id != other.dir_id) return dir_id < other.dir_id;
		if(obj_id != other.obj_id) return obj_id < other.obj_id;
		return initial_pos < other.initial_pos;
	}
	bool match(grub_uint32_t _dir_id, grub_uint32_t _obj_id, grub_off_t _initial_pos) const {
		if(dir_id != _dir_id) return false;
		if(obj_id != _obj_id) return false;
		if(initial_pos > _initial_pos) return false;
		return true;
	}
};

struct CacheValue {
	grub_off_t cur_pos;
	grub_uint64_t key_offset;
};

typedef Cache<CacheKey, CacheValue, 1000> CacheT;
static CacheT cache;

void _forward_optimal_position(grub_uint32_t dir_id, grub_uint32_t obj_id, grub_off_t initial_pos, grub_off_t* cur_pos, grub_uint64_t* key_offset) {
	CacheKey key;
	key.dir_id = dir_id;
	key.obj_id = obj_id;
	key.initial_pos = initial_pos;
	CacheT::Item* item = cache.lower_bound(key);
	if(item && item->key.match(dir_id, obj_id, initial_pos)) {
		*cur_pos = item->value.cur_pos;
		*key_offset = item->value.key_offset;
	}
}

void _save_pos(grub_uint32_t dir_id, grub_uint32_t obj_id, grub_off_t cur_pos, grub_uint64_t key_offset) {
	CacheKey key;
	key.dir_id = dir_id;
	key.obj_id = obj_id;
	key.initial_pos = cur_pos;
	CacheValue value;
	value.cur_pos = cur_pos;
	value.key_offset = key_offset;
	cache.push_back(key, value);
}
