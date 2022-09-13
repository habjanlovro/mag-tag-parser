#include <stdio.h>

float a = 1.0;
double b = 1.0;
float c = 2.0;


int main() {

	float test = a * c;
	test += (float) b;
	test /= c;
	double d_test = (double) a / c;

	printf("%f\n", test);
	printf("%lf\n", d_test);

	return 0;
}
