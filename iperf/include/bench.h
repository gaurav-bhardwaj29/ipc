#pragma once
#include <stddef.h>
#include <stdint.h>
#include <sys/resource.h>

typedef struct {
    size_t payload_bytes;
    uint64_t n_messages;         // iterations (latency mode)
    uint32_t n_pairs;            // parallel pairs
    uint32_t seconds;            // duration in throughput mode
    int latency_mode;            // 1=ping-pong, 0=throughput
    int noise_us;                // receiver sleep microseconds per message
    int warmup_ms;
    const char *backend;
    const char *out_csv;         // optional results CSV file path
    const char *out_json;        // optional results JSON file path
} bench_cfg_t;

typedef struct {
    double avg_latency_us;
    double p99_latency_us;
    double msgs_per_sec;
    double mbps;
    struct rusage sender_ru;
    struct rusage receiver_ru;
} bench_result_t;
