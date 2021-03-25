// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/sqe.h
 *
 *  SQE
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__SQE_H
#define GWBOT__SQE_H

#include <errno.h>
#include <stdalign.h>
#include <gwbot/base.h>


#ifdef SQE_TEST
#  define inline_prod __no_inline
#else
#  define inline_prod inline
#endif

struct sqe_node {
	struct sqe_node 	*next;
	struct sqe_node 	*prev;
	size_t			len;
	struct_pad(0, 8);
	alignas(16) char	data[];
};


struct sqe_master {
	struct sqe_node		*head;
	struct sqe_node		*tail;
	uint32_t		num;
	uint32_t		capacity;
};


static inline_prod struct sqe_master *sqe_init(struct sqe_master *sqe,
					       uint32_t capacity)
{
	sqe->head     = NULL;
	sqe->tail     = NULL;
	sqe->num      = 0u;
	sqe->capacity = capacity;

	return sqe;
}


static inline_prod uint32_t sqe_count(struct sqe_master *sqe)
{
	return sqe->num;
}


static inline_prod bool sqe_is_empty(struct sqe_master *sqe)
{
	return sqe_count(sqe) == 0u;
}


static inline_prod struct sqe_node *sqe_enqueue(struct sqe_master *sqe,
						const void *data, size_t len)
{
	struct sqe_node *node, *old_tail, *new_tail;
	uint32_t num = sqe->num;

	if (unlikely(num >= sqe->capacity)) {
		errno = EAGAIN;
		return NULL;
	}

	node = calloc(1, sizeof(*node) + len + 1);
	if (unlikely(node == NULL)) {
		int err = errno;
		pr_err("calloc(): " PRERF, PREAR(err));
		errno = ENOMEM;
		return NULL;
	}

	node->len = len;
	memcpy(node->data, data, len);

	node->next = NULL;
	new_tail   = node;

	if (unlikely(num == 0)) {
		node->prev = NULL;
		sqe->head  = node;
	} else {
		old_tail       = sqe->tail;
		old_tail->next = new_tail;
		new_tail->prev = old_tail;
		new_tail->next = NULL;
	}

	sqe->tail = new_tail;
	sqe->num++;
	return node;
}


static inline_prod void sqe_destroy(struct sqe_master *sqe)
{
	uint32_t num;
	struct sqe_node *head, *tmp;

	num  = sqe->num;
	head = sqe->head;

	while (head) {
		num--;
		tmp  = head;
		head = head->next;
		free(tmp);
	}

	TASSERT(num == 0);

	sqe->head     = NULL;
	sqe->tail     = NULL;
	sqe->num      = 0u;
	sqe->capacity = 0u;
}


static inline_prod struct sqe_node *sqe_dequeue(struct sqe_master *sqe)
{
	uint32_t num;
	struct sqe_node *ret, *old_head, *new_head;

	num = sqe->num;
	if (unlikely(num == 0))
		return NULL;

	ret            = sqe->head;
	old_head       = sqe->head;
	new_head       = old_head->next;

	if (new_head)
		new_head->prev = NULL; /* Head must not have prev member */

	sqe->head = new_head;
	sqe->num--;
	return ret;
}


static inline_prod void sqe_destroy_dequeued(struct sqe_node *node)
{
	free(node);
}


#endif /* #ifndef GWBOT__SQE_H */
