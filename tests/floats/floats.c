#include <stdio.h>

float a = 3.0;
double b = 7.0;
float c = 2.0;


int main() {

	float test = a * c;
	test += (float) b;
	test /= c;
	double d_test = (double) a / c;

	a = test;
	b = d_test;

	printf("%f\n", test);
	printf("%lf\n", d_test);

	return 0;
}
