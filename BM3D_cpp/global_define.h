#ifndef __GLOBAL_DEFINE_H__
#define __GLOBAL_DEFINE_H__

#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef uint8_t ImageType; // ��� ������ ��������/��������� ����������� (�� 12 ��� ��� ������������� ������)
typedef uint32_t DistType; // ��� ������ ���������� ����� ����� �������

#define USE_INTEGER 0 // ������������ ������������� ������ ��� ������ � ��������� ������
#define USE_L2_DIST 1 // ������������ L2 (������������) ��� L1 (����������) ����������

#define HARD_THRES_MULTIPLIER 2.7f // ��������� �� ����� - ����� ������� ���������� 

#define USE_THREADS_NUM 10 // ���������� ������� ����������, ������� ����� ���� ������������ �� ����� �����������

#if USE_INTEGER

#define COEFF_DICI_BITS 1 // ���������� ���� �������������� ������������� (� ������������ � ����������� ��������������)
typedef int32_t PatchType; // int32_t ���������� ��� ������������� �����������

#else

#define COEFF_DICI_BITS 0
typedef float PatchType;

#endif


#endif

