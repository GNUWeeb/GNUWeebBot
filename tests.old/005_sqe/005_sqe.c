// SPDX-License-Identifier: GPL-2.0-only
/*
 *  tests/004_tstack/004_tstack.c
 *
 *  Test case for TCP slot stack.
 *
 *  This functionality is supposed to retrieve channel in O(1).
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#include <criterion/criterion.h>

#define SQE_TEST
#include <gwbot/sqe.h>

Test(sqe, init_sqe_must_be_empty)
{
	struct sqe_master sqe;

	cr_assert_eq(sqe_init(&sqe, 100), &sqe);
	cr_assert_eq(sqe_is_empty(&sqe), true);
	cr_assert_eq(sqe_count(&sqe), 0);

	sqe_destroy(&sqe);
}


Test(sqe, enqueue_must_increment_count)
{
	struct sqe_master sqe;

	cr_assert_eq(sqe_init(&sqe, 100), &sqe);
	cr_assert_eq(sqe_is_empty(&sqe), true);
	cr_assert_eq(sqe_count(&sqe), 0);

	cr_assert_neq(sqe_enqueue(&sqe, "Hello", sizeof("Hello")), NULL);
	cr_assert_eq(sqe_is_empty(&sqe), false);
	cr_assert_eq(sqe_count(&sqe), 1);

	cr_assert_neq(sqe_enqueue(&sqe, "Hello", sizeof("Hello")), NULL);
	cr_assert_eq(sqe_is_empty(&sqe), false);
	cr_assert_eq(sqe_count(&sqe), 2);

	cr_assert_neq(sqe_enqueue(&sqe, "Hello", sizeof("Hello")), NULL);
	cr_assert_eq(sqe_is_empty(&sqe), false);
	cr_assert_eq(sqe_count(&sqe), 3);

	cr_assert_neq(sqe_enqueue(&sqe, "Hello", sizeof("Hello")), NULL);
	cr_assert_eq(sqe_is_empty(&sqe), false);
	cr_assert_eq(sqe_count(&sqe), 4);

	cr_assert_neq(sqe_enqueue(&sqe, "Hello", sizeof("Hello")), NULL);
	cr_assert_eq(sqe_is_empty(&sqe), false);
	cr_assert_eq(sqe_count(&sqe), 5);

	sqe_destroy(&sqe);
}



Test(sqe, first_enqueue_make_head_and_tail_identical)
{
	struct sqe_master sqe;

	cr_assert_eq(sqe_init(&sqe, 100), &sqe);

	cr_assert_neq(sqe_enqueue(&sqe, "Hello", sizeof("Hello")), NULL);

	cr_assert_eq(sqe.head->len, sizeof("Hello"));
	cr_assert_str_eq(sqe.head->data, "Hello");
	cr_assert_eq(sqe.tail->len, sizeof("Hello"));
	cr_assert_str_eq(sqe.tail->data, "Hello");
	cr_assert_eq(sqe.tail, sqe.head);

	sqe_destroy(&sqe);
}


Test(sqe, enqueue_must_have_correct_order)
{
	struct sqe_master sqe;
	struct sqe_node *tmp;

	cr_assert_eq(sqe_init(&sqe, 100), &sqe);

	cr_assert_neq(sqe_enqueue(&sqe, "H1", sizeof("H1")), NULL);
	cr_assert_neq(sqe_enqueue(&sqe, "H2", sizeof("H2")), NULL);
	cr_assert_neq(sqe_enqueue(&sqe, "H3", sizeof("H3")), NULL);
	cr_assert_neq(sqe_enqueue(&sqe, "H4", sizeof("H4")), NULL);
	cr_assert_neq(sqe_enqueue(&sqe, "H5", sizeof("H5")), NULL);
	cr_assert_neq(sqe_enqueue(&sqe, "H6", sizeof("H6")), NULL);

	cr_assert_neq(sqe.tail, sqe.head);

	tmp = sqe.head;
	cr_assert_str_eq(tmp->data, "H1");
	cr_assert_eq(tmp->len, sizeof("H1"));

	tmp = tmp->next;
	cr_assert_str_eq(tmp->data, "H2");
	cr_assert_eq(tmp->len, sizeof("H2"));

	tmp = tmp->next;
	cr_assert_str_eq(tmp->data, "H3");
	cr_assert_eq(tmp->len, sizeof("H3"));

	tmp = tmp->next;
	cr_assert_str_eq(tmp->data, "H4");
	cr_assert_eq(tmp->len, sizeof("H4"));

	tmp = tmp->next;
	cr_assert_str_eq(tmp->data, "H5");
	cr_assert_eq(tmp->len, sizeof("H5"));

	tmp = tmp->next;
	cr_assert_str_eq(tmp->data, "H6");
	cr_assert_eq(tmp->len, sizeof("H6"));

	cr_assert_eq(tmp, sqe.tail);

	cr_assert_eq(tmp->next, NULL);

	tmp = tmp->prev;
	cr_assert_str_eq(tmp->data, "H5");
	cr_assert_eq(tmp->len, sizeof("H5"));

	tmp = tmp->prev;
	cr_assert_str_eq(tmp->data, "H4");
	cr_assert_eq(tmp->len, sizeof("H4"));

	tmp = tmp->prev;
	cr_assert_str_eq(tmp->data, "H3");
	cr_assert_eq(tmp->len, sizeof("H3"));

	tmp = tmp->prev;
	cr_assert_str_eq(tmp->data, "H2");
	cr_assert_eq(tmp->len, sizeof("H2"));

	tmp = tmp->prev;
	cr_assert_str_eq(tmp->data, "H1");
	cr_assert_eq(tmp->len, sizeof("H1"));

	cr_assert_eq(tmp->prev, NULL);
	cr_assert_eq(tmp, sqe.head);

	sqe_destroy(&sqe);
}



Test(sqe, enqueue_more_than_capacity_returns_null_and_doesnt_add_counter)
{
	struct sqe_master sqe;

	cr_assert_eq(sqe_init(&sqe, 5), &sqe);

	cr_assert_neq(sqe_enqueue(&sqe, "H1", sizeof("H1")), NULL);
	cr_assert_eq(sqe.tail->next, NULL);
	cr_assert_neq(sqe_enqueue(&sqe, "H2", sizeof("H2")), NULL);
	cr_assert_eq(sqe.tail->next, NULL);
	cr_assert_neq(sqe_enqueue(&sqe, "H3", sizeof("H3")), NULL);
	cr_assert_eq(sqe.tail->next, NULL);
	cr_assert_neq(sqe_enqueue(&sqe, "H4", sizeof("H4")), NULL);
	cr_assert_eq(sqe.tail->next, NULL);
	cr_assert_neq(sqe_enqueue(&sqe, "H5", sizeof("H5")), NULL);
	cr_assert_eq(sqe.tail->next, NULL);

	cr_assert_eq(sqe_enqueue(&sqe, "H6", sizeof("H6")), NULL);
	cr_assert_eq(sqe_count(&sqe), 5); // shouldn't add the count

	cr_assert_eq(sqe_enqueue(&sqe, "H6", sizeof("H6")), NULL);
	cr_assert_eq(sqe_count(&sqe), 5); // shouldn't add the count

	cr_assert_eq(sqe_enqueue(&sqe, "H6", sizeof("H6")), NULL);
	cr_assert_eq(sqe_count(&sqe), 5); // shouldn't add the count

	cr_assert_eq(sqe_enqueue(&sqe, "H6", sizeof("H6")), NULL);
	cr_assert_eq(sqe_count(&sqe), 5); // shouldn't add the count

	sqe_destroy(&sqe);
}



Test(sqe, dequeue_must_decrement_count_and_in_correct_order)
{
	struct sqe_master sqe;
	struct sqe_node *tmp, *old_head, *new_head;

	cr_assert_eq(sqe_init(&sqe, 100), &sqe);
	cr_assert_eq(sqe_is_empty(&sqe), true);
	cr_assert_eq(sqe_count(&sqe), 0);

	cr_assert_neq(sqe_enqueue(&sqe, "H1", sizeof("H1")), NULL);
	cr_assert_neq(sqe_enqueue(&sqe, "H2", sizeof("H2")), NULL);
	cr_assert_neq(sqe_enqueue(&sqe, "H3", sizeof("H3")), NULL);
	cr_assert_neq(sqe_enqueue(&sqe, "H4", sizeof("H4")), NULL);
	cr_assert_neq(sqe_enqueue(&sqe, "H5", sizeof("H5")), NULL);
	cr_assert_eq(sqe_is_empty(&sqe), false);
	cr_assert_eq(sqe_count(&sqe), 5);

	old_head = sqe.head;
	new_head = sqe.head->next;
	tmp      = sqe_dequeue(&sqe);

	cr_assert_eq(tmp, old_head);
	cr_assert_str_eq(tmp->data, "H1");
	cr_assert_eq(tmp->len, sizeof("H1"));
	cr_assert_eq(sqe.head->prev, NULL);
	sqe_destroy_dequeued(tmp);
	cr_assert_eq(sqe_count(&sqe), 4);


	old_head = sqe.head;
	new_head = sqe.head->next;
	tmp      = sqe_dequeue(&sqe);

	cr_assert_eq(tmp, old_head);
	cr_assert_str_eq(tmp->data, "H2");
	cr_assert_eq(tmp->len, sizeof("H2"));
	cr_assert_eq(sqe.head->prev, NULL);
	sqe_destroy_dequeued(tmp);
	cr_assert_eq(sqe_count(&sqe), 3);


	old_head = sqe.head;
	new_head = sqe.head->next;
	tmp      = sqe_dequeue(&sqe);

	cr_assert_eq(tmp, old_head);
	cr_assert_str_eq(tmp->data, "H3");
	cr_assert_eq(tmp->len, sizeof("H3"));
	cr_assert_eq(sqe.head->prev, NULL);
	sqe_destroy_dequeued(tmp);
	cr_assert_eq(sqe_count(&sqe), 2);


	old_head = sqe.head;
	new_head = sqe.head->next;
	tmp      = sqe_dequeue(&sqe);

	cr_assert_eq(tmp, old_head);
	cr_assert_str_eq(tmp->data, "H4");
	cr_assert_eq(tmp->len, sizeof("H4"));
	cr_assert_eq(sqe.head->prev, NULL);
	sqe_destroy_dequeued(tmp);
	cr_assert_eq(sqe_count(&sqe), 1);


	old_head = sqe.head;
	new_head = sqe.head->next;
	tmp      = sqe_dequeue(&sqe);

	cr_assert_eq(tmp, old_head);
	cr_assert_str_eq(tmp->data, "H5");
	cr_assert_eq(tmp->len, sizeof("H5"));
	cr_assert_eq(sqe.head, NULL);
	sqe_destroy_dequeued(tmp);
	cr_assert_eq(sqe_count(&sqe), 0);

	sqe_destroy(&sqe);
}
