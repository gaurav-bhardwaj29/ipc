#include "../../include/ipc_backend.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>

typedef struct {
    int q1; int q2; size_t payload;
    int is_sender;
} st_t;

struct msgbuf_ { long mtype; char data[1]; };

static int pair_init(ipc_pair_t *pair, size_t payload) {
    st_t *s = calloc(1, sizeof(*s));
    s->q1 = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    s->q2 = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    s->payload = payload;
    pair->opaque = s;
    return 0;
}
static int pair_start(ipc_pair_t *pair, int is_sender){ ((st_t*) pair->opaque)->is_sender = is_sender; return 0;}

static int mq_send(int q, const void *buf, size_t len) {
    size_t sz = sizeof(struct msgbuf_) - 1 + len;
    struct msgbuf_ *m = malloc(sz);
    if (!m) return -1;
    m->mtype = 1;
    memcpy(m-> data, buf, len);
    int r = msgsnd(q, m, len, 0);
    free(m);
    return r < 0 ? -1 : 0;
}
static int mq_recv(int q, void *buf, size_t len){
    ssize_t r = msgrcv(q, buf, len, 1, MSG_NOERROR);
    return (r == (ssize_t)len) ? 0 : -1;
}
static int pair_send(ipc_pair_t *pair, const void *buf, size_t len){
    st_t *s = (st_t*)pair->opaque;
    return mq_send(s->is_sender ? s->q1 : s->q2, buf, len);
}
static int pair_recv(ipc_pair_t *pair, void *buf, size_t len){
    st_t *s = (st_t*)pair->opaque;
    return mq_recv(s->is_sender ? s->q2 : s->q1, buf, len);
}

static int pair_close(ipc_pair_t *pair){
    st_t *s = (st_t*)pair->opaque;
    msgctl(s->q1, IPC_RMID, NULL);
    msgctl(s->q2, IPC_RMID, NULL);
    free(s);
    return 0;
}

ipc_backend_t BACKEND_SYSV_MSGQ = { "sysv_msgq", &(ipc_backend_vtbl_t){
    .pair_init = pair_init,
    .pair_start = pair_start,
    .pair_send = pair_send, 
    .pair_recv = pair_recv,
    .pair_close = pair_close
}};