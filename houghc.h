#ifndef HOUGHC_H_
#define HOUGHC_H_

#include "img.h"
#include "vec.h"

int houghcircles(struct img_ctx *im, struct vec3 **circles, int rmin, int rmax, int step, int min_dist, struct img_gradient *grad);

#endif /* HOUGHC_H_ */
