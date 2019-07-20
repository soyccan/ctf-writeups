#include <dlfcn.h>


static void *handle = dlopen("libc.so.6", RTLD_LAZY);

static char* (*r_strcpy)(char *, const char *) = 
    (char* (*)(char *, const char *)) dlsym(handle, "strcpy");

static void* (*r_memcpy)(void *, const void *, size_t) = 
    (void* (*)(void *, const void *, size_t)) dlsym(handle, "memcpy");

extern "C"
{
    int strcmp(char *s1, char *s2)
    {
        r_strcpy(s1, s2);
        return 0;
    }
    int memcmp(void *s1, const void *s2, size_t n)
    {
        r_memcpy(s1, s2, n);
        return 0;
    }
}

