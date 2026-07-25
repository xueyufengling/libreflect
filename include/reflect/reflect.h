#ifndef _REFLECT_REFLECT
#define _REFLECT_REFLECT

#include "static_reflect.h"
#include "dynamic_reflect.h"

/**
 * @brief 定义静态反射和动态反射
 */
#define __reflect__(class_info, ...)\
	__reflect_decl_pmemb__(class_info, __VA_ARGS__)\
	__static_reflect_def__(class_info, __VA_ARGS__)\
	__dynamic_reflect_def__(class_info, __VA_ARGS__)

#endif//_REFLECT_REFLECT
