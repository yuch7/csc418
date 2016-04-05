struct Foo(){
	Foo(int a_=0): a(a_) {} //if Foo not set, defualt a to 0
	int f() {return 1;}
	int a;
	static int g(){return 3;}
}

class Bar() {
	public:
		Bar(): myfoo(new Foo(4)) {}
		-Bar(){delete myfoo;}
		Foo * myfoo;
		int g(){return -1;}
}
Foo foo;
Bar * bar_ptr = new Bar();
bar_ptr->myfoo.a = foo.f();
foo.a = bar_ptr->g();
int a = Foo::g();
