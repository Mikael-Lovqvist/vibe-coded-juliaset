//$ gcc -Ofast -ffast-math -fno-math-errno -march=native julia.c -lpng -lm -o julia.elf
//$ gcc -march=native julia.c -lpng -lm -o julia.elf


/* julia_starfish.c
 *
 * C (C11), uses libpng
 *
 * Build:
 *   cc -O3 -std=c11 julia_starfish.c -lpng -lm -o julia_starfish
 *
 * Usage:
 *   ./julia_starfish [N] [M] [out.png]
 * Defaults:
 *   N=1024, M=1024, out=julia_starfish.png
 */

#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

static void hsv_to_rgb(double h, double s, double v,
                       uint8_t *r, uint8_t *g, uint8_t *b)
{
	double rf, gf, bf;

	double i = floor(h * 6.0);
	double f = h * 6.0 - i;
	double p = v * (1.0 - s);
	double q = v * (1.0 - f * s);
	double t = v * (1.0 - (1.0 - f) * s);

	switch ((int)i % 6)
	{
		case 0: rf = v; gf = t; bf = p; break;
		case 1: rf = q; gf = v; bf = p; break;
		case 2: rf = p; gf = v; bf = t; break;
		case 3: rf = p; gf = q; bf = v; break;
		case 4: rf = t; gf = p; bf = v; break;
		default: rf = v; gf = p; bf = q; break;
	}

	*r = (uint8_t)(rf * 255.0);
	*g = (uint8_t)(gf * 255.0);
	*b = (uint8_t)(bf * 255.0);
}

static void write_png(const char *path, int w, int h, uint8_t *rgba)
{
	FILE *fp = fopen(path, "wb");
	if (!fp)
	{
		perror("fopen");
		exit(1);
	}

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) exit(1);

	png_infop info = png_create_info_struct(png);
	if (!info) exit(1);

	if (setjmp(png_jmpbuf(png))) exit(1);

	png_init_io(png, fp);

	png_set_IHDR(
		png, info,
		w, h,
		8,
		PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
	);

	png_write_info(png, info);

	png_bytep *rows = malloc(sizeof(png_bytep) * h);
	for (int y = 0; y < h; y++)
		rows[y] = rgba + (size_t)y * w * 4;

	png_write_image(png, rows);
	png_write_end(png, NULL);

	free(rows);
	png_destroy_write_struct(&png, &info);
	fclose(fp);
}

int main(int argc, char **argv)
{
	int N = (argc > 1) ? atoi(argv[1]) : 1024;
	int M = (argc > 2) ? atoi(argv[2]) : 1024;
	const char *out = (argc > 3) ? argv[3] : "julia_starfish-c.png";

	if (N <= 0 || M <= 0)
	{
		fprintf(stderr, "Invalid N or M\n");
		return 1;
	}

	const double cr = -0.4;
	const double ci =  0.6;

	const double xmin = -1.6, xmax = 1.6;
	const double ymin = -1.6, ymax = 1.6;

	const double dx = (xmax - xmin) / (N - 1);
	const double dy = (ymax - ymin) / (N - 1);

	const double escape2 = 4.0;
	const double inv_log2 = 1.0 / log(2.0);

	uint8_t *img = malloc((size_t)N * N * 4);
	if (!img) return 1;

	for (int y = 0; y < N; y++)
	{
		double zi0 = ymax - y * dy;
		for (int x = 0; x < N; x++)
		{
			double zr = xmin + x * dx;
			double zi = zi0;

			int i = 0;
			double zr2 = zr * zr;
			double zi2 = zi * zi;

			while (i < M && (zr2 + zi2) <= escape2)
			{
				zi = (zr + zr) * zi + ci;
				zr = (zr2 - zi2) + cr;
				zr2 = zr * zr;
				zi2 = zi * zi;
				i++;
			}

			uint8_t *p = img + ((size_t)y * N + x) * 4;

			if (i >= M)
			{
				p[0] = p[1] = p[2] = 0;
				p[3] = 255;
			}
			else
			{
				double mag = sqrt(zr2 + zi2);
				double nu = i + 1.0 - log(log(mag)) * inv_log2;
				double t = nu / M;
				if (t < 0.0) t = 0.0;
				if (t > 1.0) t = 1.0;

				double h = fmod(0.66 + 1.4 * t, 1.0);
				double s = 0.85;
				double v = 0.15 + 0.95 * t;

				hsv_to_rgb(h, s, v, &p[0], &p[1], &p[2]);
				p[3] = 255;
			}
		}
	}

	write_png(out, N, N, img);
	free(img);

	printf("Wrote %s (%dx%d, M=%d)\n", out, N, N, M);
	return 0;
}
