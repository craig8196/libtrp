

#include "libtrp.h"

#include <assert.h>


int
main()
{
    trip_memory_t *m = trip_memory_default();
    void *p = m->alloc(m->ud, 4);
    assert(NULL != p);
    p = m->realloc(m->ud, p, 8);
    assert(NULL != p);
    m->free(m->ud, p);
    return 0;
}
