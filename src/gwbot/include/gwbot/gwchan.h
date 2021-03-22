// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/gwchan.h
 *
 *  Channel
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__GWCHAN_H
#define GWBOT__GWCHAN_H

#include <time.h>
#include <pthread.h>
#include <gwbot/base.h>


struct chan_pkt {
	uint16_t		len;
	char			data[8096];
};


/*
 * Channel
 */
struct gwchan {
	int			cli_fd;
	struct_pad(0, 4);
	time_t			started_at;
	size_t			recv_s;
	pthread_t		thread;
	struct chan_pkt		pkt;
	struct_pad(1, 6);
};


#endif /* #ifndef GWBOT__GWCHAN_H */
