import vr as r

fn main() {
	println("hello")
	r.begin()
	r.eval("a <- rnorm(10)")
	// r.eval("capabilities()")
	// r.eval("png('toto.png')")
	// r.eval("plot(1:10)")
	// r.eval("dev.off()")
	println(r.f64("a"))
	println(r.int("1:10"))
	println(r.string("paste0('a',1:10)"))
}