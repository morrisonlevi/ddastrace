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

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

static void (*_prev_ast_process)(zend_ast *ast);
static void _ddastrace_ast_process(zend_ast *ast)
{
	/* find function and method declarations
	 * wrap body in (roughly):
	 *
	 * ddastrace_span_begin();
	 * try { ... }
	 * catch (Throwable $ex) {
	 *   ddastrace_span_end_exception($ex);
	 *   throw $ex;
	 * }
	 * ddastrace_span_end_void();
	 *
	 * Also need to walk the body to find `return` nodes and wrap them in:
	 * ddastrace_span_end(...) or ddastrace_span_end_by_ref(...)
	 * Be careful not to go into other functions such as closures and methods in an anonymous class!
	 */
	if (_prev_ast_process) {
		_prev_ast_process(ast);
	}
}

static PHP_MINIT_FUNCTION(ddastrace)
{
	_prev_ast_process = zend_ast_process;
	zend_ast_process = _ddastrace_ast_process;
	return SUCCESS;
}

static PHP_MSHUTDOWN_FUNCTION(ddastrace)
{
	zend_ast_process = _prev_ast_process;
	return SUCCESS;
}
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(ddastrace)
{
#if defined(ZTS) && defined(COMPILE_DL_DDASTRACE)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

static PHP_RSHUTDOWN_FUNCTION(ddastrace) {
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

/* {{{ arginfo
 */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ddastrace_span_open, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ddastrace_span_close_void, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ddastrace_span_close, 0, 0, 1)
ZEND_ARG_INFO(0, retval)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ddastrace_span_close_by_ref, 0, 0, 1)
ZEND_ARG_INFO(1, retval_ref)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ddastrace_span_close_exception, 0, 0, 1)
ZEND_ARG_INFO(0, throwable)
ZEND_END_ARG_INFO()
/* }}} */

static PHP_FUNCTION(ddastrace_span_open) {
	ZEND_PARSE_PARAMETERS_NONE();
	// TODO Check that we're in a function
	// TODO Start the timer
	php_printf("Called: ddastrace_span_open()\n");
}

static PHP_FUNCTION(ddastrace_span_close_void) {
	ZEND_PARSE_PARAMETERS_NONE();
	// TODO Check that we're in a function
	// TODO Stop the timer
	php_printf("Called: ddastrace_span_close_void()\n");
}

static PHP_FUNCTION(ddastrace_span_close) {
	zval *retval = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(retval)
	ZEND_PARSE_PARAMETERS_END();

	// TODO Check that we're in a function
	// TODO Stop the timer
	if (Z_TYPE_P(retval) == IS_STRING) {
		php_printf("Called: ddastrace_span_close(): %s\n", Z_STRVAL_P(retval));
	} else {
		php_printf("Called: ddastrace_span_close()\n");
	}
}

static PHP_FUNCTION(ddastrace_span_close_by_ref) {
	zval *retval = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(retval)
	ZEND_PARSE_PARAMETERS_END();

	if (!Z_ISREF_P(retval)) {
		zend_throw_exception_ex(zend_ce_type_error, 0, "Return value must be passed by reference");
		return;
	}

	// TODO Check that we're in a function
	// TODO Stop the timer
	php_printf("Called: ddastrace_span_close_by_ref()\n");

	RETURN_ZVAL(retval, 0, 0);
}

static PHP_FUNCTION(ddastrace_span_close_exception) {
	zval *exception = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_OBJECT_OF_CLASS(exception, zend_ce_throwable)
	ZEND_PARSE_PARAMETERS_END();

	// TODO Check that we're in a function
	// TODO Stop the timer
	php_printf("Called: ddastrace_span_close_exception()\n");

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
