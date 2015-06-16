#ifndef CANNY_H_
#define CANNY_H_

#include "img.h"

typedef void (*callback_type_t)(void);

struct edge {
	unsigned int *edge_pixels;	/* indexes of edges */
	size_t edge_size;		/* size in bytes */
	size_t edge_used;
	unsigned int edge_last;		/* last index */
	callback_type_t refresh;	/* fill edge_pixels with 0 */
	callback_type_t	free;	/* free memory used by edge_pixels */
};

struct edge edge_ctx;
extern struct edge edge_ctx;

int canny(int thigh, int tlow, struct img_ctx *img, struct img_gradient *grad, struct img_ctx *edges);

#endif /* CANNY_H_ */
