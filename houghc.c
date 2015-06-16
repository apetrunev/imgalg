#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>

#include "xmalloc.h"
#include "houghc.h"
#include "canny.h"
#include "sobel.h"
#include "common.h"
#include "vec.h"

struct vec_array {
	struct vec3 *arr;
	size_t size;
	size_t used;
	unsigned int last;
};

static unsigned int *edge_pixels;
static unsigned int edge_last;

static int *get_radii_range(int minr, int maxr);
static int *get_cells(struct img_ctx *im, char axis, int step);
static int get_cell_idx(int cell0, int val, int step, int ncell);
static void houghspace_free(int **hspace, int len);
static void houghspace_increment(int **hspace, int *hlens, int hsize, int ridx, int idx);

static int *get_radii_range(int minr, int maxr)
{
	int i, len;
	int *r;

	len = maxr - minr + 1;
	r = xmalloc(sizeof(*r)*len);
	
	for (i = 0; i < len; i++) {
		r[i] = minr + i;
	}
	
	return r;
}

static int *get_cells(struct img_ctx *im, char axis, int step)
{
	int *cells, i, len;
	
	assert(im != NULL);

	if (axis == 'x') {
		len = im->w / step;
		cells = xmalloc(sizeof(*cells)*len);
		for (i = 0; i < len; i++) {
			cells[i] = i*step;
		}
	} else if (axis == 'y') {
		len = im->h / step;
		cells = xmalloc(sizeof(*cells)*len);
		for (i = 0; i < len; i++) {
			cells[i] = i*step;
		}
	}

	return cells;
}

/* find the cell the value belong to */
static int get_cell_idx(int cell0, int val, int step, int ncell)
{
	int idx;

	idx = (int)((val - cell0) / step);
	if (idx < 0) 
		return 0;
	else if (idx > ncell - 1)
		return ncell - 1;
	
	return idx;
}


static void houghspace_free(int **hspace, int len)
{
	int i;

	assert(hspace != NULL);

	for (i = 0; i < len; i++) {
		if (hspace[i] != NULL)
			xfree(hspace[i]);
	}

	xfree(hspace);
}

/* increment weight of specified element in hough space */
static void houghspace_increment(int **hspace, int *hlens, int hsize, int ridx, int idx)
{
	int *p, size, nsize, n, nn;

	assert(hspace != NULL);
	assert(hlens != NULL);
	
	if (ridx >= hsize) {
		fprintf(stderr, "houghspace_increment: index larger than array size\n");
		assert(ridx <= hsize);
	}
	
	/* size of array of indexes for specified radii in bytes */
	size = hlens[ridx];	
	/* number of element in the array */
	n = size / sizeof(*hspace[ridx]);	
	/* allocate more memory */	
	if (idx >= n) {
		/* take more elements */
		nn = 2*(idx + 1);
		/* new size */
		nsize = nn*sizeof(*hspace[ridx]);
		p = xrealloc(hspace[ridx], nsize);
		/* set part of space behind old size to zero */
		memset(p + n, 0, nsize - size);
		hlens[ridx] = nsize; 
		hspace[ridx] = p;
	}
	/* increment weight in a hough space */
	hspace[ridx][idx]++;
}

#define COS22 0.92388
#define SIN22 0.38268
#define COS67 0.38268
#define SIN67 0.92388
#define COS112 (-0.38268)
#define COS157 (-0.92388)

static void vector_append(struct vec_array *array, int x, int y, int r)
{
	struct vec3 *p;

	assert(array != NULL);
	
	if (array->used >= array->size) {
		array->size += BUFSIZE;
		p = xrealloc(array->arr, array->size);
		array->arr = p;
	}
	/* move pointet to the next available element */
	p = array->arr + array->last;	
	/* add values */	
	p->x = x;
	p->y = y;
	p->z = r;

	array->last++;
	array->used += sizeof(*(array->arr));
}


static int _houghcircles(struct img_ctx *im, struct vec3 **circles, int rmin, int rmax, int step, int min_dist, struct img_gradient *grad)
{
	int i, idx, cidx, ridx, rlen, r, rr, mweight, size, n;	
	int x, xp, xc, xx, *xcells, wcell, xstart_idx, xfinish_idx, xidx;
	int y, yp, yc, yy, *ycells, hcell, ystart_idx, yfinish_idx, yidx;
	int *radiuses;
	/* for every radii define a set of points */
	int **hspace, *hlens;
	struct vec_array centers;

	rlen = rmax - rmin + 1;
	radiuses = get_radii_range(rmin, rmax);
	
	/* array of pairs (r,idx) */
	hspace = xmalloc(sizeof(*hspace)*rlen);
	memset(hspace, 0, sizeof(*hspace)*rlen);
	/* lengths of arrays of indexes for every radii */
	hlens = xmalloc(sizeof(*hlens)*rlen);
	memset(hlens, 0, sizeof(*hlens)*rlen);
	/* init placeholder for local maximas */
	memset(&centers, 0, sizeof(centers));
	
	xcells = get_cells(im, 'x', step);
	ycells = get_cells(im, 'y', step);
	/* width expressed in cell units */
	wcell = im->w / step;
	hcell = im->h / step;
	
	/* run through edge pixels */
	for (i = 0; i < edge_last; i++) {
		idx = edge_pixels[i];
		yp = (int)(idx / im->w);
		xp = (int)(idx % im->w); 
			
		switch(grad->gdir[idx]) {
		case DIR_HORIZONTAL:	/* 0 -- 22.5 */
			for (ridx = 0; ridx < rlen; ridx++) {
				r = radiuses[ridx];
				rr = r*r;
			
				ystart_idx = get_cell_idx(ycells[0], (int)(yp - r*SIN22), step, wcell);
				yfinish_idx = get_cell_idx(ycells[0], (int)(yp - r*SIN22), step, wcell);
				
				for (yidx = ystart_idx; yidx <= yfinish_idx; yidx++) {
					yc = ycells[yidx];
					yy = (yp - yc)*(yp - yc);
					
					x = (int)sqrt(rr - yy);
					
					if (xp - x > 0) {
						xidx = get_cell_idx(xcells[0], xp - x, step, wcell);
						cidx = wcell*yidx + xidx;
						houghspace_increment(hspace, hlens, rlen, ridx, cidx);	
					}

					if (xp + x < im->w) {
						xidx = get_cell_idx(xcells[0], xp + x, step, wcell); 
						cidx = wcell*yidx + xidx;
						houghspace_increment(hspace, hlens, rlen, ridx, cidx);
					}
				} 
			}	
			break;
		case DIR_POSITIVE_DIAG: /* 22.5 -- 67.5 */
			for (ridx = 0; ridx < rlen; ridx++) {
				r = radiuses[ridx];
				rr = r*r;
				
				xstart_idx = get_cell_idx(xcells[0], (int)(xp + r*COS67), step, wcell);
				xfinish_idx = get_cell_idx(xcells[0], (int)(xp + r*COS22), step, wcell);

				for (xidx = xstart_idx; xidx <= xfinish_idx; xidx++) {
					xc = xcells[xidx];
					xx = (xp - xc)*(xp - xc);
			
					y = sqrt(rr - xx);
					if (yp - y > 0) {
						yidx = get_cell_idx(ycells[0], yp - y, step, wcell);
						cidx = wcell*yidx + xidx;
						houghspace_increment(hspace, hlens, rlen, ridx, cidx);	
					}
				}			
				
				xstart_idx = get_cell_idx(xcells[0], (int)(xp - r*COS22), step, wcell);
				xfinish_idx = get_cell_idx(xcells[0], (int)(xp - r*COS67), step, wcell);

				for (xidx = xstart_idx; xidx <= xfinish_idx; xidx++) {
					xc = xcells[xidx];
					xx = (xp - xc)*(xp - xc);
			
					y = sqrt(rr - xx);
					/* if no beyond of the image bounds */	
					if (yp + y < im->h) {
						yidx = get_cell_idx(ycells[0], yp + y, step, wcell);
						cidx = wcell*yidx + xidx;
						houghspace_increment(hspace, hlens, rlen, ridx, cidx);	
					}
				}
				
			}
			break;
		case DIR_VERTICAL:	/* 67.5 -- 112.5 */
			for (ridx = 0; ridx < rlen; ridx++) {
				r = radiuses[ridx];
				rr = r*r;
				
				xstart_idx = get_cell_idx(xcells[0], (int)(xp - r*COS67), step, wcell);
				xfinish_idx = get_cell_idx(xcells[0], (int)(xp + r*COS67), step, wcell);
					
				for (xidx = xstart_idx; xidx <= xfinish_idx; xidx++) {
					xc = xcells[xidx];
					xx = (xp - xc)*(xp - xc);

					y = sqrt(rr - xx);
					/* do not consider points out of bounds */	
					if (yp - y > 0) {
						yidx = get_cell_idx(ycells[0], yp - y, step, hcell);
						cidx = wcell*yidx + xidx;
						houghspace_increment(hspace, hlens, rlen, ridx, cidx);
					}

					if (yp + y < im->h) {
						yidx = get_cell_idx(ycells[0], yp + y, step, hcell);
						cidx = wcell*yidx + xidx;
						houghspace_increment(hspace, hlens, rlen, ridx, cidx);
					}
				}		
			}
			break;
		case DIR_NEGATIVE_DIAG:	/* 112.5 -- 157.5 */
			for (ridx = 0; ridx < rlen; ridx++) {
				r = radiuses[ridx];
				rr = r*r;
				
				xstart_idx = get_cell_idx(xcells[0], (int)(xp + r*COS157), step, wcell);
				xfinish_idx = get_cell_idx(xcells[0], (int)(xp + r*COS112), step, wcell);
				
				for (xidx = xstart_idx; xidx <= xfinish_idx; xidx++) {
					xc = xcells[xidx];
					xx = (xp - xc)*(xp - xc);
			
					y = sqrt(rr - xx);
					if (yp + y < im->h) {
						yidx = get_cell_idx(ycells[0], yp + y, step, wcell);
						cidx = wcell*yidx + xidx;
						houghspace_increment(hspace, hlens, rlen, ridx, cidx);	
					}
				}

				xstart_idx = get_cell_idx(xcells[0], (int)(xp - r*COS112), step, wcell);
				xfinish_idx = get_cell_idx(xcells[0], (int)(xp - r*COS157), step, wcell);
				
				for (xidx = xstart_idx; xidx <= xfinish_idx; xidx++) {
					xc = xcells[xidx];
					xx = (xp - xc)*(xp - xc);
			
					y = sqrt(rr - xx);
					if (yp - y > 0) {
						yidx = get_cell_idx(ycells[0], yp - y, step, wcell);
						cidx = wcell*yidx + xidx;
						houghspace_increment(hspace, hlens, rlen, ridx, cidx);	
					}
				}
			}
			
			break;
		}
	}
	
	/* find local maxima */
	for (ridx = 0; ridx < rlen; ridx++) {
		/* size in bytes */
		size = hlens[ridx];
		/* number of elements */
		n = size / sizeof(*(hspace[ridx]));
		for (i = 0; i < n; i += min_dist) {
			int j;
			mweight = 0;
			cidx = 0;
			if (i + min_dist < n) {
				for (j = i; j < i + min_dist; j++) {
					if (hspace[ridx][j] > mweight) {
						cidx = j;
						mweight = hspace[ridx][j];
					}	
				}
				
				if (mweight > 0) {
					yidx = cidx / wcell;
					xidx = cidx % wcell;
					vector_append(&centers, xidx, yidx, radiuses[ridx]);	
				}
			} else {
				/* search for reamining part */	
				for (j = i; j < n; j++) {
					if (hspace[ridx][j] > mweight) {
						cidx = j;
						mweight = hspace[ridx][j];
					}	
				}

				if (mweight > 0) {
					yidx = cidx / wcell;
					xidx = cidx % wcell;
					vector_append(&centers, xidx, yidx, radiuses[ridx]);	
				}
			}
		}

	}
	
	/* return local maximas */
	*circles = centers.arr;
	/* number of found circles */
	n = centers.last;

	houghspace_free(hspace, rlen);
	xfree(radiuses);
	xfree(xcells);
	xfree(ycells);
	xfree(hlens);

	return n;
}

/* min_dist define amount of neighhour pixels to calculate local maxima
 */
int houghcircles(struct img_ctx *img, struct vec3 **circles, int rmin, int rmax, int step, int min_dist, struct img_gradient *grad)
{
	int tmp, n;

	assert(img != NULL);
	assert(grad != NULL);
	assert(circles != NULL);
	
	if (rmin > rmax) {
		tmp = rmax;
		rmax = rmin;
		rmin = tmp;
	}

	/* init variables */
	edge_pixels = edge_ctx.edge_pixels;
	edge_last = edge_ctx.edge_last;
	/* return number of found circles */
	n = _houghcircles(img, circles, rmin, rmax, step, min_dist, grad);

	return n;
}
