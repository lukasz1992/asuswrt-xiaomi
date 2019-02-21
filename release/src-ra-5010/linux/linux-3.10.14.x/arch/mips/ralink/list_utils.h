#ifndef __LIST_H__
#define __LIST_H__

//#include <assert.h>


/*******************************************************************************/
/* LIST definitions                                                            */
/*******************************************************************************/
#define INIT_LIST(ptr) { (ptr)->prev = (ptr); (ptr)->next = (ptr); }

#define LIST_ENTRY(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

#define LIST_INIT(name) struct list name = { &(name), &(name) }

#define LIST_FOR_EACH(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define LIST_FOR_EACH_INIT(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define LIST_FOR_EACH_PREV(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)


#define LIST_FOR_EACH_SAFE(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)
	
#define LIST_FOR_EACH_PREV_SAFE(pos, p, head) \
	for (pos = (head)->prev, p = pos->prev; pos != (head); pos = p, p = pos->prev)

#ifdef DEBUG
#define LIST_CHECK(x)	assert(((x)->prev->next == (x)) && ((x) == (x)->next->prev));
#else
#define LIST_CHECK(x,y)	if(!x) return y;
#endif

#define LIST_CHECK_FLIST(x)	if(!x) return NULL;
	

/*******************************************************************************/
/* LIST prototypes                                                             */
/*******************************************************************************/
struct list {
	struct list *prev, *next;
};


/*******************************************************************************/
/* LIST variables                                                              */
/*******************************************************************************/


/*******************************************************************************/
/* LIST functions                                                              */
/*******************************************************************************/
#if 0
extern void list_add(struct list *entry, struct list *head);
extern void list_add_tail(struct list *entry, struct list *head);
extern void list_del(struct list *entry);
extern void list_del_init(struct list *entry);
extern void list_replace(struct list *orig, struct list *repl);
extern int list_empty(const struct list *head);
#endif

#endif	// __LIST_H__
