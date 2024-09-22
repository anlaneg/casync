/* SPDX-License-Identifier: LGPL-2.1+ */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse-util.h"
#include "time-util.h"
#include "util.h"

int parse_size(const char *t, uint64_t *size) {

        struct table {
                const char *suffix;
                unsigned long long factor;
        };

        static const struct table table[] = {
                { "E", 1024ULL*1024ULL*1024ULL*1024ULL*1024ULL*1024ULL },
                { "P", 1024ULL*1024ULL*1024ULL*1024ULL*1024ULL },
                { "T", 1024ULL*1024ULL*1024ULL*1024ULL },
                { "G", 1024ULL*1024ULL*1024ULL },
                { "M", 1024ULL*1024ULL },
                { "K", 1024ULL },
                { "B", 1ULL },
                { "",  1ULL },
        };

        const char *p;
        unsigned long long r = 0;
        unsigned n_entries, start_pos = 0;

        assert(t);
        assert(size);

        n_entries = ELEMENTSOF(table);/*table元素数*/

        p = t;
        do {
                unsigned long long l, tmp;
                double frac = 0;
                char *e;
                unsigned i;

                /*跳前导的空字符*/
                p += strspn(p, WHITESPACE);

                if (*p == '-')
                	/*不能以‘-’开头*/
                        return -ERANGE;

                /*转换为数字*/
                errno = 0;
                l = strtoull(p, &e, 10);
                if (errno > 0)
                        return -errno;
                if (e == p)
                        return -EINVAL;

                /*遇到'.'*/
                if (*e == '.') {
                        e++;

                        /* strtoull() itself would accept space/+/- */
                        if (*e >= '0' && *e <= '9') {
                        	/*只能是数字，且为10进制*/
                                unsigned long long l2;
                                char *e2;

                                l2 = strtoull(e, &e2, 10);
                                if (errno > 0)
                                        return -errno;

                                /* Ignore failure. E.g. 10.M is valid */
                                frac = l2;
                                for (; e < e2; e++)
                                        frac /= 10;
                        }
                }

                /*跳空字符*/
                e += strspn(e, WHITESPACE);
                /*确定单位*/
                for (i = start_pos; i < n_entries; i++)
                        if (startswith(e, table[i].suffix))
                                break;

                /*未找到合适的单位，报错*/
                if (i >= n_entries)
                        return -EINVAL;

                if (l + (frac > 0) > ULLONG_MAX / table[i].factor)
                        return -ERANGE;

                tmp = l * table[i].factor + (unsigned long long) (frac * table[i].factor);
                if (tmp > ULLONG_MAX - r)
                        return -ERANGE;

                /*将结果合入到r中*/
                r += tmp;
                if ((unsigned long long) (uint64_t) r != r)
                        return -ERANGE;

                p = e + strlen(table[i].suffix);

                start_pos = i + 1;

        } while (*p);

        /*设置内容*/
        *size = r;

        return 0;
}

char *format_bytes(char *buf, size_t l, uint64_t t) {
        unsigned i;

        /* This only does IEC units so far */

        static const struct {
                const char *suffix;
                uint64_t factor;
        } table[] = {
                { "E", UINT64_C(1024)*UINT64_C(1024)*UINT64_C(1024)*UINT64_C(1024)*UINT64_C(1024)*UINT64_C(1024) },
                { "P", UINT64_C(1024)*UINT64_C(1024)*UINT64_C(1024)*UINT64_C(1024)*UINT64_C(1024) },
                { "T", UINT64_C(1024)*UINT64_C(1024)*UINT64_C(1024)*UINT64_C(1024) },
                { "G", UINT64_C(1024)*UINT64_C(1024)*UINT64_C(1024) },
                { "M", UINT64_C(1024)*UINT64_C(1024) },
                { "K", UINT64_C(1024) },
        };

        if (t == (uint64_t) -1)
                return NULL;

        for (i = 0; i < ELEMENTSOF(table); i++) {

                if (t >= table[i].factor) {
                        snprintf(buf, l,
                                 "%" PRIu64 ".%" PRIu64 "%s",
                                 t / table[i].factor,
                                 ((t*UINT64_C(10)) / table[i].factor) % UINT64_C(10),
                                 table[i].suffix);

                        goto finish;
                }
        }

        snprintf(buf, l, "%" PRIu64 "B", t);

finish:
        buf[l-1] = 0;
        return buf;

}
