#ifndef UST_CORE_H
#define UST_CORE_H

/*
 * Copyright (C) 2010  Pierre-Marc Fournier
 * Copyright (C) 2011  Mathieu Desnoyers <mathieu.desnoyers@efficios.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; version 2.1 of
 * the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <sys/types.h>
#include <lttng/config.h>
#include <urcu/arch.h>
#include <urcu/compiler.h>

/* ARRAYS */

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))


/* ALIGNMENT SHORTCUTS */

#include <unistd.h>

#define ALIGN(x,a)		__ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)	(((x)+(mask))&~(mask))
#define PAGE_ALIGN(addr) ALIGN(addr, PAGE_SIZE)
#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)
#define PAGE_MASK (~(PAGE_SIZE-1))

/* ERROR OPS */
#define MAX_ERRNO	4095

#define IS_ERR_VALUE(x) caa_unlikely((x) >= (unsigned long)-MAX_ERRNO)

static inline void *ERR_PTR(long error)
{
	return (void *) error;
}

static inline long PTR_ERR(const void *ptr)
{
	return (long) ptr;
}

static inline long IS_ERR(const void *ptr)
{
	return IS_ERR_VALUE((unsigned long)ptr);
}


/* Min / Max */

#define min_t(type, x, y) ({                    \
	type __min1 = (x);                      \
	type __min2 = (y);                      \
	__min1 < __min2 ? __min1: __min2; })

#define max_t(type, x, y) ({                    \
	type __max1 = (x);                      \
	type __max2 = (y);                      \
	__max1 > __max2 ? __max1: __max2; })


/* MUTEXES */

#include <pthread.h>

#define DEFINE_MUTEX(m) pthread_mutex_t (m) = PTHREAD_MUTEX_INITIALIZER;
#define DECLARE_MUTEX(m) extern pthread_mutex_t (m);

/* MALLOCATION */

#include <stdlib.h>

static inline
void *zmalloc(size_t len)
{
	return calloc(1, len);
}

static inline
void *malloc_align(size_t len)
{
	return malloc(ALIGN(len, CAA_CACHE_LINE_SIZE));
}

static inline
void *zmalloc_align(size_t len)
{
	return calloc(1, ALIGN(len, CAA_CACHE_LINE_SIZE));
}

/* MATH */

#include <lttng/processor.h>
static inline unsigned int hweight32(unsigned int w)
{
	unsigned int res = w - ((w >> 1) & 0x55555555);
	res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
	res = (res + (res >> 4)) & 0x0F0F0F0F;
	res = res + (res >> 8);
	return (res + (res >> 16)) & 0x000000FF;
}

static __inline__ int get_count_order(unsigned int count)
{
	int order;
	
	order = fls(count) - 1;
	if (count & (count - 1))
		order++;
	return order;
}

#define _ust_container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

#ifndef inline_memcpy
#define inline_memcpy memcpy
#endif

#ifndef __same_type
#define __same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
#endif

#ifndef UST_VALGRIND

static __inline__ int ust_get_cpu(void)
{
	int cpu;

	cpu = sched_getcpu();
	if (caa_likely(cpu >= 0))
		return cpu;
	/*
	 * If getcpu(2) is not implemented in the Kernel use CPU 0 as fallback.
	 */
	return 0;
}

#else	/* #else #ifndef UST_VALGRIND */

static __inline__ int ust_get_cpu(void)
{
	/*
	 * Valgrind does not support the sched_getcpu() vsyscall.
	 * It causes it to detect a segfault in the program and stop it.
	 * So if we want to check libust with valgrind, we have to refrain
	 * from using this call. TODO: it would probably be better to return
	 * other values too, to better test it.
	 */
	return 0;
}

#endif	/* #else #ifndef UST_VALGRIND */

#endif /* UST_CORE_H */