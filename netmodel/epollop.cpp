#ifdef __linux__
#include <stdint.h>
#include <sys/types.h>
#include <sys/resource.h>

#include <sys/time.h>
#include <sys/queue.h>
#include <sys/epoll.h>
#include <signal.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <iostream>
#include "modelmanager.h"
#include "netmodeldef.h"

struct epollop {
	struct epoll_event *events;
	int nevents;
	int epfd;
};

long
evutil_tv_to_msec(const struct timeval *tv)
{
	if (tv->tv_usec > 1000000 || tv->tv_sec > MAX_SECONDS_IN_MSEC_LONG)
		return -1;

	return (tv->tv_sec * 1000) + ((tv->tv_usec + 999) / 1000);
}

static void *epoll_init(void *base);
static int epoll_dispatch(void *base, struct timeval *tv);
static void epoll_dealloc(void *base);

static int epoll_add(void *base, sockfd fd, short old,
    short events, void *p);
static int epoll_del(void *base, sockfd fd, short old,
    short events, void *p);

static int
epoll_apply_one_change(void* base,
    struct epollop *epollop,
    const struct event_change *ch);

struct ModelOp epollops = {
	"epoll",
	epoll_init,
	epoll_add,
	epoll_del,
	epoll_dispatch,
	epoll_dealloc
};

//oldevent 为我们自己定义的事件类型如EV_READ/EV_WRITE等表示过去的事件类型
//curevent表示现在的事件类型EV_READ/EV_WRITE等
//epollevent 为EPOLL内核定义的事件类型，如EPOLLMOD/EPOLLDEL/EPOLLADD等
struct event_change {
	int fd;
	short oldevent;
	short curevent;
	int epollevent;
	int op;
};

#define INITIAL_NEVENT 32
#define MAX_NEVENT 4096

/* On Linux kernels at least up to 2.6.24.4, epoll can't handle timeout
 * values bigger than (LONG_MAX - 999ULL)/HZ.  HZ in the wild can be
 * as big as 1000, and LONG_MAX can be as small as (1<<31)-1, so the
 * largest number of msec we can support here is 2147482.  Let's
 * round that down by 47 seconds.
 */
#define MAX_EPOLL_TIMEOUT_MSEC (35*60*1000)

int
evutil_make_socket_closeonexec(sockfd fd)
{

	int flags;
	if ((flags = fcntl(fd, F_GETFD, NULL)) < 0) {
		std::cout<< "fcntl(%d, F_GETFD)" <<fd <<std::endl;
		return -1;
	}
	if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
		std::cout<<"fcntl(%d, F_SETFD)" <<fd<<std::endl;
		return -1;
	}

	return 0;
}


static void *
epoll_init(void *base)
{
	int epfd;
	struct epollop *epollop;

	/* Initialize the kernel queue.  (The size field is ignored since
	 * 2.6.8.) */
	if ((epfd = epoll_create(32000)) == -1) {
		if (errno != ENOSYS)
			printf("epoll_create");
		return (NULL);
	}

	evutil_make_socket_closeonexec(epfd);

	if (!(epollop = (struct epollop*)calloc(1, sizeof(struct epollop)))) {
		close(epfd);
		return (NULL);
	}

	epollop->epfd = epfd;

	/* Initialize fields */
	epollop->events = (struct epoll_event*)calloc(INITIAL_NEVENT, sizeof(struct epoll_event));
	if (epollop->events == NULL) {
		free(epollop);
		close(epfd);
		return (NULL);
	}
	epollop->nevents = INITIAL_NEVENT;

	return (epollop);
}


static const char *
epoll_op_to_string(int op)
{
	return op == EPOLL_CTL_ADD?"ADD":
	    op == EPOLL_CTL_DEL?"DEL":
	    op == EPOLL_CTL_MOD?"MOD":
	    "???";
}


static int
epoll_add(void *base, sockfd fd,short old,
   short events, void *p)
{
	int epollevent = 0; 
	int op = 0 ;

	op = EPOLL_CTL_ADD;
	
	//旧事件不为0
	if(old)
	{
		op = EPOLL_CTL_MOD;
	}

	cout << "1" << epollevent <<endl;	
	if(events & EV_READ)
	{
		epollevent |= EPOLLIN;
	cout << "read"<<epollevent <<endl;
	}

	if(events & EV_WRITE)
	{
		epollevent |= EPOLLOUT;
	cout << "write"<<epollevent <<endl;
	}

	epollevent |= EPOLLET;
	cout << "EPOLLET is: " << EPOLLET <<endl;
	cout << "et"<<epollevent << endl;

	struct event_change eventch;
	eventch.fd = fd;
	eventch.epollevent = epollevent;
	eventch.oldevent = old;
	eventch.curevent = events;
	eventch.op = op;

	
	struct epollop * epollops = (struct epollop*)(((ModelManager*)base)->getModelData());
	
	return epoll_apply_one_change(base, epollops, &eventch);

}


static int
epoll_del(void *base, sockfd fd,
    short old, short events, void *p)
{
	int op = 0;
	int epollevent = 0;
	
	op = EPOLL_CTL_DEL;
	
	
	
	//旧事件中有读，新事件中没读
	if((old&EV_READ)&& !(events&EV_READ))
	{
		if(events&EV_WRITE)
		{
			//由于当前事件中有写事件
			//不能从epfd中删除该描述符
			//更改模式
			op = EPOLL_CTL_MOD;
			epollevent |= EPOLLOUT;
		}

	}

	if((old&EV_WRITE)&&!(events&EV_WRITE))
	{
		if(events&EV_READ)
		{
			op = EPOLL_CTL_MOD;
			epollevent |= EPOLLIN;
		}
	}

	epollevent |= EPOLLET;

	struct event_change eventch;
	eventch.fd = fd;
	eventch.oldevent = old;
	eventch.curevent = events;
	eventch.epollevent = epollevent;
	eventch.op = op;

	
	struct epollop * epollops = (struct epollop*)(((ModelManager*)base)->getModelData());
	return epoll_apply_one_change(base, epollops, &eventch);

	
}


static int
epoll_apply_one_change(void *base,
    struct epollop *epollop, const struct event_change *ch)
	{
		struct epoll_event epev;
		
		memset(&epev, 0, sizeof(epev));
		epev.data.fd = ch->fd;
		epev.events = ch->epollevent;
		
		if (epoll_ctl(epollop->epfd, ch->op, ch->fd, &epev) == -1) {
			if (ch->op == EPOLL_CTL_MOD && errno == ENOENT) {
				/* If a MOD operation fails with ENOENT, the
				 * fd was probably closed and re-opened.  We
				 * should retry the operation as an ADD.
				 */
				if (epoll_ctl(epollop->epfd, EPOLL_CTL_ADD, ch->fd, &epev) == -1) {
					printf("Epoll MOD(%d) on %d retried as ADD; that failed too\n",
					    (int)epev.events, ch->fd);
					return -1;
				} else {
					
					printf("Epoll MOD %d on %d retried as ADD; succeeded.\n",(int)(epev.events), ch->fd);
				}
			} else if (ch->op == EPOLL_CTL_ADD && errno == EEXIST) {
				/* If an ADD operation fails with EEXIST,
				 * either the operation was redundant (as with a
				 * precautionary add), or we ran into a fun
				 * kernel bug where using dup*() to duplicate the
				 * same file into the same fd gives you the same epitem
				 * rather than a fresh one.  For the second case,
				 * we must retry with MOD. */
				if (epoll_ctl(epollop->epfd, EPOLL_CTL_MOD, ch->fd, &epev) == -1) {
					
					printf("Epoll ADD(%d) on %d retried as MOD; that failed too\n",
					    (int)epev.events, ch->fd);
					return -1;
				} else {
				
					printf("Epoll ADD %d on %d retried as MOD; succeeded\n", (int)(epev.events), ch->fd);
				}
			} else if (ch->op == EPOLL_CTL_DEL &&
			    (errno == ENOENT || errno == EBADF ||
				errno == EPERM)) {
				/* If a delete fails with one of these errors,
				 * that's fine too: we closed the fd before we
				 * got around to calling epoll_dispatch. */
				printf(("Epoll DEL(%d) on fd %d gave %s: DEL was unnecessary.\n",
					(int)epev.events,
					ch->fd,
					strerror(errno)));
			} else { //if (ch->op == EPOLL_CTL_DEL &&(errno == ENOENT || errno == EBADF ||errno == EPERM))
				printf("Epoll %s(%d) on fd %d failed.  Old events were %d; to change New event are %d\n",
				    epoll_op_to_string(ch->op),
				    (int)epev.events,
				    (int)ch->fd,
				    (int)ch->oldevent,
				    (int)ch->curevent);
				return -1;
			}
		} else {//if (epoll_ctl(epollop->epfd, ch->op, ch->fd, &epev) == -1)
			
			printf("Epoll %s(%d) on fd %d okay. Old events were %d; to change New event are %d\n",
				epoll_op_to_string(ch->op),
				(int)epev.events,
				(int)ch->fd,
				(int)ch->oldevent,
				(int)ch->curevent);
		} 

	return 0;
}




static int
epoll_dispatch(void *base, struct timeval *tv)
{
	struct epollop * epollop = (struct epollop *)(((ModelManager*)base)->getModelData());
	
	struct epoll_event *events = epollop->events;
	int i, res;
	long timeout = -1;

	if (tv != NULL) {
		timeout = evutil_tv_to_msec(tv);
		if (timeout < 0 || timeout > MAX_EPOLL_TIMEOUT_MSEC) {
			/* Linux kernels can wait forever if the timeout is
			 * too big; see comment on MAX_EPOLL_TIMEOUT_MSEC. */
			timeout = MAX_EPOLL_TIMEOUT_MSEC;
		}
	}

	res = epoll_wait(epollop->epfd, events, epollop->nevents, timeout);



	if (res == -1) {
		if (errno != EINTR) {
			printf("epoll_wait");
			return (-1);
		}

		return (0);
	}

	printf("%s: epoll_wait reports %d\n", __func__, res);
	

	for (i = 0; i < res; i++) {
		int what = events[i].events;
		short ev = 0;

		if (what & (EPOLLHUP|EPOLLERR)) {
			ev = EV_READ | EV_WRITE;
		} else {
			if (what & EPOLLIN)
				ev |= EV_READ;
			if (what & EPOLLOUT)
				ev |= EV_WRITE;
		}

		if (!ev)
			continue;

		//更新evmap，并且将事件放入active队列
		((ModelManager *)base)->insertActiveList(events[i].data.fd, ev);
	}
	//epoll 扩容，最多支持MAX_NEVENT个事件
	if (res == epollop->nevents && epollop->nevents < MAX_NEVENT) {
		/* We used all of the event space this time.  We should
		   be ready for more events next time. */
		int new_nevents = epollop->nevents * 2;
		struct epoll_event *new_events;

		new_events = (struct epoll_event *)realloc(epollop->events,
		    new_nevents * sizeof(struct epoll_event));
		if (new_events) {
			epollop->events = new_events;
			epollop->nevents = new_nevents;
		}
	}

	return (0);
}


static void
epoll_dealloc(void *base)
{

	struct epollop * epollop = (struct epollop *)(((ModelManager*)base)->getModelData());
	if (epollop->events)
		free(epollop->events);
	if (epollop->epfd >= 0)
		close(epollop->epfd);

	memset(epollop, 0, sizeof(struct epollop));
	free(epollop);
}

#endif
