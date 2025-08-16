#include "../../include/ipc_backend.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    char shmname[64];
    char sem_empty[64];
    char sem_full[64];
    int fd;
    sem_t *empty, *full;
    size_t slots;
    size_t slot_size;
    volatile uint32_t *w;
    volatile uint32_t *r;
    char *data;
    int is_sender;
    size_t mapped_size;
}
st_t;
static int pair_init(ipc_pair_t *pair, size_t payload){
    st_t *s = calloc(1, sizeof(*s));
    snprintf(s->shmname, sizeof(s->shmname), "/ipc_pshm_%d_%ld", getpid(), random());
    snprintf(s->sem_empty, sizeof(s->sem_empty), "/ipc_psem_e_%d_%ld", getpid(), random());
    snprintf(s->sem_full, sizeof(s->sem_empty), "/ipc_psem_f_%d_%ld", getpid(), random());
    s->slots = 256;
    s->slot_size = payload;
    size_t hdr = sizeof(uint32_t)*2; // w & r
    s->mapped_size = hdr + s->slots * s->slot_size;

    shm_unlink(s->shmname);
    s->fd = shm_open(s->shmname, O_CREAT | O_RDWR, 0600);
    if (s->fd < 0) {
        free(s); return -1;
    }
    if (ftruncate(s->fd, s->mapped_size)!=0){
        close(s->fd); shm_unlink(s->shmname);
        free(s);
        return -1;
    }
    void *m = mmap(NULL, s->mapped_size, PROT_READ | PROT_WRITE, MAP_SHARED, s->fd, 0);
    if (m == MAP_FAILED){
        close(s->fd);
        shm_unlink(s->shmname);
        free(s);
        return -1;
    }
    s->w = (volatile uint32_t *)m;
    s->r = (volatile uint32_t *)((char*)m + sizeof(uint32_t));
    s->data = (char*)m + sizeof(uint32_t)*2;
    *s->w = 0; *s->r = 0;

    sem_unlink(s->sem_empty); sem_unlink(s->sem_full);
    s->empty = sem_open(s->sem_empty, O_CREAT, 0600, s->slots);
    s->full = sem_open(s->sem_full, O_CREAT, 0600, 0);
    if (s->empty == SEM_FAILED || s->full == SEM_FAILED){
        munmap(m, s->mapped_size); close(s->fd); shm_unlink(s->shmname);
        free(s); return -1;
    }

    pair -> opaque = s;
    return 0;
}

static int pair_start(ipc_pair_t *pair, int is_sender) {((st_t*)pair->opaque)->is_sender = is_sender; return 0;}
static int pair_send(ipc_pair_t *pair, const void *buf, size_t len){
    st_t *s = (st_t*)pair->opaque;
    sem_wait(s->empty);
    uint32_t idx = (*s->w)++ % (uint32_t)s->slots;
    char *slot = s->data + idx * s->slot_size;
    memcpy(slot, buf, len);
    sem_post(s->full);
    return 0;
}

static int pair_recv(ipc_pair_t *pair, void *buf, size_t len){
    st_t *s = (st_t*)pair->opaque;
    sem_wait(s->full);
    uint32_t idx = (*s->r)++ % (uint32_t)s->slots;
    char *slot = s->data + idx * s->slot_size;
    memcpy(buf, slot, len);
    sem_post(s->empty);
    return 0;
}
static int pair_close(ipc_pair_t *pair){
    st_t *s = (st_t*)pair->opaque;
    size_t map_size = s->mapped_size;
    munmap((void*)s->w, map_size);
    close(s->fd); shm_unlink(s->shmname);
    sem_close(s->empty); sem_close(s->full);
    sem_unlink(s->sem_empty); sem_unlink(s->sem_full);
    free(s);
    return 0;
}

ipc_backend_t BACKEND_POSIX_SHM_SEM = {"posix_shm_sem", &(ipc_backend_vtbl_t){
    .pair_init = pair_init,
    .pair_start = pair_start,
    .pair_send = pair_send, 
    .pair_recv = pair_recv,
    .pair_close = pair_close
}};