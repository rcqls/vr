import vr as r

fn main() {
	// no output
	r.run("a <- rnorm(10)")
	// output echo
	r.eval("a <- rnorm(10)")
    // getting array (i.e. R Vector)
    a := r.f64("a")
	println(a)
	i := r.int("1:10")
    println(i)
	s := r.string("paste0('a',1:10)")
    println(s)
    // set a as double R vector from f64 array and check inside R
    r.set_f64("a",[1., 3, 2])
	println(r.f64("a"))
	// set b as integer R vector from int array and check inside R
    r.set_int("b",[1,3,2])
	println(r.int("b"))
    // set c as logical R vector from bool array and check inside R
	r.set_bool("c",[true,true,false])
	println(r.bool("c"))
    // set d as character R vector from string array and check inside R
	r.set_string("d",["tutu","ititit"])
	println(r.string("d"))
	// r.eval("capabilities()")
	// r.eval("png('toto.png')")
	// r.eval("plot(1:10)")
	// r.eval("dev.off()")
}