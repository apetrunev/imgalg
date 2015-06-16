#ifndef X_MALLOC_H_
#define X_MALLOC_H

 /* If memory was not allocated, terminate a process. */
void *xmalloc(size_t size);
void *xmalloc0(size_t size);
void *xrealloc(void *ptr, size_t size);
void xfree(void *ptr);
char *xstrdup(const char *str);

#endif /* X_MALLOC_H_ */
