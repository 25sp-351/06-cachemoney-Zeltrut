/* 
    Assignment 6: Cache Money
    CPSC 351 Spring 2025
    Justin Lam
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "collatz.h"
#include "cache.h"

#define MAX_MEMOIZED_VALUE 150000  // Memoize values under $1500
#define STRING_BUFFER_SIZE 128
#define CENTS_IN_DOLLAR 100

// Cache instance
static Cache *cache;

// Arrays for converting numbers to words
static const char *SINGLE_DIGITS[] = {
    "", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"
};
static const char *TEEN_NUMBERS[] = {
    "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen",
    "sixteen", "seventeen", "eighteen", "nineteen"
};
static const char *DOUBLE_DIGITS[] = {
    "", "", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"
};
static const char *HUNDREDS_PLACE[] = {
    "", "one hundred", "two hundred", "three hundred", "four hundred",
    "five hundred", "six hundred", "seven hundred", "eight hundred", "nine hundred"
};

// Converts a number (less than 1000) into words
static void convert_number_to_words(int number, char *output_buffer) {
    if (number >= 1000) {
        sprintf(output_buffer + strlen(output_buffer), "%s thousand",
                SINGLE_DIGITS[number / 1000]);
        number %= 1000;
        if (number > 0) {
            strcat(output_buffer, " ");
        }
    }

    if (number >= 100) {
        sprintf(output_buffer + strlen(output_buffer), "%s",
                HUNDREDS_PLACE[number / 100]);
        number %= 100;
        if (number > 0) {
            strcat(output_buffer, " ");
        }
    }

    if (number >= 10 && number < 20) {
        sprintf(output_buffer + strlen(output_buffer), "%s",
                TEEN_NUMBERS[number - 10]);
        return;
    }

    if (number >= 20) {
        sprintf(output_buffer + strlen(output_buffer), "%s",
                DOUBLE_DIGITS[number / 10]);
        if (number % 10 != 0) {
            strcat(output_buffer, "-");
            sprintf(output_buffer + strlen(output_buffer), "%s",
                    SINGLE_DIGITS[number % 10]);
        }
        return;
    }

    if (number > 0) {
        sprintf(output_buffer + strlen(output_buffer), "%s",
                SINGLE_DIGITS[number]);
    }
}

// Converts money (in cents) into words
static char *convert_money_to_text(int total_cents) {
    if (total_cents < 0) {
        return "negative values not supported";
    }

    char *cached_value = (char *)cache->get_statistics();

    if (cached_value) {
        return cached_value;
    }

    static char result_string[STRING_BUFFER_SIZE];
    result_string[0] = '\0';

    int whole_dollars = total_cents / CENTS_IN_DOLLAR;
    int remaining_cents = total_cents % CENTS_IN_DOLLAR;

    if (whole_dollars > 0) {
        convert_number_to_words(whole_dollars, result_string);
        strcat(result_string, whole_dollars == 1 ? " dollar" : " dollars");
    }

    if (remaining_cents > 0) {
        if (whole_dollars > 0) {
            strcat(result_string, " and ");
        }
        convert_number_to_words(remaining_cents, result_string);
        strcat(result_string, remaining_cents == 1 ? " cent" : " cents");
    }

    cache->reset_statistics();
    return result_string;
}

int main(void) {
    cache = load_cache_module("./cache.dylib");
    if (!cache) {
        fprintf(stderr, "Failed to load cache module\n");
        return EXIT_FAILURE;
    }

    int input_cents;
    
    while (scanf("%d", &input_cents) != EOF) {
        printf("%s\n", convert_money_to_text(input_cents));
    }

    CacheStat *stats = cache->get_statistics();
    if (stats) {
        print_cache_stats(1, stats);
        free(stats);
    }

    cache->cache_cleanup();
    free(cache);

    return 0;
}
