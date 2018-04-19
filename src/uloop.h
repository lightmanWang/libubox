/*
 * uloop - event loop implementation
 *
 * Copyright (C) 2010-2013 Felix Fietkau <nbd@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifndef __ULOOP_H__
#define __ULOOP_H__

#include <sys/time.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>
#include <signal.h>

#if defined(__APPLE__) || defined(__FreeBSD__)
#define USE_KQUEUE
#else
#define USE_EPOLL
#endif

#include "list.h"

struct uloop_fd;
struct uloop_timeout;
struct uloop_process;

typedef void (*uloop_fd_handler)(struct uloop_fd *u, unsigned int events);
typedef void (*uloop_timeout_handler)(struct uloop_timeout *t);
typedef void (*uloop_process_handler)(struct uloop_process *c, int ret);

#define ULOOP_READ		(1 << 0)
#define ULOOP_WRITE		(1 << 1)
#define ULOOP_EDGE_TRIGGER	(1 << 2)
#define ULOOP_BLOCKING		(1 << 3)

#define ULOOP_EVENT_MASK	(ULOOP_READ | ULOOP_WRITE)

/* internal flags */
#define ULOOP_EVENT_BUFFERED	(1 << 4)
#ifdef USE_KQUEUE
#define ULOOP_EDGE_DEFER	(1 << 5)
#endif

#define ULOOP_ERROR_CB		(1 << 6)

struct uloop_fd
{
	uloop_fd_handler cb; /* 文件描述符对应的处理函数(基于events) */
	int fd;
	bool eof;
	bool error;
	bool registered; /*是否已经添加到epoll的监控队列*/
	uint8_t flags;   /*ULOOP_READ | ULOOP_WRITE | ULOOP_BLOCKING等*/
};

struct uloop_timeout
{
	struct list_head list;
	bool pending;    //添加一个新的timeout pending是true， false删除该节点timeout

	uloop_timeout_handler cb;  //超时处理函数
	struct timeval time;       //超时时间
};

struct uloop_process
{
	struct list_head list;
	bool pending;    //添加一个新的子进程 pending是true， false删除该节点

	uloop_process_handler cb;  // 子进程退出时调用
	pid_t pid;
};

extern bool uloop_cancelled;
extern bool uloop_handle_sigchld;

int uloop_fd_add(struct uloop_fd *sock, unsigned int flags);
int uloop_fd_delete(struct uloop_fd *sock);

int uloop_timeout_add(struct uloop_timeout *timeout);
int uloop_timeout_set(struct uloop_timeout *timeout, int msecs);
int uloop_timeout_cancel(struct uloop_timeout *timeout);
int uloop_timeout_remaining(struct uloop_timeout *timeout);

int uloop_process_add(struct uloop_process *p);
int uloop_process_delete(struct uloop_process *p);

static inline void uloop_end(void)
{
	uloop_cancelled = true;
}

int uloop_init(void);
void uloop_run(void);
void uloop_done(void);

#endif
