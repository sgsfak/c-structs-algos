#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef uint32_t (*lch_hfn)(const char* s, size_t len);

    /*
     * The Dan Bernstein popuralized hash..  See
     * https://github.com/pjps/ndjbdns/blob/master/cdb_hash.c#L26 Due to hash
     * collisions it seems to be replaced with "siphash" in n-djbdns, see
     * https://github.com/pjps/ndjbdns/commit/16cb625eccbd68045737729792f09b4945a4b508
     */
    uint32_t djb33_hash(const char* s, size_t len);



    /*
     *
     * The Java hash, but really no-one seems to know where it came from, see
     * https://bugs.java.com/bugdatabase/view_bug.do?bug_id=4045622
     */
    uint32_t h31_hash(const char* s, size_t len);


    /*
     * The FNV Hash
     * See: http://www.isthe.com/chongo/tech/comp/fnv/
     *      https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
     */
    uint32_t fnv32_hash(const char *str, size_t len);



    /*
     * "This came from ejb's hsearch."
     */
    uint32_t ejb_hash(const char *s, size_t len);


    /*
     * Bob Jenkins "One-at-a-time" hash
     */
    uint32_t oat_hash(const char *s, size_t len);


    uint32_t jen_hash(const char *k, size_t length);
    uint32_t elf_hash(const char *key, size_t len);

#ifdef __cplusplus
}
#endif
