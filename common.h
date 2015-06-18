#ifndef COMMON_H_
#define COMMON_H_

#define BUFSIZE (4*1024)

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE !(FALSE)
#endif

#define TAN_22 0.41421356237	/* 22.5  */
#define TAN_67 2.41421356237	/* 67.5 */
#define	TAN_112 -2.41421356237	/* 112.5 */
#define	TAN_157 -0.41421356237	/* 157.5 */

typedef enum {
	TYPE_RGB,
	TYPE_GRAY
} img_type_t;

typedef enum {
	DIR_NONE = 0,
	DIR_VERTICAL = 1,	
	DIR_HORIZONTAL,		/* 2 */
	DIR_POSITIVE_DIAG,	/* 3 */
	DIR_NEGATIVE_DIAG	/* 4 */
} dir_type_t;

typedef enum {
	RET_ERR = -1,
	RET_OK = 0,
} ret_type_t;

typedef enum {
	C_NONE = -1,
	C_BLACK = 0,
} color_type_t;

#endif /* COMMON_H_ */
