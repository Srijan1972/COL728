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
	}
	return b;
}

int modexp(int a, int n, int m)
{
  int r;
  r = 1;
  while (n > 0) {
    if ((n & 1) == 1) {
      n = n-1;
      r = (a*r) % m;
    } else {
      n = n/2;
      a = (a*a) % m;
    }
  }
  return r;
}

int printf(char*format, ...);

int main()
{
  printf("%d %d %d %d\n", factorial(1), factorial(2), factorial(4), fibonacci(factorial(1), factorial(2), factorial(4)));
  printf("%d\n", modexp(1729, 1729, 2020));
  return 0;
}
