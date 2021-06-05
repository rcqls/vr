module vr

#flag -I $env('RHOME')/include
#flag -L $env('RHOME')/lib -lR
#flag darwin $env('RHOME')/lib/libR.dylib
#include "@VMODROOT/vr.h"

fn C.vr_init() int
fn C.vr_eval(cmds &char, print int) int
fn C.vr_get_ary(cmd &char, typ &int, len &int) voidptr
fn C.vr_set_ary(name &char, arr voidptr, typ int, len int)
fn C.vr_as_double_ary(res voidptr) &f64
fn C.vr_as_int_ary(res voidptr) &int
fn C.vr_as_bool_ary(res voidptr) &bool
fn C.vr_as_string_ary(res voidptr) &&char

fn init() {
	C.vr_init()
}

pub fn eval(cmd string) {
	C.vr_eval(cmd.str, true)
}

pub fn run(cmd string) {
	C.vr_eval(cmd.str, false)
}

type Res = []f64 | []int | []bool | []string
pub fn get(cmd string) Res {
	typ, len := 0, 0
	arr := C.vr_get_ary(cmd.str, &typ, &len)
	println("typ=$typ, len=$len")
	if typ == 0 {
		res := C.vr_as_double_ary(arr)
		mut ary := []f64{}
		for i in 0..len {
			ary << unsafe{res[i]}
		}
		return ary
	} else if typ == 1 {
		res := C.vr_as_int_ary(arr)
		mut ary := []int{}
		for i in 0..len {
			ary << unsafe{res[i]}
		}
		return ary
	} else if typ == 2 {
		res := C.vr_as_bool_ary(arr)
		mut ary := []bool{}
		for i in 0..len {
			ary << unsafe{res[i]}
		}
		return ary
	} else if typ == 3 {
		res := C.vr_as_string_ary(arr)
		mut ary := []string{}
		for i in 0..len {
			ary << unsafe{cstring_to_vstring(res[i])}
		}
		return ary
	}
	return [0]
}

pub fn int(cmd string) []int {
	res := get(cmd)
	if res is []int {
		return res
	} else {
		return []int{}
	}
}

pub fn bool(cmd string) []bool {
	res := get(cmd)
	if res is []bool {
		return res
	} else {
		return []bool{}
	}
}

pub fn f64(cmd string) []f64 {
	res := get(cmd)
	if res is []f64 {
		return res
	} else {
		return []f64{}
	}
}

pub fn string(cmd string) []string {
	res := get(cmd)
	if res is []string {
		return res
	} else {
		return []string{}
	}
}

pub fn set_f64(name string, arr []f64) {
	C.vr_set_ary(name.str, arr.data, 0, arr.len)
}

pub fn set_int(name string, arr []int) {
	C.vr_set_ary(name.str, arr.data, 1, arr.len)
}

pub fn set_bool(name string, arr []bool) {
	C.vr_set_ary(name.str, arr.data, 2, arr.len)
}

pub fn set_string(name string, arr []string) {
	res := arr.map(it.str)
	C.vr_set_ary(name.str, res.data, 3, arr.len)
}





