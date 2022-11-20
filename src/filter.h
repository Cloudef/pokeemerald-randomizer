#pragma once

struct filter;
enum filter_policy { ALL, IN, OUT, ALLOW };
struct filter* filter_load(const char *path);
struct filter* filter_load_mem(void *buf, size_t size);
enum filter_policy filter_policy_for_input(struct filter *filter, const char *input);
void filter_free(struct filter *filter);
