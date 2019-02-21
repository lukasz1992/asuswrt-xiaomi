/* -*- mode: C; c-basic-offset: 8; indent-tabs-mode: nil; tab-width: 8 -*- */

#ifndef LLIST_H
#define LLIST_H

typedef struct _List 		List;

struct _List {
        void *data;
        List *next;
};



/* Funcs */

#ifdef PROTOTYPES

List *		list_new(void);
List *		list_append(List *, void *);

#endif /* PROTOTYPES */

#endif /* LLIST_H */
