//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2022/2023
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de fun��es n�o seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "vc.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Alocar mem�ria para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
	IVC *image = (IVC *)malloc(sizeof(IVC));

	if (image == NULL)
		return NULL;
	if ((levels <= 0) || (levels > 255))
		return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char *)malloc(image->width * image->height * image->channels * sizeof(char));

	if (image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}

// Libertar mem�ria de uma imagem
IVC *vc_image_free(IVC *image)
{
	if (image != NULL)
	{
		if (image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

char *netpbm_get_token(FILE *file, char *tok, int len)
{
	char *t;
	int c;

	for (;;)
	{
		while (isspace(c = getc(file)))
			;
		if (c != '#')
			break;
		do
			c = getc(file);
		while ((c != '\n') && (c != EOF));
		if (c == EOF)
			break;
	}

	t = tok;

	if (c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

		if (c == '#')
			ungetc(c, file);
	}

	*t = 0;

	return tok;
}

long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char *p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}

void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char *p = databit;

	countbits = 1;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				// datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}

IVC *vc_read_image(char *filename)
{
	FILE *file = NULL;
	IVC *image = NULL;
	unsigned char *tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;

	// Abre o ficheiro
	if ((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if (strcmp(tok, "P4") == 0)
		{
			channels = 1;
			levels = 1;
		} // Se PBM (Binary [0,1])
		else if (strcmp(tok, "P5") == 0)
			channels = 1; // Se PGM (Gray [0,MY_MAX(level,255)])
		else if (strcmp(tok, "P6") == 0)
			channels = 3; // Se PPM (RGB [0,MY_MAX(level,255)])
		else
		{
#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif

			fclose(file);
			return NULL;
		}

		if (levels == 1) // PBM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL)
				return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char *)malloc(sizeofbinarydata);
			if (tmp == NULL)
				return 0;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL)
				return NULL;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			size = image->width * image->height * image->channels;

			if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}

		fclose(file);
	}
	else
	{
#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
	}

	return image;
}

int vc_write_image(char *filename, IVC *image)
{
	FILE *file = NULL;
	unsigned char *tmp;
	long int totalbytes, sizeofbinarydata;

	if (image == NULL)
		return 0;

	if ((file = fopen(filename, "wb")) != NULL)
	{
		if (image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char *)malloc(sizeofbinarydata);
			if (tmp == NULL)
				return 0;

			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

			if (fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				return 0;
			}
		}

		fclose(file);

		return 1;
	}

	return 0;
}

int vc_gray_negative(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
		return 0;
	if (channels != 1)
		return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos] = 255 - data[pos];
		}
	}
	return 1;
}

int vc_rgb_negative(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
		return 0;
	if (channels != 3)
		return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos] = 255 - data[pos];
			data[pos + 1] = 255 - data[pos + 1];
			data[pos + 2] = 255 - data[pos + 2];
		}
	}

	return 1;
}

int vc_rgb_get_red_gray(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
		return 0;
	if (channels != 3)
		return 0;

	// Extrai a componente RED
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos + 1] = data[pos]; // Green
			data[pos + 2] = data[pos]; // Blue
		}
	}

	return 1;
};

int vc_rgb_get_green_gray(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
		return 0;
	if (channels != 3)
		return 0;

	// Extrai a componente RED
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos] = data[pos + 1];
			data[pos + 2] = data[pos + 1];
		}
	}

	return 1;
};

int vc_rgb_get_blue_gray(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
		return 0;
	if (channels != 3)
		return 0;

	// Extrai a componente RED
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos] = data[pos + 2];
			data[pos + 1] = data[pos + 2];
		}
	}

	return 1;
};

int vc_rgb_to_gray(IVC *src, IVC *dst)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	int byterperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char *)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;

	if (src->width <= 0 || src->height <= 0 || src->data == NULL)
		return 0;
	if (src->width != dst->width || src->height != dst->height)
		return 0;
	if (src->channels != 3 || dst->channels != 1)
		return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * byterperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src + 1];
			bf = (float)datasrc[pos_src + 2];

			datadst[pos_dst] = (unsigned char)((rf * 0.299) + (gf * 0.587) + (bf * 0.114));
		}
	}
	return 1;
}

int vc_rgb_to_hsv(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	float r, g, b, saturation, hue, value;
	int i, size;
	int pos_src;
	float rgb_max;
	float rgb_min;

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
		return 0;
	if (channels != 3)
		return 0;

	size = width * height * channels;

	for (i = 0; i < size; i += channels)
	{
		pos_src = i;

		r = (float)data[pos_src];
		g = (float)data[pos_src + 1];
		b = (float)data[pos_src + 2];

		rgb_max = fmaxf(r, fmaxf(g, b));
		rgb_min = fminf(r, fminf(g, b));

		value = rgb_max;

		if (value == 0.0f)
		{
			hue = 0.0f;
			saturation = 0.0f;
		}
		else
		{
			saturation = ((rgb_max - rgb_min) / rgb_max) * 255.0f;

			if (saturation == 0.0f)
			{
				hue = 0.0f;
			}
			else
			{
				if ((rgb_max == r) && (g >= b))
				{
					hue = 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if ((rgb_max == r) && (b > g))
				{
					hue = 360 + 60 * (g - b) / (rgb_max - rgb_min);
				}
				else if (rgb_max == g)
				{
					hue = 120 + 60 * (b - r) / (rgb_max - rgb_min);
				}
				else if (rgb_max == b)
				{
					hue = 240 + 60 * (r - g) / (rgb_max - rgb_min);
				}
			}
		}

		data[i] = (unsigned char)(hue / 360.0f * 255.0f);
		data[i + 1] = (unsigned char)(saturation);
		data[i + 2] = (unsigned char)(value);
	}

	return 1;
}

int vc_hsv_segmentation(IVC *src, IVC *dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	int byterperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char *)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float h, s, v;

	if (src->width <= 0 || src->height <= 0 || src->data == NULL)
		return 0;
	if (src->width != dst->width || src->height != dst->height)
		return 0;
	if (src->channels != 3 || dst->channels != 1)
		return 0;

	// Segmentation loop
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * byterperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			// Assuming HSV values are stored in src and are normalized [0, 255]
			h = (int)(((float)datasrc[pos_src]) / 255.0f * 360.0f);
			s = (int)(((float)datasrc[pos_src + 1]) / 255.0f * 100.0f);
			v = (int)(((float)datasrc[pos_src + 2]) / 255.0f * 100.0f);

			// Check if the pixel falls within the specified HSV range
			if (h >= hmin && h <= hmax && s >= smin && s <= smax && v >= vmin && v <= vmax)
			{
				datadst[pos_dst] = 255; // Pixel is within range, mark as white
			}
			else
			{
				datadst[pos_dst] = 0; // Pixel is outside range, mark as black
			}
		}
	}

	return 1; // Success
}

int vc_scale_gray_to_rgb(IVC *src, IVC *dst)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	int byterperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char *)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	unsigned char gray, red, green, blue;

	// Check for valid conditions
	if (src->width <= 0 || src->height <= 0 || src->data == NULL)
		return 0;
	if (src->width != dst->width || src->height != dst->height)
		return 0;
	if (src->channels != 1 || dst->channels != 3)
		return 0;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			pos_src = y * byterperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			gray = datasrc[pos_src];

			if (gray < 128)
				red = 0;
			else if (gray >= 128 && gray <= 192)
				red = (unsigned char)(((gray - 128) / 64.0) * 255);
			else
				red = 255;

			if (gray <= 64)
				green = (unsigned char)((gray / 64.0) * 255);
			else if (gray > 64 && gray <= 192)
				green = 255;
			else
				green = (unsigned char)(((255 - gray) / 63.0) * 255);

			if (gray <= 64)
				blue = 255;
			else if (gray > 64 && gray <= 128)
				blue = (unsigned char)(((128 - gray) / 64.0) * 255);
			else
				blue = 0;

			datadst[pos_dst] = red;
			datadst[pos_dst + 1] = green;
			datadst[pos_dst + 2] = blue;
		}
	}

	return 1;
}

void combine_segmentations(IVC *dst, IVC *src1, IVC *src2)
{
	if (src1->width != src2->width || src1->height != src2->height)
	{
		printf("ERROR -> combine_segmentations():\n\tImages size mismatch!\n");
		return;
	}

	for (int y = 0; y < src1->height; y++)
	{
		for (int x = 0; x < src1->width; x++)
		{
			int pos = y * (src1->width) + x; // Assume imagens de destino são monocromáticas

			// Se algum dos pixels nas imagens temporárias for branco, defina o correspondente em dst como branco
			if (src1->data[pos] == 255 || src2->data[pos] == 255)
			{
				dst->data[pos] = 255;
			}
			else
			{
				dst->data[pos] = 0;
			}
		}
	}
}

int vc_white_pixels_quantitie(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y, w;
	long int pos;
	w = 0;

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
		return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			if (data[pos] == 255)
			{
				w++;
			}
		}
	}
	return w;
}

int vc_gray_to_binary(IVC *srcdst, int threshold)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y, w;
	long int pos;

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
		return 0;

	srcdst->levels = 1;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			if (data[pos] <= (unsigned char)threshold)
				data[pos] = 0;
			else
				data[pos] = 255;
		}
	}
	return 1;
}

int vc_gray_to_binary_global_mean(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y, w;
	long int pos, soma = 0;
	int threshold;

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
		return 0;

	for (y = 0; y < height; y++)
	{

		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			soma += data[pos];
		}
	}

	threshold = soma / (width * height);

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			if (data[pos] <= (unsigned char)threshold)
				data[pos] = 0;
			else
				data[pos] = 255;
		}
	}

	return 1;
}

int vc_gray_to_binary_midpoint(IVC *src, IVC *dst, int kernel)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int offset = (kernel - 1) / 2;
	int x, y, kx, ky;
	int min, max;
	long int pos, posk;
	unsigned char threshold;

	// Check for valid conditions
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
		return 0;
	if (channels != 1)
		return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			max = 0;
			min = 255;

			for (ky = -offset; ky <= offset; ky++)
			{
				for (kx = -offset; kx <= offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels;

						if (datasrc[posk] > max)
							max = datasrc[posk];
						if (datasrc[posk] < min)
							min = datasrc[posk];
					}
				}
			}

			threshold = (unsigned char)((float)(max + min) / (float)2);
			if (datasrc[pos] > threshold)
				datadst[pos] = 255;
			else
				datadst[pos] = 0;
		}
	}
	return 1;
}

int vc_gray_to_binary_bernsen(IVC *src, IVC *dst, int kernel, int cmin)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int offset = (kernel - 1) / 2;
	int x, y, kx, ky;
	int min, max;
	long int pos, posk;
	unsigned char threshold;
	int levels = src->levels;

	// Check for valid conditions
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
		return 0;
	if (channels != 1)
		return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			max = 0;
			min = 255;

			for (ky = -offset; ky <= offset; ky++)
			{
				for (kx = -offset; kx <= offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels;

						if (datasrc[posk] > max)
							max = datasrc[posk];
						if (datasrc[posk] < min)
							min = datasrc[posk];
					}
				}
			}

			if (((float)(max - min)) < cmin)
			{
				threshold = (unsigned char)((float)levels / (float)2);
			}
			else
			{
				threshold = (unsigned char)((float)(max + min) / (float)2);
			}
			if (datasrc[pos] > threshold)
				datadst[pos] = 255;
			else
				datadst[pos] = 0;
		}
	}
	return 1;
}

int vc_gray_to_binary_niblack(IVC *src, IVC *dst, int kernel, float k)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int offset = (kernel - 1) / 2;
	int x, y, xx, yy;
	int min, max, counter;
	long int pos, posk;
	unsigned char threshold;

	// Check for valid conditions
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
		return 0;
	if (channels != 1)
		return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			max = datasrc[pos];
			min = datasrc[pos];

			float mean = 0.0f;

			for (counter = 0, yy = -offset; yy <= offset; yy++)
			{
				for (xx = -offset; xx <= offset; xx++)
				{
					if ((y + yy >= 0) && (y + yy < height) && (x + xx >= 0) && (x + xx < width))
					{
						posk = (y + yy) * bytesperline + (x + xx) * channels;

						mean += (float)datasrc[posk];

						counter++;
					}
				}
			}

			mean /= counter;

			float sdeviatan = 0.0f;

			for (counter = 0, yy = -offset; yy <= offset; yy++)
			{
				for (xx = -offset; xx <= offset; xx++)
				{
					if ((y + yy >= 0) && (y + yy < height) && (x + xx >= 0) && (x + xx < width))
					{
						posk = (y + yy) * bytesperline + (x + xx) * channels;

						sdeviatan += powf(((float)datasrc[posk]) - mean, 2);
						counter++;
					}
				}
			}
			sdeviatan = sqrtf(sdeviatan / counter);
			threshold = mean + k * sdeviatan;

			if (datasrc[pos] > threshold)
				datadst[pos] = 255;
			else
				datadst[pos] = 0;
		}
	}
	return 1;
}

int vc_binary_dilate(IVC *src, IVC *dst, int kernel)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int offset = (kernel - 1) / 2;
	int x, y, kx, ky;
	int soma;
	long int pos, posk;
	unsigned char threshold;

	// Check for valid conditions
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
		return 0;
	if (channels != 1)
		return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			soma = 0;
			for (ky = -offset; ky <= offset; ky++)
			{

				for (kx = -offset; kx <= offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels;

						soma += datasrc[posk];
					}
				}
			}

			if (soma >= 1)
				datadst[pos] = 255;
			else
				datadst[pos] = 0;
		}
	}
	return 1;
}

int vc_binary_erode(IVC *src, IVC *dst, int kernel)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int offset = (kernel - 1) / 2;
	int x, y, kx, ky;
	int ww;
	long int pos, posk;
	unsigned char threshold;

	// Check for valid conditions
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
		return 0;
	if (channels != 1)
		return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			ww = 0;
			for (ky = -offset; ky <= offset; ky++)
			{

				for (kx = -offset; kx <= offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels;

						if (datasrc[posk] == 0)
						{
							ww = 1;
						}
					}
				}
			}

			if (ww == 1)
				datadst[pos] = 0;
			else
				datadst[pos] = 255;
		}
	}
	return 1;
}

int vc_binary_open(IVC *src, IVC *dst, int kernel)
{
	int ret = 1;
	IVC *tmp = vc_image_new(src->width, src->height, src->channels, src->levels);

	ret &= vc_binary_erode(src, tmp, kernel);
	ret &= vc_binary_dilate(tmp, dst, kernel);

	vc_image_free(tmp);

	return ret;
}

int vc_binary_close(IVC *src, IVC *dst, int kernel)
{
	int ret = 1;
	IVC *tmp = vc_image_new(src->width, src->height, src->channels, src->levels);

	ret &= vc_binary_dilate(src, tmp, kernel);
	ret &= vc_binary_erode(tmp, dst, kernel);

	vc_image_free(tmp);

	return ret;
}

int vc_gray_dilate(IVC *src, IVC *dst, int kernel)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int offset = (kernel - 1) / 2;
	int x, y, kx, ky;
	int soma, max, w;
	long int pos, posk;
	unsigned char threshold;

	// Check for valid conditions
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
		return 0;
	if (channels != 1)
		return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			max = 0;

			for (ky = -offset; ky <= offset; ky++)
			{

				for (kx = -offset; kx <= offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels;

						if (max < datasrc[posk])
						{
							max = datasrc[posk];
						}
					}
				}
			}

			datadst[pos] = max;
		}
	}
	return 1;
}

int vc_gray_erode(IVC *src, IVC *dst, int kernel)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int offset = (kernel - 1) / 2;
	int x, y, kx, ky;
	int min;
	long int pos, posk;
	unsigned char threshold;

	// Check for valid conditions
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
		return 0;
	if (channels != 1)
		return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			min = 255;
			for (ky = -offset; ky <= offset; ky++)
			{

				for (kx = -offset; kx <= offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels;

						if (min > datasrc[posk])
						{
							min = datasrc[posk];
						}
					}
				}
			}

			datadst[pos] = min;
		}
	}
	return 1;
}

int vc_gray_open(IVC *src, IVC *dst, int kernel)
{
	int ret = 1;
	IVC *tmp = vc_image_new(src->width, src->height, src->channels, src->levels);

	ret &= vc_gray_erode(src, tmp, kernel);
	ret &= vc_gray_dilate(tmp, dst, kernel);

	vc_image_free(tmp);

	return ret;
}

int vc_gray_close(IVC *src, IVC *dst, int kernel)
{
	int ret = 1;
	IVC *tmp = vc_image_new(src->width, src->height, src->channels, src->levels);

	ret &= vc_gray_dilate(src, tmp, kernel);
	ret &= vc_gray_erode(tmp, dst, kernel);

	vc_image_free(tmp);

	return ret;
}

void combine_segmentations2(IVC *dst, IVC *src1, IVC *src2)
{
	if (src1->width != src2->width || src1->height != src2->height)
	{
		printf("ERROR -> combine_segmentations():\n\tImages size mismatch!\n");
		return;
	}

	for (int y = 0; y < src1->height; y++)
	{
		for (int x = 0; x < src1->width; x++)
		{
			int pos = y * (src1->width) + x; // Assume imagens de destino são monocromáticas

			// Se algum dos pixels nas imagens temporárias for branco, defina o correspondente em dst como branco

			dst->data[pos] = src1->data[pos] - src2->data[pos];

			if (dst->data[pos] > 255)
			{
				dst->data[pos] = 255;
			}

			if (dst->data[pos] < 0)
			{
				dst->data[pos] = 0;
			}
		}
	}
}

void brancoparaoriginal(IVC *dst, IVC *src1, IVC *src2)
{
	if (src1->width != src2->width || src1->height != src2->height)
	{
		printf("ERROR -> combine_segmentations():\n\tImages size mismatch!\n");
		return;
	}

	for (int y = 0; y < src1->height; y++)
	{
		for (int x = 0; x < src1->width; x++)
		{
			int pos = y * (src1->width) + x; // Assume imagens de destino são monocromáticas

			if (src1->data[pos] == 255)
			{
				dst->data[pos] = src2->data[pos];
			}
			else
			{
				dst->data[pos] = 0;
			}
		}
	}
}

OVC *vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = {0};
	int labelarea[256] = {0};
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC *blobs; // Apontador para array de blobs (objectos) que ser� retornado desta fun��o.

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
		return NULL;
	if (channels != 1)
		return NULL;

	// Copia dados da imagem bin�ria para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pix�is de plano de fundo devem obrigat�riamente ter valor 0
	// Todos os pix�is de primeiro plano devem obrigat�riamente ter valor 255
	// Ser�o atribu�das etiquetas no intervalo [1,254]
	// Este algoritmo est� assim limitado a 254 labels
	for (i = 0, size = bytesperline * height; i < size; i++)
	{
		if (datadst[i] != 0)
			datadst[i] = 255;
	}

	// Limpa os rebordos da imagem bin�ria
	for (y = 0; y < height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x < width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels;		// B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels;		// D
			posX = y * bytesperline + x * channels;				// X

			// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					// Se A est� marcado
					if (datadst[posA] != 0)
						num = labeltable[datadst[posA]];
					// Se B est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num))
						num = labeltable[datadst[posB]];
					// Se C est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num))
						num = labeltable[datadst[posC]];
					// Se D est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num))
						num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posD]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	// printf("\nMax Label = %d\n", label);

	// Contagem do n�mero de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a < label - 1; a++)
	{
		for (b = a + 1; b < label; b++)
		{
			if (labeltable[a] == labeltable[b])
				labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que n�o hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a < label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++;						  // Conta etiquetas
		}
	}

	// Se n�o h� blobs
	if (*nlabels == 0)
		return NULL;

	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC *)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a < (*nlabels); a++)
			blobs[a].label = labeltable[a];
	}
	else
		return NULL;

	return blobs;
}

int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs)
{
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if (channels != 1)
		return 0;

	// Conta �rea de cada blob
	for (i = 0; i < nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y < height - 1; y++)
		{
			for (x = 1; x < width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// �rea
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x)
						xmin = x;
					if (ymin > y)
						ymin = y;
					if (xmax < x)
						xmax = x;
					if (ymax < y)
						ymax = y;

					// Per�metro
					// Se pelo menos um dos quatro vizinhos n�o pertence ao mesmo label, ent�o � um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		// Centro de Gravidade
		// blobs[i].xc = (xmax - xmin) / 2;
		// blobs[i].yc = (ymax - ymin) / 2;
		blobs[i].xc = sumx / MY_MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MY_MAX(blobs[i].area, 1);
	}

	return 1;
}

int vc_coloring(IVC *src, IVC *dst, int nlabels)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_src = src->bytesperline;
	int bytesperline_dst = dst->bytesperline;
	int channels_src = src->channels;
	int channels_dst = dst->channels;
	int x, y;

	// Define gray range and compute increment
	int min_gray = 50;
	int max_gray = 255;
	float gray_increment = (nlabels > 1) ? (max_gray - min_gray) / (float)(nlabels - 1) : 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			long int pos = y * bytesperline_src + x * channels_src;
			unsigned char label = datasrc[pos];

			if (label == 0)
			{
				datadst[pos] = 0; // Background remains black
			}
			else if (label > 0 && label <= nlabels)
			{
				float gray_shade = min_gray + (label - 1) * gray_increment;
				datadst[pos] = (unsigned char)(gray_shade + 0.5f); // Rounding to nearest integer
			}
		}
	}

	return 1;
}

int vc_draw_boundingbox(IVC *srcdst, OVC *blobs, int nblobs)
{
	unsigned char *data = srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y, i;

	// Verify the validity of input parameters
	if (!data || width <= 0 || height <= 0 || channels < 1)
		return 0;

	// Process each blob
	for (i = 0; i < nblobs; i++)
	{
		int xmin = blobs[i].x;
		int ymin = blobs[i].y;
		int xmax = xmin + blobs[i].width;
		int ymax = ymin + blobs[i].height;

		// Ensure the bounding box is within the image boundaries
		xmin = MY_MAX(0, xmin);
		ymin = MY_MAX(0, ymin);
		xmax = MY_MIN(width - 1, xmax);
		ymax = MY_MIN(height - 1, ymax);

		// Draw horizontal lines at the top and bottom of the bounding box
		for (x = xmin; x <= xmax; x++)
		{
			int topPos = ymin * bytesperline + x * channels;
			int bottomPos = ymax * bytesperline + x * channels;
			for (int channel = 0; channel < channels; channel++)
			{
				data[topPos + channel] = 255;	 // Set to white
				data[bottomPos + channel] = 255; // Set to white
			}
		}

		// Draw vertical lines on the left and right of the bounding box
		for (y = ymin; y <= ymax; y++)
		{
			int leftPos = y * bytesperline + xmin * channels;
			int rightPos = y * bytesperline + xmax * channels;
			for (int channel = 0; channel < channels; channel++)
			{
				data[leftPos + channel] = 255;	// Set to white
				data[rightPos + channel] = 255; // Set to white
			}
		}
	}

	return 1;
}

int vc_draw_centerofgravity(IVC *srcdst, OVC *blobs, int nblobs, int s)
{
	unsigned char *data = srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int c, x, y;
	int xmin, xmax, ymin, ymax;

	// Check for valid data and image dimensions
	if (!data || width <= 0 || height <= 0 || channels < 1)
		return 0;

	for (c = 0; c < nblobs; c++)
	{
		int xc = blobs[c].xc;
		int yc = blobs[c].yc;

		ymin = yc - s;
		ymax = yc + s;
		xmin = xc - s;
		xmax = xc + s;

		// Draw vertical line of the cross
		for (y = MY_MAX(0, ymin); y <= MY_MIN(ymax, height - 1); y++)
		{
			int pos = y * bytesperline + xc * channels;
			if (xc >= 0 && xc < width)
			{
				for (int channel = 0; channel < channels; channel++)
				{
					data[pos + channel] = 255; // Set to white
				}
			}
		}

		// Draw horizontal line of the cross
		for (x = MY_MAX(0, xmin); x <= MY_MIN(xmax, width - 1); x++)
		{
			int pos = yc * bytesperline + x * channels;
			if (yc >= 0 && yc < height)
			{
				for (int channel = 0; channel < channels; channel++)
				{
					data[pos + channel] = 255; // Set to white
				}
			}
		}
	}

	return 1;
}

int vc_gray_histogram_show(IVC *src, IVC *dst)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_src = src->bytesperline;
	int bytesperline_dst = dst->bytesperline;
	int channels_dst = dst->channels;
	int x, y;
	int hist[256] = {0}; // Initialize histogram array to zero
	int max_count = 0;
	int normalized_height[256];

	// Compute histogram
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			int pixel_value = datasrc[y * bytesperline_src + x]; // Access pixel value
			hist[pixel_value]++;
		}
	}

	// Find maximum count in histogram for normalization
	for (int i = 0; i < 256; i++)
	{
		if (hist[i] > max_count)
		{
			max_count = hist[i];
		}
	}

	// Normalize histogram heights
	for (int i = 0; i < 256; i++)
	{
		normalized_height[i] = (hist[i] * height) / max_count; // Scale to image height
	}

	for (int i = 0; i < 256; i++)
	{
		printf("Intensity %d: %d pixels |||  %d \n", i, normalized_height[i], hist[i]);
	}

	printf("Maximco: %d \n", max_count);

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < 256; x++)
		{

			datadst[y * bytesperline_dst + x * channels_dst] = 0;
		}
	}

	// Draw histogram in destination image
	for (int i = 0; i < 256 && i < width; i++)
	{
		for (y = height - 1; y >= height - normalized_height[i] && y >= 0; y--)
		{
			long int posdst = y * bytesperline_dst + i * channels_dst;
			datadst[posdst] = 255;
		}
	}

	return 1;
}

int vc_gray_histogram_equalization(IVC *src, IVC *dst)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_src = src->width * src->channels;
	int bytesperline_dst = dst->width * dst->channels;
	int total_pixels = width * height;
	int histogram[256] = {0};
	int cumulative_histogram[256] = {0};

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int pos_src = y * bytesperline_src + x;
			histogram[datasrc[pos_src]]++;
		}
	}

	cumulative_histogram[0] = histogram[0];
	for (int i = 1; i < 256; i++)
	{
		cumulative_histogram[i] = cumulative_histogram[i - 1] + histogram[i];
	}

	float equalization_function[256];
	for (int i = 0; i < 256; i++)
	{
		equalization_function[i] = ((float)cumulative_histogram[i] / total_pixels) * 255;
	}

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int pos_src = y * bytesperline_src + x;
			int pos_dst = y * bytesperline_dst + x;
			datadst[pos_dst] = equalization_function[datasrc[pos_src]];
		}
	}

	return 1;
}

int vc_gray_edge_prewitt(IVC *src, IVC *dst, float th) // th = [0.001, 1.000]
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int posX, posA, posB, posC, posD, posE, posF, posG, posH;
	int i, size;
	float histmax;
	int histthreshold;
	int sumx, sumy;
	float hist[256] = {0.0f};

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
		return 0;
	if (channels != 1)
		return 0;

	size = width * height;

	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			// PosA PosB PosC
			// PosD PosX PosE
			// PosF PosG PosH

			posA = (y - 1) * bytesperline + (x - 1) * channels;
			posB = (y - 1) * bytesperline + x * channels;
			posC = (y - 1) * bytesperline + (x + 1) * channels;
			posD = y * bytesperline + (x - 1) * channels;
			posX = y * bytesperline + x * channels;
			posE = y * bytesperline + (x + 1) * channels;
			posF = (y + 1) * bytesperline + (x - 1) * channels;
			posG = (y + 1) * bytesperline + x * channels;
			posH = (y + 1) * bytesperline + (x + 1) * channels;

			// PosA*(-1) PosB*0 PosC*(1)
			// PosD*(-1) PosX*0 PosE*(1)
			// PosF*(-1) PosG*0 PosH*(1)

			sumx = datasrc[posA] * -1;
			sumx += datasrc[posD] * -1;
			sumx += datasrc[posF] * -1;

			sumx += datasrc[posC] * +1;
			sumx += datasrc[posE] * +1;
			sumx += datasrc[posH] * +1;
			sumx = sumx / 3; // 3 = 1 + 1 + 1

			// PosA*(-1) PosB*(-1) PosC*(-1)
			// PosD*0    PosX*0    PosE*0
			// PosF*(1)  PosG*(1)  PosH*(1)

			sumy = datasrc[posA] * -1;
			sumy += datasrc[posB] * -1;
			sumy += datasrc[posC] * -1;

			sumy += datasrc[posF] * +1;
			sumy += datasrc[posG] * +1;
			sumy += datasrc[posH] * +1;
			sumy = sumy / 3; // 3 = 1 + 1 + 1

			// datadst[posX] = (unsigned char)sqrt((double)(sumx*sumx + sumy*sumy));
			datadst[posX] = (unsigned char)(sqrt((double)(sumx * sumx + sumy * sumy)) / sqrt(2.0));
			// Explicação:
			// Queremos que no caso do pior cenário, em que sumx = sumy = 255, o resultado
			// da operação se mantenha no intervalo de valores admitido, isto é, entre [0, 255].
			// Se se considerar que:
			// max = 255
			// Então,
			// sqrt(pow(max,2) + pow(max,2)) * k = max <=> sqrt(2*pow(max,2)) * k = max <=> k = max / (sqrt(2) * max) <=>
			// k = 1 / sqrt(2)
		}
	}

	// Calcular o histograma com o valor das magnitudes
	for (i = 0; i < size; i++)
	{
		hist[datadst[i]]++;
	}

	// Definir o threshold.
	// O threshold é definido pelo nível de intensidade (das magnitudes)
	// quando se atinge uma determinada percentagem de pixeis, definida pelo utilizador.
	// Por exemplo, se o parâmetro 'th' tiver valor 0.8, significa the o threshold será o
	// nível de magnitude, abaixo do qual estão pelo menos 80% dos pixeis.
	histmax = 0.0f;
	for (i = 0; i <= 255; i++)
	{
		histmax += hist[i];

		// th = Prewitt Threshold
		if (histmax >= (((float)size) * th))
			break;
	}
	histthreshold = i == 0 ? 1 : i;

	// Aplicada o threshold
	for (i = 0; i < size; i++)
	{
		if (datadst[i] >= (unsigned char)histthreshold)
			datadst[i] = 255;
		else
			datadst[i] = 0;
	}

	return 1;
}

int vc_bgr_to_rgb(IVC *srcdst)
{
	if (srcdst == NULL)
		return 0;
	if (srcdst->channels != 3)
		return 0;

	unsigned char temp;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	unsigned char *data = srcdst->data;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int pos = y * bytesperline + x * channels;

			temp = data[pos];
			data[pos] = data[pos + 2];
			data[pos + 2] = temp;
		}
	}
	return 1;
}

int vc_bgr_to_hsv(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	float r, g, b, saturation, hue, value;
	int i, size;
	int pos_src;
	float rgb_max;
	float rgb_min;

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
		return 0;
	if (channels != 3)
		return 0;

	size = width * height * channels;

	for (i = 0; i < size; i += channels)
	{
		pos_src = i;

		b = (float)data[pos_src];
		g = (float)data[pos_src + 1];
		r = (float)data[pos_src + 2];

		rgb_max = fmaxf(r, fmaxf(g, b));
		rgb_min = fminf(r, fminf(g, b));

		value = rgb_max;

		if (value == 0.0f)
		{
			hue = 0.0f;
			saturation = 0.0f;
		}
		else
		{
			saturation = ((rgb_max - rgb_min) / rgb_max) * 255.0f;

			if (saturation == 0.0f)
			{
				hue = 0.0f;
			}
			else
			{
				if ((rgb_max == r) && (g >= b))
				{
					hue = 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if ((rgb_max == r) && (b > g))
				{
					hue = 360.0f + 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if (rgb_max == g)
				{
					hue = 120.0f + 60.0f * (b - r) / (rgb_max - rgb_min);
				}
				else if (rgb_max == b)
				{
					hue = 240.0f + 60.0f * (r - g) / (rgb_max - rgb_min);
				}
			}
		}

		data[i] = (unsigned char)(hue / 360.0f * 255.0f);
		data[i + 1] = (unsigned char)(saturation);
		data[i + 2] = (unsigned char)(value);
	}

	return 1;
}

void brancoparaoriginal_trabalho(IVC *dst, IVC *src1, IVC *src2)
{
	if (src1->width != src2->width || src1->height != src2->height)
	{
		printf("ERROR -> brancoparaoriginal_trabalho():\n\tImages size mismatch!\n");
		return;
	}

	if (src1->channels != 1 || src2->channels != 3 || dst->channels != 3)
	{
		printf("ERROR -> brancoparaoriginal_trabalho():\n\tChannel number mismatch!\n");
		return;
	}

	int start = 150;
	int end = src1->width - 150;

	for (int y = 0; y < src1->height; y++)
	{
		for (int x = 0; x < src1->width; x++)
		{
			int posSrc1 = y * src1->bytesperline + x;	  // Position in the binary image
			int posSrc2 = y * src2->bytesperline + 3 * x; // Position in the 3-channel image

			if (x >= start && x < end)
			{
				if (src1->data[posSrc1] == 255)
				{
					// Set dst pixels to white if src1's corresponding pixel is 255 within specified range
					dst->data[posSrc2] = 255;
					dst->data[posSrc2 + 1] = 255;
					dst->data[posSrc2 + 2] = 255;
				}
				else
				{
					// Copy pixel from src2 to dst within specified range
					dst->data[posSrc2] = src2->data[posSrc2];
					dst->data[posSrc2 + 1] = src2->data[posSrc2 + 1];
					dst->data[posSrc2 + 2] = src2->data[posSrc2 + 2];
				}
			}
			else
			{
				// Copy pixel from src2 to dst outside specified range
				dst->data[posSrc2] = src2->data[posSrc2];
				dst->data[posSrc2 + 1] = src2->data[posSrc2 + 1];
				dst->data[posSrc2 + 2] = src2->data[posSrc2 + 2];
			}
		}
	}
}

int vc_hsv_segmentation_resistencia_corpo(IVC *src, IVC *dst)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	int byterperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char *)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float h, s, v;

	if (src->width <= 0 || src->height <= 0 || src->data == NULL)
		return 0;
	if (src->width != dst->width || src->height != dst->height)
		return 0;
	if (src->channels != 3 || dst->channels != 1)
		return 0;

	// Segmentation loop
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * byterperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			// Assuming HSV values are stored in src and are normalized [0, 255]
			h = (int)(((float)datasrc[pos_src]) / 255.0f * 360.0f);
			s = (int)(((float)datasrc[pos_src + 1]) / 255.0f * 100.0f);
			v = (int)(((float)datasrc[pos_src + 2]) / 255.0f * 100.0f);

			// Check if the pixel falls within the specified HSV range
			if ((h >= 29 && h <= 46 && s >= 31 && s <= 62 && v >= 54 && v <= 91) || (h >= 29 && h <= 38 && s >= 33 && s <= 46 && v >= 46 && v <= 56))
			{
				datadst[pos_dst] = 255; // Pixel is within range, mark as white
			}
			else
			{
				datadst[pos_dst] = 0; // Pixel is outside range, mark as black
			}
		}
	}

	return 1; // Success
}

int vc_hsv_segmentation_vermelho(IVC *src, IVC *dst)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	int byterperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char *)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float h, s, v;

	if (src->width <= 0 || src->height <= 0 || src->data == NULL)
		return 0;
	if (src->width != dst->width || src->height != dst->height)
		return 0;
	if (src->channels != 3 || dst->channels != 1)
		return 0;

	// Segmentation loop
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * byterperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			// Assuming HSV values are stored in src and are normalized [0, 255]
			h = (int)(((float)datasrc[pos_src]) / 255.0f * 360.0f);
			s = (int)(((float)datasrc[pos_src + 1]) / 255.0f * 100.0f);
			v = (int)(((float)datasrc[pos_src + 2]) / 255.0f * 100.0f);

			// Check if the pixel falls within the specified HSV range
			if ((h >= 0 && h <= 11 && s >= 45 && s <= 69 && v >= 55 && v <= 89) || (h >= 354 && h <= 360 && s >= 45 && s <= 75 && v >= 55 && v <= 75))
			{
				datadst[pos_dst] = 255; // Pixel is within range, mark as white
			}
			else
			{
				datadst[pos_dst] = 0; // Pixel is outside range, mark as black
			}
		}
	}

	return 1; // Success
}

int vc_hsv_segmentation_castanho(IVC *src, IVC *dst)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	int byterperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char *)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float h, s, v;

	if (src->width <= 0 || src->height <= 0 || src->data == NULL)
		return 0;
	if (src->width != dst->width || src->height != dst->height)
		return 0;
	if (src->channels != 3 || dst->channels != 1)
		return 0;

	// Segmentation loop
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * byterperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			// Assuming HSV values are stored in src and are normalized [0, 255]
			h = (int)(((float)datasrc[pos_src]) / 255.0f * 360.0f);
			s = (int)(((float)datasrc[pos_src + 1]) / 255.0f * 100.0f);
			v = (int)(((float)datasrc[pos_src + 2]) / 255.0f * 100.0f);

			// Check if the pixel falls within the specified HSV range
			if ((h >= 10 && h <= 30 && s >= 23 && s <= 46 && v >= 329 && v <= 51) || (h >= 9 && h <= 25 && s >= 39 && s <= 61 && v >= 38 && v <= 61))
			{
				datadst[pos_dst] = 255; // Pixel is within range, mark as white
			}
			else
			{
				datadst[pos_dst] = 0; // Pixel is outside range, mark as black
			}
		}
	}

	return 1; // Success
}

int vc_hsv_segmentation_final(IVC *src, IVC *dst)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	int byterperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char *)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float h, s, v;

	if (src->width <= 0 || src->height <= 0 || src->data == NULL)
		return 0;
	if (src->width != dst->width || src->height != dst->height)
		return 0;
	if (src->channels != 3 || dst->channels != 1)
		return 0;

	// Segmentation loop
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * byterperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			// Assuming HSV values are stored in src and are normalized [0, 255]
			h = (int)(((float)datasrc[pos_src]) / 255.0f * 360.0f);
			s = (int)(((float)datasrc[pos_src + 1]) / 255.0f * 100.0f);
			v = (int)(((float)datasrc[pos_src + 2]) / 255.0f * 100.0f);

			// Check if the pixel falls within the specified HSV range
			if ((h >= 12 && h <= 28 && s >= 25 && s <= 44 && v >= 31 && v <= 49) || (h >= 11 && h <= 23 && s >= 42 && s <= 58 && v >= 41 && v <= 58) || (h >= 0 && h <= 11 && s >= 45 && s <= 69 && v >= 55 && v <= 89) || (h >= 354 && h <= 360 && s >= 45 && s <= 75 && v >= 55 && v <= 75) || (h >= 79 && h <= 105 && s >= 28 && s <= 45 && v >= 35 && v <= 50) || (h >= 155 && h <= 200 && s >= 16 && s <= 40 && v >= 36 && v <= 52) || (h >= 35 && h <= 200 && s >= 3 && s <= 19 && v >= 15 && v <= 37) || (h >= 6 && h <= 12 && s >= 68 && s <= 78 && v >= 80 && v <= 92) || (h >= 29 && h <= 46 && s >= 31 && s <= 62 && v >= 54 && v <= 91) || (h >= 29 && h <= 38 && s >= 33 && s <= 46 && v >= 46 && v <= 56))
			{
				datadst[pos_dst] = 255; // Pixel is within range, mark as white
			}
			else
			{
				datadst[pos_dst] = 0; // Pixel is outside range, mark as black
			}
		}
	}

	return 1; // Success
}

int vc_hsv_segmentation_resistencias(IVC *src, IVC *dst)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	int byterperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char *)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float h, s, v;

	if (src->width <= 0 || src->height <= 0 || src->data == NULL)
		return 0;
	if (src->width != dst->width || src->height != dst->height)
		return 0;
	if (src->channels != 3 || dst->channels != 1)
		return 0;

	// Segmentation loop
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * byterperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			// Assuming HSV values are stored in src and are normalized [0, 255]
			h = (int)(((float)datasrc[pos_src]) / 255.0f * 360.0f);
			s = (int)(((float)datasrc[pos_src + 1]) / 255.0f * 100.0f);
			v = (int)(((float)datasrc[pos_src + 2]) / 255.0f * 100.0f);

			// Check if the pixel falls within the specified HSV range
			if ((h >= 12 && h <= 28 && s >= 25 && s <= 44 && v >= 31 && v <= 49) || (h >= 11 && h <= 23 && s >= 42 && s <= 58 && v >= 41 && v <= 58) || (h >= 0 && h <= 11 && s >= 45 && s <= 69 && v >= 55 && v <= 89) || (h >= 354 && h <= 360 && s >= 45 && s <= 75 && v >= 55 && v <= 75) || (h >= 79 && h <= 105 && s >= 28 && s <= 45 && v >= 35 && v <= 50) || (h >= 155 && h <= 200 && s >= 16 && s <= 40 && v >= 36 && v <= 52) || (h >= 35 && h <= 200 && s >= 3 && s <= 19 && v >= 15 && v <= 37) || (h >= 6 && h <= 12 && s >= 68 && s <= 78 && v >= 80 && v <= 92))
			{
				datadst[pos_dst] = 255; // Pixel is within range, mark as white
			}
			else
			{
				datadst[pos_dst] = 0; // Pixel is outside range, mark as black
			}
		}
	}

	return 1; // Success
}

void lookForWhite(IVC *src, int yc, int *widths)
{
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x;
	long int pos;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return;

	if (yc < 0 || yc >= src->height) // Ensure yc is within bounds
		return;

	int state = 0; // 0: looking for first white, 1: looking for transition to black, 2: looking for next white after black
	int count = 0;

	for (x = 0; x < width; x++)
	{
		pos = yc * bytesperline + x * channels;

		if (state == 0)
		{
			if (data[pos] == 255)
			{
				if (count < 3)
				{
					widths[count++] = x + 4;
				}

				state = 1;
			}
		}
		else if (state == 1)
		{
			if (data[pos] == 0)
			{
				state = 2;
			}
		}
		else if (state == 2)
		{
			if (data[pos] == 255)
			{
				if (count < 3)
				{
					widths[count++] = x + 4;
				}

				state = 1;
			}
		}
	}
}

int comparePixelsAtPosition(IVC *src1, IVC *src2, int yc, int width)
{
	unsigned char *data1 = (unsigned char *)src1->data;
	unsigned char *data2 = (unsigned char *)src2->data;
	int bytesperline = src1->bytesperline;
	int channels = src1->channels;

	long int pos = yc * bytesperline + width * channels;

	if (data1[pos] == data2[pos])
	{
		return 1;
	}
	return 0;
}

int vc_bgr_to_hsv(IVC *src, IVC *dst)
{
	unsigned char *data_src = (unsigned char *)src->data;
	unsigned char *data_dst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	float r, g, b, saturation, hue, value;
	int i, size;
	int pos_src, pos_dst;
	float rgb_max;
	float rgb_min;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if (channels != 3 || dst->channels != 3)
		return 0;

	size = width * height * channels;

	for (i = 0; i < size; i += channels)
	{
		pos_src = i;
		pos_dst = i;

		b = (float)data_src[pos_src];
		g = (float)data_src[pos_src + 1];
		r = (float)data_src[pos_src + 2];

		rgb_max = fmaxf(r, fmaxf(g, b));
		rgb_min = fminf(r, fminf(g, b));

		value = rgb_max;

		if (value == 0.0f)
		{
			hue = 0.0f;
			saturation = 0.0f;
		}
		else
		{
			saturation = ((rgb_max - rgb_min) / rgb_max) * 255.0f;

			if (saturation == 0.0f)
			{
				hue = 0.0f;
			}
			else
			{
				if ((rgb_max == r) && (g >= b))
				{
					hue = 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if ((rgb_max == r) && (b > g))
				{
					hue = 360.0f + 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if (rgb_max == g)
				{
					hue = 120.0f + 60.0f * (b - r) / (rgb_max - rgb_min);
				}
				else if (rgb_max == b)
				{
					hue = 240.0f + 60.0f * (r - g) / (rgb_max - rgb_min);
				}
			}
		}

		data_dst[pos_dst] = (unsigned char)(hue / 360.0f * 255.0f);
		data_dst[pos_dst + 1] = (unsigned char)(saturation);
		data_dst[pos_dst + 2] = (unsigned char)(value);
	}

	return 1;
}
