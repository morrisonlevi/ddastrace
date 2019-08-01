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
ZEND_BEGIN_ARG_INFO(arginfo_ddastrace_test1, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ddastrace_test2, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ ddastrace_functions[]
 */
static const zend_function_entry ddastrace_functions[] = {
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
