#include <regex.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <err.h>
#include "filter.h"

struct rule {
   regex_t regex;
   enum filter_policy policy;
};

struct filter {
   struct rule *rules;
   size_t num_rules;
};

struct filter* filter_load_f(FILE *f) {
   struct filter *filter;
   if (!(filter = calloc(1, sizeof(struct filter))))
      err(EXIT_FAILURE, "calloc");

   size_t num_rules = 0, num_allocated = 1024;
   if (!(filter->rules = calloc(num_allocated, sizeof(struct rule))))
      err(EXIT_FAILURE, "calloc");

   char buf[1024];
   size_t line = 1;
   for (; fgets(buf, sizeof(buf), f); ++line) {
      if (num_rules > num_allocated && !(filter->rules = realloc(filter->rules, num_allocated *= 2)))
         err(EXIT_FAILURE, "realloc");

      char *ws;
      if ((ws = strrchr(buf, '\n'))) *ws = 0;

      if (!(ws = strrchr(buf, ' ')))
         errx(EXIT_FAILURE, "error parsing filter rules: missing whitespace at line %zu", line);

      *ws = 0;

      enum filter_policy policy;
      if (!strcmp(ws + 1, "ALL")) {
         policy = ALL;
      } else if (!strcmp(ws + 1, "IN")) {
         policy = IN;
      } else if (!strcmp(ws + 1, "OUT")) {
         policy = OUT;
      } else if (!strcmp(ws + 1, "ALLOW")) {
         policy = ALLOW;
      } else {
         errx(EXIT_FAILURE, "unknown policy '%s' given for pattern: %s", ws + 1, buf);
      }

      struct rule rule = { .policy = policy };
      if (regcomp(&rule.regex, buf, 0))
         err(EXIT_FAILURE, "failed to compile regex for pattern: %s", buf);

      filter->rules[num_rules++] = rule;
   }

   filter->num_rules = num_rules;
   return filter;
}

struct filter* filter_load(const char *path) {
   FILE *f;
   if (!(f = fopen(path, "rb")))
      err(EXIT_FAILURE, "fopen(%s, rb)", path);

   struct filter *filter = filter_load_f(f);
   fclose(f);
   return filter;
}

struct filter* filter_load_mem(void *buf, size_t size) {
   FILE *f;
   if (!(f = fmemopen(buf, size, "rb")))
      err(EXIT_FAILURE, "fmemopen");

   struct filter *filter = filter_load_f(f);
   fclose(f);
   return filter;
}

enum filter_policy filter_policy_for_input(struct filter *filter, const char *input) {
   for (size_t i = 0; i < filter->num_rules; ++i) {
      int ret = regexec(&filter->rules[i].regex, input, 0, NULL, 0);
      if (!ret) {
         return filter->rules[i].policy;
      } else if (ret != REG_NOMATCH) {
         char buf[100];
         regerror(ret, &filter->rules[i].regex, buf, sizeof(buf));
         errx(EXIT_FAILURE, "regex error: %s", buf);
      }
   }
   return ALLOW;
}

void filter_free(struct filter *filter) {
   for (size_t i = 0; i < filter->num_rules; ++i) regfree(&filter->rules[i].regex);
   free(filter);
}
