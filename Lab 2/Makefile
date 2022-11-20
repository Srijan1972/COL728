cc: cc.cpp c.tab.cpp c.lex.cpp semantics.cpp codegen.cpp optimize.cpp
	clear
	g++ -I/usr/lib/llvm-6.0/include -g c.tab.cpp c.lex.cpp semantics.cpp codegen.cpp optimize.cpp cc.cpp -lm -ll -lfl `llvm-config-6.0 --ldflags --system-libs --libs all` -o $@

c.tab.cpp c.tab.hpp: c.y
	bison -o c.tab.cpp -d c.y

c.lex.cpp: c.l c.tab.hpp
	flex -o c.lex.cpp -l c.l

clean::
	rm -f c.tab.cpp c.tab.hpp c.lex.cpp cc c.output
	clear
