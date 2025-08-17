#pragma once
#include <stddef.h>
#include <stdio.h>

void *xmalloc(size_t n);
void msleep(int ms);
void clear_screen(void);
void move_home(void);
void hide_cursor(void);
void show_cursor(void);
void print_sparkline_double(double *v, int n, double max, FILE *out);