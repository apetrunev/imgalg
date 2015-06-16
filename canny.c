#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>

#include "canny.h"
#include "sobel.h"
#include "common.h"

struct edge edge_ctx;

static int non_max_suppression(struct img_ctx *ctx, struct img_gradient *g)
{
	int x, y, w, h;
	
	assert(ctx != NULL);
	assert(g != NULL);
	
	w = ctx->w;
	h = ctx->h;
	
	/* we didn't compute gradient for border pixels 
	 * lets ignore them 
	 */
	for (y = 1 ; y < h; y++) {
		for (x = 1; x < w; x++) {
			switch (g->gdir[y*w + x]) {
			case DIR_VERTICAL:
				/* compare with pixels above and bellow */
				if (g->gmag[y*w + x] > g->gmag[(y - 1)*w + x] &&
				    g->gmag[y*w + x] > g->gmag[(y + 1)*w + x]) {
					/* compare with threshold */
					ctx->pix[y*w + x] = g->gmag[y*w + x];
				} else {
					ctx->pix[y*w + x] = 0x00;
				}
				break;
			case DIR_HORIZONTAL:
				/* compare with left and right pixels */
				if (g->gmag[y*w + x] > g->gmag[(y)*w + x - 1] &&
				    g->gmag[y*w + x] > g->gmag[(y)*w + x + 1]) {
					ctx->pix[y*w + x] = g->gmag[y*w + x];
				} else {
					ctx->pix[y*w + x] = 0x00;
				}
				break;
			case DIR_POSITIVE_DIAG: /* 45 degrees */
				if (g->gmag[y*w + x] > g->gmag[(y + 1)*w + x + 1] &&
				    g->gmag[y*w + x] > g->gmag[(y - 1)*w + x - 1]) {
					ctx->pix[y*w + x] = g->gmag[y*w + x];
				} else {
					ctx->pix[y*w + x] = 0x00;
				}
				break;
			case DIR_NEGATIVE_DIAG: /* 135 degrees */
				if (g->gmag[y*w + x] > g->gmag[(y - 1)*w + x + 1] &&
				    g->gmag[y*w + x] > g->gmag[(y + 1)*w + x - 1]) {
					ctx->pix[y*w + x] = g->gmag[y*w + x];
				} else {
					ctx->pix[y*w + x] = 0x00;
				}
				break;
			default: 
				break;
			}
		}
	}

	return RET_OK;
}

static int double_thresholding(struct img_ctx *ctx, unsigned int lth, unsigned int hth)
{
	int y, x, w, h;

	assert(ctx != NULL);
	
	w = ctx->w;
	h = ctx->h;

	for (y = 1; y < h ; y++) {
		for (x = 1; x < w; x++) {
			/* strong edge */
			if (ctx->pix[y*w + x] >= hth) {
				ctx->pix[y*w + x] = 0xFF;
			} else {
				if (ctx->pix[y*w + x] <= lth)
					ctx->pix[y*w + x] = 0x00;	
				else /* weak edge */
					ctx->pix[y*w + x] = lth;
			}
		}
	}

	return RET_OK;
}

int out_of_range(struct img_ctx *ctx, int x, int y)
{
	int w, h;

	assert(ctx != NULL);

	w = ctx->w;
	h = ctx->h;
	
	/* beyond image borders or at border pixels */
	if ((x <= 0) || (x >= w))
		return TRUE;

	if ((y <= 0) || (y >= h))
		return TRUE;

	return FALSE;
}

/* store inforation on edge into global buffer */
static void edge_append(int idx)
{
	void *p;

	if (edge_ctx.edge_used >= edge_ctx.edge_size) {
		
		edge_ctx.edge_size += BUFSIZE;
		p = realloc(edge_ctx.edge_pixels, /* sizeof(*edge_ctx.edge_pixels)* */edge_ctx.edge_size);	
	
		if (p == NULL) {
			fprintf(stderr, "edge_append: cannot allocate memory\n");
			exit(1);
		}

		edge_ctx.edge_pixels = p;
	}
	/* add index to buffer */
	edge_ctx.edge_pixels[edge_ctx.edge_last++] = idx;
	edge_ctx.edge_used += sizeof(*edge_ctx.edge_pixels);
}

static int trace(int x, int y, int lth, struct img_ctx *in, struct img_ctx *out)
{
	int i, w;
	/* offset define neighbour pixels */
	int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
	int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};

	assert(in != NULL);
	assert(out != NULL);
	
	w = in->w;
	
	/* x and y is coordinates of border pixel in input image
	 * mark it as such on output if not yet done
	 */

	if (out->pix[y*w + x] == 0x00) {
		out->pix[y*w + x] = 0xFF;
		/* store the edge pixel index into buffer */
		edge_append(y*w + x);
		/* consider eight directions */
		for (i = 0; i < 8; i++) {
			if (!out_of_range(in, x + dx[i], y + dy[i]) && in->pix[(y + dy[i])*w + x + dx[i]] >= lth) {
				/* trace pixels until we run out of range
				 * or step back into already marked pixel
				 */
				return trace(x + dx[i], y + dy[i], lth, in, out);
			}
		}
		
		return RET_OK;
	}
	/* neighbours have marked or there is no pixels connected to strong edge*/	
	return RET_OK;
}

static int hysteresis (unsigned int lth, unsigned int hth, struct img_ctx *in, struct img_ctx *out)
{
	int x, y, w, h;

	assert(in != NULL);
	assert(out != NULL);

	w = in->w;
	h = in->h;

	/* skip border pixels
	 */
	for (y = 1; y < h - 1; y++) {
		for (x = 1; x < w - 1; x++) {
			if (in->pix[y*w + x] >= hth) 
				trace(x, y, lth, in, out);
		}
	}

	return RET_OK;
}

int canny(int t_low, int t_high, struct img_ctx *img, struct img_gradient *grad, struct img_ctx *edges)
{
	assert(img != NULL);
	assert(img->type == TYPE_GRAY);
	assert(grad != NULL);
	assert(edges != NULL);

	/* clear data from global buffer */
	memset(edge_ctx.edge_pixels, 0, edge_ctx.edge_size);
	edge_ctx.edge_size = 0;

	non_max_suppression(img, grad);
	double_thresholding(img, t_low, t_high);
	hysteresis(t_low, t_high, img, edges);

	return RET_OK;
}
