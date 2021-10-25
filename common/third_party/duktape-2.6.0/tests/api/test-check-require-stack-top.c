/*===
*** test_1 (duk_safe_call)
==> rc=1, result='RangeError: cannot push beyond allocated stack'
*** check_1 (duk_safe_call)
rc=1
final top: 1000
==> rc=0, result='undefined'
*** check_2 (duk_safe_call)
final top: 1000
==> rc=0, result='undefined'
*** check_3 (duk_safe_call)
rc=0
final top: 0
==> rc=0, result='undefined'
*** check_4 (duk_safe_call)
rc=0
final top: 0
==> rc=0, result='undefined'
*** require_1 (duk_safe_call)
final top: 1000
==> rc=0, result='undefined'
*** require_2 (duk_safe_call)
final top: 1000
==> rc=0, result='undefined'
*** require_3 (duk_safe_call)
==> rc=1, result='RangeError: valstack limit'
*** require_4 (duk_safe_call)
==> rc=1, result='RangeError: valstack limit'
===*/

/* demonstrate how pushing too many elements causes an error */
static duk_ret_t test_1(duk_context *ctx, void *udata) {
	int i;

	(void) udata;

	for (i = 0; i < 1000; i++) {
		duk_push_int(ctx, 123);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* demonstrate how using one large duk_check_stack_top() before such
 * a loop works.
 */
static duk_ret_t check_1_inner(duk_context *ctx) {
	int i;
	duk_ret_t rc;

	rc = duk_check_stack_top(ctx, 1000);
	printf("rc=%d\n", (int) rc);

	for (i = 0; i < 1000; i++) {
		duk_push_int(ctx, 123);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t check_1(duk_context *ctx, void *udata) {
	int i;

	(void) udata;

	/* dummy filler */
	for (i = 0; i < 10000; i++) {
		duk_require_stack(ctx, 1);
		duk_push_uint(ctx, 123);
	}

	duk_push_c_function(ctx, check_1_inner, 0);
	duk_call(ctx, 0);

	return 0;
}

/* same test but with one element checks, once per loop */
static duk_ret_t check_2(duk_context *ctx, void *udata) {
	int i;
	duk_ret_t rc;

	(void) udata;

	for (i = 0; i < 1000; i++) {
		rc = duk_check_stack_top(ctx, i + 1);
		if (rc == 0) {
			printf("duk_check_stack failed\n");
		}
		duk_push_int(ctx, 123);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* try to extend value stack too much with duk_check_stack_top */
static duk_ret_t check_3(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	rc = duk_check_stack_top(ctx, 1000*1000*1000);
	printf("rc=%d\n", (int) rc);  /* should print 0: fail */

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* try to extend value stack too much; value is high enough to wrap */
static duk_ret_t check_4(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	rc = duk_check_stack_top(ctx, DUK_IDX_MAX);
	printf("rc=%d\n", (int) rc);  /* should print 0: fail */

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* same as check_1 but with duk_require_stack_top() */
static duk_ret_t require_1_inner(duk_context *ctx) {
	int i;

	duk_require_stack_top(ctx, 1000);

	for (i = 0; i < 1000; i++) {
		duk_push_int(ctx, 123);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t require_1(duk_context *ctx, void *udata) {
	int i;

	(void) udata;

	/* dummy filler */
	for (i = 0; i < 10000; i++) {
		duk_require_stack(ctx, 1);
		duk_push_uint(ctx, 123);
	}

	duk_push_c_function(ctx, require_1_inner, 0);
	duk_call(ctx, 0);

	return 0;
}

/* same as check_2 but with duk_require_stack_top() */
static duk_ret_t require_2(duk_context *ctx, void *udata) {
	int i;

	(void) udata;

	for (i = 0; i < 1000; i++) {
		duk_require_stack_top(ctx, i + 1);
		duk_push_int(ctx, 123);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* same as check_3 but with duk_require_stack_top */
static duk_ret_t require_3(duk_context *ctx, void *udata) {
	(void) udata;

	duk_require_stack_top(ctx, 1000*1000*1000);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* same as check_4 but with duk_require_stack_top */
static duk_ret_t require_4(duk_context *ctx, void *udata) {
	(void) udata;

	duk_require_stack_top(ctx, DUK_IDX_MAX);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(check_1);
	TEST_SAFE_CALL(check_2);
	TEST_SAFE_CALL(check_3);
	TEST_SAFE_CALL(check_4);
	TEST_SAFE_CALL(require_1);
	TEST_SAFE_CALL(require_2);
	TEST_SAFE_CALL(require_3);
	TEST_SAFE_CALL(require_4);
}
