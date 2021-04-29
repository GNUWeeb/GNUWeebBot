// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/gwthread.h
 *
 *  GNUWeebBot thread wrapper
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__GWTHREAD_H
#define GWBOT__GWTHREAD_H

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>

#ifdef NDEBUG
#  define TEST_INIT_LEAK 0
#else
#  define TEST_INIT_LEAK 1
#endif

typedef struct _gwlock_t {
#if TEST_INIT_LEAK
	void			*init;
#endif
	bool			need_destroy;
	pthread_mutex_t		mutex;
} gwlock_t;


typedef struct _gwcond_t {
#if TEST_INIT_LEAK
	void			*init;
#endif
	bool			need_destroy;
	pthread_cond_t		cond;
} gwcond_t;


static __always_inline int gw_mutex_init(gwlock_t *lock,
					 const pthread_mutexattr_t *attr)
{
#if TEST_INIT_LEAK
	lock->init = malloc(1);
#endif
	lock->need_destroy = true;
	return pthread_mutex_init(&lock->mutex, attr);
}


static __always_inline int gw_cond_init(gwcond_t *cond,
					const pthread_condattr_t *attr)
{
#if TEST_INIT_LEAK
	cond->init = malloc(1);
#endif
	cond->need_destroy = true;
	return pthread_cond_init(&cond->cond, attr);
}


static __always_inline int gw_mutex_destroy(gwlock_t *lock)
{
	int ret = 0;
	if (likely(lock->need_destroy)) {
		ret = pthread_mutex_destroy(&lock->mutex);
		lock->need_destroy = false;
	}
#if TEST_INIT_LEAK
	free(lock->init);
#endif
	return ret;
}


static __always_inline int gw_cond_destroy(gwcond_t *cond)
{
	int ret = 0;
	if (likely(cond->need_destroy)) {
		ret = pthread_cond_destroy(&cond->cond);
		cond->need_destroy = false;
	}
#if TEST_INIT_LEAK
	free(cond->init);
#endif
	return ret;
}


static __always_inline int gw_mutex_lock(gwlock_t *restrict lock)
{
	assert(lock->need_destroy);
	return pthread_mutex_lock(&lock->mutex);
}


static __always_inline int gw_mutex_unlock(gwlock_t *restrict lock)
{
	assert(lock->need_destroy);
	return pthread_mutex_unlock(&lock->mutex);
}


static __always_inline int gw_cond_wait(gwcond_t *restrict cond,
					gwlock_t *restrict lock)
{
	assert(cond->need_destroy);
	assert(lock->need_destroy);
	return pthread_cond_wait(&cond->cond, &lock->mutex);
}


static __always_inline int gw_cond_signal(gwcond_t *restrict cond)
{
	assert(cond->need_destroy);
	return pthread_cond_signal(&cond->cond);
}


static __always_inline int gw_cond_timedwait(gwcond_t *restrict cond,
					     gwlock_t *restrict lock,
		      			     const
		      			     struct timespec *restrict abstime)
{
	assert(cond->need_destroy);
	assert(lock->need_destroy);
	return pthread_cond_timedwait(&cond->cond, &lock->mutex, abstime);
}


static __always_inline int gw_cond_timedwait_rel(gwcond_t *restrict cond,
						 gwlock_t *restrict lock,
						 uint32_t rel_time)
{
	int ret;
	struct timeval tmvl;
	struct timespec abstime;

	memset(&abstime, 0, sizeof(abstime));
	if (unlikely(gettimeofday(&tmvl, NULL) < 0)) {
		ret = errno;
		pr_err("gettimeofday(): " PRERF, PREAR(ret));
		return -ret;
	}

	abstime.tv_sec = tmvl.tv_sec + rel_time;
	return gw_cond_timedwait(cond, lock, &abstime);
}


#endif /* #ifndef GWBOT__GWTHREAD_H */
