#ifndef CANNY_H_
#define CANNY_H_

#include "img.h"

struct edge {
	unsigned int *edge_pixels; /* indexes of edges */
	size_t edge_size;	   /* size in bytes */
	size_t edge_used;
	unsigned int edge_last;	   /* last index */
};

extern struct edge edge_ctx;

int canny(int thigh, int tlow, struct img_ctx *img, struct img_gradient *grad, struct img_ctx *edges);

#endif /* CANNY_H_ */
