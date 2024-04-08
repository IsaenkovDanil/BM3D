#include "transform.h"
#include "global_define.h"
#include <vector>
#include <fftw3.h>
#include <math.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <iostream>

#define NN 8 // ������ ����� DCT
using namespace std;

/* Inplace-���������� ������� 2D �������-�������������� 8x8 Bior-1.5.
 * ������� � ������ ������, � ����� � ������� ������� (��������) �������� ����� 8x8 ����������� ������� 8x8, �������������� ����.
 * [ 64, 64, 11, -11, 0, 0, -11, 11] [x0]
 * [-11, 11, 64, 64, 11, -11, 0, 0] [x1]
 * 1 [ 0, 0, -11, 11, 64, 64, 11, -11] [x2]
 * ----------- * [ 11, -11, 0, 0, -11, 11, 64, 64] [x3]
 * 64*sqrt(2) [ 64, -64, 0, 0, 0, 0, 0, 0, 0] [x4]
 * [ 0, 0, 64, -64, 0, 0, 0, 0] [x5]
 * [ 0, 0, 0, 0, 64, -64, 0, 0] [x6]
 * [ 0, 0, 0, 0, 0, 0, 64, -64] [x7]
 *
 * ��-������, ����������� ���� ������� 4x4 ����������� � ������ ������, � ����� � ������� ������� ������-������ ����� 4x4
 * ��������� ���������� ������� ����, � ������� �������� ��� ��������� (����� ���������������, ���� ����������).
 * [1, 1, 0, 0] [x0]
 * [0, 0, 1, 1] [x1]
 * 1/sqrt(2) * [1, -1, 0, 0] [x2]
 * [0, 0, 1, -1] [x3]
 *
 * �������, ����������� ���� ������� 2x2 ����������� � ������ ������, � ����� � ������� ������� ������ �������� ������� 2x2
 * ������ ������� ����, � ������� �������� ��� ��������� (����� ���������������, ���� ����������).
 * [1, 1] [x0]
 * 1/sqrt(2) * [1, -1] [x1]
 */
void Iinplace_forward_bior15_2d_8x8(float* src)
{
	static float buf[4];
	float* org_src = src;

	// horizontal transform of the 1st step (8x8)
	for (int i = 0; i < 8; i++)
	{
		// row (i)
		buf[0] = src[0] - src[1];
		buf[1] = src[2] - src[3];
		buf[2] = src[4] - src[5];
		buf[3] = src[6] - src[7];

		src[0] = 64 * (src[0] + src[1]) + 11 * (buf[1] - buf[3]);
		src[1] = 64 * (src[2] + src[3]) + 11 * (buf[2] - buf[0]);
		src[2] = 64 * (src[4] + src[5]) + 11 * (buf[3] - buf[1]);
		src[3] = 64 * (src[6] + src[7]) + 11 * (buf[0] - buf[2]);

		src[4] = buf[0] * 64;
		src[5] = buf[1] * 64;
		src[6] = buf[2] * 64;
		src[7] = buf[3] * 64;
		src += 8;
	}

	// vertical transform of the 1st step (8x8)
	src = org_src;
	for (int j = 0; j < 8; j++)
	{
		// row (i)
		buf[0] = src[0 * 8 + j] - src[1 * 8 + j];
		buf[1] = src[2 * 8 + j] - src[3 * 8 + j];
		buf[2] = src[4 * 8 + j] - src[5 * 8 + j];
		buf[3] = src[6 * 8 + j] - src[7 * 8 + j];

		src[0 * 8 + j] = 64 * (src[0 * 8 + j] + src[1 * 8 + j]) + 11 * (buf[1] - buf[3]);
		src[1 * 8 + j] = 64 * (src[2 * 8 + j] + src[3 * 8 + j]) + 11 * (buf[2] - buf[0]);
		src[2 * 8 + j] = 64 * (src[4 * 8 + j] + src[5 * 8 + j]) + 11 * (buf[3] - buf[1]);
		src[3 * 8 + j] = 64 * (src[6 * 8 + j] + src[7 * 8 + j]) + 11 * (buf[0] - buf[2]);

		src[4 * 8 + j] = buf[0] * 64;
		src[5 * 8 + j] = buf[1] * 64;
		src[6 * 8 + j] = buf[2] * 64;
		src[7 * 8 + j] = buf[3] * 64;
	}

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++) {
			src[8 * i + j] /= 8192.f;	// (64 * 2^0.5)^2, normalization
		}
	}

	// horizontal transform of the 2nd step (4x4)
	for (int i = 0; i < 4; i++)
	{
		buf[0] = src[0] - src[1];
		buf[1] = src[2] - src[3];
		src[0] = src[0] + src[1];
		src[1] = src[2] + src[3];
		src[2] = buf[0];
		src[3] = buf[1];
		src += 8;
	}

	// vertical transform of the 2nd step (4x4)
	src = org_src;
	for (int j = 0; j < 4; j++)
	{
		buf[0] = src[0 * 8 + j] - src[1 * 8 + j];
		buf[1] = src[2 * 8 + j] - src[3 * 8 + j];
		src[0 * 8 + j] = src[0 * 8 + j] + src[1 * 8 + j];
		src[1 * 8 + j] = src[2 * 8 + j] + src[3 * 8 + j];
		src[2 * 8 + j] = buf[0];
		src[3 * 8 + j] = buf[1];
	}

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++) {
			src[8 * i + j] /= 2.f;	// (2^0.5)^2, normalization
		}
	}

	// horizontal transform of the 3rd step (2x2)
	for (int i = 0; i < 2; i++)
	{
		buf[0] = src[0] - src[1];
		src[0] = src[0] + src[1];
		src[1] = buf[0];
		src += 8;
	}

	// vertical transform of the 3rd step (2x2)
	src = org_src;
	for (int j = 0; j < 2; j++)
	{
		buf[0] = src[0 * 8 + j] - src[1 * 8 + j];
		src[0 * 8 + j] = src[0 * 8 + j] + src[1 * 8 + j];
		src[1 * 8 + j] = buf[0];
	}

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++) {
			src[8 * i + j] /= 2.f;	// (2^0.5)^2, normalization
		}
	}
}

/* Inplace-���������� ��������� 2D �������-�������������� 8x8 Bior-1.5.
 * ������� � ������� �������, � ����� � ������ ������ (��������) ����������� ������� 2x2, ����������� ����.
 * ������ �������� ����� 2x2 �������� �������, � ������� �������� ��� ��������� (����� ���������������, ���� ����������).
 * �������� ��������, ��� �������� ���� 2x2 ������� �������� ����� ������� ���� 2x2 �����.
 * [1, 1] [x0]
 * 1/sqrt(2) * [1, -1] [x1]
 *
 * ��-������, ������� 4x4 ����������� � ������� �������, � ����� � ������ ������ ������ �������� ����� 4x4
 * ������� ������ ����� ������� ����, � ������� �������� ��� ��������� (����� ���������������, ���� ����������).
 * [1, 0, 1, 0] [x0]
 * [1, 0, -1, 0] [x1]
 * 1/sqrt(2) * [0, 1, 0, 1] [x2]
 * [0, 1, 0, -1] [x3]
 *
 * �������, ������� 8x8 ����������� � ������� �������, � ����� � ������ ������
 * ����� �������� ����� ����� ������� ����.
 * [ 64, 0, 0, 0, 64, -11, 0, 11] [x0]
 * [ 64, 0, 0, 0, -64, -11, 0, 11] [x1]
 * 1 [ 0, 64, 0, 0, 11, 64, -11, 0] [x2]
 * ----------- * [ 0, 64, 0, 0, 11, -64, -11, 0] [x3]
 * 64*sqrt(2) [ 0., 0, 64, 0, 0, 11, 64, -11] [x4]
 * [ 0., 0, 64, 0, 0, 11, -64, -11] [x5]
 * [ 0, 0, 0, 64, -11, 0, 11, 64] [x6]
 * [ 0, 0, 0, 64, -11, 0, 11, -64] [x7]
 */
void Iinplace_backward_bior15_2d_8x8(float* src)
{
	static float buf[4];
	float* org_src = src;

	// vertical transform of the 1st step (2x2)
	for (int j = 0; j < 2; j++)
	{
		buf[0] = src[0 * 8 + j] - src[1 * 8 + j];
		src[0 * 8 + j] = src[0 * 8 + j] + src[1 * 8 + j];
		src[1 * 8 + j] = buf[0];
	}

	// horizontal transform of the 1st step (2x2)
	for (int i = 0; i < 2; i++)
	{
		buf[0] = src[0] - src[1];
		src[0] = src[0] + src[1];
		src[1] = buf[0];
		src += 8;
	}
	src = org_src;

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++) {
			src[8 * i + j] /= 2.f;	// (2^0.5)^2, normalization
		}
	}

	// vertical transform of the 2nd step (4x4)
	for (int j = 0; j < 4; j++)
	{
		buf[0] = src[0 * 8 + j] - src[2 * 8 + j];
		buf[1] = src[1 * 8 + j] - src[3 * 8 + j];
		src[0 * 8 + j] = src[0 * 8 + j] + src[2 * 8 + j];
		src[2 * 8 + j] = src[1 * 8 + j] + src[3 * 8 + j];
		src[1 * 8 + j] = buf[0];
		src[3 * 8 + j] = buf[1];
	}

	// horizontal transform of the 2nd step (4x4)
	for (int i = 0; i < 4; i++)
	{
		buf[0] = src[0] - src[2];
		buf[1] = src[1] - src[3];
		src[0] = src[0] + src[2];
		src[2] = src[1] + src[3];
		src[1] = buf[0];
		src[3] = buf[1];
		src += 8;
	}
	src = org_src;

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			src[8 * i + j] /= 2.f;	// (2^0.5)^2, normalization

	// vertical transform of the 3rd step (8x8)
	for (int j = 0; j < 8; j++)
	{
		buf[0] = (src[0 * 8 + j] - src[4 * 8 + j]) * 64;
		buf[1] = (src[1 * 8 + j] - src[5 * 8 + j]) * 64;
		buf[2] = (src[2 * 8 + j] - src[6 * 8 + j]) * 64;
		buf[3] = (src[3 * 8 + j] - src[7 * 8 + j]) * 64;

		src[0 * 8 + j] = (src[0 * 8 + j] + src[4 * 8 + j]) * 64;
		src[1 * 8 + j] = (src[1 * 8 + j] + src[5 * 8 + j]) * 64;
		src[2 * 8 + j] = (src[2 * 8 + j] + src[6 * 8 + j]) * 64;
		src[3 * 8 + j] = (src[3 * 8 + j] + src[7 * 8 + j]) * 64;

		src[5 * 8 + j] = (src[5 * 8 + j] - src[7 * 8 + j]) * 11;
		src[6 * 8 + j] = (src[6 * 8 + j] - src[4 * 8 + j]) * 11;

		src[4 * 8 + j] = src[5 * 8 + j];
		src[5 * 8 + j] = buf[2] + src[5 * 8 + j];
		src[7 * 8 + j] = buf[3] + src[6 * 8 + j];

		src[0 * 8 + j] = src[0 * 8 + j] - src[4 * 8 + j];

		buf[2] = src[1 * 8 + j];
		buf[3] = src[2 * 8 + j];
		src[1 * 8 + j] = buf[0] - src[4 * 8 + j];
		src[2 * 8 + j] = buf[2] - src[6 * 8 + j];

		buf[0] = src[3 * 8 + j];
		src[3 * 8 + j] = buf[1] - src[6 * 8 + j];
		src[4 * 8 + j] = buf[3] + src[4 * 8 + j];
		src[6 * 8 + j] = buf[0] + src[6 * 8 + j];
	}

	// horizontal transform of the 3rd step (8x8)
	for (int i = 0; i < 8; i++)
	{
		buf[0] = (src[0] - src[4]) * 64;
		src[0] = (src[0] + src[4]) * 64;
		buf[1] = (src[1] - src[5]) * 64;
		src[1] = (src[1] + src[5]) * 64;
		buf[2] = (src[2] - src[6]) * 64;
		src[2] = (src[2] + src[6]) * 64;
		buf[3] = (src[3] - src[7]) * 64;
		src[3] = (src[3] + src[7]) * 64;

		src[5] = (src[5] - src[7]) * 11;
		src[6] = (src[6] - src[4]) * 11;
		src[4] = src[5];
		src[5] = buf[2] + src[5];
		src[7] = buf[3] + src[6];
		src[0] = src[0] - src[4];

		buf[2] = src[1];
		buf[3] = src[2];
		src[1] = buf[0] - src[4];
		src[2] = buf[2] - src[6];
		buf[0] = src[3];
		src[3] = buf[1] - src[6];
		src[4] = buf[3] + src[4];
		src[6] = buf[0] + src[6];
		src += 8;
	}
	src = org_src;

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++) {
			src[8 * i + j] /= 8192.f;	// (64 * 2^0.5)^2, normalization
		}
	}
}

void per_ext_ind(
	vector<unsigned>& ind_per
	, const unsigned N
	, const unsigned L
) {
	for (unsigned k = 0; k < N; k++)
		ind_per[k + L] = k;

	int ind1 = (N - L);
	while (ind1 < 0)
		ind1 += N;
	unsigned ind2 = 0;
	unsigned k = 0;
	while (k < L)
	{
		ind_per[k] = (unsigned)ind1;
		ind_per[k + L + N] = ind2;
		ind1 = ((unsigned)ind1 < N - 1 ? (unsigned)ind1 + 1 : 0);
		ind2 = (ind2 < N - 1 ? ind2 + 1 : 0);
		k++;
	}
}

void bior_2d_forward(
	vector<float> const& input
	, vector<float>& output
	, const unsigned N
	, const unsigned d_i
	, const unsigned r_i
	, const unsigned d_o
	, vector<float> const& lpd
	, vector<float> const& hpd
) {
#pragma omp parallel for num_threads(USE_THREADS_NUM)
	for (unsigned i = 0; i < N; i++)
		for (unsigned j = 0; j < N; j++)
			output[i * N + j + d_o] = input[i * r_i + j + d_i];

	const unsigned iter_max = log2(N);
	unsigned N_1 = N;
	unsigned N_2 = N / 2;
	const unsigned S_1 = lpd.size();
	const unsigned S_2 = S_1 / 2 - 1;
#pragma omp parallel for num_threads(USE_THREADS_NUM)

	for (unsigned iter = 0; iter < iter_max; iter++)
	{
		//! Periodic extension index initialization
		vector<float> tmp(N_1 + 2 * S_2);
		vector<unsigned> ind_per(N_1 + 2 * S_2);
		per_ext_ind(ind_per, N_1, S_2);

		//! Implementing row filtering
		for (unsigned i = 0; i < N_1; i++)
		{
#pragma omp parallel for num_threads(USE_THREADS_NUM)

			//! ������������� ���������� ������� � ����
			for (unsigned j = 0; j < tmp.size(); j++)
				tmp[j] = output[d_o + i * N + ind_per[j]];
#pragma omp parallel for num_threads(USE_THREADS_NUM)

			//! Low and High frequencies filtering
			for (unsigned j = 0; j < N_2; j++)
			{
				float v_l = 0.0f, v_h = 0.0f;
				for (unsigned k = 0; k < S_1; k++)
				{
					v_l += tmp[k + j * 2] * lpd[k];
					v_h += tmp[k + j * 2] * hpd[k];
				}
				output[d_o + i * N + j] = v_l;
				output[d_o + i * N + j + N_2] = v_h;
			}
		}
#pragma omp parallel for num_threads(USE_THREADS_NUM)

		//! Implementing column filtering
		for (unsigned j = 0; j < N_1; j++)
		{
			//! Periodic extension of the signal in column
			for (unsigned i = 0; i < tmp.size(); i++)
				tmp[i] = output[d_o + j + ind_per[i] * N];
#pragma omp parallel for num_threads(USE_THREADS_NUM)

			//! Low and High frequencies filtering
			for (unsigned i = 0; i < N_2; i++)
			{
				float v_l = 0.0f, v_h = 0.0f;
				for (unsigned k = 0; k < S_1; k++)
				{
					v_l += tmp[k + i * 2] * lpd[k];
					v_h += tmp[k + i * 2] * hpd[k];
				}
				output[d_o + j + i * N] = v_l;
				output[d_o + j + (i + N_2) * N] = v_h;
			}
		}

		//! Sizes update
		N_1 /= 2;
		N_2 /= 2;
	}
}

void bior15_coef(
	vector<float>& lp1
	, vector<float>& hp1
	, vector<float>& lp2
	, vector<float>& hp2
) {
	const float coef_norm = 1.f / (sqrtf(2.f) * 128.f);
	const float sqrt2_inv = 1.f / sqrtf(2.f);

	lp1.resize(10);
	lp1[0] = 3.f;
	lp1[1] = -3.f;
	lp1[2] = -22.f;
	lp1[3] = 22.f;
	lp1[4] = 128.f;
	lp1[5] = 128.f;
	lp1[6] = 22.f;
	lp1[7] = -22.f;
	lp1[8] = -3.f;
	lp1[9] = 3.f;

	hp1.resize(10);
	hp1[0] = 0.f;
	hp1[1] = 0.f;
	hp1[2] = 0.f;
	hp1[3] = 0.f;
	hp1[4] = -sqrt2_inv;
	hp1[5] = sqrt2_inv;
	hp1[6] = 0.f;
	hp1[7] = 0.f;
	hp1[8] = 0.f;
	hp1[9] = 0.f;

	lp2.resize(10);
	lp2[0] = 0.f;
	lp2[1] = 0.f;
	lp2[2] = 0.f;
	lp2[3] = 0.f;
	lp2[4] = sqrt2_inv;
	lp2[5] = sqrt2_inv;
	lp2[6] = 0.f;
	lp2[7] = 0.f;
	lp2[8] = 0.f;
	lp2[9] = 0.f;

	hp2.resize(10);
	hp2[0] = 3.f;
	hp2[1] = 3.f;
	hp2[2] = -22.f;
	hp2[3] = -22.f;
	hp2[4] = 128.f;
	hp2[5] = -128.f;
	hp2[6] = 22.f;
	hp2[7] = 22.f;
	hp2[8] = -3.f;
	hp2[9] = -3.f;

	for (unsigned k = 0; k < 10; k++)
	{
		lp1[k] *= coef_norm;
		hp2[k] *= coef_norm;
	}
}
void inplace_forward_bior15_2d_8x8(float* src)
{
	std::vector<float> lpd, hpd, lpr, hpr;

	bior15_coef(lpd, hpd, lpr, hpr);
	std::vector<float> out(64, 0);
	std::vector<float> value;
#pragma omp parallel for num_threads(USE_THREADS_NUM)

	for (size_t i = 0; i < 64; i++)
	{
		value.push_back(src[i]);

	}


	bior_2d_forward(value, out, 8, 0, 0, 0, lpd, hpd);
}

/* Inplace-���������� ��������� 2D �������-�������������� 8x8 Bior-1.5.
 * ������� � ������� �������, � ����� � ������ ������ (��������) ����������� ������� 2x2, ����������� ����.
 * ������ �������� ����� 2x2 �������� �������, � ������� �������� ��� ��������� (����� ���������������, ���� ����������).
 * �������� ��������, ��� �������� ���� 2x2 ������� �������� ����� ������� ���� 2x2 �����.
 * [1, 1] [x0]
 * 1/sqrt(2) * [1, -1] [x1]
 *
 * ��-������, ������� 4x4 ����������� � ������� �������, � ����� � ������ ������ ������ �������� ����� 4x4
 * ������� ������ ����� ������� ����, � ������� �������� ��� ��������� (����� ���������������, ���� ����������).
 * [1, 0, 1, 0] [x0]
 * [1, 0, -1, 0] [x1]
 * 1/sqrt(2) * [0, 1, 0, 1] [x2]
 * [0, 1, 0, -1] [x3]
 *
 * �������, ������� 8x8 ����������� � ������� �������, � ����� � ������ ������
 * ����� �������� ����� ����� ������� ����.
 * [ 64, 0, 0, 0, 64, -11, 0, 11] [x0]
 * [ 64, 0, 0, 0, -64, -11, 0, 11] [x1]
 * 1 [ 0, 64, 0, 0, 11, 64, -11, 0] [x2]
 * ----------- * [ 0, 64, 0, 0, 11, -64, -11, 0] [x3]
 * 64*sqrt(2) [ 0., 0, 64, 0, 0, 11, 64, -11] [x4]
 * [ 0., 0, 64, 0, 0, 11, -64, -11] [x5]
 * [ 0, 0, 0, 64, -11, 0, 11, 64] [x6]
 * [ 0, 0, 0, 64, -11, 0, 11, -64] [x7]
 */

void bior_2d_inverse(
	vector<float>& signal
	, const unsigned N
	, const unsigned d_s
	, vector<float> const& lpr
	, vector<float> const& hpr
) {
	//! Initialization
	const unsigned iter_max = log2(N);
	unsigned N_1 = 2;
	unsigned N_2 = 1;
	const unsigned S_1 = lpr.size();
	const unsigned S_2 = S_1 / 2 - 1;
#pragma omp parallel for num_threads(USE_THREADS_NUM)

	for (unsigned iter = 0; iter < iter_max; iter++)
	{

		vector<float> tmp(N_1 + S_2 * N_1);
		vector<unsigned> ind_per(N_1 + 2 * S_2 * N_2);
		per_ext_ind(ind_per, N_1, S_2 * N_2);

		//! Implementing column filtering
		for (unsigned j = 0; j < N_1; j++)
		{
#pragma omp parallel for num_threads(USE_THREADS_NUM)

			//! Periodic extension of the signal in column
			for (unsigned i = 0; i < tmp.size(); i++)
				tmp[i] = signal[d_s + j + ind_per[i] * N];
#pragma omp parallel for num_threads(USE_THREADS_NUM)

			//! Low and High frequencies filtering
			for (unsigned i = 0; i < N_2; i++)
			{
#pragma omp parallel for num_threads(USE_THREADS_NUM)

				float v_l = 0.0f, v_h = 0.0f;
				for (unsigned k = 0; k < S_1; k++)
				{
					v_l += lpr[k] * tmp[k * N_2 + i];
					v_h += hpr[k] * tmp[k * N_2 + i];
				}

				signal[d_s + i * 2 * N + j] = v_h;
				signal[d_s + (i * 2 + 1) * N + j] = v_l;
			}
		}
#pragma omp parallel for num_threads(USE_THREADS_NUM)

		//! Implementing row filtering
		for (unsigned i = 0; i < N_1; i++)
		{
			//! Periodic extension of the signal in row
			for (unsigned j = 0; j < tmp.size(); j++)
				tmp[j] = signal[d_s + i * N + ind_per[j]];
#pragma omp parallel for num_threads(USE_THREADS_NUM)

			//! Low and High frequencies filtering
			for (unsigned j = 0; j < N_2; j++)
			{
				float v_l = 0.0f, v_h = 0.0f;
				for (unsigned k = 0; k < S_1; k++)
				{
					v_l += lpr[k] * tmp[k * N_2 + j];
					v_h += hpr[k] * tmp[k * N_2 + j];
				}

				signal[d_s + i * N + j * 2] = v_h;
				signal[d_s + i * N + j * 2 + 1] = v_l;
			}
		}

		//! Sizes update
		N_1 *= 2;
		N_2 *= 2;
	}
}

void inplace_backward_bior15_2d_8x8(float* src)
{
	std::vector<float> lpd, hpd, lpr, hpr;

	bior15_coef(lpd, hpd, lpr, hpr);
	std::vector<float> out(64, 0);
	std::vector<float> value;
#pragma omp parallel for num_threads(USE_THREADS_NUM)

	for (size_t i = 0; i < 64; i++)
	{
		value.push_back(src[i]);

	}


	bior_2d_inverse(value, 8, 0, lpr, hpr);
}


/* ������������� ������ ������� �������������� 8x8 Bior-1.5.
 * ����� �� �� �����, ��� � ������ � ��������� ������, �� ������ � ����������������.
 * �������� ������������ ���������� �� 2 �� ��������� � �������������� � ��������� ������.
 * �������� ������������ ����� �� 5 ��� ������, ��� ������� ��������, � ����� ��������
 * ��� ������������� ����������� �� ��������� � �������� ���������� ��������� 18 ���, ��������, 26 ��� ��� uint8.
 */
void inplace_forward_bior15_2d_8x8(int* src)
{
	static int buf[4];
	int* org_src = src;

	// �������������� �������������� 1-�� ���� (8x8)
	for (int i = 0; i < 8; i++)
	{
		// ��� (i)
		buf[0] = src[0] - src[1];
		buf[1] = src[2] - src[3];
		buf[2] = src[4] - src[5];
		buf[3] = src[6] - src[7];

		src[0] = 64 * (src[0] + src[1]) + 11 * (buf[1] - buf[3]);
		src[1] = 64 * (src[2] + src[3]) + 11 * (buf[2] - buf[0]);
		src[2] = 64 * (src[4] + src[5]) + 11 * (buf[3] - buf[1]);
		src[3] = 64 * (src[6] + src[7]) + 11 * (buf[0] - buf[2]);

		src[4] = buf[0] * 64;
		src[5] = buf[1] * 64;
		src[6] = buf[2] * 64;
		src[7] = buf[3] * 64;
		src += 8;
	}

	// ������������ �������������� 1-�� ���� (8x8)
	src = org_src;
	for (int j = 0; j < 8; j++)
	{
		// ��� (i)
		buf[0] = src[0 * 8 + j] - src[1 * 8 + j];
		buf[1] = src[2 * 8 + j] - src[3 * 8 + j];
		buf[2] = src[4 * 8 + j] - src[5 * 8 + j];
		buf[3] = src[6 * 8 + j] - src[7 * 8 + j];

		src[0 * 8 + j] = 64 * (src[0 * 8 + j] + src[1 * 8 + j]) + 11 * (buf[1] - buf[3]);
		src[1 * 8 + j] = 64 * (src[2 * 8 + j] + src[3 * 8 + j]) + 11 * (buf[2] - buf[0]);
		src[2 * 8 + j] = 64 * (src[4 * 8 + j] + src[5 * 8 + j]) + 11 * (buf[3] - buf[1]);
		src[3 * 8 + j] = 64 * (src[6 * 8 + j] + src[7 * 8 + j]) + 11 * (buf[0] - buf[2]);

		src[4 * 8 + j] = buf[0] * 64;
		src[5 * 8 + j] = buf[1] * 64;
		src[6 * 8 + j] = buf[2] * 64;
		src[7 * 8 + j] = buf[3] * 64;
	}

	// �������������� �������������� 2-�� ���� (4x4)
	for (int i = 0; i < 4; i++)
	{
		buf[0] = src[0] - src[1];
		buf[1] = src[2] - src[3];
		src[0] = src[0] + src[1];
		src[1] = src[2] + src[3];
		src[2] = buf[0];
		src[3] = buf[1];
		src += 8;
	}

	// ������������ �������������� 2-�� ���� (4x4)
	src = org_src;
	for (int j = 0; j < 4; j++)
	{
		buf[0] = src[0 * 8 + j] - src[1 * 8 + j];
		buf[1] = src[2 * 8 + j] - src[3 * 8 + j];
		src[0 * 8 + j] = (src[0 * 8 + j] + src[1 * 8 + j] + 1) >> 1;
		src[1 * 8 + j] = (src[2 * 8 + j] + src[3 * 8 + j] + 1) >> 1;
		src[2 * 8 + j] = (buf[0] + 1) >> 1;
		src[3 * 8 + j] = (buf[1] + 1) >> 1;
	}

	// �������������� �������������� 3-�� ���� (2x2)
	for (int i = 0; i < 2; i++)
	{
		buf[0] = src[0] - src[1];
		src[0] = src[0] + src[1];
		src[1] = buf[0];
		src += 8;
	}

	// ������������ �������������� 3-�� ���� (2x2)
	src = org_src;
	for (int j = 0; j < 2; j++)
	{
		buf[0] = src[0 * 8 + j] - src[1 * 8 + j];
		src[0 * 8 + j] = (src[0 * 8 + j] + src[1 * 8 + j] + 1) >> 1;
		src[1 * 8 + j] = (buf[0] + 1) >> 1;
	}

	// ������������
	for (int i = 0; i < 64; i++)
	{
		src[i] = (src[i] + (1 << 11)) >> 12;
	}
}



/* ������������� ������ ��������� �������������� 8x8 Bior-1.5.
 * ��� ����� �� �� �����, ��� � ������ � ��������� ������, �� ������ � ����������������.
 * �������� ������ ����� �� �� ����, ��� � �������� �������� �����������.
 * ����, ����������� ��� ������������� �����������, �������� ���� ��, ��� � � ������ ��������������.
 */
void inplace_backward_bior15_2d_8x8(int* src)
{
	static int buf[4];
	int* org_src = src;

	// vertical transform of the 1st step (2x2)
	for (int j = 0; j < 2; j++)
	{
		buf[0] = src[0 * 8 + j] - src[1 * 8 + j];
		src[0 * 8 + j] = src[0 * 8 + j] + src[1 * 8 + j];
		src[1 * 8 + j] = buf[0];
	}

	// horizontal transform of the 1st step (2x2)
	for (int i = 0; i < 2; i++)
	{
		buf[0] = src[0] - src[1];
		src[0] = (src[0] + src[1] + 1) >> 1;
		src[1] = (buf[0] + 1) >> 1;
		src += 8;
	}
	src = org_src;

	// vertical transform of the 2nd step (4x4)
	for (int j = 0; j < 4; j++)
	{
		buf[0] = src[0 * 8 + j] - src[2 * 8 + j];
		buf[1] = src[1 * 8 + j] - src[3 * 8 + j];
		src[0 * 8 + j] = src[0 * 8 + j] + src[2 * 8 + j];
		src[2 * 8 + j] = src[1 * 8 + j] + src[3 * 8 + j];
		src[1 * 8 + j] = buf[0];
		src[3 * 8 + j] = buf[1];
	}

	// horizontal transform of the 2nd step (4x4)
	for (int i = 0; i < 4; i++)
	{
		buf[0] = src[0] - src[2];
		buf[1] = src[1] - src[3];
		src[0] = (src[0] + src[2] + 1) >> 1;
		src[2] = (src[1] + src[3] + 1) >> 1;
		src[1] = (buf[0] + 1) >> 1;
		src[3] = (buf[1] + 1) >> 1;
		src += 8;
	}
	src = org_src;

	// vertical transform of the 3rd step (8x8)
	for (int j = 0; j < 8; j++)
	{
		buf[0] = (src[0 * 8 + j] - src[4 * 8 + j]) * 64;
		buf[1] = (src[1 * 8 + j] - src[5 * 8 + j]) * 64;
		buf[2] = (src[2 * 8 + j] - src[6 * 8 + j]) * 64;
		buf[3] = (src[3 * 8 + j] - src[7 * 8 + j]) * 64;

		src[0 * 8 + j] = (src[0 * 8 + j] + src[4 * 8 + j]) * 64;
		src[1 * 8 + j] = (src[1 * 8 + j] + src[5 * 8 + j]) * 64;
		src[2 * 8 + j] = (src[2 * 8 + j] + src[6 * 8 + j]) * 64;
		src[3 * 8 + j] = (src[3 * 8 + j] + src[7 * 8 + j]) * 64;

		src[5 * 8 + j] = (src[5 * 8 + j] - src[7 * 8 + j]) * 11;
		src[6 * 8 + j] = (src[6 * 8 + j] - src[4 * 8 + j]) * 11;

		src[4 * 8 + j] = src[5 * 8 + j];
		src[5 * 8 + j] = buf[2] + src[5 * 8 + j];
		src[7 * 8 + j] = buf[3] + src[6 * 8 + j];

		src[0 * 8 + j] = src[0 * 8 + j] - src[4 * 8 + j];

		buf[2] = src[1 * 8 + j];
		buf[3] = src[2 * 8 + j];
		src[1 * 8 + j] = buf[0] - src[4 * 8 + j];
		src[2 * 8 + j] = buf[2] - src[6 * 8 + j];

		buf[0] = src[3 * 8 + j];
		src[3 * 8 + j] = buf[1] - src[6 * 8 + j];
		src[4 * 8 + j] = buf[3] + src[4 * 8 + j];
		src[6 * 8 + j] = buf[0] + src[6 * 8 + j];
	}

	// horizontal transform of the 3rd step (8x8)
	for (int i = 0; i < 8; i++)
	{
		buf[0] = (src[0] - src[4]) * 64;
		src[0] = (src[0] + src[4]) * 64;
		buf[1] = (src[1] - src[5]) * 64;
		src[1] = (src[1] + src[5]) * 64;
		buf[2] = (src[2] - src[6]) * 64;
		src[2] = (src[2] + src[6]) * 64;
		buf[3] = (src[3] - src[7]) * 64;
		src[3] = (src[3] + src[7]) * 64;

		src[5] = (src[5] - src[7]) * 11;
		src[6] = (src[6] - src[4]) * 11;
		src[4] = src[5];
		src[5] = buf[2] + src[5];
		src[7] = buf[3] + src[6];
		src[0] = src[0] - src[4];

		buf[2] = src[1];
		buf[3] = src[2];
		src[1] = buf[0] - src[4];
		src[2] = buf[2] - src[6];
		buf[0] = src[3];
		src[3] = buf[1] - src[6];
		src[4] = buf[3] + src[4];
		src[6] = buf[0] + src[6];
		src += 8;
	}
	src = org_src;

	// normalization
	for (int i = 0; i < 64; i++)
	{
		src[i] = (src[i] + (1 << 13)) >> 14;
	}
}















#define NN 8 // ������ ����� DCT

// ������� ��� ���������� ������� DCT-II � ������ ���������� � src
void dct(float* src) {
	// ������� �������� �����
	float* out = (float*)fftwf_malloc(sizeof(float) * NN * NN);

	// ������� ���� ��� FFTW. �� ���������� 'fftwf' ������� ��� float ������.
	fftwf_plan plan = fftwf_plan_r2r_2d(NN, NN, src, out, FFTW_REDFT10, FFTW_REDFT10, FFTW_ESTIMATE);

	// ��������� ��������������
	fftwf_execute(plan);

	// �������� ��������� ������� � src
#pragma omp parallel for num_threads(4)

	for (int i = 0; i < NN * NN; ++i) {
		src[i] = out[i];
	}

	// ����������� �������� �����
	fftwf_free(out);

	// ������� ����
	fftwf_destroy_plan(plan);
}

// ������� ��� ���������� ��������� DCT-III � ������ ���������� � src
void idct(float* src) {
	// ������� �������� �����
	float* out = (float*)fftwf_malloc(sizeof(float) * NN * NN);

	// ������� ���� ��� FFTW.
	fftwf_plan plan = fftwf_plan_r2r_2d(NN, NN, src, out, FFTW_REDFT01, FFTW_REDFT01, FFTW_ESTIMATE);

	// ��������� ��������������
	fftwf_execute(plan);

	// �������� ��������� ������� � src
#pragma omp parallel for num_threads(4)
	for (int i = 0; i < NN * NN; ++i) {
		src[i] = out[i];
	}

	// ����������� �������� �����
	fftwf_free(out);

	// ������� ����
	fftwf_destroy_plan(plan);
}





void inplace_forward_dct_8x8(float* srcData)
{
	//dct(src);

	


	// ������� cv::Mat �� ����������� �������
	cv::Mat src(NN, NN, CV_32F, srcData);

	//std::cout << "Original Matrix:\n" << src << "\n\n";

	// ��������� ������ DCT
	cv::Mat dst;
	cv::dct(src, dst);


	//std::cout << "Original Matrix:\n" << dst << "\n\n";


	// �������� ���������� ������� � srcData, ���� ����������
#pragma omp parallel for num_threads(USE_THREADS_NUM)

	for (int i = 0; i < NN; ++i) {
		for (int j = 0; j < NN; ++j) {
			srcData[i * NN + j] = dst.at<float>(i, j);
		}
	}

}



void inplace_backward_dct_8x8(float* srcData)
{
	//idct(src);



	// ������� cv::Mat �� ����������� �������
	cv::Mat src(NN, NN, CV_32F, srcData);

	//std::cout << "Original Matrix:\n" << src << "\n\n";


	// ��������� �������� DCT
	cv::Mat idst;
	cv::idct(src, idst);

//	std::cout <<"After IDCT:\n" << idst << "\n\n";

	// �������� ���������� ������� � srcData, ���� ����������
#pragma omp parallel for num_threads(USE_THREADS_NUM)

	for (int i = 0; i < NN; ++i) {
		for (int j = 0; j < NN; ++j) {
			srcData[i * NN + j] = idst.at<float>(i, j);
		}
	}
}




























