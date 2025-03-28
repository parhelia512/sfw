//--STRIP
#ifndef ARRAY_H
#define ARRAY_H
//--STRIP

/*************************************************************************/
/*  array.h                                                              */
/*  From https://github.com/Relintai/pandemonium_engine (MIT)            */
/*************************************************************************/

//--STRIP
#include "core/typedefs.h"
//--STRIP

class Variant;
class ArrayPrivate;
class Object;
class StringName;
class String;

class Array {
	mutable ArrayPrivate *_p;
	void _ref(const Array &p_from) const;
	void _unref() const;

	inline int _clamp_slice_index(int p_index) const;

public:
	Variant &operator[](int p_idx);
	const Variant &operator[](int p_idx) const;

	void set(int p_idx, const Variant &p_value);
	const Variant &get(int p_idx) const;

	int size() const;
	bool empty() const;
	void clear();

	bool deep_equal(const Array &p_array, int p_recursion_count = 0) const;
	bool operator==(const Array &p_array) const;

	uint32_t hash() const;
	uint32_t recursive_hash(int p_recursion_count) const;
	void operator=(const Array &p_array);

	void push_back(const Variant &p_value);
	_FORCE_INLINE_ void append(const Variant &p_value) { push_back(p_value); } //for python compatibility
	void append_array(const Array &p_array);
	Error resize(int p_new_size);

	void insert(int p_pos, const Variant &p_value);
	void remove(int p_pos);
	void fill(const Variant &p_value);

	Variant front() const;
	Variant back() const;

	Array &sort();
	Array &sort_custom(Object *p_obj);
	void shuffle();
	int bsearch(const Variant &p_value, bool p_before = true);
	int bsearch_custom(const Variant &p_value, Object *p_obj, const StringName &p_function, bool p_before = true);
	Array &invert();

	int find(const Variant &p_value, int p_from = 0) const;
	int rfind(const Variant &p_value, int p_from = -1) const;
	int find_last(const Variant &p_value) const;
	int count(const Variant &p_value) const;
	bool has(const Variant &p_value) const;

	void erase(const Variant &p_value);

	void push_front(const Variant &p_value);
	Variant pop_back();
	Variant pop_front();
	Variant pop_at(int p_pos);

	Array duplicate(bool p_deep = false) const;

	Array slice(int p_begin, int p_end, int p_step = 1, bool p_deep = false) const;

	Variant min() const;
	Variant max() const;

	bool operator<(const Array &p_array) const;
	bool operator<=(const Array &p_array) const;
	bool operator>(const Array &p_array) const;
	bool operator>=(const Array &p_array) const;

	const void *id() const;

	String sprintf(const String &p_format, bool *error) const;

	Array(const Array &p_from);
	Array();
	~Array();
};

//--STRIP
#endif // ARRAY_H
//--STRIP
