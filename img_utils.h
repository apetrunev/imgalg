#ifndef IMG_UTILS_H_
#define IMG_UTILS_H_

#include "img.h"

int img_grayscale(struct img_ctx *src, struct img_ctx *dst);
int img_otsu_threshold(struct img_ctx *gray);
int img_gaussian_blur(struct img_ctx *src, struct img_ctx *dst);

#endif /* IMG_UTILS_H_ */
