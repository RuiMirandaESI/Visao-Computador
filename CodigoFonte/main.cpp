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
	IVC *imageFinal = vc_image_new(video.width, video.height, 1, 255);
	IVC *cenas = vc_image_new(video.width, video.height, 1, 255);
	IVC *cenas2 = vc_image_new(video.width, video.height, 1, 255);
	IVC *imagesrc = vc_image_new(video.width, video.height, 3, 255);
	IVC *imagemfinal = vc_image_new(video.width, video.height, 3, 255);
	IVC *image10 = vc_image_new(video.width, video.height, 1, 255);
	IVC *wow = vc_image_new(video.width, video.height, 1, 255);
	IVC *image11 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image12 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image13 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image14 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image15 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image26 = vc_image_new(video.width, video.height, 1, 255);
	IVC *imageVerde = vc_image_new(video.width, video.height, 1, 255);

	int nblobs, i;
	OVC *blobs;

	long resistencias[6] = {0, 0, 0, 0, 0, 0};
	int resistorIndex = 0;

	int totalResistorCount = 0;

	cv::Mat cenasMat;
	cv::Mat cenasMat2;
	cv::Mat image25;

	while (key != 'q')
	{
		int currentResistorCount = 0;
		/* Leitura de uma frame do v�deo */
		capture.read(frame);
		

		/* Verifica se conseguiu ler a frame */
		if (frame.empty())
			break;

		/* N�mero da frame a processar */
		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

		memcpy(image->data, frame.data, video.width * video.height * 3);
		memcpy(imagesrc->data, frame.data, video.width * video.height * 3);

		vc_bgr_to_hsv(image);

		vc_hsv_segmentation_final(image, imageFinal);

		

		cv::Mat imageFinalMat = IVC_to_Mat1Channel(imageFinal);

		int kernelSize = 7;
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize, kernelSize));
		cv::dilate(imageFinalMat, cenasMat, kernel);

		memcpy(cenas->data, cenasMat.data, video.width * video.height);

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
					currentResistorCount++;

					if (blobs[i].yc >= 100 && blobs[i].yc < 108)
					{
						totalResistorCount++;

						vc_hsv_segmentation_resistencias(image, imageVerde);

						cv::Mat imageVerdecena = IVC_to_Mat1Channel(imageVerde);

						int kernelSize2 = 8;
						cv::Mat kernel2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize2, kernelSize2));
						cv::dilate(imageVerdecena, cenasMat2, kernel2);

						memcpy(wow->data, cenasMat2.data, video.width * video.height);

						int widths[3] = {0, 0, 0};
						int cores[3] = {0, 0, 0};

						lookForWhite(wow, blobs[i].yc, widths);

						for (int j = 0; j < 3; j++)
						{

							vc_hsv_segmentation(image, image10, 79, 105, 28, 45, 35, 50);

							vc_hsv_segmentation(image, image11, 155, 200, 16, 40, 36, 52);

							vc_hsv_segmentation_vermelho(image, image12);

							cv::Mat image24 = IVC_to_Mat1Channel(image12);

							int kernelSize3 = 4;
							cv::Mat kernel3 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize3, kernelSize3));
							cv::dilate(image24, image25, kernel3);

							memcpy(image26->data, image25.data, video.width * video.height);

							vc_hsv_segmentation_castanho(image, image13);

							vc_hsv_segmentation(image, image14, 35, 200, 3, 19, 15, 37);

							vc_hsv_segmentation(image, image15, 6, 12, 68, 78, 80, 92);

							if (comparePixelsAtPosition(wow, image10, blobs[i].yc, widths[j]))
							{

								cores[j] = (j == 2) ? 100000 : 5;
							}

							if (comparePixelsAtPosition(wow, image11, blobs[i].yc, widths[j]))
							{

								cores[j] = (j == 2) ? 1000000 : 6;
							}
							if (comparePixelsAtPosition(wow, image26, blobs[i].yc, widths[j]))
							{

								cores[j] = (j == 2) ? 100 : 2;
							}
							if (comparePixelsAtPosition(wow, image13, blobs[i].yc, widths[j]))
							{

								cores[j] = (j == 2) ? 10 : 1;
							}
							if (comparePixelsAtPosition(wow, image14, blobs[i].yc, widths[j]))
							{

								cores[j] = (j == 2) ? 1 : 0;
							}
							if (comparePixelsAtPosition(wow, image15, blobs[i].yc, widths[j]))
							{

								cores[j] = (j == 2) ? 1000 : 3;
							}
						}

						if (resistorIndex < 6)
						{
							int valor1 = cores[0];
							int valor2 = cores[1];
							long multiplicador = cores[2];

							resistencias[resistorIndex] = (valor1 * 10 + valor2) * multiplicador;

							resistorIndex++;
						}
					}
				}
			}
			free(blobs);
		}

		brancoparaoriginal_trabalho(imagemfinal, cenas2, imagesrc);
		memcpy(frame.data, imagemfinal->data, video.width * video.height * 3);
		std::string textCurrent = "Numero de Resistencias Atuais: " + std::to_string(currentResistorCount);
		int fontFace = cv::FONT_HERSHEY_TRIPLEX;
		double fontScale = 0.5;
		int thickness = 1;
		cv::Point textOrgCurrent(10, 50);
		cv::putText(frame, textCurrent, textOrgCurrent, fontFace, fontScale, cv::Scalar(0, 0, 0), thickness, 8);

		std::string textTotal = "Numero total de Resistencias: " + std::to_string(totalResistorCount);
		int fontFacee = cv::FONT_HERSHEY_TRIPLEX;
		double fontScalee = 0.5;
		int thicknesse = 1;
		cv::Point textOrgTotal(10, 80);
		cv::putText(frame, textTotal, textOrgTotal, fontFacee, fontScalee, cv::Scalar(0, 0, 255), thicknesse, 8);

		int fontFaceee = cv::FONT_HERSHEY_TRIPLEX;
		double fontScaleee = 0.5;
		int thicknessee = 1;
		cv::Point textOrgOhms(10, 110);

		std::string textOhms = "Ohms da Resistencia Atual: " + std::to_string(resistencias[resistorIndex - 1]);
		cv::putText(frame, textOhms, cv::Point(10, 110), cv::FONT_HERSHEY_TRIPLEX, 0.5, cv::Scalar(255, 0, 0), 1, 8);

		cv::imshow("VC - VIDEO", frame);

		//cv::Mat grayMat = IVC_to_Mat1Channel(image10);
		//cv::imshow("VC - VIDEO", grayMat);

		key = cv::waitKey(1);
	}

	vc_image_free(image);
	vc_image_free(image10);
	vc_image_free(image11);
	vc_image_free(image12);
	vc_image_free(image13);
	vc_image_free(image14);
	vc_image_free(image15);
	vc_image_free(image26);
	vc_image_free(imageFinal);
	vc_image_free(cenas);
	vc_image_free(cenas2);
	vc_image_free(imagesrc);
	vc_image_free(imagemfinal);
	vc_image_free(wow);
	vc_image_free(imageVerde);
	
	

	/* Para o timer e exibe o tempo decorrido */
	vc_timer();

	/* Fecha a janela */
	cv::destroyWindow("VC - VIDEO");

	/* Fecha o ficheiro de v�deo */
	capture.release();

	return 0;
}
