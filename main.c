int test(int a, int b) {
	return a + b;
}

int main() {
	int a = 0;

	a = 2 + 3;
	int b = a + 2 * a;
	a = test(a + 1, 2);
	return a;
}
