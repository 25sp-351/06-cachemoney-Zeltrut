#ifndef CACHE_H
#define CACHE_H

#include <stdbool.h>


// Cache key and value types.
// Changeable, but examine how they're used
// before assuming it's all good.
// FMTs are for printf (%p for pointer, %s for string,
// %u for unsigned int, etc.)
typedef int KeyType;
#define KEY_FMT "%d"

typedef int ValueType;
#define VALUE_FMT "%d"



// This is the type of the function you want to cache
// (if this is not what you want to cache, you're going to
// need more significant changes.)
typedef ValueType (*ProviderFunction)(KeyType key);

void cache_set(int key, const char *value);
char *cache_get(int key);

/* Types of cache statistics. Values are %d. */
// These are basic/standard. Add what you want.
typedef struct stat {
    enum Stat_type {
        END_OF_STATS=0,
        Cache_requests=1,
        Cache_hits=2,
        Cache_misses=3,
        Cache_evictions=4,
        Cache_size=5
    } type;
    int value;
} CacheStat;

static char *CacheStatNames[] = {
    "",
    "requests",
    "hits",
    "misses",
    "evictions",
    "size"
};



// initialize() and cleanup() function types
typedef void (*Void_fptr)(void);

// (type of a function that) takes a provider func, returns a CACHED provider func
typedef ProviderFunction (*SetProvider_fptr)(ProviderFunction);

// (type of a function that) returns NULL or a pointer to a list of CacheStat(s)
// terminated by a type=END_OF_STATS stat. Caller must free the returned pointer
typedef CacheStat* (*Stats_fptr)(void);

/*
** This is the interface that a program uses to access/use a cache.
** It is returned by load_cache_module() and then main() calls these functions.
**
** Once the provider has been set, main() uses the function returned.
**
** main() should probably call cache_cleanup() before exiting.
*/
typedef struct chooks {
    // function in library to set the provider:
    // (takes the "real" provider, returns a caching provider
    // that hits the real one as needed)
    // (main() must call this at least once, before any other calls)
    SetProvider_fptr set_provider_func;

    // function in library to return cache statistics:
    // (can be called by main() any time before cleanup().
    // Returns NULL or an allocated pointer that the caller must free
    Stats_fptr get_statistics;

    // function in library to reset the statistics of the cache:
    // (can be called by main() any time before cleanup())
    Void_fptr reset_statistics;

    // function in library to close/delete cache: main() should call once
    // before exiting.
    Void_fptr cache_cleanup;
} Cache;



// Generic cache functions. works with all cache implementations that
// honor the interface below.
// Loads the shared lib. Returns NULL on failure.
// Caller must free the returned pointer at end of program
// **This function calls initialize() in the library. Do NOT
// call it from main()**

// Hooks in Cache struct are ALL filled in, regardless of
// whether the library implements them or not.


Cache *load_cache_module(const char *libname);

// Utility: takes what statistics() returns, prints it.
void print_cache_stats(int fd, CacheStat *stats);




/* HOW TO WRITE A LOADABLE CACHE MODULE */
#if CACHE_MODULE_REQUIREMENTS

// When you write a loadable cache module
// **you must implement at least the set_provider() function.**
// The others are optional. **If you don't implement an optional
// function, an empty one is provided so that main() can still
// make the calls without checking.**


// The functions in your module must have these names and signatures.


// load_cache_module() calls this once after loading
// the library, before any other calls, if present.
void initialize(void);


// main() must call this once before any other "real" calls
// It passes in the function that will be cached.
// You must return a function that calls the original function
// and caches the result.
ProviderFunction set_provider(ProviderFunction downstream);


// may be called by main() any number of times before cleanup().
// Returns NULL or an allocated pointer that main() must free.
CacheStat* statistics(void);


// main() can call this any time before cleanup()
void reset_statistics(void);


// main() should call this once once, at end of program
void cleanup(void);

#endif


#endif