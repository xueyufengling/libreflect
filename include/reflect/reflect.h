#ifndef _REFLECT_REFLECT
#define _REFLECT_REFLECT

#include "static_reflect.h"
#include "dynamic_reflect.h"

/**
 * @brief 定义静态反射和动态反射
 */
#define __reflect__(class_info, ...)\
	__decl_reflect__(class_info, __VA_ARGS__)\
	__def_static_reflect__(class_info, __VA_ARGS__)\
	__def_dynamic_reflect__(class_info, __VA_ARGS__)

#endif//_REFLECT_REFLECT
