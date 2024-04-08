#ifndef __PATCH_3D_H__
#define __PATCH_3D_H__

#include <iostream>
#include "patch_2d.h"

struct Group3D
{
	int w; // ������ �����
	int h; // ������ �����
	int max_patches; // ������������ ���������� ���������� ������

	int num; // ���������� ������ (����� ������� �� ������� 2)
	int log_num; // log2(num)

	DistType max_dist; // ������������ ����� ���������� (L2/L1) ����� ����� �������

	PatchType thres; // ������� ����� ����������
	int nonzeros; // ���������� ��������� �������������

	Patch2D** patch; // ������ ���������� 2D ������
	Patch2D** buf; // ������ ����������, ������������ � �������� ������ (� �������������� ���������)

	static const PatchType sqrt_powN_x32[8]; // ����� ����� �� (sqrt(1<<n) * 32)

	Group3D(int w_, int h_, int maxp);
	~Group3D();

	void set_thresholds(int sigma, DistType maxd);

	void set_reference(ImageType* refer, int stride);
	void set_reference();

	// ������� ������ ��� ������� � �������� �����������
	int find_idx(DistType d);

	void insert_patch(int x, int y, DistType d);
	void fill_patches_values(ImageType* refer, int stride);

	// ������ � �������� ������� ���������, �� ����������� ���������������
	void hadamard_1d();

	void transform_3d(int i);
	void inv_transform_3d(int i);

	void hard_thresholding();

	PatchType get_weight();
};

#endif