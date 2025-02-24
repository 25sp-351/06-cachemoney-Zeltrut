#include "cache.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple linked list cache implementation
typedef struct CacheEntry {
    int key;
    char *value;
    struct CacheEntry *next;
} CacheEntry;

static CacheEntry *cache_head = NULL;

// Store key-value pair in cache
void cache_set(int key, const char *value) {
    CacheEntry *new_entry = malloc(sizeof(CacheEntry));
    if (!new_entry) {
        return;
    }
    new_entry->key = key;
    new_entry->value = strdup(value);
    new_entry->next = cache_head;
    cache_head = new_entry;
}

// Retrieve cached value
char *cache_get(int key) {
    CacheEntry *current = cache_head;
    while (current) {
        if (current->key == key) {
            return current->value;
        }
        current = current->next;
    }
    return NULL;
}

// Global pointer to store the downstream (real) provider.
static ProviderFunction global_downstream = NULL;

// The cached provider function that uses the cache.
static ValueType cached_provider(KeyType key) {
    char *cached_val = cache_get(key);
    if (cached_val != NULL) {
        // Cache hit: convert the cached string value to an integer and return it.
        return atoi(cached_val);
    }
    // Cache miss: call the downstream provider.
    ValueType result = global_downstream(key);

    // Convert the result to a string and store it in the cache.
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "%d", result);
    cache_set(key, buffer);

    return result;
}

// Implementation of the required set_provider function.
// This function is exported from your dynamic library and wraps the downstream provider.
ProviderFunction set_provider(ProviderFunction downstream) {
    if (!downstream) {
        return NULL;
    }
    global_downstream = downstream;
    return cached_provider;
}


void _do_nothing(void) {}

CacheStat *_do_nothing_stats(void) {
    return NULL;
}

Cache *load_cache_module(const char *libname) {
    void *handle = dlopen(libname, RTLD_NOW | RTLD_NODELETE);
    if (!handle) {
        fprintf(stderr, "Error: %s\n", dlerror());
        return NULL;
    }

    Cache *hooks = malloc(sizeof(Cache));

    Void_fptr cache_initialize = (Void_fptr)dlsym(handle, "initialize");
    hooks->set_provider_func = (SetProvider_fptr)dlsym(handle, "set_provider");
    hooks->get_statistics = (Stats_fptr)dlsym(handle, "statistics");
    hooks->reset_statistics = (Void_fptr)dlsym(handle, "reset_statistics");
    hooks->cache_cleanup = (Void_fptr)dlsym(handle, "cleanup");

    dlclose(handle);

    if (!hooks->get_statistics)
        hooks->get_statistics = _do_nothing_stats;
    if (!hooks->reset_statistics)
        hooks->reset_statistics = _do_nothing;
    if (!hooks->cache_cleanup)
        hooks->cache_cleanup = _do_nothing;

    if (!hooks->set_provider_func) {
        fprintf(stderr, "Error: could not resolve required symbol: (%p)\n",
                (void *)hooks->set_provider_func);
        free(hooks);
        hooks = NULL;
    }

    if (cache_initialize)
        cache_initialize();

    return hooks;
}

void print_cache_stats(int fd, CacheStat *stats) {
    if (!stats) {
        dprintf(fd, "No cache stats available\n");
        return;
    }

    printf("Cache Stats:\n");

    CacheStat *sptr = stats;
    while (sptr->type != END_OF_STATS) {
        dprintf(fd, "%-10s (%d) %4d\n", CacheStatNames[sptr->type], sptr->type,
                sptr->value);
        sptr++;
    }
}
