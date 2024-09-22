/* SPDX-License-Identifier: LGPL-2.1+ */

#include <assert.h>
#include <errno.h>

#include "cachunk.h"
#include "cachunkid.h"
#include "log.h"

static char encode_char(uint8_t x) {
        x &= 0xF;
        return (x < 10 ? '0' : 'a' - 10) + x;
}

static int decode_char(char x) {
        if (x >= '0' && x <= '9')
                return x - '0';
        if (x >= 'a' && x <= 'f')
                return x - 'a' + 10;

        return -EINVAL;
}

/*字符串形式的chunk id解析成CaChunkID*/
CaChunkID* ca_chunk_id_parse(const char *v, CaChunkID *ret) {
        CaChunkID id;
        size_t i;

        assert(v);
        assert(ret);

        for (i = 0; i < sizeof(CaChunkID); i++) {
                int x, y;

                x = decode_char(v[i*2]);
                if (x < 0)
                        return NULL;
                y = decode_char(v[i*2+1]);
                if (y < 0)
                        return NULL;

                id.bytes[i] = (uint8_t) x << 4 | (uint8_t) y;
        }

        if (v[sizeof(CaChunkID)*2] != 0)
                return NULL;

        *ret = id;
        return ret;
}

/*将id按16进制进行解释，并转为字符串，填入到v中*/
char* ca_chunk_id_format(const CaChunkID *id, char v[CA_CHUNK_ID_FORMAT_MAX]) {
        size_t i;

        assert(id);
        assert(v);

        /*将id按16进制进行解释，并转为字符串*/
        for (i = 0; i < sizeof(CaChunkID); i++) {
                v[i*2] = encode_char(id->bytes[i] >> 4);
                v[i*2+1] = encode_char(id->bytes[i] & 0xF);
        }

        v[sizeof(CaChunkID) * 2] = 0;
        return v;
}

int ca_chunk_id_make(CaDigest *digest, const void *p, size_t l, CaChunkID *ret) {
        if (!digest)
                return -EINVAL;
        if (!p)
                return -EINVAL;
        if (l < CA_CHUNK_SIZE_LIMIT_MIN)
                return -EINVAL;
        if (l > CA_CHUNK_SIZE_LIMIT_MAX)
                return -EINVAL;
        if (!ret)
                return -EINVAL;

        if (ca_digest_get_size(digest) != sizeof(CaChunkID))
                return -EINVAL;

        ca_digest_reset(digest);
        ca_digest_write(digest, p, l);

        memcpy(ret, ca_digest_read(digest), sizeof(CaChunkID));
        return 0;
}

char* ca_chunk_id_format_path(
                const char *prefix/*前缀*/,
                const CaChunkID *chunkid/*chunkid*/,
                const char *suffix/*后缀*/,
                char buffer[]/*format后的路径*/) {

        size_t n;

        assert(chunkid);
        assert(buffer);

        if (prefix) {
        		/*填充前缀*/
                n = strlen(prefix);
                memcpy(buffer, prefix, n);
        } else
                n = 0;

        /*填写chunkid*/
        ca_chunk_id_format(chunkid, buffer + n + 4 + 1);
        /*自chunkid中取其前4个字节*/
        memcpy(buffer + n, buffer + n + 4 + 1, 4);
        /*单加一个'/'*/
        buffer[n + 4] = '/';

        /*如果有需要，填充后缀*/
        if (suffix)
                strcpy(buffer + n + 4 + 1 + CA_CHUNK_ID_FORMAT_MAX - 1, suffix);

        /*返回填充好的buffer*/
        return buffer;
}
