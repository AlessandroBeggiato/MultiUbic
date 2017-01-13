
#ifndef _NODELIST_H_
#define _NODELIST_H_

struct nl {
	void * node;
	struct nl * next;
};

struct nl * nl_push (struct nl **, void *);
struct nl * nl_insert (struct nl **, void*);
struct nl* nl_insert2 (struct nl **list, void *node,
		int (* cmp) (const void *n1, const void *n2));
void nl_delete (struct nl *);
char nl_compare (const struct nl *list1, const struct nl *list2);
char nl_test (const struct nl *list, void *node); /*PAOLO*/

// FRANCESCO
/*
 * Find an element in a list using the comparator cmp passed as argument.
 */
char nl_find_custom(struct nl * list, void * element,
    int (* cmp) (void *n1, void * n2));
void nl_freememory(struct nl * list);
// \FRANCESCO
#endif
