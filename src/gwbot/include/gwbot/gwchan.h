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
	char			data[8192];
};


struct gwchan {
	int			chan_fd;
	uint32_t		chan_idx;
	time_t			started_at;
	size_t			recv_s;
	union {
		struct chan_pkt		pkt;
		char			raw_buf[sizeof(struct chan_pkt)];
	} uni_pkt;
	uint16_t		src_port;
	char			src_ip[IPV4_L];
	struct_pad(0, 4);
};


#endif /* #ifndef GWBOT__GWCHAN_H */
