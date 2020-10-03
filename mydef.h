#ifndef __MYDEF_H__
#define __MYDEF_H__

#include <stdlib.h>
#include <string.h>

#define max(a,b) ((a)>(b)?a:b)
#define min(a,b) ((a)<(b)?a:b)
/**
 * Implements a Queue using a circular buffer
 * that auto-expands as new elements are 
 * pushed into it. It can also be used as a
 * dynamic array where the elements are appended
 * in the tail...
 */
#define circq(type) struct {\
    type* v;                \
    int size;               \
    int len;                \
    int front;              \
}

#define circq_init(q) ((q)->v=NULL,(q)->size=0,(q)->len=-1,(q)->front=0)
#define circq_free(q) (free((q)->v))

#define circq_empty(q) ((q)->len<=0)
#define circq_front(q) (circq_empty(q) ? NULL : (q)->v+(q)->front)
#define circq_tail(q) (circq_empty(q) ? NULL : (q)->v+((q)->front+(q)->len-1)%((q)->size))
#define circq_size(q) (circq_empty(q) ? 0 : (q)->len)
#define circq_elem(q,pos) ((circq_empty(q) || (pos)<0 || (pos)>=(q)->len) ? NULL : (q)->v+(((q)->front+(pos))%((q)->size)))
#define circq_for(q,qi,it) for(qi=0, it=circq_elem(q,qi);it!=NULL;qi++,it=circq_elem(q,qi))
#define circq_for_rev(q,qi,it) for(qi=circq_size(q)-1, it=circq_elem(q,qi);it!=NULL;qi--,it=circq_elem(q,qi))

#define circq_extend(q) do {       \
    if ((q)->len<0) (q)->len=0;    \
    if ((q)->len == (q)->size) {   \
        int __nz = ((q)->size << 1)+1; \
        void* __v = calloc(__nz, sizeof(*(q)->v)); \
        if ((q)->len > 0) {        \
            int __k = min((q)->size,(q)->front+(q)->len)-(q)->front; \
            memcpy(__v, (q)->v+(q)->front, __k * sizeof(*(q)->v)); \
            if ((q)->front+(q)->len > (q)->size) { \
                int __l = (q)->front+(q)->len - (q)->size; \
                memcpy((char*)__v+__k*sizeof(*(q)->v), (q)->v, sizeof(*(q)->v)*__l); \
            }                      \
        }                          \
        (q)->v = __v;              \
        (q)->size = __nz;          \
        (q)->front = 0;            \
    }                              \
} while(0)

#define circq_push_back(q, elem) do { \
    circq_extend(q);                  \
    (q)->v[((q)->front+(q)->len)%(q)->size] = elem; \
    (q)->len++;                       \
} while(0)

#define circq_pop_front(q, elem) do { \
    if ((q)->len > 0) {                  \
        (elem) = (q)->v[(q)->front]; \
        (q)->front = ((q)->front+1)%(q)->size; \
        (q)->len--;                  \
    }                                \
} while(0)

#endif
