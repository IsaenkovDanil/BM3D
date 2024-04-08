#ifndef __PATCH_3D_H__
#define __PATCH_3D_H__

#include <iostream>
#include "patch_2d.h"

struct Group3D
{
	int w; // ширина патча
	int h; // высота патча
	int max_patches; // максимальное количество одинаковых патчей

	int num; // количество патчей (будет усечено до степени 2)
	int log_num; // log2(num)

	DistType max_dist; // максимальная сумма расстояний (L2/L1) между двумя патчами

	PatchType thres; // жесткий порог фильтрации
	int nonzeros; // количество ненулевых коэффициентов

	Patch2D** patch; // массив указателей 2D патчей
	Patch2D** buf; // массив указателей, используемых в качестве буфера (в преобразовании Хадамарда)

	static const PatchType sqrt_powN_x32[8]; // целое число из (sqrt(1<<n) * 32)

	Group3D(int w_, int h_, int maxp);
	~Group3D();

	void set_thresholds(int sigma, DistType maxd);

	void set_reference(ImageType* refer, int stride);
	void set_reference();

	// находим индекс для вставки с заданным расстоянием
	int find_idx(DistType d);

	void insert_patch(int x, int y, DistType d);
	void fill_patches_values(ImageType* refer, int stride);

	// прямая и обратная стороны одинаковы, за исключением масштабирования
	void hadamard_1d();

	void transform_3d(int i);
	void inv_transform_3d(int i);

	void hard_thresholding();

	PatchType get_weight();
};

#endif