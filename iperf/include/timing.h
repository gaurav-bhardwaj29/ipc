#pragma once
#include <stdint.h>

uint64_t now_ns(void);
double ns_to_us(uint64_t ns);
double ns_to_ms(uint64_t ns);

typedef struct {
    uint64_t n;
    long double mean;
    long double M2;
} stats_t;

void stats_init(stats_t *s);
void stats_add(stats_t *s, double x);
double stats_mean(const stats_t *s);
double stats_stddev(const stats_t *s);

// small reservior p99
typedef struct {
    double *buf;
    int cap, len;
} ptrack_t;

void ptrack_init(ptrack_t *p, int cap);
void ptrack_free(ptrack_t *p);
void ptrack_add(ptrack_t *p, double v);
double ptrack_p99(ptrack_t *p);