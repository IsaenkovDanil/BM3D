#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <math.h>
#include "cbm3d.h"
#include "cbm3d_wiener.h"
#include <vector>
#include <opencv2/opencv.hpp>
#include "cbm3d_wiener.h"

using namespace std;
#define YUV       0
#define YCBCR     1
#define OPP       2
#define RGB       3
int color_space_transform(
	vector<float>& img
	, const unsigned color_space
	, const unsigned width
	, const unsigned height
	, const unsigned chnls
	, const bool rgb2yuv
) {
	if (chnls == 1 || color_space == RGB)
		return EXIT_SUCCESS;

	//! Declarations
	vector<float> tmp;
	tmp.resize(chnls * width * height);
	const unsigned red = 0;
	const unsigned green = width * height;
	const unsigned blue = width * height * 2;

	//! Transformations depending on the mode
	if (color_space == YUV)
	{
		if (rgb2yuv)
		{
#pragma omp parallel for
			for (unsigned k = 0; k < width * height; k++)
			{
				//! Y
				tmp[k + red] = 0.299f * img[k + red] + 0.587f * img[k + green] + 0.114f * img[k + blue];
				//! U
				tmp[k + green] = -0.14713f * img[k + red] - 0.28886f * img[k + green] + 0.436f * img[k + blue];
				//! V
				tmp[k + blue] = 0.615f * img[k + red] - 0.51498f * img[k + green] - 0.10001f * img[k + blue];
			}
		}
		else
		{
#pragma omp parallel for
			for (unsigned k = 0; k < width * height; k++)
			{
				//! Red   channel
				tmp[k + red] = img[k + red] + 1.13983f * img[k + blue];
				//! Green channel
				tmp[k + green] = img[k + red] - 0.39465f * img[k + green] - 0.5806f * img[k + blue];
				//! Blue  channel
				tmp[k + blue] = img[k + red] + 2.03211f * img[k + green];
			}
		}
	}
	else if (color_space == YCBCR)
	{
		if (rgb2yuv)
		{
#pragma omp parallel for
			for (unsigned k = 0; k < width * height; k++)
			{
				//! Y
				tmp[k + red] = 0.299f * img[k + red] + 0.587f * img[k + green] + 0.114f * img[k + blue];
				//! U
				tmp[k + green] = -0.169f * img[k + red] - 0.331f * img[k + green] + 0.500f * img[k + blue];
				//! V
				tmp[k + blue] = 0.500f * img[k + red] - 0.419f * img[k + green] - 0.081f * img[k + blue];
			}
		}
		else
		{
#pragma omp parallel for
			for (unsigned k = 0; k < width * height; k++)
			{
				//! Red   channel
				tmp[k + red] = 1.000f * img[k + red] + 0.000f * img[k + green] + 1.402f * img[k + blue];
				//! Green channel
				tmp[k + green] = 1.000f * img[k + red] - 0.344f * img[k + green] - 0.714f * img[k + blue];
				//! Blue  channel
				tmp[k + blue] = 1.000f * img[k + red] + 1.772f * img[k + green] + 0.000f * img[k + blue];
			}
		}
	}
	else if (color_space == OPP)
	{
		if (rgb2yuv)
		{
#pragma omp parallel for
			for (unsigned k = 0; k < width * height; k++)
			{
				//! Y
				tmp[k + red] = 0.333f * img[k + red] + 0.333f * img[k + green] + 0.333f * img[k + blue];
				//! U
				tmp[k + green] = 0.500f * img[k + red] + 0.000f * img[k + green] - 0.500f * img[k + blue];
				//! V
				tmp[k + blue] = 0.250f * img[k + red] - 0.500f * img[k + green] + 0.250f * img[k + blue];
			}
		}
		else
		{
#pragma omp parallel for
			for (unsigned k = 0; k < width * height; k++)
			{
				//! Red   channel
				tmp[k + red] = 1.0f * img[k + red] + 1.0f * img[k + green] + 0.666f * img[k + blue];
				//! Green cha
				tmp[k + green] = 1.0f * img[k + red] + 0.0f * img[k + green] - 1.333f * img[k + blue];
				//! Blue  cha
				tmp[k + blue] = 1.0f * img[k + red] - 1.0f * img[k + green] + 0.666f * img[k + blue];
			}
		}
	}
	else
	{
		cout << "Wrong type of transform. Must be OPP, YUV, or YCbCr!!" << endl;
		return EXIT_FAILURE;
	}

#pragma omp parallel for
	for (unsigned k = 0; k < width * height * chnls; k++)
		img[k] = tmp[k];

	return EXIT_SUCCESS;
}
std::vector<float> imageToVector(const std::string& filename) {
	// «агрузка изображени€ в формате CV_32F (32-битные вещественные числа)
	cv::Mat image = cv::imread(filename, cv::IMREAD_UNCHANGED);
	if (image.empty()) {
		std::cerr << "Could not open or find the image\n";
		return {};
	}

	// ѕреобразование изображени€ в тип float, если это еще не сделано
	if (image.type() != CV_32F) {
		image.convertTo(image, CV_32F);
	}

	// Ќормализаци€ значений пикселей, если нужно (например, приведение к диапазону [0, 1])
	//image /= 255.0;

	// ѕеревод изображени€ в одномерный вектор
	std::vector<float> vector;
	vector.assign((float*)image.datastart, (float*)image.dataend);
	return vector;
}

double get_psnr(ImageType* img1, ImageType* img2, int pixels, ImageType vmax)
{
	double mse = 0;
	double diff;
	for (int i = 0; i < pixels; i++)
	{
		diff = (double)img1[i] - (double)img2[i];
		mse += diff * diff;
	}
	mse /= pixels;
	return (10 * log10((double)vmax * vmax / mse));
}

FILE* openfile(const char* fname, const char* mode)
{
	FILE* f = fopen(fname, mode);
	if (NULL == f)
	{
		cout << "Failed to open: " << fname << endl;
		exit(1);
	}
	return f;
}
#include <fstream>
#include <vector>
int sigma_step2 = 25;	// bigger for smoother, usually a little smaller than step1

// ‘ункци€ дл€ чтени€ изображени€ в std::vector<float>
std::vector<uint8_t> readImage(const std::string& filename, int w, int h, int chnl) {
	std::ifstream infile(filename, std::ios::binary);
	if (!infile.is_open()) {
		std::cerr << "Error: Couldn't open file " << filename << std::endl;
		return std::vector<uint8_t>(); // ¬озвращаем пустой вектор в случае ошибки
	}

	// –ассчитываем общее количество элементов
	size_t numElements = w * h * chnl;
	// —оздаем вектор дл€ хранени€ данных
	std::vector<uint8_t> imageData(numElements);

	// —читываем данные из файла в вектор
	infile.read(reinterpret_cast<char*>(imageData.data()), numElements * sizeof(uint8_t));



	return imageData; // ¬озвращаем вектор с данными изображени€
}
int countWien = 0;
int main()
{
	int w = 512, h = 512;
	int chnl = 3;			// YUV 4:0:0 or 4:4:4

	int en_bm3d_step2 = 1;	// enable step2 of bm3d
	int sigma_step1 = 36;	// здесь то же самое дл€ Y/U/V, может быть по-другому

	int frames = 1;		// кадры дл€ обработки

	FILE* gtf = openfile("test/yuv444_512x512_lena_gt.yuv", "rb");
	ImageType* gt = new ImageType[w * h * chnl];
	fread(gt, sizeof(ImageType), w * h * chnl, gtf);

	FILE* inf = openfile("test/yuv444_512x512_lena.yuv", "rb");
	FILE* ouf = openfile("test/yuv444_512x512_lena_deno.yuv", "wb");

	ImageType* noisy = new ImageType[w * h * chnl];
	ImageType* clean = new ImageType[w * h * chnl];

	BM3D* denoiser = NULL;
	if (chnl == 1) {
		// used for YUV 4:0:0
		denoiser = new BM3D(w, h, 16, 8, 3, 16, 1, 16, 1); // at present the psize must be 8
	}
	else {
		// used for YUV 4;4:4
		denoiser = new CBM3D(w, h, 16, 8, 3, 16, 1, 16, 1);
	}

	
	BM3D_WIE* denoiser_wie = NULL;
	if (chnl == 1) {
		denoiser_wie = new BM3D_WIE(w, h, 32, 8, 3, 16, 1, 16, 1); // at present the psize must be 8
	}
	else {
		denoiser_wie = new CBM3D_WIE(w, h, 32, 8, 3, 16, 1, 16, 1);
	}

	int frame = 0;
	while (frames < 0 || frame < frames)
	{
		if (fread(noisy, sizeof(ImageType), w * h * chnl, inf) != w * h * chnl) break;
		cout << "Processing frame " << frame << "..." << endl;

		denoiser->load(noisy, sigma_step1);
		denoiser->run(clean);

		cout << "noisy PSNR: " << get_psnr(noisy, gt, w * h * chnl, 255) << "    "
			<< "denoised PSNR: " << get_psnr(clean, gt, w * h * chnl, 255) << endl;

		fwrite(clean, sizeof(ImageType), w * h * chnl, ouf);

		if (en_bm3d_step2)
		{
			denoiser_wie->load(noisy, clean, sigma_step2);
			denoiser_wie->run(clean);

			cout << "noisy PSNR: " << get_psnr(noisy, gt, w * h * chnl, 255) << "    "
				<< "wiener denoised PSNR: " << get_psnr(clean, gt, w * h * chnl, 255) << endl;

			fwrite(clean, sizeof(ImageType), w * h * chnl, ouf);
		}

		cout << "Frame " << frame << " done!" << endl << endl;
		frame++;
	}


	

	delete denoiser;
	delete denoiser_wie;

	fclose(gtf);
	delete[] gt;

	fclose(inf);
	fclose(ouf);
	delete[] noisy;
	delete[] clean;




	return 0;
}