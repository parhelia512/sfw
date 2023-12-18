#ifndef HASH_MAP_H
#define HASH_MAP_H

/*************************************************************************/
/*  hash_map.h                                                           */
/*************************************************************************/
/*                         This file is part of:                         */
/*                          PANDEMONIUM ENGINE                           */
/*             https://github.com/Relintai/pandemonium_engine            */
/*************************************************************************/
/* Copyright (c) 2022-present Péter Magyar.                              */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "core/containers/hashfuncs.h"
#include "core/containers/paged_allocator.h"
#include "core/containers/pair.h"
#include "core/math/math_funcs.h"
#include "core/os/memory.h"

/**
 * A HashMap implementation that uses open addressing with Robin Hood hashing.
 * Robin Hood hashing swaps out entries that have a smaller probing distance
 * than the to-be-inserted entry, that evens out the average probing distance
 * and enables faster lookups. Backward shift deletion is employed to further
 * improve the performance and to avoid infinite loops in rare cases.
 *
 * Keys and values are stored in a double linked list by insertion order. This
 * has a slight performance overhead on lookup, which can be mostly compensated
 * using a paged allocator if required.
 *
 * The assignment operator copy the pairs from one map to the other.
 */

template <class TKey, class TValue, class Hasher = HashMapHasherDefault, class Comparator = HashMapComparatorDefault<TKey>>
class HashMap {
public:
	const uint32_t MIN_CAPACITY_INDEX = 2; // Use a prime.
	const float MAX_OCCUPANCY = 0.75;
	const uint32_t EMPTY_HASH = 0;

public:
	struct Element {
		Element *next = nullptr;
		Element *prev = nullptr;
		KeyValue<TKey, TValue> data;

		const TKey &key() const {
			return data.key;
		}

		TValue &value() {
			return data.value;
		}

		const TValue &value() const {
			return data.value;
		}

		TValue &get() {
			return data.value;
		};
		const TValue &get() const {
			return data.value;
		};

		Element() {}
		Element(const TKey &p_key, const TValue &p_value) :
				data(p_key, p_value) {}
	};

public:
	_FORCE_INLINE_ uint32_t get_capacity() const { return hash_table_size_primes[capacity_index]; }
	_FORCE_INLINE_ uint32_t size() const { return num_elements; }

	/* Standard Godot Container API */

	bool empty() const {
		return num_elements == 0;
	}

	void clear() {
		if (elements == nullptr || num_elements == 0) {
			return;
		}
		uint32_t capacity = hash_table_size_primes[capacity_index];
		for (uint32_t i = 0; i < capacity; i++) {
			if (hashes[i] == EMPTY_HASH) {
				continue;
			}

			hashes[i] = EMPTY_HASH;
			memdelete(elements[i]);
			elements[i] = nullptr;
		}

		tail_element = nullptr;
		head_element = nullptr;
		num_elements = 0;
	}

	TValue &get(const TKey &p_key) {
		uint32_t pos = 0;
		bool exists = _lookup_pos(p_key, pos);
		CRASH_COND_MSG(!exists, "HashMap key not found.");
		return elements[pos]->data.value;
	}

	const TValue &get(const TKey &p_key) const {
		uint32_t pos = 0;
		bool exists = _lookup_pos(p_key, pos);
		CRASH_COND_MSG(!exists, "HashMap key not found.");
		return elements[pos]->data.value;
	}

	const TValue *getptr(const TKey &p_key) const {
		uint32_t pos = 0;
		bool exists = _lookup_pos(p_key, pos);

		if (exists) {
			return &elements[pos]->data.value;
		}
		return nullptr;
	}

	TValue *getptr(const TKey &p_key) {
		uint32_t pos = 0;
		bool exists = _lookup_pos(p_key, pos);

		if (exists) {
			return &elements[pos]->data.value;
		}
		return nullptr;
	}

	const Element *get_element(const TKey &p_key) const {
		uint32_t pos = 0;
		bool exists = _lookup_pos(p_key, pos);

		if (exists) {
			return elements[pos];
		}

		return NULL;
	}

	Element *get_element(const TKey &p_key) {
		uint32_t pos = 0;
		bool exists = _lookup_pos(p_key, pos);

		if (exists) {
			return elements[pos];
		}

		return NULL;
	}

	_FORCE_INLINE_ const Element *find(const TKey &p_key) const {
		return get_element(p_key);
	}

	_FORCE_INLINE_ Element *find(const TKey &p_key) {
		return get_element(p_key);
	}

	/**
	 * Same as get, except it can return NULL when item was not found.
	 * This version is custom, will take a hash and a custom key (that should support operator==()
	 */

	template <class C>
	_FORCE_INLINE_ TValue *custom_getptr(C p_custom_key, uint32_t p_custom_hash) {
		if (unlikely(!elements)) {
			return NULL;
		}

		const uint32_t capacity = hash_table_size_primes[capacity_index];
		const uint64_t capacity_inv = hash_table_size_primes_inv[capacity_index];
		uint32_t hash = p_custom_hash;
		uint32_t pos = fastmod(hash, capacity_inv, capacity);
		uint32_t distance = 0;

		while (true) {
			if (hashes[pos] == EMPTY_HASH) {
				return NULL;
			}

			if (distance > _get_probe_length(pos, hashes[pos], capacity, capacity_inv)) {
				return NULL;
			}

			if (hashes[pos] == hash && Comparator::compare(elements[pos]->data.key, p_custom_key)) {
				return &elements[pos]->data.value;
			}

			pos = fastmod((pos + 1), capacity_inv, capacity);
			distance++;
		}

		return NULL;
	}

	template <class C>
	_FORCE_INLINE_ const TValue *custom_getptr(C p_custom_key, uint32_t p_custom_hash) const {
		if (unlikely(!elements)) {
			return NULL;
		}

		const uint32_t capacity = hash_table_size_primes[capacity_index];
		const uint64_t capacity_inv = hash_table_size_primes_inv[capacity_index];
		uint32_t hash = p_custom_hash;
		uint32_t pos = fastmod(hash, capacity_inv, capacity);
		uint32_t distance = 0;

		while (true) {
			if (hashes[pos] == EMPTY_HASH) {
				return NULL;
			}

			if (distance > _get_probe_length(pos, hashes[pos], capacity, capacity_inv)) {
				return NULL;
			}

			if (hashes[pos] == hash && Comparator::compare(elements[pos]->data.key, p_custom_key)) {
				return &elements[pos]->data.value;
			}

			pos = fastmod((pos + 1), capacity_inv, capacity);
			distance++;
		}

		return NULL;
	}

	_FORCE_INLINE_ bool has(const TKey &p_key) const {
		uint32_t _pos = 0;
		return _lookup_pos(p_key, _pos);
	}

	bool erase(const TKey &p_key) {
		uint32_t pos = 0;
		bool exists = _lookup_pos(p_key, pos);

		if (!exists) {
			return false;
		}

		const uint32_t capacity = hash_table_size_primes[capacity_index];
		const uint64_t capacity_inv = hash_table_size_primes_inv[capacity_index];
		uint32_t next_pos = fastmod((pos + 1), capacity_inv, capacity);
		while (hashes[next_pos] != EMPTY_HASH && _get_probe_length(next_pos, hashes[next_pos], capacity, capacity_inv) != 0) {
			SWAP(hashes[next_pos], hashes[pos]);
			SWAP(elements[next_pos], elements[pos]);
			pos = next_pos;
			next_pos = fastmod((pos + 1), capacity_inv, capacity);
		}

		hashes[pos] = EMPTY_HASH;

		if (head_element == elements[pos]) {
			head_element = elements[pos]->next;
		}

		if (tail_element == elements[pos]) {
			tail_element = elements[pos]->prev;
		}

		if (elements[pos]->prev) {
			elements[pos]->prev->next = elements[pos]->next;
		}

		if (elements[pos]->next) {
			elements[pos]->next->prev = elements[pos]->prev;
		}

		memdelete(elements[pos]);
		elements[pos] = nullptr;

		num_elements--;
		return true;
	}

	// Reserves space for a number of elements, useful to avoid many resizes and rehashes.
	// If adding a known (possibly large) number of elements at once, must be larger than old capacity.
	void reserve(uint32_t p_new_capacity) {
		uint32_t new_index = capacity_index;

		while (hash_table_size_primes[new_index] < p_new_capacity) {
			ERR_FAIL_COND_MSG(new_index + 1 == (uint32_t)HASH_TABLE_SIZE_MAX, nullptr);
			new_index++;
		}

		if (new_index == capacity_index) {
			return;
		}

		if (elements == nullptr) {
			capacity_index = new_index;
			return; // Unallocated yet.
		}
		_resize_and_rehash(new_index);
	}

	_FORCE_INLINE_ Element *front() {
		return head_element;
	}
	_FORCE_INLINE_ Element *back() {
		return tail_element;
	}

	_FORCE_INLINE_ const Element *front() const {
		return head_element;
	}
	_FORCE_INLINE_ const Element *back() const {
		return tail_element;
	}

	/**
	 * Get the next key to p_key, and the first key if p_key is null.
	 * Returns a pointer to the next key if found, NULL otherwise.
	 * Adding/Removing elements while iterating will, of course, have unexpected results, don't do it.
	 *
	 * Example:
	 *
	 * 	const TKey *k=NULL;
	 *
	 * 	while( (k=table.next(k)) ) {
	 *
	 * 		print( *k );
	 * 	}
	 *
	 * This is for backwards compatibility. Use this syntax instead for new code:
	 *
	 * for (const HashMap<K, V>::Element *E = map.front(); E; E = E->next) {
	 *     ...
	 * }
	 *
	 */
	const TKey *next(const TKey *p_key) const {
		if (unlikely(!elements)) {
			return nullptr;
		}

		if (!p_key) { /* get the first key */

			if (unlikely(!front())) {
				return nullptr;
			}

			return &front()->data.key;

		} else { /* get the next key */

			const Element *e = get_element(*p_key);
			ERR_FAIL_COND_V_MSG(!e, nullptr, "Invalid key supplied.");
			if (e->next) {
				/* if there is a "next" in the list, return that */
				return &e->next->data.key;
			}

			/* nothing found, was at end */
		}

		return nullptr; /* nothing found */
	}

	/* Indexing */

	const TValue &operator[](const TKey &p_key) const {
		uint32_t pos = 0;
		bool exists = _lookup_pos(p_key, pos);
		CRASH_COND(!exists);
		return elements[pos]->data.value;
	}

	TValue &operator[](const TKey &p_key) {
		uint32_t pos = 0;
		bool exists = _lookup_pos(p_key, pos);
		if (!exists) {
			return _insert(p_key, TValue())->data.value;
		} else {
			return elements[pos]->data.value;
		}
	}

	/* Insert */

	Element *insert(const TKey &p_key, const TValue &p_value, bool p_front_insert = false) {
		return _insert(p_key, p_value, p_front_insert);
	}

	Element *set(const TKey &p_key, const TValue &p_value, bool p_front_insert = false) {
		return _insert(p_key, p_value, p_front_insert);
	}

	/* Helpers */

	void get_key_list(List<TKey> *p_keys) const {
		if (unlikely(!elements)) {
			return;
		}

		for (const Element *E = front(); E; E = E->next) {
			p_keys->push_back(E->data.key);
		}
	}

	/* Constructors */

	HashMap(const HashMap &p_other) {
		reserve(hash_table_size_primes[p_other.capacity_index]);

		if (p_other.num_elements == 0) {
			return;
		}

		for (const Element *E = p_other.front(); E; E = E->next) {
			insert(E->data.key, E->data.value);
		}
	}

	void operator=(const HashMap &p_other) {
		if (this == &p_other) {
			return; // Ignore self assignment.
		}
		if (num_elements != 0) {
			clear();
		}

		reserve(hash_table_size_primes[p_other.capacity_index]);

		if (p_other.elements == nullptr) {
			return; // Nothing to copy.
		}

		for (const Element *E = p_other.front(); E; E = E->next) {
			insert(E->data.key, E->data.value);
		}
	}

	HashMap(uint32_t p_initial_capacity) {
		// Capacity can't be 0.
		capacity_index = 0;
		reserve(p_initial_capacity);
	}
	HashMap() {
		capacity_index = MIN_CAPACITY_INDEX;
	}

	uint32_t debug_get_hash(uint32_t p_index) {
		if (num_elements == 0) {
			return 0;
		}
		ERR_FAIL_INDEX_V(p_index, get_capacity(), 0);
		return hashes[p_index];
	}
	Element *debug_get_element(uint32_t p_index) {
		if (num_elements == 0) {
			return NULL;
		}

		ERR_FAIL_INDEX_V(p_index, get_capacity(), NULL);

		return elements[p_index];
	}

	~HashMap() {
		clear();

		if (elements != nullptr) {
			Memory::free_static(elements);
			Memory::free_static(hashes);
		}
	}

private:
	Element **elements = nullptr;
	uint32_t *hashes = nullptr;
	Element *head_element = nullptr;
	Element *tail_element = nullptr;

	uint32_t capacity_index = 0;
	uint32_t num_elements = 0;

	_FORCE_INLINE_ uint32_t _hash(const TKey &p_key) const {
		uint32_t hash = Hasher::hash(p_key);

		if (unlikely(hash == EMPTY_HASH)) {
			hash = EMPTY_HASH + 1;
		}

		return hash;
	}

	static _FORCE_INLINE_ uint32_t _get_probe_length(const uint32_t p_pos, const uint32_t p_hash, const uint32_t p_capacity, const uint64_t p_capacity_inv) {
		const uint32_t original_pos = fastmod(p_hash, p_capacity_inv, p_capacity);
		return fastmod(p_pos - original_pos + p_capacity, p_capacity_inv, p_capacity);
	}

	bool _lookup_pos(const TKey &p_key, uint32_t &r_pos) const {
		if (elements == nullptr || num_elements == 0) {
			return false; // Failed lookups, no elements
		}

		const uint32_t capacity = hash_table_size_primes[capacity_index];
		const uint64_t capacity_inv = hash_table_size_primes_inv[capacity_index];
		uint32_t hash = _hash(p_key);
		uint32_t pos = fastmod(hash, capacity_inv, capacity);
		uint32_t distance = 0;

		while (true) {
			if (hashes[pos] == EMPTY_HASH) {
				return false;
			}

			if (distance > _get_probe_length(pos, hashes[pos], capacity, capacity_inv)) {
				return false;
			}

			if (hashes[pos] == hash && Comparator::compare(elements[pos]->data.key, p_key)) {
				r_pos = pos;
				return true;
			}

			pos = fastmod((pos + 1), capacity_inv, capacity);
			distance++;
		}
	}

	void _insert_with_hash(uint32_t p_hash, Element *p_value) {
		const uint32_t capacity = hash_table_size_primes[capacity_index];
		const uint64_t capacity_inv = hash_table_size_primes_inv[capacity_index];
		uint32_t hash = p_hash;
		Element *value = p_value;
		uint32_t distance = 0;
		uint32_t pos = fastmod(hash, capacity_inv, capacity);

		while (true) {
			if (hashes[pos] == EMPTY_HASH) {
				elements[pos] = value;
				hashes[pos] = hash;

				num_elements++;

				return;
			}

			// Not an empty slot, let's check the probing length of the existing one.
			uint32_t existing_probe_len = _get_probe_length(pos, hashes[pos], capacity, capacity_inv);
			if (existing_probe_len < distance) {
				SWAP(hash, hashes[pos]);
				SWAP(value, elements[pos]);
				distance = existing_probe_len;
			}

			pos = fastmod((pos + 1), capacity_inv, capacity);
			distance++;
		}
	}

	void _resize_and_rehash(uint32_t p_new_capacity_index) {
		uint32_t old_capacity = hash_table_size_primes[capacity_index];

		// Capacity can't be 0.
		capacity_index = MAX((uint32_t)MIN_CAPACITY_INDEX, p_new_capacity_index);

		uint32_t capacity = hash_table_size_primes[capacity_index];

		Element **old_elements = elements;
		uint32_t *old_hashes = hashes;

		num_elements = 0;
		hashes = reinterpret_cast<uint32_t *>(Memory::alloc_static(sizeof(uint32_t) * capacity));
		elements = reinterpret_cast<Element **>(Memory::alloc_static(sizeof(Element *) * capacity));

		for (uint32_t i = 0; i < capacity; i++) {
			hashes[i] = 0;
			elements[i] = nullptr;
		}

		if (old_capacity == 0) {
			// Nothing to do.
			return;
		}

		for (uint32_t i = 0; i < old_capacity; i++) {
			if (old_hashes[i] == EMPTY_HASH) {
				continue;
			}

			_insert_with_hash(old_hashes[i], old_elements[i]);
		}

		Memory::free_static(old_elements);
		Memory::free_static(old_hashes);
	}

	_FORCE_INLINE_ Element *_insert(const TKey &p_key, const TValue &p_value, bool p_front_insert = false) {
		uint32_t capacity = hash_table_size_primes[capacity_index];
		if (unlikely(elements == nullptr)) {
			// Allocate on demand to save memory.

			hashes = reinterpret_cast<uint32_t *>(Memory::alloc_static(sizeof(uint32_t) * capacity));
			elements = reinterpret_cast<Element **>(Memory::alloc_static(sizeof(Element *) * capacity));

			for (uint32_t i = 0; i < capacity; i++) {
				hashes[i] = EMPTY_HASH;
				elements[i] = nullptr;
			}
		}

		uint32_t pos = 0;
		bool exists = _lookup_pos(p_key, pos);

		if (exists) {
			elements[pos]->data.value = p_value;
			return elements[pos];
		} else {
			if (num_elements + 1 > MAX_OCCUPANCY * capacity) {
				ERR_FAIL_COND_V_MSG(capacity_index + 1 == HASH_TABLE_SIZE_MAX, nullptr, "Hash table maximum capacity reached, aborting insertion.");
				_resize_and_rehash(capacity_index + 1);
			}

			Element *elem = memnew(Element(p_key, p_value));

			if (tail_element == nullptr) {
				head_element = elem;
				tail_element = elem;
			} else if (p_front_insert) {
				head_element->prev = elem;
				elem->next = head_element;
				head_element = elem;
			} else {
				tail_element->next = elem;
				elem->prev = tail_element;
				tail_element = elem;
			}

			uint32_t hash = _hash(p_key);
			_insert_with_hash(hash, elem);
			return elem;
		}
	}
};

#endif // HASH_MAP_H
