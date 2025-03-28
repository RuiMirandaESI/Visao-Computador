//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2022/2023
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define VC_DEBUG

#define MY_MAX(a, b) (a > b ? a : b)
#define MY_MIN(a, b) (a < b ? a : b)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct
{
	unsigned char *data;
	int width, height;
	int channels;	  // Bin�rio/Cinzentos=1; RGB=3
	int levels;		  // Bin�rio=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline; // width * channels
} IVC;
typedef struct
{
	int x, y, width, height; // Caixa Delimitadora (Bounding Box)
	int area;				 // �rea
	int xc, yc;				 // Centro-de-massa
	int perimeter;			 // Per�metro
	int label;				 // Etiqueta
} OVC;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROT�TIPOS DE FUN��ES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_image_free(IVC *image);

// FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);

int vc_gray_negative(IVC *srcdst);
int vc_rgb_negative(IVC *srcdst);

int vc_rgb_get_red_gray(IVC *srcdst);
int vc_rgb_get_green_gray(IVC *srcdst);
int vc_rgb_get_blue_gray(IVC *srcdst);

int vc_rgb_to_gray(IVC *src, IVC *dst);

int vc_rgb_to_hsv(IVC *srcdst);
int vc_hsv_segmentation(IVC *src, IVC *dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);

int vc_scale_gray_to_rgb(IVC *src, IVC *dst);
void combine_segmentations(IVC *dst, IVC *src1, IVC *src2);

int vc_white_pixels_quantitie(IVC *srcdst);

int vc_gray_to_binary(IVC *srcdst, int threshold);
int vc_gray_to_binary_global_mean(IVC *srcdst);

int vc_gray_to_binary_midpoint(IVC *src, IVC *dst, int kernel);
int vc_gray_to_binary_bernsen(IVC *src, IVC *dst, int kernel, int cmin);
int vc_gray_to_binary_niblack(IVC *src, IVC *dst, int kernel, float k);

int vc_binary_dilate(IVC *src, IVC *dst, int kernel);
int vc_binary_erode(IVC *src, IVC *dst, int kernel);
int vc_binary_open(IVC *src, IVC *dst, int kernel);
int vc_binary_close(IVC *src, IVC *dst, int kernel);

int vc_gray_dilate(IVC *src, IVC *dst, int kernel);
int vc_gray_erode(IVC *src, IVC *dst, int kernel);
int vc_gray_open(IVC *src, IVC *dst, int kernel);
int vc_gray_close(IVC *src, IVC *dst, int kernel);

void combine_segmentations2(IVC *dst, IVC *src1, IVC *src2);
void brancoparaoriginal(IVC *dst, IVC *src1, IVC *src2);

OVC *vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels);
int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs);

int vc_coloring(IVC *src, IVC *dst, int nlabels);

int vc_draw_boundingbox(IVC *srcdst, OVC *blobs, int nblobs);
int vc_draw_centerofgravity(IVC *srcdst, OVC *blobs, int nblobs, int s);

int vc_gray_histogram_show(IVC *src, IVC *dst);
int vc_gray_histogram_equalization(IVC *src, IVC *dst);

int vc_gray_edge_prewitt(IVC *src, IVC *dst, float th);

int vc_bgr_to_rgb(IVC *srcdst);

/*

			Para Trabalho Final

*/

int vc_bgr_to_hsv(IVC *srcdst);
void brancoparaoriginal_trabalho(IVC *dst, IVC *src1, IVC *src2);
int vc_hsv_segmentation_vermelho(IVC *src, IVC *dst);
int vc_hsv_segmentation_castanho(IVC *src, IVC *dst);
int vc_hsv_segmentation_final(IVC *src, IVC *dst);
int vc_hsv_segmentation_resistencias(IVC *src, IVC *dst);
void lookForWhite(IVC *src, int yc, int *widths);
int comparePixelsAtPosition(IVC *src1, IVC *src2, int yc, int width);
int vc_bgr_to_hsv2(IVC *src, IVC *dst);
