#include <iostream>
#include <string>
#include <chrono>
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>

extern "C"
{
#include "vc.h"
}

void vc_timer(void)
{
	static bool running = false;
	static std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();

	if (!running)
	{
		running = true;
	}
	else
	{
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		std::chrono::steady_clock::duration elapsedTime = currentTime - previousTime;

		// Tempo em segundos.
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(elapsedTime);
		double nseconds = time_span.count();

		std::cout << "Tempo decorrido: " << nseconds << "segundos" << std::endl;
		std::cout << "Pressione qualquer tecla para continuar...\n";
		std::cin.get();
	}
}

cv::Mat IVC_to_Mat1Channel(IVC *src)
{
	return cv::Mat(src->height, src->width, CV_8UC1, src->data);
}

int main(void)
{
	// V�deo
	char videofile[20] = "video_resistors.mp4";
	cv::VideoCapture capture;
	struct
	{
		int width, height;
		int ntotalframes;
		int fps;
		int nframe;
	} video;
	// Outros
	std::string str;
	int key = 0;

	/* Leitura de v�deo de um ficheiro */
	/* NOTA IMPORTANTE:
	O ficheiro video.avi dever� estar localizado no mesmo direct�rio que o ficheiro de c�digo fonte.
	*/
	capture.open(videofile);

	/* Em alternativa, abrir captura de v�deo pela Webcam #0 */
	// capture.open(0, cv::CAP_DSHOW); // Pode-se utilizar apenas capture.open(0);

	/* Verifica se foi poss�vel abrir o ficheiro de v�deo */
	if (!capture.isOpened())
	{
		std::cerr << "Erro ao abrir o ficheiro de v�deo!\n";
		return 1;
	}

	/* N�mero total de frames no v�deo */
	video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
	/* Frame rate do v�deo */
	video.fps = (int)capture.get(cv::CAP_PROP_FPS);
	/* Resolu��o do v�deo */
	video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
	video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

	/* Cria uma janela para exibir o v�deo */
	cv::namedWindow("VC - VIDEO", cv::WINDOW_AUTOSIZE);

	/* Inicia o timer */
	vc_timer();

	cv::Mat frame;

	IVC *image = vc_image_new(video.width, video.height, 3, 255);
	IVC *image2 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image3 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image4 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image5 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image6 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image7 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image8 = vc_image_new(video.width, video.height, 1, 255);
	IVC *imageFinal = vc_image_new(video.width, video.height, 1, 255);
	IVC *coresResistenciaJuntas = vc_image_new(video.width, video.height, 1, 255);
	IVC *cenas = vc_image_new(video.width, video.height, 1, 255);
	IVC *cenas2 = vc_image_new(video.width, video.height, 1, 255);
	IVC *imagesrc = vc_image_new(video.width, video.height, 3, 255);
	IVC *imagemfinal = vc_image_new(video.width, video.height, 3, 255);
	// IVC *image9 = vc_image_new(video.width, video.height, 3, 255);

	// IVC *imageRGB = vc_image_new(image->width, image->height, 3, image->levels);
	// IVC *dilatarimagem = vc_image_new(video.width, video.height, 3, 255);
	// IVC *imagemFinal = vc_image_new(video.width, video.height, 3, 255);
	// IVC *imagemFinalAux = vc_image_new(video.width, video.height, 3, 255);
	IVC *imageVerde = vc_image_new(video.width, video.height, 1, 255);
	IVC *imageAzul = vc_image_new(video.width, video.height, 1, 255);
	// IVC *imagePreto = vc_image_new(video.width, video.height, 3, 255);
	// IVC *imageVermelho = vc_image_new(video.width, video.height, 3, 255);
	// IVC *imageCastanho = vc_image_new(video.width, video.height, 3, 255);
	// IVC *imagemCoresFinal = vc_image_new(video.width, video.height, 3, 255);
	// IVC *auxBlobSegmentation = vc_image_new(video.width, video.height, 3, 255);
	// IVC *imageLaranja = vc_image_new(video.width, video.height, 3, 255);
	// IVC *imagemBoundingBox = vc_image_new(video.width, video.height, 3, 255);
	// IVC *resistenciasJuntas = vc_image_new(video.width, video.height, 1, 255);

	// Para o verde
	int nBlobsSegVerde, iteradorVerde;
	OVC *blobVerde;
	OVC *arrayVerde[6];

	// Para o azul
	int nBlobsSegAzul, iteradorAzul;
	OVC *blobAzul;
	OVC *arrayAzul[6];

	int nblobs, i;
	OVC *blobs;

	/*OVC *arrayVerde[6] = {NULL}, *arrayPreto[6] = {NULL}, *arrayVermelho[6] = {NULL}, *arrayAzul[6] = {NULL}, *arrayCastanho[6] = {NULL}, *arrayResistencia[6] = {NULL}, *arrayLaranja[6] = {NULL};
	OVC *blobSegmentation = NULL, *blobVerde = NULL, *blobPreto = NULL, *blobVermelho = NULL, *blobAzul = NULL, *blobCastanho = NULL, *blobCoresJuntas = NULL, *blobLaranja = NULL;
	*/
	while (key != 'q')
	{
		/* Leitura de uma frame do v�deo */
		capture.read(frame);
		int iteradorAzul, iteradorVermelho, iteradorCastanho, iteradorPreto, iteradorLaranja;
		int nBlobsSegmentation, nBlobsSegPreto, nBlobsSegVermelho, nBlobsSegAzul, nBlobsSegCastanho;

		/* Verifica se conseguiu ler a frame */
		if (frame.empty())
			break;

		/* N�mero da frame a processar */
		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

		memcpy(image->data, frame.data, video.width * video.height * 3);
		memcpy(imagesrc->data, frame.data, video.width * video.height * 3);

		vc_bgr_to_hsv(image);

		// corpo resistencias
		// vc_hsv_segmentation(image, image2, 29, 46, 31, 62, 54, 91);
		// vc_hsv_segmentation(image, image3, 29, 38, 33, 46, 46, 56);
		// combine_segmentations(image4, image2, image3);
		vc_hsv_segmentation_resistencia_corpo(image, image2);

		// verde
		vc_hsv_segmentation(image, image3, 79, 105, 28, 45, 35, 50);

		nBlobsSegVerde = 0;
		blobVerde = vc_binary_blob_labelling(image3, imageVerde, &nBlobsSegVerde);
		iteradorVerde = 0;
		if (blobVerde != NULL)
		{
			vc_binary_blob_info(imageVerde, blobVerde, nBlobsSegVerde);
			for (int i = 0; i < nBlobsSegVerde; i++)
			{
				// blobVerde[i].valorCor = 5;
				if (blobVerde[i].area >= 125 && blobVerde[i].area <= 1000)
				{
					arrayVerde[iteradorVerde] = (OVC *)malloc(sizeof(OVC));
					memcpy(arrayVerde[iteradorVerde], &blobVerde[i], sizeof(OVC));
					iteradorVerde++;
				}
			}
		}

		// azul
		vc_hsv_segmentation(image, image4, 155, 200, 16, 40, 36, 52);

		nBlobsSegAzul = 0;
		blobAzul = vc_binary_blob_labelling(image4, imageAzul, &nBlobsSegAzul);
		iteradorAzul = 0;
		if (blobAzul != NULL)
		{
			vc_binary_blob_info(imageAzul, blobAzul, nBlobsSegAzul);
			for (int i = 0; i < nBlobsSegAzul; i++)
			{

				if (blobAzul[i].area >= 90 && blobAzul[i].area <= 1000)
				{
					arrayAzul[iteradorAzul] = (OVC *)malloc(sizeof(OVC));
					memcpy(arrayAzul[iteradorAzul], &blobAzul[i], sizeof(OVC));
					iteradorAzul++;
				}
			}
		}

		// vermelha
		// vc_hsv_segmentation(image, image2, 0, 11, 45, 69, 55, 89);
		// vc_hsv_segmentation(image, image3, 354, 360, 45, 75, 55, 75);
		// combine_segmentations(image4, image2, image3);
		vc_hsv_segmentation_vermelho(image, image5);

		// castanho
		// vc_hsv_segmentation(image, image2, 12, 28, 25, 44, 31, 49);
		// vc_hsv_segmentation(image, image3, 11, 23, 42, 58, 41, 58);
		vc_hsv_segmentation_castanho(image, image6);

		// preto
		vc_hsv_segmentation(image, image7, 35, 200, 3, 19, 15, 37);

		// laranja
		vc_hsv_segmentation(image, image8, 6, 12, 68, 78, 80, 92);

		// final e resistencias
		vc_hsv_segmentation_final(image, imageFinal);

		vc_hsv_segmentation_resistencias(image, coresResistenciaJuntas);

		vc_binary_dilate(imageFinal, cenas, 7);

		blobs = vc_binary_blob_labelling(cenas, cenas2, &nblobs);
		if (blobs != NULL)
		{

			vc_binary_blob_info(cenas2, blobs, nblobs);

			for (i = 0; i < nblobs; i++)
			{

				if (blobs[i].area >= 6900)
				{
					vc_draw_centerofgravity(cenas2, &blobs[i], 1, 3);
					vc_draw_boundingbox(cenas2, &blobs[i], 1);
					printf("\nArea of labels: %d\n", blobs[i].area);
				}
			}
			free(blobs);
		}

		brancoparaoriginal_trabalho(imagemfinal, cenas2, imagesrc);

		memcpy(frame.data, imagemfinal->data, video.width * video.height * 3);
		cv::imshow("VC - VIDEO", frame);

		//cv::Mat grayMat = IVC_to_Mat1Channel(cenas2);
		//cv::imshow("VC - VIDEO", grayMat);

		key = cv::waitKey(1);
	}

	vc_image_free(image);
	vc_image_free(image2);
	vc_image_free(image3);
	vc_image_free(image4);
	vc_image_free(image5);
	vc_image_free(image6);
	vc_image_free(image7);
	vc_image_free(image8);
	vc_image_free(coresResistenciaJuntas);
	vc_image_free(imageFinal);
	vc_image_free(cenas);
	vc_image_free(cenas2);
	vc_image_free(imagesrc);
	vc_image_free(imagemfinal);

	/* Para o timer e exibe o tempo decorrido */
	vc_timer();

	/* Fecha a janela */
	cv::destroyWindow("VC - VIDEO");

	/* Fecha o ficheiro de v�deo */
	capture.release();

	return 0;
}
