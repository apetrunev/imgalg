#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "sobel.h"
#include "common.h"

/* 3x3 sobel gradient operator
 */
int sobel_gradient(struct img_ctx *ctx, struct img_gradient *gctx)
{
	int w, h, x, y, g_x, g_y;
	float k;

	assert(ctx != NULL);
	assert(gctx != NULL);

	/* do the same thing as in the gassian blur 
	 */

	w = ctx->w;
	h = ctx->h;

	/* do convolution
	 */ 
	for (y = 1; y < h; y ++) {
		for (x = 1; x < w; x++) {
			g_x = (2*ctx->pix[y*w + x + 1]
			       + ctx->pix[(y - 1)*w + x + 1]
			       + ctx->pix[(y + 1)*w + x + 1]
			       - 2*ctx->pix[y*w + x - 1]
			       - ctx->pix[(y - 1)*w + x - 1]
			       - ctx->pix[(y + 1)*w + x - 1]);

			g_y = (2*ctx->pix[(y - 1)*w + x]
			       + ctx->pix[(y - 1)*w + x + 1]
			       + ctx->pix[(y - 1)*w + x - 1]
			       - 2*ctx->pix[(y + 1)*w + x]
			       - ctx->pix[(y + 1)*w + x + 1]
			       - ctx->pix[(y + 1)*w + x - 1]);

			/* calculate magnitude of gradient */
			gctx->gmag[y*w + x] = sqrt(g_x*g_x + g_y*g_y);
			/* determine gradient direction based on angle value */
			if (g_x == 0) {
				/* special case */
				if (g_y == 0) 
					gctx->gdir[y*w + x] = DIR_HORIZONTAL;
				else 
					gctx->gdir[y*w + x] = DIR_VERTICAL;				
			} else {
				k = g_y/(float)g_x;
				
				if (k < 0) {
					/* angle converge to 90 degrees */
					if (k < TAN_112) {
						gctx->gdir[y*w + x] = DIR_VERTICAL;
					} else {
						/* angle is between 112.5 and 157.5 */
						if (k < TAN_157) {
							gctx->gdir[y*w + x] = DIR_NEGATIVE_DIAG;
						/* angle converge to 0 degrees */
						}  else {
							gctx->gdir[y*w + x] = DIR_HORIZONTAL; 
						}
					}
				} else {
					/* angle converge to 90 degrees */
					if (k > TAN_67) {
						gctx->gdir[y*w + x] = DIR_VERTICAL;
					} else {
						/*angle is between 22.5 and 67.6 degrees */ 
						if (k > TAN_22) {
							gctx->gdir[y*w + x] = DIR_POSITIVE_DIAG;
						} else {
							/* angle converge to 0 degrees */
							gctx->gdir[y*w + x] = DIR_HORIZONTAL;
						}
					}
				}
			}	
		}
	}

	return RET_OK;	
}
