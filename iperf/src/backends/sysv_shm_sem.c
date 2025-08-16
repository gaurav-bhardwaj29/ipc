#include "../../include/ipc_backend.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    int shmid;
    int semid;
    size_t slots;
    size_t slot_size;
    volatile uint32_t *w;
    volatile uint32_t *r;
    char *data;
    size_t mapped_size;
    int is_sender;
} st_t;

static void semP(int semid, int idx){ struct sembuf op = {idx, -1, 0}; semop(semid, &op, 1); }
static void semV(int semid, int idx){ struct sembuf op = {idx, +1, 0}; semop(semid, &op, 1); }

static int pair_init(ipc_pair_t *pair, size_t payload){
    st_t *s = calloc(1, sizeof(*s));
    s->slots = 256;
    s->slot_size = payload;
    size_t hdr = sizeof(uint32_t)*2;
    s->mapped_size = hdr + s->slots * s->slot_size;
    s->shmid = shmget(IPC_PRIVATE, s->mapped_size, IPC_CREAT | 0600);
    if (s->shmid < 0){ free(s); return -1; }
    void *m = shmat(s->shmid, NULL, 0);
    if (m == (void*)-1){ shmctl(s->shmid, IPC_RMID, NULL); free(s); return -1; }
    s->w = (volatile uint32_t*) m;
    s->r = (volatile uint32_t*) ((char*)m + sizeof(uint32_t));
    s->data = (char*)m + sizeof(uint32_t)*2;
    *s->w = 0; *s->r = 0;
    s->semid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0600);
    semctl(s->semid, 0, SETVAL, (int)s->slots); // empty
    semctl(s->semid, 1, SETVAL, 0); // full

    pair->opaque = s;
    return 0;
}

static int pair_start(ipc_pair_t *pair, int is_sender){ ((st_t*)pair->opaque)->is_sender = is_sender; return 0; }

static int pair_send(ipc_pair_t *pair, const void *buf, size_t len){
    st_t *s = (st_t*)pair->opaque;
    semP(s->semid, 0);
    uint32_t idx = (*s->w)++ % (uint32_t)s->slots;
    memcpy(s->data + idx * s->slot_size, buf, len);
    semV(s->semid, 1);
    return 0;
}

static int pair_recv(ipc_pair_t *pair, void *buf, size_t len){
    st_t *s = (st_t*)pair->opaque;
    semP(s->semid, 1);
    uint32_t idx = (*s->r)++ % (uint32_t)s->slots;
    memcpy(buf, s->data + idx * s->slot_size, len);
    semV(s->semid, 0);
    return 0;
}

static int pair_close(ipc_pair_t *pair){
    st_t *s = (st_t*)pair->opaque;
    shmdt((void*)s->w);
    shmctl(s->shmid, IPC_RMID, NULL);
    semctl(s->semid, 0, IPC_RMID);
    free(s);
    return 0;
}

ipc_backend_t BACKEND_SYSV_SHM_SEM = { "sysv_shm_sem", &(ipc_backend_vtbl_t){
    .pair_init = pair_init,
    .pair_start = pair_start,
    .pair_send = pair_send,
    .pair_recv = pair_recv,
    .pair_close = pair_close
}};
