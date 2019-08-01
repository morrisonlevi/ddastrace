#include "span.h"

#include "php.h"
#include "ext/standard/php_mt_rand.h"
#include "time.h"
#include "stdint.h"
#include "php_ddastrace.h"

ZEND_EXTERN_MODULE_GLOBALS(ddastrace);

void ddastrace_init_span_stacks() {
	DDASTRACE_G(open_spans_top) = NULL;
	DDASTRACE_G(closed_spans_top) = NULL;
}

static void _free_span_stack(ddastrace_span_stack_t *stack) {
	while (stack != NULL) {
		ddastrace_span_stack_t *tmp = stack;
		stack = stack->next;
		efree(tmp);
	}
}

void ddastrace_free_span_stacks() {
	_free_span_stack(DDASTRACE_G(open_spans_top));
	_free_span_stack(DDASTRACE_G(closed_spans_top));
}

static uint64_t _get_nanoseconds() {
	struct timespec time;
	if (clock_gettime(CLOCK_MONOTONIC, &time) == 0) {
		return time.tv_sec * 1000000 + time.tv_nsec / 1000;
	}
	return 0;
}

ddastrace_span_stack_t *ddastrace_open_span() {
	ddastrace_span_stack_t *stack = ecalloc(1, sizeof(ddastrace_span_stack_t));
	stack->next = DDASTRACE_G(open_spans_top);
	DDASTRACE_G(open_spans_top) = stack;

	if (stack->next) {
		stack->parent_id = stack->next->span_id;
	} else {
		stack->parent_id = 0;
	}
	stack->span_id = (uint64_t) php_mt_rand();
	stack->duration = 0;
	stack->start = _get_nanoseconds();
	return stack;
}

ddastrace_span_stack_t *ddastrace_close_span() {
	ddastrace_span_stack_t *stack = DDASTRACE_G(open_spans_top);
	if (stack == NULL) {
		return NULL;
	}
	DDASTRACE_G(open_spans_top) = stack->next;

	stack->duration = _get_nanoseconds() - stack->start;
	stack->next = DDASTRACE_G(closed_spans_top);
	DDASTRACE_G(closed_spans_top) = stack;
	return stack;
}
