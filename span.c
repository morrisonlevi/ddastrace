#include "span.h"

#include "php.h"
#include "ext/standard/php_mt_rand.h"
#include "time.h"
#include "stdint.h"
#include "php_ddastrace.h"

#if defined(ZTS) && defined(COMPILE_DL_DDASTRACE)
	ZEND_TSRMLS_CACHE_DEFINE()
#endif

ZEND_TLS ddastrace_span_stack_t *open_spans_top;
ZEND_TLS ddastrace_span_stack_t *closed_spans_top;

void ddastrace_init_span_stacks() {
	open_spans_top = NULL;
	closed_spans_top = NULL;
}

static void _free_span_stack(ddastrace_span_stack_t *stack) {
	while (stack != NULL) {
		ddastrace_span_stack_t *tmp = stack;
		stack = stack->next;
		efree(tmp);
	}
}

void ddastrace_free_span_stacks() {
	_free_span_stack(open_spans_top);
	_free_span_stack(closed_spans_top);
}

static uint64_t _get_nanoseconds() {
	struct timespec time;
	if (clock_gettime(CLOCK_MONOTONIC, &time) == 0) {
		return time.tv_sec * 1000000000L + time.tv_nsec;
	}
	return 0;
}

ddastrace_span_stack_t *ddastrace_open_span() {
	ddastrace_span_stack_t *stack = ecalloc(1, sizeof(ddastrace_span_stack_t));
	stack->next = open_spans_top;
	open_spans_top = stack;

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
	ddastrace_span_stack_t *stack = open_spans_top;
	if (stack == NULL) {
		return NULL;
	}
	open_spans_top = stack->next;

	stack->duration = _get_nanoseconds() - stack->start;
	stack->next = closed_spans_top;
	closed_spans_top = stack;
	return stack;
}
