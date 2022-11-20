int puts(char* a);

void func()
{
}

void const_folding_simplify_check()
{
	//int simple, tricky_1, tricky_2, tricky_3;
	int simple;
	int tricky_1;
	int tricky_2;
	int tricky_3;
	//double d1, d2;
	simple = 12+32/12;
	simple = simple*(1/2);
	tricky_1 = 0*(simple-0) - (0 | (simple | ~~0));
	tricky_2 = 1 & (simple & 3) ^ (tricky_1 & 10);
	tricky_3 = ((simple << ((simple ^ 0) & 0)) / 1) % 1;
	//d1 = 1.0001*12.54;
	//d2 = 3.0*d1 / (0.0*d1+(d1+1.0000)) + (0.0 * (d1*1.0));
}

void remove_deadcode()
{
	int simple;
	simple = 10;

	if (0) { }
	else { puts("else"); }

	while (0) {
		puts("i will be gone");
	}

	if (0+0/2+9/10+1) {
		if (0*(simple/4) || (simple+1)*0) {
			puts("Dead code speaking");
		}
		else if (0^0-3)
			if (1) {
				puts("It should be called tree-shaking");
			}
	}

	while (0+23/45 && simple) {
		puts("like never existed");
		func();
	}
}

int main(int argc, char** argv)
{
	const_folding_simplify_check();
	remove_deadcode();

	return 0;
}
