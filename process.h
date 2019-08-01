#ifndef DDASTRACE_PROCESS_H
#define DDASTRACE_PROCESS_H

#include "php.h"

extern void (*ddastrace_prev_ast_process)(zend_ast *ast);
ZEND_API void ddastrace_ast_process(zend_ast *ast);

#endif
