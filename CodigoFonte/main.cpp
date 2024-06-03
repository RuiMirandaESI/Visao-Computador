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
	IVC *image3 = vc_image_new(video.width, video.height, 3, 255);
	IVC *image4 = vc_image_new(video.width, video.height, 3, 255);
	/*IVC *image5 = vc_image_new(video.width, video.height, 3, 255);
	IVC *image6 = vc_image_new(video.width, video.height, 3, 255);
	IVC *image7 = vc_image_new(video.width, video.height, 3, 255);
	IVC *image8 = vc_image_new(video.width, video.height, 3, 255);
	IVC *image9 = vc_image_new(video.width, video.height, 3, 255);

	IVC *imageRGB = vc_image_new(image->width, image->height, 3, image->levels);
	IVC *dilatarimagem = vc_image_new(video.width, video.height, 3, 255);
	IVC *imagemFinal = vc_image_new(video.width, video.height, 3, 255);
	IVC *imagemFinalAux = vc_image_new(video.width, video.height, 3, 255);
	IVC *imageVerde = vc_image_new(video.width, video.height, 3, 255);
	IVC *imageAzul = vc_image_new(video.width, video.height, 3, 255);
	IVC *imagePreto = vc_image_new(video.width, video.height, 3, 255);
	IVC *imageVermelho = vc_image_new(video.width, video.height, 3, 255);
	IVC *imageCastanho = vc_image_new(video.width, video.height, 3, 255);
	IVC *imagemCoresFinal = vc_image_new(video.width, video.height, 3, 255);
	IVC *auxBlobSegmentation = vc_image_new(video.width, video.height, 3, 255);
	IVC *imageLaranja = vc_image_new(video.width, video.height, 3, 255);
	IVC *imagemBoundingBox = vc_image_new(video.width, video.height, 3, 255);
	IVC *resistenciasJuntas = vc_image_new(video.width, video.height, 1, 255);*/

	/*OVC *arrayVerde[6] = {NULL}, *arrayPreto[6] = {NULL}, *arrayVermelho[6] = {NULL}, *arrayAzul[6] = {NULL}, *arrayCastanho[6] = {NULL}, *arrayResistencia[6] = {NULL}, *arrayLaranja[6] = {NULL};
	OVC *blobSegmentation = NULL, *blobVerde = NULL, *blobPreto = NULL, *blobVermelho = NULL, *blobAzul = NULL, *blobCastanho = NULL, *blobCoresJuntas = NULL, *blobLaranja = NULL;
	*/
	while (key != 'q')
	{
		/* Leitura de uma frame do v�deo */
		capture.read(frame);
		int iteradorAzul, iteradorVermelho, iteradorCastanho, iteradorPreto, iteradorVerde, iteradorLaranja;
		int nBlobsSegmentation, nBlobsSegVerde, nBlobsSegPreto, nBlobsSegVermelho, nBlobsSegAzul, nBlobsSegCastanho;

		/* Verifica se conseguiu ler a frame */
		if (frame.empty())
			break;

		/* N�mero da frame a processar */
		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

		memcpy(image->data, frame.data, video.width * video.height * 3);

		vc_bgr_to_hsv(image);

		// corpo resistencias
		vc_hsv_segmentation(image, image2, 29, 46, 31, 62, 54, 91);
		vc_hsv_segmentation(image, image3, 29, 38, 33, 46, 46, 56);
		combine_segmentations(image4, image2, image3);

		cv::Mat grayMat = IVC_to_Mat1Channel(image4);
		cv::imshow("VC - VIDEO", grayMat);

		key = cv::waitKey(1);
	}

	vc_image_free(image);
	vc_image_free(image2);
	vc_image_free(image3);
	vc_image_free(image4);

	/* Para o timer e exibe o tempo decorrido */
	vc_timer();

	/* Fecha a janela */
	cv::destroyWindow("VC - VIDEO");

	/* Fecha o ficheiro de v�deo */
	capture.release();

	return 0;
}
