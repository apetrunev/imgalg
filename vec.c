#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "xmalloc.h"
#include "vec.h"

struct vec3 *vec3_new(int x, int y, int z)
{
	struct vec3 *p;

	p = xmalloc(sizeof(*p));
	p->x = x;
	p->y = y;
	p->z = z;

	return p;
}

void vec3_destroy(struct vec3 *p)
{
	assert(p != NULL);
	xfree(p);
}
