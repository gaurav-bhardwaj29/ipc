#include "../../include/ipc_backend.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <stdio.h>

typedef struct {
    int ls;
    int s1; // for connect
    int s2; // accepted
    char path[64];
    int is_sender;

} st_t;

static int pair_init(ipc_pair_t *pair, size_t payload) {
    (void)payload;
    st_t *s = calloc(1, sizeof(*s));
    s->ls = socket(AF_UNIX, SOCK_STREAM, 0);
    snprintf(s->path, sizeof(s->path), "/tmp/ipc_us_%d", getpid());
    unlink(s->path);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, s->path, sizeof(addr.sun_path)-1);
    if (bind(s->ls, (struct sockaddr*)&addr, sizeof(addr))!=0) {
        free(s); return -1;
    }
    if (listen(s->ls, 2)!=0){
        close(s->ls); free(s); return -1;
    }
    pair->opaque = s;
    return 0;
}
static int pair_start(ipc_pair_t *pair, int is_sender){
    st_t *s = (st_t*)pair->opaque;
    s->is_sender = is_sender;
    if (is_sender){
        s->s1 = socket(AF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, s->path, sizeof(addr.sun_path) -1 );
        if(connect(s->s1, (struct sockaddr*)&addr, sizeof(addr))!=0) return -1;
        s->s1 = accept(s->ls, NULL, NULL);
    }
    else {
        s->s2 = accept(s->ls, NULL, NULL);
        s->s1 = socket(AF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, s->path, sizeof(addr.sun_path)-1);
        if(connect(s->s1, (struct sockaddr*)&addr, sizeof(addr))!=0) return -1;
    }
    return 0;
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
    int fd = s->is_sender ? s->s1 : s->s2;
    return write_all(fd, buf, len);
}
static int pair_recv(ipc_pair_t *pair, void *buf, size_t len){
    st_t *s = (st_t*)pair->opaque;
    int fd = s->is_sender ? s->s2 : s->s1;
    return read_all(fd, buf, len);

}

static int pair_close(ipc_pair_t *pair){
    st_t *s = (st_t*)pair->opaque;
    close(s->s1); close(s->s2); close(s->ls);
    unlink(s->path);
    free(s);
    return 0;
}

ipc_backend_t BACKEND_UNIX_STREAM = { "unix_stream", &(ipc_backend_vtbl_t){
    .pair_init = pair_init,
    .pair_start = pair_start,
    .pair_send = pair_send, 
    .pair_recv = pair_recv,
    .pair_close = pair_close
}};











