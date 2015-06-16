#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "xmalloc.h"
#include "img.h"
#include "img_utils.h"
#include "sobel.h"
#include "canny.h"
#include "houghc.h"

int get_ctx(GdkPixbuf *pbuf, img_type_t type, struct img_ctx **imctx)
{
	struct img_ctx *ctx;
	int nchan, w, h, i, j, rowstride, skip;
	unsigned char *r, *g, *b, *pix, *p, *q;
	
	assert(pbuf != NULL);
	assert(imctx != NULL);
	
	nchan = gdk_pixbuf_get_n_channels(pbuf);
	w = gdk_pixbuf_get_width(pbuf);
	h = gdk_pixbuf_get_height(pbuf);
	rowstride = gdk_pixbuf_get_rowstride(pbuf);
	pix = gdk_pixbuf_get_pixels(pbuf);

	skip = rowstride - w*nchan;

	switch (type) {
	case TYPE_GRAY:
		ctx = img_ctx_new(w, h, TYPE_GRAY, C_NONE);
		
		p = pix;
        	q = ctx->pix;

		for (i = 0; i < h; i++) {
                	for (j = 0; j < w; j++) {
				/* the same gray value in the next two pixels */
				q[0] = p[0];
				/* skip it */
                       		p += nchan;
				q++;
                	}
                	/* skip the padding bytes */
                	p += skip;
        	}	
		break;
	case TYPE_RGB:
		ctx = img_ctx_new(w, h, TYPE_RGB, C_NONE);
		
		p = pix;
		r = ctx->r;
		g = ctx->g;
		b = ctx->b;

		for (i = 0; i < h; i++) {
			for (j = 0; j < w; j++) {
				r[0] = p[0];
				g[0] = p[1];
				b[0] = p[2];
				p += nchan;
				r++;
				g++;
				b++;
			}
			p += skip;
		}
		break;
	default:
		fprintf(stderr, "error: not implemented\n");
		abort();
	}

	*imctx = ctx;

	return RET_OK;		
}

int load_ctx(struct img_ctx *ctx, GdkPixbuf *pbuf)
{
	unsigned int nchan, w, h, i, j, rowstride, skip;
	unsigned char *pix, *p, *gray;

	assert(ctx != NULL);
	assert(pbuf != NULL);

	w = ctx->w;
	h = ctx->h;

	nchan = gdk_pixbuf_get_n_channels(pbuf);
	rowstride = gdk_pixbuf_get_rowstride(pbuf);
	pix = gdk_pixbuf_get_pixels(pbuf);
	
	skip = rowstride - w*nchan;
	
	p = pix;
	gray = ctx->pix;

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {			
			p[0] = gray[0];
			p[1] = gray[0];
			p[2] = gray[0];
			p += nchan;
			gray++;
		}
		
		p += skip;
	}

	return RET_OK;
}

/* callback for edge_ctx */
void refresh_cb(void)
{
	if (edge_ctx.edge_pixels) {
		memset((void*)edge_ctx.edge_pixels, 0, sizeof(edge_ctx.edge_size));
		edge_ctx.edge_used = 0;
		edge_ctx.edge_last = 0;
	}
}

/* callback for edge_ctx */
void free_cb(void)
{
	if (edge_ctx.edge_pixels)
		xfree(edge_ctx.edge_pixels);
}

char *get_extention(char *fname)
{
	char *p;

	assert(fname != NULL);
	
	p = strchr(fname, '.') + 1;
	return xstrdup(p);
}

int main(int argc, char **argv)
{
	GdkPixbuf *pbuf, *newbuf = NULL;
	GError *error = NULL;
	int  w, h, opt, t_low, t_high, rmin, rmax, i, n, step;
	struct img_ctx *rgb, *gray, *edges;
	struct img_gradient *grad;
	struct vec3 *circles = NULL;
	char *fname, *outname, *ext;
	
	struct option long_options[] = {
		{ "rmin", required_argument, NULL, 'm' },
		{ "rmax", required_argument, NULL, 'x' },
		{ "step", required_argument, NULL, 's' },
		{ NULL, 0, NULL, 0 }
	};

	fname = outname = ext = NULL;
	t_low = t_high = 0;
	rmin = rmax = 0;
	step = 1;

	while ((opt = getopt_long(argc, argv, "f:o:l:h:m:x:", long_options, NULL)) != -1) { 
		switch (opt) {
		case 'f':
			fname = xstrdup(optarg);
			break;
		case 'l':
			t_low = atoi(optarg);
			break;
		case 'h':
			t_high = atoi(optarg);
			break;
		case 'm':
			rmin = atoi(optarg);
			break;
		case 'x':
			rmax = atoi(optarg);
			break;
		case 's':
			step = atoi(optarg);
			break;
		case 'o':
			outname = xstrdup(optarg);
			ext = get_extention(outname);
			break;
		default:
			fprintf(stderr, "error: unknow option\n");
			exit(1);
			break;
		}
	}

	argc -= optind;
	argv += optind;

	if (fname == NULL) {
		fprintf(stderr, "error: no file name is specified\n");
		exit(1);
	}
	
	/* default output name */
	if (outname == NULL) {
		outname = xstrdup("output.png");
		ext = get_extention(outname);	
	}

	gtk_init(&argc, &argv);
		
	pbuf = gdk_pixbuf_new_from_file(fname, &error);
	if (pbuf == NULL) {
		fprintf(stderr, "error: cannot load image\n");
		exit(1);
	}

	/* do required manipulation */
	get_ctx(pbuf, TYPE_RGB, &rgb);
	
	w = rgb->w;
	h = rgb->h;

	gray = img_ctx_new(w, h, TYPE_GRAY, C_NONE);
	edges = img_ctx_new(w, h, TYPE_GRAY, C_BLACK);	
	
	img_grayscale(rgb, gray);
	img_gaussian_blur(gray, gray);
	/* init callbacks for global structure */
	edge_ctx.refresh = refresh_cb;
	edge_ctx.free = free_cb;

	/* compute gradient values */
	grad = img_gradient_new(gray);
	sobel_gradient(gray, grad);
	canny(t_low, t_high, gray, grad, edges);

	n = houghcircles(edges, &circles, rmin, rmax, step, w*h, grad);
	
	/* output detected circles */
	for (i = 0; i < n; i++) {
		struct vec3 *p;
		p = circles + i;
		fprintf(stdout, "%d %d %d\n", p->x, p->y, p->z);
	}
	
	/* the only colorspace gdk supports */
	newbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, w, h);
	if (newbuf == NULL) {
		fprintf(stderr, "error: cannot create pixbuf\n");
		exit(1);
	}
	
	load_ctx(edges, newbuf);

	if (!gdk_pixbuf_save(newbuf, outname, ext, &error, NULL)) {
		g_printerr("failed to save image: %s\n", error->message);
		g_error_free(error);
		return EXIT_FAILURE;
        }
	
	xfree(fname);
	xfree(outname);
	xfree(ext);
	
	edge_ctx.free();
	
	vec3_destroy(circles);
	
	img_destroy_ctx(rgb);
	img_destroy_ctx(gray);
	img_destroy_ctx(edges);
	img_gradient_destroy(grad);
	
	g_object_unref(G_OBJECT(pbuf));
	g_object_unref(G_OBJECT(newbuf));
	
	return EXIT_SUCCESS;
}
