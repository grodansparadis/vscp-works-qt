/*===
*** test_1 (duk_safe_call)
00: valididx=1 und=1 null=0 noru=1 bool=0 num=0 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 construct=0 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=1 objcoerc=0
01: valididx=1 und=0 null=1 noru=1 bool=0 num=0 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 construct=0 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=1 objcoerc=0
02: valididx=1 und=0 null=0 noru=0 bool=1 num=0 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 construct=0 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=1 objcoerc=1
03: valididx=1 und=0 null=0 noru=0 bool=1 num=0 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 construct=0 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=1 objcoerc=1
04: valididx=1 und=0 null=0 noru=0 bool=0 num=1 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 construct=0 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=1 objcoerc=1
05: valididx=1 und=0 null=0 noru=0 bool=0 num=1 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 construct=0 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=1 objcoerc=1
06: valididx=1 und=0 null=0 noru=0 bool=0 num=1 nan=1 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 construct=0 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=1 objcoerc=1
07: valididx=1 und=0 null=0 noru=0 bool=0 num=1 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 construct=0 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=1 objcoerc=1
08: valididx=1 und=0 null=0 noru=0 bool=0 num=1 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 construct=0 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=1 objcoerc=1
09: valididx=1 und=0 null=0 noru=0 bool=0 num=0 nan=0 str=1 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 construct=0 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=1 objcoerc=1
10: valididx=1 und=0 null=0 noru=0 bool=0 num=0 nan=0 str=1 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 construct=0 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=1 objcoerc=1
11: valididx=1 und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=1 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 construct=0 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=0 objcoerc=1
12: valididx=1 und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=1 arr=1 fun=0 cfun=0 efun=0 bfun=0 call=0 construct=0 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=0 objcoerc=1
13: valididx=1 und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=1 arr=0 fun=1 cfun=1 efun=0 bfun=0 call=1 construct=1 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=0 objcoerc=1
14: valididx=1 und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=1 arr=0 fun=1 cfun=0 efun=1 bfun=0 call=1 construct=1 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=0 objcoerc=1
15: valididx=1 und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=1 arr=0 fun=1 cfun=0 efun=0 bfun=1 call=1 construct=1 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=0 objcoerc=1
16: valididx=1 und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=1 arr=0 fun=1 cfun=1 efun=0 bfun=0 call=1 construct=0 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=0 objcoerc=1
17: valididx=1 und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=1 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 construct=0 thr=1 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=0 objcoerc=1
18: valididx=1 und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 construct=0 thr=0 buf=1 dyn=0 fix=1 ext=0 ptr=0 prim=0 objcoerc=1
19: valididx=1 und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 construct=0 thr=0 buf=1 dyn=1 fix=0 ext=0 ptr=0 prim=0 objcoerc=1
20: valididx=1 und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 construct=0 thr=0 buf=1 dyn=0 fix=0 ext=1 ptr=0 prim=0 objcoerc=1
21: valididx=1 und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 construct=0 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=1 prim=1 objcoerc=1
22: valididx=1 und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=1 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 construct=0 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=0 objcoerc=1
23: valididx=1 und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=1 arr=0 fun=1 cfun=0 efun=0 bfun=0 call=1 construct=0 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=0 objcoerc=1
24: valididx=1 und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=1 arr=0 fun=1 cfun=0 efun=0 bfun=0 call=1 construct=1 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=0 objcoerc=1
25: valididx=0 und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 construct=0 thr=0 buf=0 dyn=0 fix=0 ext=0 ptr=0 prim=0 objcoerc=0
==> rc=0, result='undefined'
===*/

#include <math.h>

static duk_ret_t my_c_func(duk_context *ctx) {
	return 0;
}

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	duk_idx_t i, n;

	(void) udata;

	/*
	 *  push test values
	 */

	/* 0 */
	duk_push_undefined(ctx);

	/* 1 */
	duk_push_null(ctx);

	/* 2 */
	duk_push_true(ctx);

	/* 3 */
	duk_push_false(ctx);

	/* 4 */
	duk_push_int(ctx, 123);

	/* 5 */
	duk_push_number(ctx, 123.4);

	/* 6 */
	duk_push_nan(ctx);

	/* 7 */
	duk_push_number(ctx, INFINITY);

	/* 8 */
	duk_push_number(ctx, -INFINITY);

	/* 9 */
	duk_push_string(ctx, "");

	/* 10 */
	duk_push_string(ctx, "foo");

	/* 11 */
	duk_push_object(ctx);

	/* 12 */
	duk_push_array(ctx);

	/* 13 */
	duk_push_c_function(ctx, my_c_func, DUK_VARARGS);

	/* 14 */
	duk_eval_string(ctx, "(function() { print('hello'); })");

	/* 15 */
	duk_eval_string(ctx, "escape.bind(null, 'foo')");

	/* 16 */
	duk_eval_string(ctx, "Math.cos");  /* callable, not constructable */

	/* 17 */
	duk_push_thread(ctx);

	/* 18 */
	duk_push_buffer(ctx, 1024, 0 /*dynamic*/);

	/* 19 */
	duk_push_buffer(ctx, 1024, 1 /*dynamic*/);

	/* 20 */
	duk_push_external_buffer(ctx);

	/* 21 */
	duk_push_pointer(ctx, (void *) 0xf00);

	/* 22 */
	duk_eval_string(ctx, "new Proxy({}, {})");  /* non-callable proxy */

	/* 23 */
	duk_eval_string(ctx, "new Proxy(Math.sin, {})");  /* callable but not constructable proxy */

	/* 24 */
	duk_eval_string(ctx, "new Proxy(function dummy() {}, {})");  /* callable and constructable proxy */

	/*
	 *  call checkers for each
	 *
	 *  intentionally overstep by one to see what each returns for an
	 *  invalid index
	 */

	n = duk_get_top(ctx);
	for (i = 0; i <= n; i++) {
		printf("%02ld:", (long) i);
		printf(" valididx=%d", (int) duk_is_valid_index(ctx, i));
		printf(" und=%d", (int) duk_is_undefined(ctx, i));
		printf(" null=%d", (int) duk_is_null(ctx, i));
		printf(" noru=%d", (int) duk_is_null_or_undefined(ctx, i));
		printf(" bool=%d", (int) duk_is_boolean(ctx, i));
		printf(" num=%d", (int) duk_is_number(ctx, i));
		printf(" nan=%d", (int) duk_is_nan(ctx, i));
		printf(" str=%d", (int) duk_is_string(ctx, i));
		printf(" obj=%d", (int) duk_is_object(ctx, i));
		printf(" arr=%d", (int) duk_is_array(ctx, i));
		printf(" fun=%d", (int) duk_is_function(ctx, i));
		printf(" cfun=%d", (int) duk_is_c_function(ctx, i));
		printf(" efun=%d", (int) duk_is_ecmascript_function(ctx, i));
		printf(" bfun=%d", (int) duk_is_bound_function(ctx, i));
		printf(" call=%d", (int) duk_is_callable(ctx, i));
		printf(" construct=%d", (int) duk_is_constructable(ctx, i));
		printf(" thr=%d", (int) duk_is_thread(ctx, i));
		printf(" buf=%d", (int) duk_is_buffer(ctx, i));
		printf(" dyn=%d", (int) duk_is_dynamic_buffer(ctx, i));
		printf(" fix=%d", (int) duk_is_fixed_buffer(ctx, i));
		printf(" ext=%d", (int) duk_is_external_buffer(ctx, i));
		printf(" ptr=%d", (int) duk_is_pointer(ctx, i));
		printf(" prim=%d", (int) duk_is_primitive(ctx, i));
		printf(" objcoerc=%d", (int) duk_is_object_coercible(ctx, i));
		printf("\n");
	}

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
