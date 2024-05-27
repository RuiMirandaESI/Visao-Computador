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

	IVC *srcimage = vc_image_new(video.width, video.height, 3, 255);
	IVC *srcimage2 = vc_image_new(video.width, video.height, 3, 255);
	// IVC *dstimageGray1channel = vc_image_new(video.width, video.height, 1, 255);
	//  IVC *dstimageGray3channel = vc_image_new(video.width, video.height, 3, 255);
	IVC *image = vc_image_new(video.width, video.height, 1, 255);
	IVC *image2 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image3 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image4 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image5 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image6 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image7 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image8 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image9 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image10 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image11 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image12 = vc_image_new(video.width, video.height, 3, 255);
	

	while (key != 'q')
	{
		/* Leitura de uma frame do v�deo */
		capture.read(frame);

		/* Verifica se conseguiu ler a frame */
		if (frame.empty())
			break;

		/* N�mero da frame a processar */
		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

		/* Exemplo de inser��o texto na frame */
		/*
		str = std::string("RESOLUCAO: ").append(std::to_string(video.width)).append("x").append(std::to_string(video.height));
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("TOTAL DE FRAMES: ").append(std::to_string(video.ntotalframes));
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("FRAME RATE: ").append(std::to_string(video.fps));
		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("N. DA FRAME: ").append(std::to_string(video.nframe));
		cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);*/

		// Faça o seu código aqui...
		/*
		// Cria uma nova imagem IVC
		IVC image = vc_image_new(video.width, video.height, 3, 255);
		// Copia dados de imagem da estrutura cv::Mat para uma estrutura IVC
		memcpy(image->data, frame.data, video.width video.height * 3);
		// Executa uma função da nossa biblioteca vc
		vc_rgb_get_green(image);
		// Copia dados de imagem da estrutura IVC para uma estrutura cv::Mat
		memcpy(frame.data, image->data, video.width * video.height * 3);
		// Liberta a memória da imagem IVC que havia sido criada
		vc_image_free(image);
		*/
		// +++++++++++++

		memcpy(srcimage->data, frame.data, video.width * video.height * 3);
		memcpy(srcimage2->data, frame.data, video.width * video.height * 3);

		vc_bgr_to_hsv(srcimage);
		vc_hsv_segmentation_trabalho(srcimage, image, 30, 80, 30, 100, 30, 100);
		vc_hsv_segmentation_trabalho(srcimage, image2, 0, 360, 0, 40, 0, 40);	 // preto
		vc_hsv_segmentation_trabalho(srcimage, image3, 0, 15, 50, 100, 50, 100); // vermelho
		vc_hsv_segmentation_trabalho(srcimage, image4, 340, 360, 50, 100, 50, 100);
		vc_hsv_segmentation_trabalho(srcimage, image5, 100, 270, 30, 100, 30, 100); // verde e azul
		combine_segmentations_trabalho(image6, image2, image);
		combine_segmentations_trabalho(image7, image3, image6);
		combine_segmentations_trabalho(image8, image4, image7);
		combine_segmentations_trabalho(image9, image5, image8);
		// vc_binary_erode_trabalho(image9, image10, 6);

		vc_binary_dilate_trabalho(image9, image10, 6);

		
		int nblobs, i;
		OVC *blobs;
		blobs = vc_binary_blob_labelling_trabalho(image10, image11, &nblobs);
		if (blobs != NULL)
		{

			vc_binary_blob_info_trabalho(image11, blobs, nblobs);

			printf("\nNumber of labels: %d\n", nblobs);

			for (i = 0; i < nblobs; i++)
			{

				if (blobs[i].area > 5000)
				{
					vc_draw_centerofgravity(image11, &blobs[i], 1, 3);
					vc_draw_boundingbox(image11, &blobs[i], 1);
					printf("\nArea of labels: %d\n", blobs[i].area);
				}
			}
		free(blobs);
		
		}

		brancoparaoriginal_trabalho(image12, image11, srcimage2);
		

		
		// Para correr imagem final com 3 channels

		
		//vc_gray_to_rgb(image11, image12);
		memcpy(frame.data, image12->data, video.width * video.height * 3);
		cv::imshow("VC - VIDEO", frame);

		// Para correr imagem final com 1 channel

		/*
		cv::Mat grayMat = IVC_to_Mat1Channel(image12);
		cv::imshow("VC - VIDEO", grayMat);*/

		/* Sai da aplica��o, se o utilizador premir a tecla 'q' */
		key = cv::waitKey(1);
	}

	vc_image_free(srcimage);
	vc_image_free(srcimage2);
	vc_image_free(image);
	vc_image_free(image2);
	vc_image_free(image3);
	vc_image_free(image4);
	vc_image_free(image5);
	vc_image_free(image6);
	vc_image_free(image7);
	vc_image_free(image8);
	vc_image_free(image9);
	vc_image_free(image10);
	vc_image_free(image11);
	vc_image_free(image12);
	

	/* Para o timer e exibe o tempo decorrido */
	vc_timer();

	/* Fecha a janela */
	cv::destroyWindow("VC - VIDEO");

	/* Fecha o ficheiro de v�deo */
	capture.release();

	return 0;
}
