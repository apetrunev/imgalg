#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <err.h>

void *xmalloc(size_t size)
{
	void *ptr;

	ptr = malloc(size);
	if (ptr == NULL)
		errx(EXIT_FAILURE, "out of memory allocation %lu bytes",
							(unsigned long)size);

	return ptr;
}

void *xmalloc0(size_t size)
{
	void *ptr;

	ptr = malloc(size);
	if (ptr == NULL)
		errx(EXIT_FAILURE, "out of memory allocation %lu bytes",
							(unsigned long)size);

	memset(ptr, 0, size);

	return ptr;
}

void *xrealloc(void *ptr, size_t size)
{
	void *p;

	p = realloc(ptr, size);
	if (p == NULL)
		errx(EXIT_FAILURE, "out of memory allocation %lu bytes",
							(unsigned long)size);

	return p;
}

char *xstrdup(const char *s)
{
	char *str;

	assert(s != NULL);

	str = strdup(s);
	if (str == NULL)
		errx(EXIT_FAILURE, "out of memory string duplication");

	return str;
}

void xfree(void *ptr)
{
	assert(ptr != NULL);

	free(ptr);
}

