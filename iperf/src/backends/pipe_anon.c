#include "../../include/ipc_backend.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct {
    int a2b[2];
    int b2a[2];
    int is_sender;
} st_t;

static int pair_init(ipc_pair_t *pair, size_t payload){
    (void)payload;
    st_t *s = calloc(1, sizeof(*s));
    if (pipe(s->a2b) || pipe(s->b2a)) {
        free(s); return -1;
    }
    pair -> opaque = s;
    return 0;
}

static int pair_start(ipc_pair_t *pair, int is_sender) {
    ((st_t*)pair->opaque)->is_sender = is_sender; return 0;
}

static int write_all(int fd, const void *buf, size_t len){
    const char *p = buf;
    size_t n = 0;
    while (n<len){
        ssize_t r = write(fd, p+n, len-n);
        if (r < 0){
            if (errno == EINTR) continue;
            return -1;
        }
        n+=(size_t)r;
    }
    return 0;
}

static int read_all(int fd, void *buf, size_t len){
    char *p = buf;
    size_t n = 0;
    while (n<len){
        ssize_t r = read(fd, p + n, len-n);
        if (r<0){
            if (errno == EINTR) continue;
            return -1;
        }
        if (r == 0) return -1;
        n += (size_t)r;
    }
    return 0;
}

static int pair_send(ipc_pair_t *pair, const void *buf, size_t len){
    st_t *s = (st_t*)pair->opaque;
    int fd = s->is_sender ? s->a2b[1] : s->b2a[1];
    return write_all(fd, buf, len);
}
static int pair_recv(ipc_pair_t *pair, void *buf, size_t len){
    st_t *s = (st_t*)pair->opaque;
    int fd = s->is_sender ? s->b2a[0] : s->a2b[0];
    return read_all(fd, buf, len);

}

static int pair_close(ipc_pair_t *pair){
    st_t *s = (st_t*)pair->opaque;
    close(s->a2b[0]); close(s->a2b[1]); close(s->b2a[0]); close(s->b2a[1]);
    free(s);
    return 0;
}

ipc_backend_t BACKEND_PIPE_ANON = { "pipe_anon", &(ipc_backend_vtbl_t){
    .pair_init = pair_init,
    .pair_start = pair_start,
    .pair_send = pair_send, 
    .pair_recv = pair_recv,
    .pair_close = pair_close
}};