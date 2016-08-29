#include "modelmanager.h"


#ifdef WIN32
#include "netmodeldef.h"
#define XFREE(ptr) do { if (ptr) free(ptr); } while (0)


struct win_fd_set {
	u_int fd_count;
	SOCKET fd_array[1];
};

struct win32op {
	unsigned num_fds_in_fd_sets;
	int resize_out_sets;
	struct win_fd_set *readset_in;
	struct win_fd_set *writeset_in;
	struct win_fd_set *readset_out;
	struct win_fd_set *writeset_out;
	struct win_fd_set *exset_out;
	unsigned signals_are_broken : 1;
};

static void *win32_init(void *);
static int win32_add(void *, sockfd, short old, short events, void *_idx);
static int win32_del(void *, sockfd, short old, short events, void *_idx);
static int win32_dispatch(void *base, struct timeval *);
static void win32_dealloc(void *);

struct ModelOp win32ops = {
	"win32",
	win32_init,
	win32_add,
	win32_del,
	win32_dispatch,
	win32_dealloc,
};

#define FD_SET_ALLOC_SIZE(n) ((sizeof(struct win_fd_set) + ((n)-1)*sizeof(SOCKET)))

static int
grow_fd_sets(struct win32op *op, unsigned new_num_fds)
{
	size_t size;

	if( !(new_num_fds >= op->readset_in->fd_count &&
		new_num_fds >= op->writeset_in->fd_count) )
		return -1;
	if( !(new_num_fds >= 1) )
		return -1;

	size = FD_SET_ALLOC_SIZE(new_num_fds);
	if (!(op->readset_in = (struct win_fd_set *)realloc(op->readset_in, size)))
		return (-1);
	if (!(op->writeset_in = (struct win_fd_set *)realloc(op->writeset_in, size)))
		return (-1);
	op->resize_out_sets = 1;
	op->num_fds_in_fd_sets = new_num_fds;
	return (0);
}

static int
do_fd_set(struct win32op *op, struct SocketIndex *ent, SOCKET s, int read)
{
	struct win_fd_set *set = read ? op->readset_in : op->writeset_in;
	if (read) {
		if (ent->read_pos_plus1 > 0)
			return (0);
	} else {
		if (ent->write_pos_plus1 > 0)
			return (0);
	}
	if (set->fd_count == op->num_fds_in_fd_sets) {
		if (grow_fd_sets(op, op->num_fds_in_fd_sets*2))
			return (-1);
		// set pointer will have changed and needs reiniting!
		set = read ? op->readset_in : op->writeset_in;
	}
	set->fd_array[set->fd_count] = s;
	if (read)
		ent->read_pos_plus1 = set->fd_count+1;
	else
		ent->write_pos_plus1 = set->fd_count+1;
	return (set->fd_count++);
}

static int
do_fd_clear(void *base,
struct win32op *op, struct SocketIndex *ent, int read)
{
	ModelManager* pDispatcher = (ModelManager*)base;

	int i;
	struct win_fd_set *set = read ? op->readset_in : op->writeset_in;
	if (read) {
		i = ent->read_pos_plus1 - 1;
		ent->read_pos_plus1 = 0;
	} else {
		i = ent->write_pos_plus1 - 1;
		ent->write_pos_plus1 = 0;
	}
	if (i < 0)
		return (0);
	if (--set->fd_count != (unsigned)i) {
		struct SocketIndex *ent2;
		SOCKET s2;
		s2 = set->fd_array[i] = set->fd_array[set->fd_count];

		ent2 = pDispatcher->getSocketIndex( s2 );

		if (!ent2) // This indicates a bug.
			return (0);
		if (read)
			ent2->read_pos_plus1 = i+1;
		else
			ent2->write_pos_plus1 = i+1;
	}
	return (0);
}

#define NEVENT 32
void *
win32_init(void *base)
{
	struct win32op *winop;
	size_t size;
	if (!(winop = (struct win32op*)malloc( sizeof(struct win32op))))
		return NULL;
	winop->num_fds_in_fd_sets = NEVENT;
	size = FD_SET_ALLOC_SIZE(NEVENT);
	if (!(winop->readset_in = (struct win_fd_set *)malloc(size)))
		goto err;
	if (!(winop->writeset_in = (struct win_fd_set *)malloc(size)))
		goto err;
	if (!(winop->readset_out = (struct win_fd_set *)malloc(size)))
		goto err;
	if (!(winop->writeset_out = (struct win_fd_set *)malloc(size)))
		goto err;
	if (!(winop->exset_out = (struct win_fd_set *)malloc(size)))
		goto err;
	winop->readset_in->fd_count = winop->writeset_in->fd_count = 0;
	winop->readset_out->fd_count = winop->writeset_out->fd_count
		= winop->exset_out->fd_count = 0;

	winop->resize_out_sets = 0;

	return (winop);
err:
	XFREE(winop->readset_in);
	XFREE(winop->writeset_in);
	XFREE(winop->readset_out);
	XFREE(winop->writeset_out);
	XFREE(winop->exset_out);
	XFREE(winop);
	return (NULL);
}

int
win32_add(void *base, SOCKET fd,
		  short old, short events, void *_idx)
{
	ModelManager* pDispatcher = (ModelManager*)base;
	struct win32op *winop = (struct win32op *)pDispatcher->getModelData();
	struct SocketIndex *idx = (struct SocketIndex *)_idx;

	if (!(events & (EV_READ|EV_WRITE)))
		return (0);

	//event_debug(("%s: adding event for %d", __func__, (int)fd));
	if (events & EV_READ) {
		if (do_fd_set(winop, idx, fd, 1)<0)
			return (-1);
	}
	if (events & EV_WRITE) {
		if (do_fd_set(winop, idx, fd, 0)<0)
			return (-1);
	}
	return (0);
}

int
win32_del(void *base, SOCKET fd, short old, short events,
		  void *_idx)
{
	ModelManager* pDispatcher = (ModelManager*)base;
	struct win32op *winop = (struct win32op *)pDispatcher->getModelData();
	struct SocketIndex *idx = (struct SocketIndex *)_idx;

	//event_debug(("%s: Removing event for "EV_SOCK_FMT,__func__, EV_SOCK_ARG(fd)));
	if ( (old & EV_READ) && !(events & EV_READ) )
		do_fd_clear(base, winop, idx, 1);
	if ( (old & EV_WRITE) && !(events & EV_WRITE) )
		do_fd_clear(base, winop, idx, 0);

	return 0;
}

static void
fd_set_copy(struct win_fd_set *out, const struct win_fd_set *in)
{
	out->fd_count = in->fd_count;
	memcpy(out->fd_array, in->fd_array, in->fd_count * (sizeof(SOCKET)));
}

/*
static void dump_fd_set(struct win_fd_set *s)
{
unsigned int i;
printf("[ ");
for(i=0;i<s->fd_count;++i)
printf("%d ",(int)s->fd_array[i]);
printf("]\n");
}
*/

int
win32_dispatch(void *base, struct timeval *tv)
{
	ModelManager* pDispatcher = (ModelManager*)base;
	struct win32op *winop = (struct win32op *)pDispatcher->getModelData();
	int res = 0;
	unsigned j, i;
	int fd_count;
	SOCKET s;

	if (winop->resize_out_sets) {
		size_t size = FD_SET_ALLOC_SIZE(winop->num_fds_in_fd_sets);
		if (!(winop->readset_out = (struct win_fd_set *)realloc(winop->readset_out, size)))
			return (-1);
		if (!(winop->exset_out = (struct win_fd_set *)realloc(winop->exset_out, size)))
			return (-1);
		if (!(winop->writeset_out = (struct win_fd_set *)realloc(winop->writeset_out, size)))
			return (-1);
		winop->resize_out_sets = 0;
	}

	fd_set_copy(winop->readset_out, winop->readset_in);
	fd_set_copy(winop->exset_out, winop->writeset_in);
	fd_set_copy(winop->writeset_out, winop->writeset_in);

	fd_count =
		(winop->readset_out->fd_count > winop->writeset_out->fd_count) ?
		winop->readset_out->fd_count : winop->writeset_out->fd_count;

	if (!fd_count) {
		Sleep(tv->tv_usec/1000);
		return (0);
	}


	res = select(fd_count,
		(struct fd_set*)winop->readset_out,
		(struct fd_set*)winop->writeset_out,
		(struct fd_set*)winop->exset_out, tv);


	//event_debug(("%s: select returned %d", __func__, res));

	if (res <= 0) {
		if( res == -1 )
		{
			printf("error:%d\n", getErrno() );
		}
		return res;
	}

	if (winop->readset_out->fd_count) {
		i = rand() % winop->readset_out->fd_count;
		for (j=0; j<winop->readset_out->fd_count; ++j) {
			if (++i >= winop->readset_out->fd_count)
				i = 0;
			s = winop->readset_out->fd_array[i];
			pDispatcher->insertActiveList( s, EV_READ);
		}
	}
	if (winop->exset_out->fd_count) {
		i = rand() % winop->exset_out->fd_count;
		for (j=0; j<winop->exset_out->fd_count; ++j) {
			if (++i >= winop->exset_out->fd_count)
				i = 0;
			s = winop->exset_out->fd_array[i];
			pDispatcher->insertActiveList( s, EV_WRITE);
		}
	}
	if (winop->writeset_out->fd_count) {
		SOCKET s;
		i = rand() % winop->writeset_out->fd_count;
		for (j=0; j<winop->writeset_out->fd_count; ++j) {
			if (++i >= winop->writeset_out->fd_count)
				i = 0;
			s = winop->writeset_out->fd_array[i];
			pDispatcher->insertActiveList( s, EV_WRITE);
		}
	}
	return (0);
}

void
win32_dealloc(void *base)
{
	ModelManager* pDispatcher = (ModelManager*)base;
	struct win32op *winop = (struct win32op *)pDispatcher->getModelData();

	if (winop->readset_in)
		free(winop->readset_in);
	if (winop->writeset_in)
		free(winop->writeset_in);
	if (winop->readset_out)
		free(winop->readset_out);
	if (winop->writeset_out)
		free(winop->writeset_out);
	if (winop->exset_out)
		free(winop->exset_out);


	memset(winop, 0, sizeof(winop));
	free(winop);
}

#endif
