#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "xmalloc.h"
#include "img.h"

struct img_ctx *img_ctx_new(int w, int h, img_type_t type, color_type_t color)
{
	struct img_ctx *c;
	int len;

	c = xmalloc(sizeof(*c));
	
	c->type = type;
	c->w = w;
	c->h = h;

	len = w*h;
		
	switch (type) {
	case TYPE_GRAY:
		c->pix = xmalloc0(len*sizeof(*(c->pix)));
		/* initialize pixels to spcified color */
		if (color != C_NONE)
			memset(c->pix, color, len*sizeof(*(c->pix)));
		break;
	case TYPE_RGB:
		c->r = xmalloc(len*sizeof(*(c->r)));
		c->g = xmalloc(len*sizeof(*(c->g)));
		c->b = xmalloc(len*sizeof(*(c->b)));
		break;
	default:
		abort();	 
	}	

	return c;
}

void img_destroy_ctx(struct img_ctx *ctx)
{
	assert(ctx != NULL);

	switch (ctx->type) {
	case TYPE_GRAY:
		if (ctx->pix != NULL)
			xfree(ctx->pix);
		break;
	case TYPE_RGB:
		if (ctx->r != NULL)
			xfree(ctx->r);
		if (ctx->g != NULL)
			xfree(ctx->g);
		if (ctx->b != NULL)
			xfree(ctx->b);
		break;
	default:
		fprintf(stderr, "error: uknown type\n");
		assert(0);
	}

	xfree(ctx);
}

/* gradient constructor
 */
struct img_gradient *img_gradient_new(struct img_ctx *ctx)
{
	struct img_gradient *g;
	size_t size;

	assert(ctx != NULL);

	g = xmalloc(sizeof(*g));	
			
	g->w = ctx->w;
	g->h = ctx->h;		
	size = ctx->w*ctx->h;

	/* magnitude and direction of gradient at every point */
	g->gmag = xmalloc(size*sizeof(*(g->gmag)));
	g->gdir = xmalloc(size*sizeof(*(g->gdir)));
	
	return g;
}

void img_gradient_destroy(struct img_gradient *g)
{
	assert(g != NULL);

	if (g->gmag != NULL)
		xfree(g->gmag);
	if (g->gdir != NULL)
		xfree(g->gdir);

	xfree(g);
}
