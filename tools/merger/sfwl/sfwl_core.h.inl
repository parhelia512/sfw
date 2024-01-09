#ifndef SFWL_H
#define SFWL_H

{{FILEINLINE:tools/merger/sfw_readme.inl.h}}
{{FILEINLINE:tools/merger/sfw_linceses.inl.h}}

#define _REENTRANT

#include <float.h>
#include <math.h>
#include <string.h>
#include <stddef.h>

//===================  CORE SECTION  ===================

//--STRIP
//no includes
//--STRIP
{{FILE:sfwl/core/int_types.h}}
//--STRIP
//no includes
//--STRIP
{{FILE:sfwl/core/math_defs.h}}
//--STRIP
//no includes
//--STRIP
{{FILE:sfwl/core/error_list.h}}
//--STRIP
//no includes
//--STRIP
{{FILE:sfwl/core/logger.h}}

//--STRIP
//#include "core/int_types.h"
//#include "core/error_list.h"
//--STRIP
{{FILE:sfwl/core/typedefs.h}}


//--STRIP
//#include "core/int_types.h"
//--STRIP
{{FILE:sfwl/core/stime.h}}
//--STRIP
//#include "core/typedefs.h"
//--STRIP
{{FILE:sfwl/core/safe_refcount.h}}

//--STRIP
//#include "core/logger.h"
//#include "core/typedefs.h"
//--STRIP
{{FILE:sfwl/core/error_macros.h}}


//--STRIP
//#include "core/error_macros.h"
//#include "core/safe_refcount.h"
//--STRIP
{{FILE:sfwl/core/memory.h}}


//--STRIP
//#include "core/error_list.h"
//#include "core/typedefs.h"
//--STRIP
{{FILE:sfwl/core/mutex.h}}
//--STRIP
//#include "core/error_list.h"
//--STRIP
{{FILE:sfwl/core/rw_lock.h}}
//--STRIP
//#include "core/typedefs.h"
//--STRIP
{{FILE:sfwl/core/spin_lock.h}}
//--STRIP
//#include "core/mutex.h"
//--STRIP
{{FILE:sfwl/core/thread_safe.h}}

//--STRIP
//#include "core/typedefs.h"
//--STRIP
{{FILE:sfwl/core/pcg.h}}
//--STRIP
//#include "core/math_defs.h"
//#include "core/pcg.h"
//--STRIP
{{FILE:sfwl/core/random_pcg.h}}

//--STRIP
//#include "core/error_macros.h"
//#include "core/math_defs.h"
//#include "core/random_pcg.h"
//#include "core/typedefs.h"
//#include "core/pcg.h"
//--STRIP
{{FILE:sfwl/core/math_funcs.h}}

//--STRIP
//Simple almost dependencyless containers
//--STRIP

//--STRIP
//#include "core/error_list.h"
//#include "core/error_macros.h"
//#include "core/memory.h"
//#include "core/safe_refcount.h"
//--STRIP
{{FILE:sfwl/core/cowdata.h}}
//--STRIP
//#include "core/error_macros.h"
//#include "core/typedefs.h"
//--STRIP
{{FILE:sfwl/core/sort_array.h}}
//--STRIP
//#include "core/error_macros.h"
//#include "core/memory.h"
//--STRIP
{{FILE:sfwl/core/rb_map.h}}
//--STRIP
//#include "core/memory.h"
//#include "core/typedefs.h"
//--STRIP
{{FILE:sfwl/core/rb_set.h}}
//--STRIP
//#include "cowdata.h"
//#include "core/typedefs.h"
//--STRIP
{{FILE:sfwl/core/vmap.h}}
//--STRIP
//#include "cowdata.h"
//#include "core/sort_array.h"
//#include "core/error_macros.h"
//#include "core/memory.h"
//--STRIP
{{FILE:sfwl/core/vector.h}}
//--STRIP
//#include "core/vector.h"
//#include "core/typedefs.h"
//--STRIP
{{FILE:sfwl/core/vset.h}}
//--STRIP
//#include "core/sort_array.h"
//#include "core/error_macros.h"
//#include "core/memory.h"
//--STRIP
{{FILE:sfwl/core/list.h}}
//--STRIP
//#include "core/vector.h"
//--STRIP
{{FILE:sfwl/core/ring_buffer.h}}
//--STRIP
//#include "core/memory.h"
//#include "spin_lock.h"
//#include "core/typedefs.h"
//--STRIP
{{FILE:sfwl/core/paged_allocator.h}}
//--STRIP
//#include "core/typedefs.h"
//--STRIP
{{FILE:sfwl/core/pool_allocator.h}}

//--STRIP
//Strings they need vector.h, and cowdata.h
//--STRIP

//--STRIP
//#include "core/typedefs.h"
//--STRIP
{{FILE:sfwl/core/char_range.inc}}
//--STRIP
//#include "core/typedefs.h"
//#include "core/char_range.inc"
//--STRIP
{{FILE:sfwl/core/char_utils.h}}
//--STRIP
//#include "cowdata.h"
//#include "core/vector.h"
//#include "char_utils.h"
//#include "core/typedefs.h"
//--STRIP
{{FILE:sfwl/core/ustring.h}}

//--STRIP
//#include "core/mutex.h"
//#include "core/safe_refcount.h"
//#include "core/ustring.h"
//--STRIP
{{FILE:sfwl/core/string_name.h}}

//--STRIP
//Needs ustring.h
//--STRIP

//--STRIP
//#include "core/memory.h"
//#include "core/mutex.h"
//#include "pool_allocator.h"
//#include "rw_lock.h"
//#include "core/safe_refcount.h"
//#include "core/ustring.h"
//--STRIP
{{FILE:sfwl/core/pool_vector.h}}
//--STRIP
//#include "core/pool_vector.h"
//#include "core/sort_array.h"
//#include "core/vector.h"
//#include "core/error_macros.h"
//#include "core/memory.h"
//--STRIP
{{FILE:sfwl/core/tight_local_vector.h}}
//--STRIP
//#include "core/pool_vector.h"
//#include "core/sort_array.h"
//#include "core/vector.h"
//#include "core/error_macros.h"
//#include "core/memory.h"
//--STRIP
{{FILE:sfwl/core/local_vector.h}}

//--STRIP
//Math classes
//--STRIP

//--STRIP
//hashfuncs.h Needs most math classes
//--STRIP

//--STRIP
//#include "core/aabb.h"
//#include "core/math_defs.h"
//#include "core/math_funcs.h"
//#include "core/rect2.h"
//#include "core/rect2i.h"
//#include "core/vector2.h"
//#include "core/vector2i.h"
//#include "core/vector3.h"
//#include "core/vector3i.h"
//#include "core/vector4.h"
//#include "core/vector4i.h"
//#include "core/string_name.h"
//#include "core/ustring.h"
//#include "core/typedefs.h"
//--STRIP
{{FILE:sfwl/core/hashfuncs.h}}

//--STRIP
//Containers that need hashfuncs.h
//--STRIP

//--STRIP
//#include "core/hashfuncs.h"
//#include "core/typedefs.h"
//--STRIP
{{FILE:sfwl/core/pair.h}}
//--STRIP
//#include "core/hashfuncs.h"
//#include "core/list.h"
//#include "core/error_macros.h"
//#include "core/math_funcs.h"
//#include "core/memory.h"
//#include "core/ustring.h"
//--STRIP
{{FILE:sfwl/core/og_hash_map.h}}
//--STRIP
//#include "core/list.h"
//#include "core/og_hash_map.h"
//#include "core/pair.h"
//--STRIP
{{FILE:sfwl/core/ordered_hash_map.h}}
//--STRIP
//#include "core/hashfuncs.h"
//#include "paged_allocator.h"
//#include "pair.h"
//#include "core/math_funcs.h"
//#include "core/memory.h"
//#include "list.h"
//--STRIP
{{FILE:sfwl/core/hash_map.h}}
//--STRIP
//#include "core/hash_map.h"
//#include "core/hashfuncs.h"
//#include "core/math_funcs.h"
//#include "core/memory.h"
//--STRIP
{{FILE:sfwl/core/hash_set.h}}

//--STRIP
//#include "core/error_list.h"
//#include "core/ustring.h"
//also needs vector
//--STRIP
{{FILE:sfwl/core/file_access.h}}

//--STRIP
//#include "core/error_list.h"
//#include "core/ustring.h"
//--STRIP
{{FILE:sfwl/core/dir_access.h}}

//--STRIP
//no includes
//--STRIP
{{FILE:sfwl/core/sfw_core.h}}

#endif