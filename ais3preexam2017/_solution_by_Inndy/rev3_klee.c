#include <assert.h>
#include <stdint.h>
#include <klee/klee.h>

const int known[5][5] = {
	{ 0x11, 0x00, 0x00, 0x0A, 0x00 },
	{ 0x00, 0x00, 0x06, 0x00, 0x12 },
	{ 0x01, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x15, 0x02 },
	{ 0x00, 0x10, 0x00, 0x00, 0x00 }
};

int data[5][5];

int main()
{
	klee_make_symbolic(data, sizeof(data), "data");
	int I = 0, flatten[25];

	for(int i = 0; i < 5; i++) {
		for(int j = 0; j < 5; j++) {
			if(known[i][j]) data[i][j] = known[i][j];
			flatten[I++] = known[i][j];
			klee_assume(data[i][j] >= 1);
			klee_assume(data[i][j] <= 25);
		}
	}

	for(int i = 0; i < 25; i++) {
		for(int j = 0; j < 25; j++) {
			if(i != j) {
				klee_assume(flatten[i] != flatten[j]);
			}
		}
	}

	int vector[10] = { 0 };

	for(int i = 0; i < 5; i++) {
		for(int j = 0; j < 5; j++) {
			vector[i] += data[i][j];
			vector[j + 5] += data[i][j];
		}
	}

	for(int i = 1; i < 10; i++) {
		klee_assume(vector[i] == vector[i - 1]);
	}

	klee_assume(
		data[1][1] -
		data[3][1] +
		data[4][4] -
		data[1][3] +
		data[3][3] -
		data[4][0] +
		data[0][0] -
		data[0][4] == 0);

	klee_assert(0);
}
