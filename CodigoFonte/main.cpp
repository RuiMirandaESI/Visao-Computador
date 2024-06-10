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

/**
 * @brief Cria uma nova imagem IVC para a imagem original em RGB.
 * 
 * @param video.width Largura do vídeo.
 * @param video.height Altura do vídeo.
 * @param 3 Número de canais de cor (RGB).
 * @param 255 Valor máximo dos pixels.
 * @return IVC* Ponteiro para a nova imagem IVC.
 */
IVC *imagesrc = vc_image_new(video.width, video.height, 3, 255);

/**
 * @brief Cria uma nova imagem IVC para a segmentação das resistências (binária).
 * 
 * @param video.width Largura do vídeo.
 * @param video.height Altura do vídeo.
 * @param 1 Número de canais de cor (binário).
 * @param 255 Valor máximo dos pixels.
 * @return IVC* Ponteiro para a nova imagem IVC.
 */
IVC *image_resistances_segmentation = vc_image_new(video.width, video.height, 1, 255);

/**
 * @brief Cria uma nova imagem IVC para a segmentação dilatada das resistências (binária).
 * 
 * @param video.width Largura do vídeo.
 * @param video.height Altura do vídeo.
 * @param 1 Número de canais de cor (binário).
 * @param 255 Valor máximo dos pixels.
 * @return IVC* Ponteiro para a nova imagem IVC.
 */
IVC *resistances_segmentation_dilate = vc_image_new(video.width, video.height, 1, 255);

/**
 * @brief Cria uma nova imagem IVC para resistências rotuladas (binária).
 * 
 * @param video.width Largura do vídeo.
 * @param video.height Altura do vídeo.
 * @param 1 Número de canais de cor (binário).
 * @param 255 Valor máximo dos pixels.
 * @return IVC* Ponteiro para a nova imagem IVC.
 */
IVC *resistances_labelled = vc_image_new(video.width, video.height, 1, 255);

/**
 * @brief Cria uma nova imagem IVC para uma cópia da imagem original (RGB).
 * 
 * @param video.width Largura do vídeo.
 * @param video.height Altura do vídeo.
 * @param 3 Número de canais de cor (RGB).
 * @param 255 Valor máximo dos pixels.
 * @return IVC* Ponteiro para a nova imagem IVC.
 */
IVC *imagesrc2 = vc_image_new(video.width, video.height, 3, 255);

/**
 * @brief Cria uma nova imagem IVC para a imagem final a ser exibida (RGB).
 * 
 * @param video.width Largura do vídeo.
 * @param video.height Altura do vídeo.
 * @param 3 Número de canais de cor (RGB).
 * @param 255 Valor máximo dos pixels.
 * @return IVC* Ponteiro para a nova imagem IVC.
 */
IVC *final_image = vc_image_new(video.width, video.height, 3, 255);

/**
 * @brief Cria uma nova imagem IVC para a segmentação da cor verde (binária).
 * 
 * @param video.width Largura do vídeo.
 * @param video.height Altura do vídeo.
 * @param 1 Número de canais de cor (binário).
 * @param 255 Valor máximo dos pixels.
 * @return IVC* Ponteiro para a nova imagem IVC.
 */
IVC *green_segmentation = vc_image_new(video.width, video.height, 1, 255);

/**
 * @brief Cria uma nova imagem IVC para a segmentação dilatada das bandas de resistência (binária).
 * 
 * @param video.width Largura do vídeo.
 * @param video.height Altura do vídeo.
 * @param 1 Número de canais de cor (binário).
 * @param 255 Valor máximo dos pixels.
 * @return IVC* Ponteiro para a nova imagem IVC.
 */
IVC *resistance_bands_dilate = vc_image_new(video.width, video.height, 1, 255);

/**
 * @brief Cria uma nova imagem IVC para a segmentação da cor azul (binária).
 * 
 * @param video.width Largura do vídeo.
 * @param video.height Altura do vídeo.
 * @param 1 Número de canais de cor (binário).
 * @param 255 Valor máximo dos pixels.
 * @return IVC* Ponteiro para a nova imagem IVC.
 */
IVC *blue_segmentation = vc_image_new(video.width, video.height, 1, 255);

/**
 * @brief Cria uma nova imagem IVC para a segmentação da cor vermelha (binária).
 * 
 * @param video.width Largura do vídeo.
 * @param video.height Altura do vídeo.
 * @param 1 Número de canais de cor (binário).
 * @param 255 Valor máximo dos pixels.
 * @return IVC* Ponteiro para a nova imagem IVC.
 */
IVC *red_segmentation = vc_image_new(video.width, video.height, 1, 255);

/**
 * @brief Cria uma nova imagem IVC para a segmentação da cor castanha (binária).
 * 
 * @param video.width Largura do vídeo.
 * @param video.height Altura do vídeo.
 * @param 1 Número de canais de cor (binário).
 * @param 255 Valor máximo dos pixels.
 * @return IVC* Ponteiro para a nova imagem IVC.
 */
IVC *brown_segmentation = vc_image_new(video.width, video.height, 1, 255);

/**
 * @brief Cria uma nova imagem IVC para a segmentação da cor preta (binária).
 * 
 * @param video.width Largura do vídeo.
 * @param video.height Altura do vídeo.
 * @param 1 Número de canais de cor (binário).
 * @param 255 Valor máximo dos pixels.
 * @return IVC* Ponteiro para a nova imagem IVC.
 */
IVC *black_segmentation = vc_image_new(video.width, video.height, 1, 255);

/**
 * @brief Cria uma nova imagem IVC para a segmentação da cor laranja (binária).
 * 
 * @param video.width Largura do vídeo.
 * @param video.height Altura do vídeo.
 * @param 1 Número de canais de cor (binário).
 * @param 255 Valor máximo dos pixels.
 * @return IVC* Ponteiro para a nova imagem IVC.
 */
IVC *orange_segmentation = vc_image_new(video.width, video.height, 1, 255);

/**
 * @brief Cria uma nova imagem IVC para a segmentação dilatada da cor vermelha (binária).
 * 
 * @param video.width Largura do vídeo.
 * @param video.height Altura do vídeo.
 * @param 1 Número de canais de cor (binário).
 * @param 255 Valor máximo dos pixels.
 * @return IVC* Ponteiro para a nova imagem IVC.
 */
IVC *red_segmentation_dilate = vc_image_new(video.width, video.height, 1, 255);

/**
 * @brief Cria uma nova imagem IVC para a segmentação das bandas de resistência (binária).
 * 
 * @param video.width Largura do vídeo.
 * @param video.height Altura do vídeo.
 * @param 1 Número de canais de cor (binário).
 * @param 255 Valor máximo dos pixels.
 * @return IVC* Ponteiro para a nova imagem IVC.
 */
IVC *resistance_bands = vc_image_new(video.width, video.height, 1, 255);

// Variáveis para número de blobs e índice de iteração
int nblobs, i; 

// Array de blobs
OVC *blobs; 

// Array para armazenar valores de resistências
long resistencias[6] = {0, 0, 0, 0, 0, 0}; 

// Índice para o array de resistências
int resistorIndex = 0; 

// Contagem total de resistências
int totalResistorCount = 0; 

// Matriz OpenCV para a imagem segmentada e dilatada
cv::Mat Mat_resistances_segmentation_dilate; 

// Matriz auxiliar para operações de imagem
cv::Mat Mat2;

// Matriz OpenCV para a imagem segmentada e dilatada da cor vermelha
cv::Mat mat_red_segmentation_dilate; 

while (key != 'q') 
{
	// Contagem de resistências na frame atual
	int currentResistorCount = 0; 

	/**
	 * @brief Leitura de uma frame do vídeo.
	 */
	capture.read(frame);

	/**
	 * @brief Verifica se conseguiu ler a frame.
	 */
	if (frame.empty())
		break;

	/**
	 * @brief Número da frame a processar.
	 */
	video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

	/**
	 * @brief Copia a frame para a imagem fonte.
	 */
	memcpy(imagesrc2->data, frame.data, video.width * video.height * 3);

	/**
	 * @brief Converte a imagem de BGR para HSV.
	 */
	vc_bgr_to_hsv2(imagesrc2, imagesrc);

	/**
	 * @brief Segmenta a imagem para resistências.
	 */
	vc_hsv_segmentation_final(imagesrc, image_resistances_segmentation);

	/**
	 * @brief Converte a imagem segmentada para matriz OpenCV.
	 * 
	 * @param image_resistances_segmentation Imagem IVC segmentada.
	 * @return cv::Mat Matriz OpenCV da imagem segmentada.
	 */
	cv::Mat Mat_resistances_segmentation = IVC_to_Mat1Channel(image_resistances_segmentation);

	int kernelSize = 7;

	/**
	 * @brief Cria um elemento estrutural para dilatação.
	 * 
	 * @param kernelSize Tamanho do kernel.
	 * @return cv::Mat Elemento estrutural.
	 */
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize, kernelSize));

	/**
	 * @brief Aplica dilatação na imagem segmentada.
	 * 
	 * @param Mat_resistances_segmentation Matriz OpenCV da imagem segmentada.
	 * @param Mat_resistances_segmentation_dilate Matriz OpenCV da imagem segmentada e dilatada.
	 * @param kernel Elemento estrutural.
	 */
	cv::dilate(Mat_resistances_segmentation, Mat_resistances_segmentation_dilate, kernel);

	/**
	 * @brief Copia a imagem dilatada para a estrutura IVC.
	 */
	memcpy(resistances_segmentation_dilate->data, Mat_resistances_segmentation_dilate.data, video.width * video.height);

	/**
	 * @brief Rotula os blobs na imagem.
	 * 
	 * @param resistances_segmentation_dilate Imagem segmentada e dilatada das resistências.
	 * @param resistances_labelled Imagem com resistências rotuladas.
	 * @param nblobs Número de blobs detectados.
	 * @return OVC* Array de blobs.
	 */
	blobs = vc_binary_blob_labelling(resistances_segmentation_dilate, resistances_labelled, &nblobs);

	if (blobs != NULL)
	{
		/**
		 * @brief Obtém informações dos blobs.
		 * 
		 * @param resistances_labelled Imagem com resistências rotuladas.
		 * @param blobs Array de blobs.
		 * @param nblobs Número de blobs detectados.
		 */
		vc_binary_blob_info(resistances_labelled, blobs, nblobs);

		for (i = 0; i < nblobs; i++)
		{
			/**
			 * @brief Verifica se a área do blob é suficientemente grande para ser uma resistência.
			 */
			if (blobs[i].area >= 6900)
			{
				/**
				 * @brief Desenha o centro de gravidade do blob.
				 * 
				 * @param resistances_labelled Imagem com resistências rotuladas.
				 * @param blobs[i] Blob atual.
				 * @param 1 Cor do centro de gravidade.
				 * @param 3 Espessura da linha.
				 */
				vc_draw_centerofgravity(resistances_labelled, &blobs[i], 1, 3);

				/**
				 * @brief Desenha a caixa delimitadora do blob.
				 * 
				 * @param resistances_labelled Imagem com resistências rotuladas.
				 * @param blobs[i] Blob atual.
				 * @param 1 Cor da caixa delimitadora.
				 */
				vc_draw_boundingbox(resistances_labelled, &blobs[i], 1);

				currentResistorCount++; // Incrementa a contagem de resistências na frame atual

				/**
				 * @brief Verifica se a posição do centro de gravidade está na faixa de interesse.
				 */
				if (blobs[i].yc >= 100 && blobs[i].yc < 108)
				{
					totalResistorCount++; // Incrementa a contagem total de resistências

					/**
					 * @brief Segmenta as bandas de resistência.
					 */
					vc_hsv_segmentation_resistencias(imagesrc, resistance_bands);

					/**
					 * @brief Converte a imagem segmentada para matriz OpenCV.
					 * 
					 * @param resistance_bands Imagem IVC segmentada das bandas de resistência.
					 * @return cv::Mat Matriz OpenCV da imagem segmentada das bandas.
					 */
					cv::Mat mat_resistance_bands = IVC_to_Mat1Channel(resistance_bands);

					int kernelSize2 = 8;

					/**
					 * @brief Cria um elemento estrutural para dilatação.
					 * 
					 * @param kernelSize2 Tamanho do kernel.
					 * @return cv::Mat Elemento estrutural.
					 */
					cv::Mat kernel2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize2, kernelSize2));

					/**
					 * @brief Aplica dilatação na imagem segmentada das bandas.
					 * 
					 * @param mat_resistance_bands Matriz OpenCV da imagem segmentada das bandas.
					 * @param Mat2 Matriz auxiliar para operações de imagem.
					 * @param kernel2 Elemento estrutural.
					 */
					cv::dilate(mat_resistance_bands, Mat2, kernel2);

					/**
					 * @brief Copia a imagem dilatada para a estrutura IVC.
					 */
					memcpy(resistance_bands_dilate->data, Mat2.data, video.width * video.height);

					// Array para armazenar as larguras das bandas
					int widths[3] = {0, 0, 0};

					// Array para armazenar as cores das bandas 
					int cores[3] = {0, 0, 0}; 

					/**
					 * @brief Procura por bandas brancas na imagem dilatada das resistências.
					 * 
					 * @param resistance_bands_dilate Imagem dilatada das resistências.
					 * @param blobs[i].yc Coordenada y do centro de gravidade do blob atual.
					 * @param widths Array para armazenar larguras das bandas.
					 */
					lookForWhite(resistance_bands_dilate, blobs[i].yc, widths);

					/**
					 * @brief Segmenta a cor verde na imagem.
					 * 
					 * @param imagesrc Imagem fonte em HSV.
					 * @param green_segmentation Imagem segmentada para a cor verde.
					 * @param 79 Valor mínimo do hue.
					 * @param 105 Valor máximo do hue.
					 * @param 28 Valor mínimo da saturação.
					 * @param 45 Valor máximo da saturação.
					 * @param 35 Valor mínimo do valor.
					 * @param 50 Valor máximo do valor.
					 */
					vc_hsv_segmentation(imagesrc, green_segmentation, 79, 105, 28, 45, 35, 50);

					/**
					 * @brief Segmenta a cor azul na imagem.
					 * 
					 * @param imagesrc Imagem fonte em HSV.
					 * @param blue_segmentation Imagem segmentada para a cor azul.
					 * @param 155 Valor mínimo do hue.
					 * @param 200 Valor máximo do hue.
					 * @param 16 Valor mínimo da saturação.
					 * @param 40 Valor máximo da saturação.
					 * @param 36 Valor mínimo do valor.
					 * @param 52 Valor máximo do valor.
					 */
					vc_hsv_segmentation(imagesrc, blue_segmentation, 155, 200, 16, 40, 36, 52);

					/**
					 * @brief Segmenta a cor vermelha na imagem.
					 * 
					 * @param imagesrc Imagem fonte em HSV.
					 * @param red_segmentation Imagem segmentada para a cor vermelha.
					 */
					vc_hsv_segmentation_vermelho(imagesrc, red_segmentation);

					/**
					 * @brief Converte a imagem segmentada para matriz OpenCV.
					 * 
					 * @param red_segmentation Imagem IVC segmentada para a cor vermelha.
					 * @return cv::Mat Matriz OpenCV da imagem segmentada da cor vermelha.
					 */
					cv::Mat mat_red_segmentation = IVC_to_Mat1Channel(red_segmentation);

					int kernelSize3 = 4;

					/**
					 * @brief Cria um elemento estrutural para dilatação.
					 * 
					 * @param kernelSize3 Tamanho do kernel.
					 * @return cv::Mat Elemento estrutural.
					 */
					cv::Mat kernel3 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize3, kernelSize3));

					/**
					 * @brief Aplica dilatação na imagem segmentada da cor vermelha.
					 * 
					 * @param mat_red_segmentation Matriz OpenCV da imagem segmentada da cor vermelha.
					 * @param mat_red_segmentation_dilate Matriz OpenCV da imagem segmentada e dilatada da cor vermelha.
					 * @param kernel3 Elemento estrutural.
					 */
					cv::dilate(mat_red_segmentation, mat_red_segmentation_dilate, kernel3);

					/**
					 * @brief Copia a imagem dilatada para a estrutura IVC.
					 */
					memcpy(red_segmentation_dilate->data, mat_red_segmentation_dilate.data, video.width * video.height);

					/**
					 * @brief Segmenta a cor castanha na imagem.
					 * 
					 * @param imagesrc Imagem fonte em HSV.
					 * @param brown_segmentation Imagem segmentada para a cor castanha.
					 */
					vc_hsv_segmentation_castanho(imagesrc, brown_segmentation);

					/**
					 * @brief Segmenta a cor preta na imagem.
					 * 
					 * @param imagesrc Imagem fonte em HSV.
					 * @param black_segmentation Imagem segmentada para a cor preta.
					 * @param 35 Valor mínimo do hue.
					 * @param 200 Valor máximo do hue.
					 * @param 3 Valor mínimo da saturação.
					 * @param 19 Valor máximo da saturação.
					 * @param 15 Valor mínimo do valor.
					 * @param 37 Valor máximo do valor.
					 */
					vc_hsv_segmentation(imagesrc, black_segmentation, 35, 200, 3, 19, 15, 37);

					/**
					 * @brief Segmenta a cor laranja na imagem.
					 * 
					 * @param imagesrc Imagem fonte em HSV.
					 * @param orange_segmentation Imagem segmentada para a cor laranja.
					 * @param 6 Valor mínimo do hue.
					 * @param 12 Valor máximo do hue.
					 * @param 68 Valor mínimo da saturação.
					 * @param 78 Valor máximo da saturação.
					 * @param 80 Valor mínimo do valor.
					 * @param 92 Valor máximo do valor.
					 */
					vc_hsv_segmentation(imagesrc, orange_segmentation, 6, 12, 68, 78, 80, 92);

						for (int j = 0; j < 3; j++)
						{

							// compara imagem das faixas todas com a imagem das faixas verde
							if (comparePixelsAtPosition(resistance_bands_dilate, green_segmentation, blobs[i].yc, widths[j]))
							{

								cores[j] = (j == 2) ? 100000 : 5;
							}

							// compara imagem das faixas todas com a imagem das faixas azul
							if (comparePixelsAtPosition(resistance_bands_dilate, blue_segmentation, blobs[i].yc, widths[j]))
							{

								cores[j] = (j == 2) ? 1000000 : 6;
							}

							// compara imagem das faixas todas com a imagem das faixas vermelho
							if (comparePixelsAtPosition(resistance_bands_dilate, red_segmentation_dilate, blobs[i].yc, widths[j]))
							{

								cores[j] = (j == 2) ? 100 : 2;
							}

							// compara imagem das faixas todas com a imagem das faixas castanho
							if (comparePixelsAtPosition(resistance_bands_dilate, brown_segmentation, blobs[i].yc, widths[j]))
							{

								cores[j] = (j == 2) ? 10 : 1;
							}

							// compara imagem das faixas todas com a imagem das faixas preto
							if (comparePixelsAtPosition(resistance_bands_dilate, black_segmentation, blobs[i].yc, widths[j]))
							{

								cores[j] = (j == 2) ? 1 : 0;
							}

							// compara imagem das faixas todas com a imagem das faixas laranja
							if (comparePixelsAtPosition(resistance_bands_dilate, orange_segmentation, blobs[i].yc, widths[j]))
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

	/**
	 * @brief Converte uma imagem IVC de 1 canal para uma matriz OpenCV.
	 * 
	 * @param src Imagem IVC de origem.
	 * @return cv::Mat Imagem convertida para matriz OpenCV.
	 */
	brancoparaoriginal_trabalho(final_image, resistances_labelled, imagesrc2);
	memcpy(frame.data, final_image->data, video.width * video.height * 3);

	/**
	 * @brief Texto indicando o número de resistências atuais.
	 */
	std::string textCurrent = "Numero de Resistencias Atuais: " + std::to_string(currentResistorCount);
	int fontFace = cv::FONT_HERSHEY_TRIPLEX;
	double fontScale = 0.5;
	int thickness = 1;
	cv::Point textOrgCurrent(10, 50);
	cv::putText(frame, textCurrent, textOrgCurrent, fontFace, fontScale, cv::Scalar(0, 0, 0), thickness, 8);

	/**
	 * @brief Texto a indicar o número total de resistências.
	 */
	std::string textTotal = "Numero total de Resistencias: " + std::to_string(totalResistorCount);
	int fontFacee = cv::FONT_HERSHEY_TRIPLEX;
	double fontScalee = 0.5;
	int thicknesse = 1;
	cv::Point textOrgTotal(10, 80);
	cv::putText(frame, textTotal, textOrgTotal, fontFacee, fontScalee, cv::Scalar(0, 0, 255), thicknesse, 8);

	/**
	 * @brief Texto a indicar a resistência atual em Ohms.
	 */
	int fontFaceee = cv::FONT_HERSHEY_TRIPLEX;
	double fontScaleee = 0.5;
	int thicknessee = 1;
	cv::Point textOrgOhms(10, 110);

	std::string textOhms;
	if (resistorIndex > 0)
	{
		textOhms = "Ohms da Resistencia Atual: " + std::to_string(resistencias[resistorIndex - 1]) + " ohms";
	}
	else
	{
		textOhms = "Ohms da Resistencia Atual: --";
	}
	cv::putText(frame, textOhms, cv::Point(10, 110), cv::FONT_HERSHEY_TRIPLEX, 0.5, cv::Scalar(255, 0, 0), 1, 8);

	cv::imshow("VC - VIDEO", frame);


		// cv::Mat grayMat = IVC_to_Mat1Channel(orange_segmentation);
		// cv::imshow("VC - VIDEO", grayMat);

		key = cv::waitKey(1);
	}

	vc_image_free(imagesrc);
	vc_image_free(imagesrc2);
	vc_image_free(green_segmentation);
	vc_image_free(blue_segmentation);
	vc_image_free(red_segmentation);
	vc_image_free(brown_segmentation);
	vc_image_free(black_segmentation);
	vc_image_free(orange_segmentation);
	vc_image_free(red_segmentation_dilate);
	vc_image_free(image_resistances_segmentation);
	vc_image_free(resistances_segmentation_dilate);
	vc_image_free(resistances_labelled);
	vc_image_free(final_image);
	vc_image_free(resistance_bands_dilate);
	vc_image_free(resistance_bands);

	/* Para o timer e exibe o tempo decorrido */
	vc_timer();

	/* Fecha a janela */
	cv::destroyWindow("VC - VIDEO");

	/* Fecha o ficheiro de v�deo */
	capture.release();

	return 0;
}
