#ifndef VEC_H_
#define VEC_H_

struct vec3 {
	int x;
	int y;
	int z;
};

struct vec3 *vec3_new(int x, int y, int z);
void vec3_destroy(struct vec3* p);

#endif /* VEC_H_ */
