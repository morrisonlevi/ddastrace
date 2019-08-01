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

#ifndef PHP_DDASTRACE_H
# define PHP_DDASTRACE_H
#include "span.h"

extern zend_module_entry ddastrace_module_entry;
# define phpext_ddastrace_ptr &ddastrace_module_entry

# define PHP_DDASTRACE_VERSION "0.1.0"

ZEND_BEGIN_MODULE_GLOBALS(ddastrace)
	ddastrace_span_stack_t *open_spans_top;
	ddastrace_span_stack_t *closed_spans_top;
ZEND_END_MODULE_GLOBALS(ddastrace)

#ifdef ZTS
#define DDASTRACE_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(ddastrace, v)
#else
#define DDASTRACE_G(v) (ddastrace_globals.v)
#endif

# if defined(ZTS) && defined(COMPILE_DL_DDASTRACE)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_DDASTRACE_H */
