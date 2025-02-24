#include <stdio.h>

#include "collatz.h"

#define IS_EVEN(X) (!((X)&1))

int next_value_in_sequence(int number) {
    return IS_EVEN(number)? number/2 : 3*number+1;
}

int steps_for_number(int number) {
    int steps = 0;
    int working_value = number;
    while (working_value > 1) {
        working_value = next_value_in_sequence(working_value);
        steps++;
    }
    fprintf(stderr, __FILE__ ":%d steps_for_number(%d) = %d\n",
        __LINE__, number, steps);
    return steps;
}