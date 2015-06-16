#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "img.h"

struct img_ctx *img_ctx_new(int w, int h, img_type_t type, color_type_t color)
{
	struct img_ctx *c;
	int len;

	c = malloc(sizeof(*c));
	
	c->type = type;
	c->w = w;
	c->h = h;

	len = w*h;
		
	switch (type) {
	case TYPE_GRAY:
		c->pix = malloc(len*sizeof(*(c->pix)));
		/* initialize pixels to spcified color */
		if (color != C_NONE)
			memset(c->pix, color, len*sizeof(*(c->pix)));
		break;
	case TYPE_RGB:
		c->r = malloc(len*sizeof(*(c->r)));
		c->g = malloc(len*sizeof(*(c->g)));
		c->b = malloc(len*sizeof(*(c->b)));
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
			free(ctx->pix);
		break;
	case TYPE_RGB:
		if (ctx->r != NULL)
			free(ctx->r);
		if (ctx->g != NULL)
			free(ctx->g);
		if (ctx->b != NULL)
			free(ctx->b);
		break;
	default:
		fprintf(stderr, "error: uknown type\n");
		assert(0);
	}

	free(ctx);
}

/* gradient constructor
 */
struct img_gradient *img_gradient_new(struct img_ctx *ctx)
{
	struct img_gradient *g;
	size_t size;

	assert(ctx != NULL);

	g = malloc(sizeof(*g));	
			
	g->w = ctx->w;
	g->h = ctx->h;		
	size = ctx->w*ctx->h;

	/* magnitude and direction of gradient at every point */
	g->gmag = malloc(size*sizeof(*(g->gmag)));
	g->gdir = malloc(size*sizeof(*(g->gdir)));
	
	return g;
}

void img_gradient_destroy(struct img_gradient *g)
{
	assert(g != NULL);

	if (g->gmag != NULL)
		free(g->gmag);
	if (g->gdir != NULL)
		free(g->gdir);

	free(g);
}
