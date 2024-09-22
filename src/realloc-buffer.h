/* SPDX-License-Identifier: LGPL-2.1+ */

#ifndef fooreallocbufferhfoo
#define fooreallocbufferhfoo

#include <sys/types.h>

#include "util.h"

typedef struct ReallocBuffer {
        void *data;
        size_t allocated;
        size_t start;/*buffer可用的起始偏移*/
        size_t end;/*buffer可用的终止偏移*/
} ReallocBuffer;


/*取buffer可用的起始地址*/
static inline void *realloc_buffer_data(ReallocBuffer *buffer) {
        assert(buffer);
        assert(buffer->start <= buffer->end);
        assert(buffer->end <= buffer->allocated);
        assert(buffer->data || buffer->allocated == 0);

        if (!buffer->data)
                return buffer;

        return (uint8_t*) buffer->data + buffer->start;
}

static inline void *realloc_buffer_data_offset(ReallocBuffer *buffer, size_t offset) {
        size_t p;

        assert(buffer);
        assert(buffer->start <= buffer->end);
        assert(buffer->end <= buffer->allocated);

        p = buffer->start + offset;
        if (p < buffer->start) /* overflow? */
                return NULL;
        if (p > buffer->end) /* out of bounds? */
                return NULL;

        return (uint8_t*) buffer->data + p;
}

static inline size_t realloc_buffer_size(ReallocBuffer *buffer) {
        assert(buffer);
        assert(buffer->start <= buffer->end);
        assert(buffer->end <= buffer->allocated);

        /*返回buffer可用长度*/
        return buffer->end - buffer->start;
}

void* realloc_buffer_acquire(ReallocBuffer *b, size_t size);
void* realloc_buffer_acquire0(ReallocBuffer *b, size_t size);
void* realloc_buffer_extend(ReallocBuffer *b, size_t size);
void* realloc_buffer_extend0(ReallocBuffer *b, size_t size);
void* realloc_buffer_append(ReallocBuffer *b, const void *p, size_t size);

static inline void* realloc_buffer_append_byte(ReallocBuffer *b, uint8_t q) {
        return realloc_buffer_append(b, &q, sizeof(q));
}

void realloc_buffer_free(ReallocBuffer *b);

static inline void realloc_buffer_empty(ReallocBuffer *b) {
        b->start = b->end = 0;
}

int realloc_buffer_advance(ReallocBuffer *b, size_t sz);
int realloc_buffer_shorten(ReallocBuffer *b, size_t sz);
int realloc_buffer_truncate(ReallocBuffer *b, size_t sz);

int realloc_buffer_read_size(ReallocBuffer *b, int fd, size_t add);

static inline int realloc_buffer_read(ReallocBuffer *b, int fd) {
        return realloc_buffer_read_size(b, fd, (size_t) -1/*强转成无符号数*/);
}

int realloc_buffer_read_full(ReallocBuffer *b, int fd, size_t limit);

int realloc_buffer_write(ReallocBuffer *b, int fd);
int realloc_buffer_write_maybe(ReallocBuffer *b, int fd);

int realloc_buffer_read_target(ReallocBuffer *b, int fd, size_t target_size);

void* realloc_buffer_steal(ReallocBuffer *b);

void* realloc_buffer_donate(ReallocBuffer *b, void *p, size_t size);

int realloc_buffer_printf(ReallocBuffer *b, const char *fmt, ...) _printf_(2,3);

int realloc_buffer_memchr(ReallocBuffer *buffer, uint8_t c);

#endif
