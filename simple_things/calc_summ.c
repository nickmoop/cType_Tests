#include <stdio.h>

void NIterationsLoop(int n) {
    int summ = 0;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            summ += i + j;
        }
    }
}
