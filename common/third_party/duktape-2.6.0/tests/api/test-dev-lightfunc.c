/*
 *  Behavior of lightweight functions from C code in various situations.
 *
 *  Also documents the detailed behavior and limitations of lightfuncs.
 */

static duk_ret_t my_addtwo_lfunc(duk_context *ctx) {
	printf("addtwo entry top: %ld\n", (long) duk_get_top(ctx));

	duk_push_current_function(ctx);
	duk_get_prop_string(ctx, -1, "length");
	printf("addtwo 'length' property: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
	printf("addtwo duk_get_length: %ld\n", (long) duk_get_length(ctx, -1));
	printf("addtwo magic: %ld\n", (long) duk_get_magic(ctx, -1));
	printf("current magic: %ld\n", (long) duk_get_current_magic(ctx));
	duk_pop(ctx);

	duk_push_number(ctx, duk_require_number(ctx, 0) + duk_require_number(ctx, 1));
	printf("addtwo final top: %ld\n", (long) duk_get_top(ctx));
	return 1;
}

static duk_ret_t my_dummy_func(duk_context *ctx) {
	(void) ctx;
	return DUK_RET_ERROR;
}

/*===
*** test_is_lightfunc (duk_safe_call)
0: is_lightfunc: 0
1: is_lightfunc: 0
2: is_lightfunc: 0
3: is_lightfunc: 0
4: is_lightfunc: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_is_lightfunc(duk_context *ctx, void *udata) {
	duk_idx_t i, n;

	(void) udata;

	/* Just a few spot checks. */

	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_object(ctx);
	duk_push_c_function(ctx, my_dummy_func, 0);
	duk_push_c_lightfunc(ctx, my_dummy_func, 0, 0, 0);

	for (i = 0, n = duk_get_top(ctx); i < n; i++) {
		printf("%ld: is_lightfunc: %ld\n", (long) i, (long) duk_is_lightfunc(ctx, i));
	}

	return 0;
}

/*===
*** test_simple_push (duk_safe_call)
top before lfunc push: 2
push retval: 2
top after lfunc push: 3
type at top: 9
typemask at top: 0x0200
addtwo entry top: 2
addtwo 'length' property: 3
addtwo duk_get_length: 3
addtwo magic: -66
current magic: -66
addtwo final top: 3
result: 357
final top: 3
==> rc=0, result='undefined'
===*/

static duk_ret_t test_simple_push(duk_context *ctx, void *udata) {
	duk_idx_t ret;

	(void) udata;

	duk_set_top(ctx, 0);
	duk_push_undefined(ctx);  /* dummy padding */
	duk_push_undefined(ctx);

	printf("top before lfunc push: %ld\n", (long) duk_get_top(ctx));
	ret = duk_push_c_lightfunc(ctx, my_addtwo_lfunc, 2 /*nargs*/, 3 /*length*/, -0x42 /*magic*/);
	printf("push retval: %ld\n", (long) ret);
	printf("top after lfunc push: %ld\n", (long) duk_get_top(ctx));
	printf("type at top: %ld\n", (long) duk_get_type(ctx, -1));
	printf("typemask at top: 0x%04lx\n", (long) duk_get_type_mask(ctx, -1));

	duk_push_string(ctx, "dummy this");
	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_push_int(ctx, 345);
	duk_call_method(ctx, 3 /*nargs*/);  /* [ ... lfunc this 123 234 345 ] -> [ ... retval ] */
	printf("result: %s\n", duk_safe_to_string(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_magic (duk_safe_call)
i=-256, res=TypeError: invalid args
i=-255, res=TypeError: invalid args
i=-254, res=TypeError: invalid args
i=-253, res=TypeError: invalid args
i=-252, res=TypeError: invalid args
i=-251, res=TypeError: invalid args
i=-250, res=TypeError: invalid args
i=-249, res=TypeError: invalid args
i=-248, res=TypeError: invalid args
i=-247, res=TypeError: invalid args
i=-246, res=TypeError: invalid args
i=-245, res=TypeError: invalid args
i=-244, res=TypeError: invalid args
i=-243, res=TypeError: invalid args
i=-242, res=TypeError: invalid args
i=-241, res=TypeError: invalid args
i=-240, res=TypeError: invalid args
i=-239, res=TypeError: invalid args
i=-238, res=TypeError: invalid args
i=-237, res=TypeError: invalid args
i=-236, res=TypeError: invalid args
i=-235, res=TypeError: invalid args
i=-234, res=TypeError: invalid args
i=-233, res=TypeError: invalid args
i=-232, res=TypeError: invalid args
i=-231, res=TypeError: invalid args
i=-230, res=TypeError: invalid args
i=-229, res=TypeError: invalid args
i=-228, res=TypeError: invalid args
i=-227, res=TypeError: invalid args
i=-226, res=TypeError: invalid args
i=-225, res=TypeError: invalid args
i=-224, res=TypeError: invalid args
i=-223, res=TypeError: invalid args
i=-222, res=TypeError: invalid args
i=-221, res=TypeError: invalid args
i=-220, res=TypeError: invalid args
i=-219, res=TypeError: invalid args
i=-218, res=TypeError: invalid args
i=-217, res=TypeError: invalid args
i=-216, res=TypeError: invalid args
i=-215, res=TypeError: invalid args
i=-214, res=TypeError: invalid args
i=-213, res=TypeError: invalid args
i=-212, res=TypeError: invalid args
i=-211, res=TypeError: invalid args
i=-210, res=TypeError: invalid args
i=-209, res=TypeError: invalid args
i=-208, res=TypeError: invalid args
i=-207, res=TypeError: invalid args
i=-206, res=TypeError: invalid args
i=-205, res=TypeError: invalid args
i=-204, res=TypeError: invalid args
i=-203, res=TypeError: invalid args
i=-202, res=TypeError: invalid args
i=-201, res=TypeError: invalid args
i=-200, res=TypeError: invalid args
i=-199, res=TypeError: invalid args
i=-198, res=TypeError: invalid args
i=-197, res=TypeError: invalid args
i=-196, res=TypeError: invalid args
i=-195, res=TypeError: invalid args
i=-194, res=TypeError: invalid args
i=-193, res=TypeError: invalid args
i=-192, res=TypeError: invalid args
i=-191, res=TypeError: invalid args
i=-190, res=TypeError: invalid args
i=-189, res=TypeError: invalid args
i=-188, res=TypeError: invalid args
i=-187, res=TypeError: invalid args
i=-186, res=TypeError: invalid args
i=-185, res=TypeError: invalid args
i=-184, res=TypeError: invalid args
i=-183, res=TypeError: invalid args
i=-182, res=TypeError: invalid args
i=-181, res=TypeError: invalid args
i=-180, res=TypeError: invalid args
i=-179, res=TypeError: invalid args
i=-178, res=TypeError: invalid args
i=-177, res=TypeError: invalid args
i=-176, res=TypeError: invalid args
i=-175, res=TypeError: invalid args
i=-174, res=TypeError: invalid args
i=-173, res=TypeError: invalid args
i=-172, res=TypeError: invalid args
i=-171, res=TypeError: invalid args
i=-170, res=TypeError: invalid args
i=-169, res=TypeError: invalid args
i=-168, res=TypeError: invalid args
i=-167, res=TypeError: invalid args
i=-166, res=TypeError: invalid args
i=-165, res=TypeError: invalid args
i=-164, res=TypeError: invalid args
i=-163, res=TypeError: invalid args
i=-162, res=TypeError: invalid args
i=-161, res=TypeError: invalid args
i=-160, res=TypeError: invalid args
i=-159, res=TypeError: invalid args
i=-158, res=TypeError: invalid args
i=-157, res=TypeError: invalid args
i=-156, res=TypeError: invalid args
i=-155, res=TypeError: invalid args
i=-154, res=TypeError: invalid args
i=-153, res=TypeError: invalid args
i=-152, res=TypeError: invalid args
i=-151, res=TypeError: invalid args
i=-150, res=TypeError: invalid args
i=-149, res=TypeError: invalid args
i=-148, res=TypeError: invalid args
i=-147, res=TypeError: invalid args
i=-146, res=TypeError: invalid args
i=-145, res=TypeError: invalid args
i=-144, res=TypeError: invalid args
i=-143, res=TypeError: invalid args
i=-142, res=TypeError: invalid args
i=-141, res=TypeError: invalid args
i=-140, res=TypeError: invalid args
i=-139, res=TypeError: invalid args
i=-138, res=TypeError: invalid args
i=-137, res=TypeError: invalid args
i=-136, res=TypeError: invalid args
i=-135, res=TypeError: invalid args
i=-134, res=TypeError: invalid args
i=-133, res=TypeError: invalid args
i=-132, res=TypeError: invalid args
i=-131, res=TypeError: invalid args
i=-130, res=TypeError: invalid args
i=-129, res=TypeError: invalid args
i=-128, res=1
i=-127, res=1
i=-126, res=1
i=-125, res=1
i=-124, res=1
i=-123, res=1
i=-122, res=1
i=-121, res=1
i=-120, res=1
i=-119, res=1
i=-118, res=1
i=-117, res=1
i=-116, res=1
i=-115, res=1
i=-114, res=1
i=-113, res=1
i=-112, res=1
i=-111, res=1
i=-110, res=1
i=-109, res=1
i=-108, res=1
i=-107, res=1
i=-106, res=1
i=-105, res=1
i=-104, res=1
i=-103, res=1
i=-102, res=1
i=-101, res=1
i=-100, res=1
i=-99, res=1
i=-98, res=1
i=-97, res=1
i=-96, res=1
i=-95, res=1
i=-94, res=1
i=-93, res=1
i=-92, res=1
i=-91, res=1
i=-90, res=1
i=-89, res=1
i=-88, res=1
i=-87, res=1
i=-86, res=1
i=-85, res=1
i=-84, res=1
i=-83, res=1
i=-82, res=1
i=-81, res=1
i=-80, res=1
i=-79, res=1
i=-78, res=1
i=-77, res=1
i=-76, res=1
i=-75, res=1
i=-74, res=1
i=-73, res=1
i=-72, res=1
i=-71, res=1
i=-70, res=1
i=-69, res=1
i=-68, res=1
i=-67, res=1
i=-66, res=1
i=-65, res=1
i=-64, res=1
i=-63, res=1
i=-62, res=1
i=-61, res=1
i=-60, res=1
i=-59, res=1
i=-58, res=1
i=-57, res=1
i=-56, res=1
i=-55, res=1
i=-54, res=1
i=-53, res=1
i=-52, res=1
i=-51, res=1
i=-50, res=1
i=-49, res=1
i=-48, res=1
i=-47, res=1
i=-46, res=1
i=-45, res=1
i=-44, res=1
i=-43, res=1
i=-42, res=1
i=-41, res=1
i=-40, res=1
i=-39, res=1
i=-38, res=1
i=-37, res=1
i=-36, res=1
i=-35, res=1
i=-34, res=1
i=-33, res=1
i=-32, res=1
i=-31, res=1
i=-30, res=1
i=-29, res=1
i=-28, res=1
i=-27, res=1
i=-26, res=1
i=-25, res=1
i=-24, res=1
i=-23, res=1
i=-22, res=1
i=-21, res=1
i=-20, res=1
i=-19, res=1
i=-18, res=1
i=-17, res=1
i=-16, res=1
i=-15, res=1
i=-14, res=1
i=-13, res=1
i=-12, res=1
i=-11, res=1
i=-10, res=1
i=-9, res=1
i=-8, res=1
i=-7, res=1
i=-6, res=1
i=-5, res=1
i=-4, res=1
i=-3, res=1
i=-2, res=1
i=-1, res=1
i=0, res=1
i=1, res=1
i=2, res=1
i=3, res=1
i=4, res=1
i=5, res=1
i=6, res=1
i=7, res=1
i=8, res=1
i=9, res=1
i=10, res=1
i=11, res=1
i=12, res=1
i=13, res=1
i=14, res=1
i=15, res=1
i=16, res=1
i=17, res=1
i=18, res=1
i=19, res=1
i=20, res=1
i=21, res=1
i=22, res=1
i=23, res=1
i=24, res=1
i=25, res=1
i=26, res=1
i=27, res=1
i=28, res=1
i=29, res=1
i=30, res=1
i=31, res=1
i=32, res=1
i=33, res=1
i=34, res=1
i=35, res=1
i=36, res=1
i=37, res=1
i=38, res=1
i=39, res=1
i=40, res=1
i=41, res=1
i=42, res=1
i=43, res=1
i=44, res=1
i=45, res=1
i=46, res=1
i=47, res=1
i=48, res=1
i=49, res=1
i=50, res=1
i=51, res=1
i=52, res=1
i=53, res=1
i=54, res=1
i=55, res=1
i=56, res=1
i=57, res=1
i=58, res=1
i=59, res=1
i=60, res=1
i=61, res=1
i=62, res=1
i=63, res=1
i=64, res=1
i=65, res=1
i=66, res=1
i=67, res=1
i=68, res=1
i=69, res=1
i=70, res=1
i=71, res=1
i=72, res=1
i=73, res=1
i=74, res=1
i=75, res=1
i=76, res=1
i=77, res=1
i=78, res=1
i=79, res=1
i=80, res=1
i=81, res=1
i=82, res=1
i=83, res=1
i=84, res=1
i=85, res=1
i=86, res=1
i=87, res=1
i=88, res=1
i=89, res=1
i=90, res=1
i=91, res=1
i=92, res=1
i=93, res=1
i=94, res=1
i=95, res=1
i=96, res=1
i=97, res=1
i=98, res=1
i=99, res=1
i=100, res=1
i=101, res=1
i=102, res=1
i=103, res=1
i=104, res=1
i=105, res=1
i=106, res=1
i=107, res=1
i=108, res=1
i=109, res=1
i=110, res=1
i=111, res=1
i=112, res=1
i=113, res=1
i=114, res=1
i=115, res=1
i=116, res=1
i=117, res=1
i=118, res=1
i=119, res=1
i=120, res=1
i=121, res=1
i=122, res=1
i=123, res=1
i=124, res=1
i=125, res=1
i=126, res=1
i=127, res=1
i=128, res=TypeError: invalid args
i=129, res=TypeError: invalid args
i=130, res=TypeError: invalid args
i=131, res=TypeError: invalid args
i=132, res=TypeError: invalid args
i=133, res=TypeError: invalid args
i=134, res=TypeError: invalid args
i=135, res=TypeError: invalid args
i=136, res=TypeError: invalid args
i=137, res=TypeError: invalid args
i=138, res=TypeError: invalid args
i=139, res=TypeError: invalid args
i=140, res=TypeError: invalid args
i=141, res=TypeError: invalid args
i=142, res=TypeError: invalid args
i=143, res=TypeError: invalid args
i=144, res=TypeError: invalid args
i=145, res=TypeError: invalid args
i=146, res=TypeError: invalid args
i=147, res=TypeError: invalid args
i=148, res=TypeError: invalid args
i=149, res=TypeError: invalid args
i=150, res=TypeError: invalid args
i=151, res=TypeError: invalid args
i=152, res=TypeError: invalid args
i=153, res=TypeError: invalid args
i=154, res=TypeError: invalid args
i=155, res=TypeError: invalid args
i=156, res=TypeError: invalid args
i=157, res=TypeError: invalid args
i=158, res=TypeError: invalid args
i=159, res=TypeError: invalid args
i=160, res=TypeError: invalid args
i=161, res=TypeError: invalid args
i=162, res=TypeError: invalid args
i=163, res=TypeError: invalid args
i=164, res=TypeError: invalid args
i=165, res=TypeError: invalid args
i=166, res=TypeError: invalid args
i=167, res=TypeError: invalid args
i=168, res=TypeError: invalid args
i=169, res=TypeError: invalid args
i=170, res=TypeError: invalid args
i=171, res=TypeError: invalid args
i=172, res=TypeError: invalid args
i=173, res=TypeError: invalid args
i=174, res=TypeError: invalid args
i=175, res=TypeError: invalid args
i=176, res=TypeError: invalid args
i=177, res=TypeError: invalid args
i=178, res=TypeError: invalid args
i=179, res=TypeError: invalid args
i=180, res=TypeError: invalid args
i=181, res=TypeError: invalid args
i=182, res=TypeError: invalid args
i=183, res=TypeError: invalid args
i=184, res=TypeError: invalid args
i=185, res=TypeError: invalid args
i=186, res=TypeError: invalid args
i=187, res=TypeError: invalid args
i=188, res=TypeError: invalid args
i=189, res=TypeError: invalid args
i=190, res=TypeError: invalid args
i=191, res=TypeError: invalid args
i=192, res=TypeError: invalid args
i=193, res=TypeError: invalid args
i=194, res=TypeError: invalid args
i=195, res=TypeError: invalid args
i=196, res=TypeError: invalid args
i=197, res=TypeError: invalid args
i=198, res=TypeError: invalid args
i=199, res=TypeError: invalid args
i=200, res=TypeError: invalid args
i=201, res=TypeError: invalid args
i=202, res=TypeError: invalid args
i=203, res=TypeError: invalid args
i=204, res=TypeError: invalid args
i=205, res=TypeError: invalid args
i=206, res=TypeError: invalid args
i=207, res=TypeError: invalid args
i=208, res=TypeError: invalid args
i=209, res=TypeError: invalid args
i=210, res=TypeError: invalid args
i=211, res=TypeError: invalid args
i=212, res=TypeError: invalid args
i=213, res=TypeError: invalid args
i=214, res=TypeError: invalid args
i=215, res=TypeError: invalid args
i=216, res=TypeError: invalid args
i=217, res=TypeError: invalid args
i=218, res=TypeError: invalid args
i=219, res=TypeError: invalid args
i=220, res=TypeError: invalid args
i=221, res=TypeError: invalid args
i=222, res=TypeError: invalid args
i=223, res=TypeError: invalid args
i=224, res=TypeError: invalid args
i=225, res=TypeError: invalid args
i=226, res=TypeError: invalid args
i=227, res=TypeError: invalid args
i=228, res=TypeError: invalid args
i=229, res=TypeError: invalid args
i=230, res=TypeError: invalid args
i=231, res=TypeError: invalid args
i=232, res=TypeError: invalid args
i=233, res=TypeError: invalid args
i=234, res=TypeError: invalid args
i=235, res=TypeError: invalid args
i=236, res=TypeError: invalid args
i=237, res=TypeError: invalid args
i=238, res=TypeError: invalid args
i=239, res=TypeError: invalid args
i=240, res=TypeError: invalid args
i=241, res=TypeError: invalid args
i=242, res=TypeError: invalid args
i=243, res=TypeError: invalid args
i=244, res=TypeError: invalid args
i=245, res=TypeError: invalid args
i=246, res=TypeError: invalid args
i=247, res=TypeError: invalid args
i=248, res=TypeError: invalid args
i=249, res=TypeError: invalid args
i=250, res=TypeError: invalid args
i=251, res=TypeError: invalid args
i=252, res=TypeError: invalid args
i=253, res=TypeError: invalid args
i=254, res=TypeError: invalid args
i=255, res=TypeError: invalid args
i=256, res=TypeError: invalid args
==> rc=0, result='undefined'
===*/

static duk_ret_t test_magic_raw(duk_context *ctx, void *udata) {
	int i = duk_require_int(ctx, -1);
	duk_idx_t ret;

	(void) udata;

	ret = duk_push_c_lightfunc(ctx, my_addtwo_lfunc, 2 /*nargs*/, 3 /*length*/, i /*magic*/);
	duk_push_int(ctx, (duk_int_t) ret);
	return 1;
}

static duk_ret_t test_magic(duk_context *ctx, void *udata) {
	int i;

	(void) udata;

	for (i = -256; i <= 256; i++) {
		duk_push_int(ctx, i);
		duk_safe_call(ctx, test_magic_raw, NULL, 1, 1);
		printf("i=%ld, res=%s\n", (long) i, duk_safe_to_string(ctx, -1));
		duk_pop(ctx);
	}
	return 0;
}

/*===
*** test_length_values (duk_safe_call)
i=-16, res=TypeError: invalid args
i=-15, res=TypeError: invalid args
i=-14, res=TypeError: invalid args
i=-13, res=TypeError: invalid args
i=-12, res=TypeError: invalid args
i=-11, res=TypeError: invalid args
i=-10, res=TypeError: invalid args
i=-9, res=TypeError: invalid args
i=-8, res=TypeError: invalid args
i=-7, res=TypeError: invalid args
i=-6, res=TypeError: invalid args
i=-5, res=TypeError: invalid args
i=-4, res=TypeError: invalid args
i=-3, res=TypeError: invalid args
i=-2, res=TypeError: invalid args
i=-1, res=TypeError: invalid args
i=0, res=1
i=1, res=1
i=2, res=1
i=3, res=1
i=4, res=1
i=5, res=1
i=6, res=1
i=7, res=1
i=8, res=1
i=9, res=1
i=10, res=1
i=11, res=1
i=12, res=1
i=13, res=1
i=14, res=1
i=15, res=1
i=16, res=TypeError: invalid args
==> rc=0, result='undefined'
===*/

static duk_ret_t test_length_raw(duk_context *ctx, void *udata) {
	int i = duk_require_int(ctx, -1);
	duk_idx_t ret;

	(void) udata;

	ret = duk_push_c_lightfunc(ctx, my_addtwo_lfunc, 2 /*nargs*/, i /*length*/, 0x42 /*magic*/);
	duk_push_int(ctx, (duk_int_t) ret);
	return 1;
}

static duk_ret_t test_length_values(duk_context *ctx, void *udata) {
	int i;

	(void) udata;

	for (i = -16; i <= 16; i++) {
		duk_push_int(ctx, i);
		duk_safe_call(ctx, test_length_raw, NULL, 1, 1);
		printf("i=%ld, res=%s\n", (long) i, duk_safe_to_string(ctx, -1));
		duk_pop(ctx);
	}

	return 0;
}

/*===
*** test_nargs_values (duk_safe_call)
i=-16, nargs=-16, res=TypeError: invalid args
i=-15, nargs=-15, res=TypeError: invalid args
i=-14, nargs=-14, res=TypeError: invalid args
i=-13, nargs=-13, res=TypeError: invalid args
i=-12, nargs=-12, res=TypeError: invalid args
i=-11, nargs=-11, res=TypeError: invalid args
i=-10, nargs=-10, res=TypeError: invalid args
i=-9, nargs=-9, res=TypeError: invalid args
i=-8, nargs=-8, res=TypeError: invalid args
i=-7, nargs=-7, res=TypeError: invalid args
i=-6, nargs=-6, res=TypeError: invalid args
i=-5, nargs=-5, res=TypeError: invalid args
i=-4, nargs=-4, res=TypeError: invalid args
i=-3, nargs=-3, res=TypeError: invalid args
i=-2, nargs=-2, res=TypeError: invalid args
i=-1, nargs=-1 (varargs), res=1
i=0, nargs=0, res=1
i=1, nargs=1, res=1
i=2, nargs=2, res=1
i=3, nargs=3, res=1
i=4, nargs=4, res=1
i=5, nargs=5, res=1
i=6, nargs=6, res=1
i=7, nargs=7, res=1
i=8, nargs=8, res=1
i=9, nargs=9, res=1
i=10, nargs=10, res=1
i=11, nargs=11, res=1
i=12, nargs=12, res=1
i=13, nargs=13, res=1
i=14, nargs=14, res=1
i=15, nargs=15, res=TypeError: invalid args
i=16, nargs=16, res=TypeError: invalid args
i=17, nargs=-1 (varargs), res=1
i=18, nargs=18, res=TypeError: invalid args
==> rc=0, result='undefined'
===*/

static duk_ret_t test_nargs_raw(duk_context *ctx, void *udata) {
	int i = duk_require_int(ctx, -1);
	duk_idx_t ret;

	(void) udata;

	ret = duk_push_c_lightfunc(ctx, my_addtwo_lfunc, i /*nargs*/, 2 /*length*/, 0x42 /*magic*/);
	duk_push_int(ctx, (duk_int_t) ret);
	return 1;
}

static duk_ret_t test_nargs_values(duk_context *ctx, void *udata) {
	int i;
	int nargs;
	int is_vararg;

	(void) udata;

	for (i = -16; i <= 18; i++) {
		if (i == 17) {
			duk_push_int(ctx, DUK_VARARGS);
		} else {
			duk_push_int(ctx, i);
		}
		nargs = duk_get_int(ctx, -1);
		is_vararg =  (nargs == DUK_VARARGS);
		duk_safe_call(ctx, test_nargs_raw, NULL, 1, 1);
		printf("i=%ld, nargs=%ld%s, res=%s\n",
		       (long) i, (long) nargs, (is_vararg ? " (varargs)" : ""),
		       duk_safe_to_string(ctx, -1));
		duk_pop(ctx);
	}

	return 0;
}

/*===
*** test_enum (duk_safe_call)
enum defaults
top: 1
enum nonenumerable
key: length
key: name
key: constructor
key: toString
key: apply
key: call
key: bind
key: __proto__
key: toLocaleString
key: valueOf
key: hasOwnProperty
key: isPrototypeOf
key: propertyIsEnumerable
key: __defineGetter__
key: __defineSetter__
key: __lookupGetter__
key: __lookupSetter__
top: 1
enum own
top: 1
enum own non-enumerable
key: length
key: name
top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_enum(duk_context *ctx, void *udata) {
	(void) udata;

	(void) duk_push_c_lightfunc(ctx, my_addtwo_lfunc, 2 /*nargs*/, 3 /*length*/, 0x42 /*magic*/);

	printf("enum defaults\n");
	duk_enum(ctx, -1, 0);
	while (duk_next(ctx, -1, 0 /*get_value*/)) {
		printf("key: %s\n", duk_to_string(ctx, -1));
		duk_pop(ctx);
	}
	duk_pop(ctx);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	printf("enum nonenumerable\n");
	duk_enum(ctx, -1, DUK_ENUM_INCLUDE_NONENUMERABLE);
	while (duk_next(ctx, -1, 0 /*get_value*/)) {
		printf("key: %s\n", duk_to_string(ctx, -1));
		duk_pop(ctx);
	}
	duk_pop(ctx);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	printf("enum own\n");
	duk_enum(ctx, -1, DUK_ENUM_OWN_PROPERTIES_ONLY);
	while (duk_next(ctx, -1, 0 /*get_value*/)) {
		printf("key: %s\n", duk_to_string(ctx, -1));
		duk_pop(ctx);
	}
	duk_pop(ctx);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	printf("enum own non-enumerable\n");
	duk_enum(ctx, -1, DUK_ENUM_OWN_PROPERTIES_ONLY | DUK_ENUM_INCLUDE_NONENUMERABLE);
	while (duk_next(ctx, -1, 0 /*get_value*/)) {
		printf("key: %s\n", duk_to_string(ctx, -1));
		duk_pop(ctx);
	}
	duk_pop(ctx);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	return 0;
}

/*===
*** test_get_length (duk_safe_call)
lightFunc len: 3
ecmaFunc.len: 3
final top: 2
==> rc=0, result='undefined'
===*/

static duk_ret_t test_get_length(duk_context *ctx, void *udata) {
	duk_size_t len;

	(void) udata;

	/*
	 *  Lightfunc length is its virtual 'length' property, same as for
	 *  ordinary functions.
	 */

	(void) duk_push_c_lightfunc(ctx, my_addtwo_lfunc, 2 /*nargs*/, 3 /*length*/, 0x42 /*magic*/);

	len = duk_get_length(ctx, -1);
	printf("lightFunc len: %ld\n", (long) len);

	duk_eval_string(ctx, "(function (a,b,c) {})");
	len = duk_get_length(ctx, -1);
	printf("ecmaFunc.len: %ld\n", (long) len);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_to_object (duk_safe_call)
tag before: 9
tag after: 6
addtwo entry top: 2
addtwo 'length' property: 3
addtwo duk_get_length: 3
addtwo magic: 66
current magic: 66
addtwo final top: 3
result: 357
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_to_object(duk_context *ctx, void *udata) {
	(void) udata;

	(void) duk_push_c_lightfunc(ctx, my_addtwo_lfunc, 2 /*nargs*/, 3 /*length*/, 0x42 /*magic*/);

	printf("tag before: %ld\n", (long) duk_get_type(ctx, -1));

	duk_to_object(ctx, -1);

	printf("tag after: %ld\n", (long) duk_get_type(ctx, -1));

	/* The coerced function works as before */

	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_push_int(ctx, 345);
	duk_call(ctx, 3);

	printf("result: %s\n", duk_safe_to_string(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_to_buffer (duk_safe_call)
function light_PTR_4232() { [lightfunc code] }
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_to_buffer(duk_context *ctx, void *udata) {
	duk_size_t sz;
	unsigned char *p;

	(void) udata;

	(void) duk_push_c_lightfunc(ctx, my_addtwo_lfunc, 2 /*nargs*/, 3 /*length*/, 0x42 /*magic*/);

	/*
	 *  Lightfunc-to-buffer coercion currently produces a string: the
	 *  lightfunc gets coerced to a string like a normal function would.
	 *  The buffer is then filled with the bytes from this coercion.
	 *
	 *  The output must be sanitized because it is platform specific.
	 */

	p = (unsigned char *) duk_to_buffer(ctx, -1, &sz);
	if (!p) {
		printf("ptr is NULL\n");
	} else {
		/* Don't print length because it depends on pointer length
		 * and thus architecture.
		 */
#if 0
		printf("%ld: ", (long) sz);
#endif

		/* Sanitize with ECMAScript because it's easier... */
		duk_eval_string(ctx, "(function (x) { return String(x)"
		                     ".replace(/\\/\\*/g, '(*').replace(/\\*\\//g, '*)')"
		                     ".replace(/light_[0-9a-fA-F]+_/g, 'light_PTR_'); })");
		duk_dup(ctx, -2);
		duk_buffer_to_string(ctx, -1);
		duk_call(ctx, 1);

		printf("%s\n", duk_to_string(ctx, -1));
		duk_pop(ctx);  /* pop temp */
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_to_pointer (duk_safe_call)
ptr is NULL: 1
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_to_pointer(duk_context *ctx, void *udata) {
	void *p;

	(void) udata;

	(void) duk_push_c_lightfunc(ctx, my_addtwo_lfunc, 2 /*nargs*/, 3 /*length*/, 0x42 /*magic*/);

	/*
	 *  Lightfunc-to-pointer coercion currently produces a NULL: there is
	 *  no portable way to cast a function pointer to a data pointer, as
	 *  there may be segmentation etc involved.  This could be improved to
	 *  work on specific platforms.
	 */

	p = duk_to_pointer(ctx, -1);
	printf("ptr is NULL: %d\n", (int) (p == NULL ? 1 : 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_is_primitive (duk_safe_call)
is_primitive: 0
final top: 1
==> rc=0, result='undefined'
===*/

/* While lightfuncs have their own type tag in the C API, they're not considered
 * a primitive type in Duktape 2.x because they are also coerced by
 * duk_to_primitive().  This was changed to match plain buffers in 2.x.
 */

static duk_ret_t test_is_primitive(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_c_lightfunc(ctx, my_dummy_func, 0, 0, 0);
	printf("is_primitive: %ld\n", (long) duk_is_primitive(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_is_object (duk_safe_call)
is_object: 0
final top: 1
==> rc=0, result='undefined'
===*/

/* Lightfuncs are not objects (they're primitive). */

static duk_ret_t test_is_object(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_c_lightfunc(ctx, my_dummy_func, 0, 0, 0);
	printf("is_object: %ld\n", (long) duk_is_object(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
/*===
*** test_is_object_coercible (duk_safe_call)
is_object_coercible: 1
final top: 1
==> rc=0, result='undefined'
===*/

/* Lightfuncs are object coercible. */

static duk_ret_t test_is_object_coercible(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_c_lightfunc(ctx, my_dummy_func, 0, 0, 0);
	printf("is_object_coercible: %ld\n", (long) duk_is_object_coercible(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
still here
===*/

void test(duk_context *ctx) {
	/* nargs / length limits, C api test, check what happens if you exceed */
	/* Example of using lightfunc as a constructor, separate testcase, doc ref */

	TEST_SAFE_CALL(test_is_lightfunc);
	TEST_SAFE_CALL(test_simple_push);
	TEST_SAFE_CALL(test_magic);
	TEST_SAFE_CALL(test_length_values);
	TEST_SAFE_CALL(test_nargs_values);
	TEST_SAFE_CALL(test_enum);
	TEST_SAFE_CALL(test_get_length);
	TEST_SAFE_CALL(test_to_object);
	TEST_SAFE_CALL(test_to_buffer);
	TEST_SAFE_CALL(test_to_pointer);
	TEST_SAFE_CALL(test_is_primitive);
	TEST_SAFE_CALL(test_is_object);
	TEST_SAFE_CALL(test_is_object_coercible);

	printf("still here\n");
	fflush(stdout);
}
