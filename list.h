#pragma once

#include <stddef.h>

#define container_of(ptr, type, member) \
    ((type *)((char *)(1 ? (ptr) : &((type *)0)->member) - offsetof(type, member)))

struct list {
    struct list* prev;
    struct list* next;
};

#define LIST_HEAD_INIT(p) {.prev=&(p), .next=&(p)}
#define DEFINE_LIST(name) struct list name = LIST_HEAD_INIT(name)

static inline void list_init_head(struct list* head)
{
	head->next = head;
	head->prev = head;
}


struct list* list_insert(struct list* head, struct list* node)
{
    node->next = head->next;
    node->prev = head;
    head->next->prev = node;
    head->next = node;
    return node;
}

struct list* list_insert_tail(struct list* head, struct list* node)
{
    node->next = head;
    node->prev = head->prev;
    head->prev->next = node;
    head->prev = node;
    return node;
}

#define list_entry(ptr,type,member) container_of(ptr,type,member)

#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

