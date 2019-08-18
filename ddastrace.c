/*
   +----------------------------------------------------------------------+
   | PHP Version 7                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) The PHP Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Sammy Powers                                                 |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "Zend/zend_exceptions.h"
#include "php_ddastrace.h"
#include "process.h"
#include "span.h"
#include "inttypes.h"

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

static PHP_MINIT_FUNCTION(ddastrace)
{
	ddastrace_prev_ast_process = zend_ast_process;
	zend_ast_process = ddastrace_ast_process;
	return SUCCESS;
}

static PHP_MSHUTDOWN_FUNCTION(ddastrace)
{
	zend_ast_process = ddastrace_prev_ast_process;
	return SUCCESS;
}
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(ddastrace)
{
#if defined(ZTS) && defined(COMPILE_DL_DDASTRACE)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	ddastrace_init_span_stacks();
	return SUCCESS;
}
/* }}} */

static PHP_RSHUTDOWN_FUNCTION(ddastrace) {
	ddastrace_free_span_stacks();
	return SUCCESS;
}

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(ddastrace)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "ddastrace support", "enabled");
	php_info_print_table_end();
}
/* }}} */

static int _validate_frame(zend_execute_data *execute_data)
{
	zend_execute_data *ex = EX(prev_execute_data);
	if (ZEND_CALL_INFO(ex) & ZEND_CALL_CODE) {
		zend_throw_exception_ex(
			zend_ce_error, 0, "%s() called without function context",
			ZSTR_VAL(EX(func)->common.function_name)
		);
		return 0;
	}
	if (zend_forbid_dynamic_call(ZSTR_VAL(EX(func)->common.function_name)) == FAILURE) {
		return 0;
	}
	return 1;
}

static void _print_span_info(char *action, ddastrace_span_stack_t *span)
{
	if (span == NULL) {
		php_printf("%s: NULL span\n", action);
		return;
	}
	php_printf(
		"%s: #%" PRIu64 "\n\tparent: #%" PRIu64 "\n\tstart: %" PRIu64 " (duration: %" PRIu64 ")\n\n",
		action,
		span->span_id,
		span->parent_id,
		span->start,
		span->duration
	);
}

/* {{{ arginfo
 */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ddastrace_span_open, 0, 0, 1)
ZEND_ARG_ARRAY_INFO(0, args, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ddastrace_span_close_void, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ddastrace_span_close, 0, 0, 1)
ZEND_ARG_INFO(0, retval)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ddastrace_span_close_by_ref, 0, 1, 1)
ZEND_ARG_INFO(1, retval_ref)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ddastrace_span_close_exception, 0, 0, 1)
ZEND_ARG_INFO(0, throwable)
ZEND_END_ARG_INFO()
/* }}} */

static PHP_FUNCTION(ddastrace_span_open) {
	zval *args;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ARRAY_EX(args, 0, 1)
	ZEND_PARSE_PARAMETERS_END();

	if (_validate_frame(execute_data) == 0) {
		return;
	}
	ddastrace_span_stack_t *span = ddastrace_open_span();
	php_printf("Called: ddastrace_span_open()\n");
	_print_span_info("Opened span", span);

	// todo: attach args to span instead of dtor
	zval_ptr_dtor(args);
}

static PHP_FUNCTION(ddastrace_span_close_void) {
	ZEND_PARSE_PARAMETERS_NONE();
	if (_validate_frame(execute_data) == 0) {
		return;
	}
	ddastrace_span_stack_t *span = ddastrace_close_span();
	php_printf("Called: ddastrace_span_close_void()\n");
	_print_span_info("Closed span (void)", span);
}

static PHP_FUNCTION(ddastrace_span_close) {
	zval *retval = NULL;

	if (_validate_frame(execute_data) == 0) {
		return;
	}

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(retval)
	ZEND_PARSE_PARAMETERS_END();

	ddastrace_span_stack_t *span = ddastrace_close_span();
	if (Z_TYPE_P(retval) == IS_STRING) {
		php_printf("Called: ddastrace_span_close(): %s\n", Z_STRVAL_P(retval));
	} else {
		php_printf("Called: ddastrace_span_close()\n");
	}
	_print_span_info("Closed span", span);

	RETURN_ZVAL(retval, 0, 0);
}

static PHP_FUNCTION(ddastrace_span_close_by_ref) {
	zval *arg;
	if (_validate_frame(execute_data) == 0) {
		return;
	}

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(arg)
	ZEND_PARSE_PARAMETERS_END();

#if 0
	if (!Z_ISREF_P(return_value)) {
		zend_throw_exception_ex(zend_ce_type_error, 0, "Return value must be passed by reference");
		return;
	}
#endif
	ZVAL_NEW_REF(return_value, arg);

	ddastrace_span_stack_t *span = ddastrace_close_span();
	php_printf("Called: ddastrace_span_close_by_ref()\n");
	_print_span_info("Closed span (by ref)", span);

}

static PHP_FUNCTION(ddastrace_span_close_exception) {
	zval *exception = NULL;

	if (_validate_frame(execute_data) == 0) {
		return;
	}

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_OBJECT_OF_CLASS(exception, zend_ce_throwable)
	ZEND_PARSE_PARAMETERS_END();

	ddastrace_span_stack_t *span = ddastrace_close_span();
	php_printf("Called: ddastrace_span_close_exception()\n");
	_print_span_info("Closed span (exception)", span);

	RETURN_ZVAL(exception, 0, 0);
}

/* {{{ ddastrace_functions[]
 */
static const zend_function_entry ddastrace_functions[] = {
	PHP_FE(ddastrace_span_open, arginfo_ddastrace_span_open)
	PHP_FE(ddastrace_span_close_void, arginfo_ddastrace_span_close_void)
	PHP_FE(ddastrace_span_close, arginfo_ddastrace_span_close)
	PHP_FE(ddastrace_span_close_by_ref, arginfo_ddastrace_span_close_by_ref)
	PHP_FE(ddastrace_span_close_exception, arginfo_ddastrace_span_close_exception)
	PHP_FE_END
};
/* }}} */

/* {{{ ddastrace_module_entry
 */
zend_module_entry ddastrace_module_entry = {
	STANDARD_MODULE_HEADER,
	"ddastrace",					/* Extension name */
	ddastrace_functions,			/* zend_function_entry */
	PHP_MINIT(ddastrace),							/* PHP_MINIT - Module initialization */
	PHP_MSHUTDOWN(ddastrace),							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(ddastrace),			/* PHP_RINIT - Request initialization */
	PHP_RSHUTDOWN(ddastrace),				/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(ddastrace),			/* PHP_MINFO - Module info */
	PHP_DDASTRACE_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_DDASTRACE
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(ddastrace)
#endif
