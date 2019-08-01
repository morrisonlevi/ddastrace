#ifndef DDASTRACE_SPAN_H
# define DDASTRACE_SPAN_H
#include "stdint.h"

typedef struct _ddastrace_span_stack_t {
	uint64_t parent_id;
	uint64_t span_id;
	uint64_t start;
	uint64_t duration;
	struct _ddastrace_span_stack_t *next;
} ddastrace_span_stack_t;

void ddastrace_init_span_stacks();
void ddastrace_free_span_stacks();
ddastrace_span_stack_t *ddastrace_open_span();
ddastrace_span_stack_t *ddastrace_close_span();

#endif	/* DDASTRACE_SPAN_H */
