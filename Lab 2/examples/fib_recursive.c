int printf(char* str,...);

int fib(int n) {

	if (n == 0 || n == 1) {
		return 1;
	}
	else {
		return (fib(n-1) + fib(n-2));
	}

	return 0;
}

int main(int argc,char** argv){
	printf("%d\n",fib(3));
	return 0;
}