/*
 *  Duktape 1.0 issue: cannot set Duktape/C function 'name' property after
 *  creation.
 *
 *  The cause: Function.prototype.name, containing an empty string, is
 *  non-writable.  This is an unintended side effect of all built-in
 *  functions (such as Array, Math.cos, etc) having a non-writable name.
 *  Function.prototype, although never really called, is technically a
 *  function so its empty name is unintentionally non-writable.
 *
 *  Fixed in Duktape 1.1 so that Function.prototype.name is writable.
 *  The best fix would actually be to have the property non-writable
 *  while simultaneously allowing a Function instance's 'name' property
 *  to be set - but ECMAScript cannot express such an access control
 *  policy.
 *
 *  Duktape 2.x follows ES2015 where .name is again non-writable, but is
 *  configurable.  So duk_def_prop() can be used to set the name.
 */

/*===
*** test_1 (duk_safe_call)
writable: false
enumerable: false
configurable: true
MyFunc.name: my_func_name
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t my_func(duk_context *ctx) {
	(void) ctx;
	return 0;
}

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	/* Check that Function.prototype.name is writable. */
	duk_eval_string_noresult(ctx,
		"var pd = Object.getOwnPropertyDescriptor(Function.prototype, 'name');\n"
		"print('writable:', pd.writable);\n"
		"print('enumerable:', pd.enumerable);\n"
		"print('configurable:', pd.configurable);\n");

	duk_push_c_function(ctx, my_func, 0);
	duk_push_string(ctx, "name");
	duk_push_string(ctx, "my_func_name");
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE);
	duk_put_global_string(ctx, "MyFunc");

	duk_eval_string_noresult(ctx,
		"print('MyFunc.name:', MyFunc.name);\n");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
