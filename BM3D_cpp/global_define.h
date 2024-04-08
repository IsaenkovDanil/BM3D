#ifndef __GLOBAL_DEFINE_H__
#define __GLOBAL_DEFINE_H__

#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef uint8_t ImageType; // тип данных входного/выходного изображения (до 12 бит для целочисленной версии)
typedef uint32_t DistType; // тип данных расстояния между двумя пятнами

#define USE_INTEGER 0 // использовать целочисленную версию или версию с плавающей точкой
#define USE_L2_DIST 1 // использовать L2 (квадратичное) или L1 (абсолютное) расстояние

#define HARD_THRES_MULTIPLIER 2.7f // умножение на сигму - порог жесткой фильтрации 

#define USE_THREADS_NUM 10 // количество потоков процессора, которые могут быть использованы на этапе группировки

#if USE_INTEGER

#define COEFF_DICI_BITS 1 // десятичные биты интергерентных коэффициентов (в соответствии с реализацией преобразования)
typedef int32_t PatchType; // int32_t достаточно для промежуточных результатов

#else

#define COEFF_DICI_BITS 0
typedef float PatchType;

#endif


#endif

