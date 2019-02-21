/* -*- mode: C; c-basic-offset: 8; indent-tabs-mode: nil; tab-width: 8 -*- */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include "llist.h"

List *
list_new(void)
{
	List *new_list;

	new_list = malloc(sizeof(List));

	new_list->data = NULL;
	new_list->next = NULL;

	return new_list;
}


List *
list_append(List *l, void *data)
{
        if (l->data == NULL) {
                l->data = data;
                return l;
        }

        while (l->next) {
                l = l->next;
        }

        l->next = list_new();
        l->next->data = data;
        l->next->next = NULL;
}
        
