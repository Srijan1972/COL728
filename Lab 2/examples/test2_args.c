int puts(char* a);
int atoi(char* a);
int printf(char* s, ...);

int factorial(int n)
{
	if (n <= 0)
		return 1;
	else return n*factorial(n-1);
}

int fibonacci(int a, int b, int n)
{
	if (n <= 0)
		return a;

	while (n > 0) {
		int t;
		t = b;
		b = a+b;
		a = t;
		n = n-1;

		if (a < 0 || b < 0) {
			puts("wrap-around");
			goto end;
		}
	}

end:
	return b;
}

int main(int argc, char** argv)
{
	if (!(argc >= 2)) {
		puts("Not enough args!");
		return 1;
	}

	while (argc >= 2) {
		int x;
		x = atoi(*(argv+argc-1));
		printf("n: %d\tFactorial: %d\tFibonacci: %d\n", x, factorial(x), fibonacci(0, 1, x));

		argc = argc-1;
	}
	return 0;
}
