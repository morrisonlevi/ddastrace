#include "process.h"

void (*ddastrace_prev_ast_process)(zend_ast *ast);

#define zend_ast_alloc(size) zend_arena_alloc(&CG(ast_arena), size);

#if defined(ZTS) && defined(COMPILE_DL_DDASTRACE)
	ZEND_TSRMLS_CACHE_DEFINE()
#endif

typedef struct {
	zend_bool in_func;
	zend_bool by_ref;
} _globals_t;

ZEND_TLS _globals_t globals = {0, 0};

static _globals_t _backup_globals(zend_bool in_func, zend_bool by_ref) {
	_globals_t backup = globals;
	globals.in_func = in_func;
	globals.by_ref = by_ref;
	return backup;
}

static inline size_t zend_ast_list_size(uint32_t children) {
	return sizeof(zend_ast_list) - sizeof(zend_ast *) + sizeof(zend_ast *) * children;
}

static zend_ast *_process_ast(zend_ast *ast);

static zend_ast *_create_ast_str(char *str, size_t len, uint32_t attr) {
	zval zv;
	zend_ast *ast;
	ZVAL_NEW_STR(&zv, zend_string_init(str, len, 0));
	ast = zend_ast_create_zval_with_lineno(&zv, 0);
	ast->attr = attr;
	return ast;
}

static zend_ast *_create_ast_catch_name(void) {
	zend_ast *name = _create_ast_str("Throwable", sizeof("Throwable") - 1, ZEND_NAME_FQ);
#if PHP_VERSION_ID < 70100
	return name;
#else
	return zend_ast_create_list(1, ZEND_AST_NAME_LIST, name);
#endif
}
static zend_ast *_create_ast_catch(uint32_t lineno) {
	zend_ast *var_name = _create_ast_str("ex", sizeof("ex") - 1, 0);
	zend_ast *type_name = _create_ast_catch_name();

	zend_ast *call = zend_ast_create(ZEND_AST_CALL,
		_create_ast_str("ddastrace_span_close_exception", sizeof("ddastrace_span_close_exception") - 1, ZEND_NAME_FQ),
		zend_ast_create_list(1, ZEND_AST_ARG_LIST,
			zend_ast_create(ZEND_AST_VAR, _create_ast_str("ex", sizeof("ex") - 1, 0))
		));

	zend_ast *throw = zend_ast_create(ZEND_AST_THROW, call);
	zend_ast *stmt_list = zend_ast_create_list(1, ZEND_AST_STMT_LIST, throw);

	zend_ast *catch = zend_ast_create_list(1, ZEND_AST_CATCH_LIST,
			zend_ast_create(ZEND_AST_CATCH, type_name, var_name, stmt_list)
			);

	catch->lineno = lineno;
	throw->lineno = lineno;
	stmt_list->lineno = lineno;

	return (zend_ast *) catch;
}

static zend_ast *_process_function(zend_ast *ast) {
	zend_ast_decl *decl = (zend_ast_decl *) ast;
	zend_ast_list *stmt_list = zend_ast_get_list(decl->child[2]);

	// guess at feasible line numbers
	uint32_t start_lineno = stmt_list->lineno;
	uint32_t stop_lineno = stmt_list->child[stmt_list->children - 1]->lineno;

	zend_ast *span_open = zend_ast_create(ZEND_AST_CALL,
		_create_ast_str("ddastrace_span_open", sizeof("ddastrace_span_open") - 1, ZEND_NAME_FQ),
		zend_ast_create_list(0, ZEND_AST_ARG_LIST));

	zend_ast *span_close = zend_ast_create(ZEND_AST_CALL,
		_create_ast_str("ddastrace_span_close_void", sizeof("ddastrace_span_close_void") - 1, ZEND_NAME_FQ),
		zend_ast_create_list(0, ZEND_AST_ARG_LIST));

	zend_ast *catch = _create_ast_catch(stop_lineno);
	zend_ast *finally = NULL;
	_globals_t globals_backup = _backup_globals(1, decl->flags & ZEND_ACC_RETURN_REFERENCE ? 1 : 0);
	zend_ast *try = zend_ast_create(ZEND_AST_TRY, _process_ast(decl->child[2]), catch, finally);
	zend_ast_list *new_list = zend_ast_alloc(zend_ast_list_size(3));
	new_list->kind = ZEND_AST_STMT_LIST;
	new_list->lineno = 0;
	new_list->children = 3;
	new_list->child[0] = span_open;
	new_list->child[1] = try;
	new_list->child[2] = span_close;

	try->lineno = start_lineno;
	new_list->lineno = start_lineno;

	decl->child[2] = (zend_ast *) new_list;

	globals = globals_backup;
	return ast;
}

static zend_ast *_process_return(zend_ast *ast) {
	zend_ast *expr_ast = ast->child[0];
	zend_ast *return_ast = ast;

	// if there isn't an active function then don't wrap it
	// e.g. return at file scope
	if (!globals.in_func) {
		return return_ast;
	}

	if (expr_ast) {
		/* Either:
		 *   return ddastrace_span_close_by_ref(expr_ast);
		 *   return ddastrace_span_close(expr_ast);
		 */
		zend_ast *call;
		char *name;
		size_t len;

		if (globals.by_ref) {
			name = "ddastrace_span_close_by_ref";
			len = sizeof("ddastrace_span_close_by_ref") - 1;
		} else {
			name = "ddastrace_span_close";
			len = sizeof("ddastrace_span_close") - 1;
		}

		call = zend_ast_create(ZEND_AST_CALL,
			_create_ast_str(name, len, ZEND_NAME_FQ),
			zend_ast_create_list(1, ZEND_AST_ARG_LIST, _process_ast(expr_ast)));

		ast->child[0] = call;
		return ast;

	} else {
		// { ddastrace_span_end_void(); return; }
		zend_ast *call = zend_ast_create(ZEND_AST_CALL,
			_create_ast_str("ddastrace_span_close_void", sizeof("ddastrace_span_close_void") - 1, ZEND_NAME_FQ),
			zend_ast_create_list(0, ZEND_AST_ARG_LIST));
		return zend_ast_create_list(2, ZEND_AST_STMT_LIST, call, ast);
	}
}

static zend_ast *_process_generic(zend_ast *ast) {
	uint32_t i, children = zend_ast_get_num_children(ast);
	for (i = 0; i < children; i++) {
		if (ast->child[i]) {
			ast->child[i] = _process_ast(ast->child[i]);
		}
	}
	return ast;
}

// inspired by zend_ast_copy
static zend_ast *_process_ast(zend_ast *ast) {
	if (ast->kind == ZEND_AST_ZVAL || ast->kind == ZEND_AST_CONSTANT) {
	} else if (zend_ast_is_list(ast)) {
		zend_ast_list *list = zend_ast_get_list(ast);
		uint32_t i;
		// todo: process list itself somehow?
		for (i = 0; i < list->children; i++) {
			if (list->child[i]) {
				list->child[i] = _process_ast(list->child[i]);
			}
		}
	} else {

		switch (ast->kind) {
			case ZEND_AST_FUNC_DECL:
			case ZEND_AST_METHOD:
				return _process_function(ast);

			case ZEND_AST_RETURN:
				return _process_return(ast);
			default:
				return _process_generic(ast);
		}
	}
	return ast;
}

ZEND_API void ddastrace_ast_process(zend_ast *ast) {
	/* find function and method declarations
	 * wrap body in (roughly):
	 *
	 * ddastrace_span_open();
	 * try { ... }
	 * catch (Throwable $ex) {
	 *   ddastrace_span_close_exception($ex);
	 *   throw $ex;
	 * }
	 * ddastrace_span_close_void();
	 *
	 * Also need to walk the body to find `return` nodes and wrap them:
	 *      ddastrace_span_close(...)
	 *   or ddastrace_span_close_by_ref(...)
	 */
	ast = _process_ast(ast);

	if (ddastrace_prev_ast_process) {
		ddastrace_prev_ast_process(ast);
	}
}
