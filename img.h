#ifndef IMAGE_H_
#define IMAGE_H_

#include "common.h"

struct img_ctx {
	img_type_t type;
	unsigned int w;
	unsigned int h;
	union {
		struct {
			unsigned char *r;
			unsigned char *g;
			unsigned char *b;
		};
		unsigned char *pix;
	};
};

struct img_gradient {
	unsigned int w;
	unsigned int h;
	unsigned int *gmag;	/* gradient magnitudes */
	int *gdir;		/* gradient directions */
};

struct img_ctx *img_ctx_new(int w, int h, img_type_t type, color_type_t fill);
void img_destroy_ctx(struct img_ctx *ctx);

struct img_gradient *img_gradient_new(struct img_ctx *ctx);
void img_gradient_destroy(struct img_gradient *g);

#endif /* IMAGE_H_ */
