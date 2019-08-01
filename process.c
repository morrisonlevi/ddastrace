#include "process.h"

void (*ddastrace_prev_ast_process)(zend_ast *ast);

#define zend_ast_alloc(size) zend_arena_alloc(&CG(ast_arena), size);

static inline size_t zend_ast_list_size(uint32_t children) {
	return sizeof(zend_ast_list) - sizeof(zend_ast *) + sizeof(zend_ast *) * children;
}

static void _process_stmt(zend_ast *ast);

static void _process_list(zend_ast *ast) {
	zend_ast_list *list = zend_ast_get_list(ast);
	zend_ast **begin, **end = &list->child[list->children];

	for (begin = list->child; begin != end; ++begin) {
		_process_stmt(*begin);
	}
}

static zend_ast *_create_ast_str(char *str, size_t len, uint32_t attr) {
	zval zv;
	zend_ast *ast;
	ZVAL_NEW_STR(&zv, zend_string_init(str, len, 0));
	ast = zend_ast_create_zval_with_lineno(&zv, 0);
	ast->attr = attr;
	return ast;
}

static zend_ast *_create_ast_var(char *str, size_t len) {
	return zend_ast_create(ZEND_AST_VAR, _create_ast_str(str, len, 0));
}

static zend_ast *_create_ast_catch_name() {
	zend_ast *name = _create_ast_str("Throwable", sizeof("Throwable") - 1, ZEND_NAME_FQ);
#if PHP_VERSION_ID < 70100
	return name;
#else
	return zend_ast_create_list(1, ZEND_AST_NAME_LIST, name);
#endif
}
static zend_ast *_create_ast_catch(void) {
	zend_ast *catch_var_name = _create_ast_var("ex", sizeof("ex") - 1);
	zend_ast *catch_type_name = _create_ast_catch_name();
	zend_ast *catch_stmt_list = zend_ast_create_list(0, ZEND_AST_STMT_LIST);

	zend_ast *catch = zend_ast_create_list(1, ZEND_AST_CATCH_LIST,
			zend_ast_create(ZEND_AST_CATCH, catch_type_name, catch_var_name, catch_stmt_list)
			);
	return catch;
}

static void _process_function(zend_ast *ast) {
	zend_ast_decl *decl = (zend_ast_decl *) ast;

	zend_ast *span_open = zend_ast_create(ZEND_AST_CALL,
		_create_ast_str("ddastrace_span_open", sizeof("ddastrace_span_open") - 1, ZEND_NAME_FQ),
		zend_ast_create_list(0, ZEND_AST_ARG_LIST));

	zend_ast *span_close = zend_ast_create(ZEND_AST_CALL,
		_create_ast_str("ddastrace_span_close_void", sizeof("ddastrace_span_close_void") - 1, ZEND_NAME_FQ),
		zend_ast_create_list(0, ZEND_AST_ARG_LIST));

	zend_ast *catch = _create_ast_catch();
	zend_ast *finally = NULL;
	zend_ast *try = zend_ast_create(ZEND_AST_TRY, decl->child[2], catch, finally);

	zend_ast_list *new_list = zend_ast_alloc(zend_ast_list_size(3));
	new_list->kind = ZEND_AST_STMT_LIST;
	new_list->lineno = 0;
	new_list->children = 3;
	new_list->child[0] = span_open;
	new_list->child[1] = try;
	new_list->child[2] = span_close;

	decl->child[2] = (zend_ast *) new_list;

}

static void _process_class(zend_ast *ast) {
	zend_ast_decl *decl = (zend_ast_decl *) ast;
	if (decl->flags & (ZEND_ACC_INTERFACE | ZEND_ACC_TRAIT)) {
		// I don't know what to do with trait methods :/
		// Interfaces can't have bodies, so skip ast manipulation
		return;
	}
}

static void _process_return(zend_ast *ast) {}

static void _process_stmt(zend_ast *ast) {
	if (!ast) {
		return;
	}

	switch (ast->kind) {
		case ZEND_AST_STMT_LIST:
			_process_list(ast);
			break;

		case ZEND_AST_FUNC_DECL:
		case ZEND_AST_METHOD:
			_process_function(ast);
			break;

		case ZEND_AST_CLASS:
			_process_class(ast);
			break;

		case ZEND_AST_RETURN:
			_process_return(ast);
			break;

	}
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
	 * Be careful not to go into other functions such as closures and
	 * methods in an anonymous class!
	 */
	_process_stmt(ast);

	if (ddastrace_prev_ast_process) {
		ddastrace_prev_ast_process(ast);
	}
}
