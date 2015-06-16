#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "img_utils.h"

int img_grayscale(struct img_ctx *src, struct img_ctx *dst)
{
	int i, j;
	unsigned char *p, *r, *g, *b;

	assert(src != NULL);
	assert(dst != NULL);
	assert(src->type == TYPE_RGB);

	dst->type = TYPE_GRAY;
	dst->w = src->w;
	dst->h = src->h;

	p = dst->pix;
	r = src->r;
	g = src->g;
	b = src->b;

	for (i = 0; i < src->h; i++) {
		for (j = 0; j < src->w; j++) {
			p[0] = (unsigned int)(0.229*r[0] + 0.587*g[0] + 0.114*b[0]);
			r++;
			g++;
			b++;
			p++;
		}
	}

	return RET_OK;
}

int img_otsu_threshold(struct img_ctx *gray)
{
	unsigned int hist[256];
	unsigned int w, h, i, j, sum_t, sum_b, n;
	double mean_b, mean_f, sigma_b, sigma_max;
	unsigned int weight_b, weight_f, t;
	unsigned char glevel;
	
	assert(gray != NULL);
	
	w = gray->w;
	h = gray->h;

	memset(hist, 0, sizeof(hist));

	/* calculte histogramm */
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			glevel = gray->pix[i*w + j];
			hist[glevel]++;
		}	
	}
	
	/* do not normalize weights to avoid excessive computations
	 */
	
	n = 0;
	sum_t = 0;

	for (i = 0; i < 256; i++) {
		n += hist[i];
		sum_t += i*hist[i];
	}

	weight_b = weight_f = 0;
	mean_b = mean_f = 0.0;
	sigma_b = sigma_max = 0.0;
	sum_b = 0;
	t = 0;

	for (i = 0; i < 256; i++) {
		weight_b += hist[i];
		
		/* skip empty gray levels */
		if (weight_b == 0) 
			continue;

		weight_f = n - weight_b;
		/* in case of one color */
		if (weight_f == 0) 
			break;
		
		sum_b += i*hist[i];

		mean_b = sum_b / weight_b;
		mean_f = (sum_t - sum_b) / weight_f;
	
		/* between class variance */
		sigma_b = weight_b*weight_f*(mean_b - mean_f)*(mean_b - mean_f);
		
		/* maximize between class variance */
		if ((unsigned int)sigma_b > (unsigned int)sigma_max) {
			sigma_max = sigma_b;
			t = i;
		}
	}

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			if (gray->pix[i*w + j] > t)
				gray->pix[i*w + j] = 0xFF;
			else
				gray->pix[i*w + j] = 0x00;
		}
	}

	return t;
}

/* use 5x5 gaussian box filter */
int img_gaussian_blur(struct img_ctx *src, struct img_ctx *dst)
{
	unsigned int w, h, x, y;
	
	assert(src != NULL);
	assert(dst != NULL);

	if ((src->w != dst->w) || (src->h != dst->h)) {
		fprintf(stderr, "error: images not the same size\n");
		return RET_ERR;
	}

	w = src->w;
	h = src->h;
	
	/* start with third line */
	for (y = 2; y < h; y++) {
		/* skip two pixel from the left */
		for (x = 2; x < w; x++) {
			/* apply convolution mask 
			 */
					   /* 1 */
			dst->pix[y*w + x] = (2*src->pix[(y - 2)*w + x - 2] +
					   4*src->pix[(y - 2)*w + x - 1] +
					   5*src->pix[(y - 2)*w + x] +
					   4*src->pix[(y - 2)*w + x + 1] +
					   2*src->pix[(y - 2)*w + x + 2] +
					   /* 2 */
					   4*src->pix[(y - 1)*w + x - 2] +
					   9*src->pix[(y - 1)*w + x - 1] +
					   12*src->pix[(y - 1)*w + x] +
					   9*src->pix[(y - 1)*w + x + 1] +
					   4*src->pix[(y - 1)*w + x + 2] +
					   /* 3 */
					   5*src->pix[y*w + x - 2] +
					   12*src->pix[y*w + x - 1] +
					   15*src->pix[y*w + x] +
					   12*src->pix[y*w + x + 1] +
					   5*src->pix[y*w + x + 2] +
					   /* 4 */
					   4*src->pix[(y + 1)*w + x - 2] +
					   9*src->pix[(y + 1)*w + x - 1] +
					   12*src->pix[(y + 1)*w + x ] +
					   9*src->pix[(y + 1)*w + x + 1] +
					   4*src->pix[(y + 1)*w + x + 2] + 
					   /* 5 */
					   2*src->pix[(y + 2)*w + x - 2] +
					   4*src->pix[(y + 2)*w + x - 1] +
					   5*src->pix[(y + 2)*w + x] +
					   4*src->pix[(y + 2)*w + x + 1] +
					   2*src->pix[(y + 2)*w + x + 2]) / 159;
			
		}

	}

	return RET_OK;	
}
