#ifndef _LIST_H
#define _LIST_H

#include <kalloc.h>
#include <panic.h>

typedef struct List List;

struct List
{
        void  *prev;
        void  *next;
};

#define LIST(_ty)     _ty *_prev, *_next
#define FOREACH(_head, _v)  \
        for (_v = (_head)->next; (List*)(_v) != _head; _v = (_v)->_next)
#define FOREACH_SAFE(_head, _v, _nv)  \
        for (_v = (_head)->next; ((List*)(_v) != _head) && ((_nv = (_v)->_next), 1); \
             _v = _nv)
#define SAN(_head)  \
        do {  \
                if (!(_head)->prev || !(_head)->next)   \
                { \
                        panic ("bad list"); \
                } \
        } while(0)

// LOL
#define FIRST(_head)  ((_head)->next)
#define SECOND(_head) (((List*)(FIRST (_head)))->next)
#define THIRD(_head)  (((List*)(SECOND (_head)))->next)
#define FOURTH(_head) (((List*)(THIRD (_head)))->next)
#define FIFTH(_head)  (((List*)(FOURTH (_head)))->next)
#define SIXTH(_head)  (((List*)(FIFTH (_head)))->next)
#define NEXT(_v)      ((_v)->next)
#define EMPTY(_head)  ((_head)->next == (_head))

#define PUSH(_head, _v) \
        do \
        {  \
                SAN (_head);  \
                List *_p = (_head)->prev;  \
                (_head)->prev = (_v); \
                (_v)->_next = (void *)(_head);  \
                (_v)->_prev = (void *)_p; \
                _p->next = (_v);  \
        } while (0)

static inline int
listlength (List *head)
{
        struct dummy
        {
                LIST (struct dummy);
        } *dum;
        int   i = 0;

        FOREACH (head, dum)
                i++;
        return i;
}

static inline void
listdelete (void *v)
{
        List *lv = (List *)v;
        List *next = lv->next;
        List *prev = lv->prev;
        next->prev = prev;
        prev->next = next;
        lv->next = lv->prev = NULL;
}

static inline void
listinit (List *l)
{
        l->next = l;
        l->prev = l;
}

static inline List *
newlist (void)
{
        // List *l = malloc (sizeof *l);
        List *l = alloc ();
        listinit (l);
        return l;
}

#endif  // _LIST_H
