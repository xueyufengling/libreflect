#ifndef _REFLECT_STATICREFLECT
#define _REFLECT_STATICREFLECT

#include <ppmp/base.h>
#include <ppmp/list_op_step.h>
#include <ppmp/loop.h>

#include "inject.h"

/**
 * 静态反射
 */

namespace reflect
{
// 带模板参数，目标类的静态反射未定义时可打印类型名称
template<typename>
struct not_reflected;

/**
 * 利用ADL机制，函数参数依赖模板参数_T，当直接调用本函数名时，除了此处的模板函数，编译器还将到_T所在的命名空间下搜索__type_info_adl()并加入候选。
 * 由于__static_reflect_def__()展开后在_T命名空间下使用using导出的__reflect::__type_info_adl()是普通函数重载，优先级比模板声明更高。
 * 因此实际调用时编译器将最终决议调用为__reflect::__type_info_adl()而非本模板函数，从而将实际储存静态反射信息的type_info_t<_T>类名提取出来。
 */
template<typename _T>
inline not_reflected<_T> __type_info_adl(tplmp::type_t<_T>) noexcept;

template<typename _T>
using type_info = decltype(__type_info_adl(tplmp::type_t<_T>()));

// ----- 静态反射类定义 -----
#if !defined(__reflect_static_def_memb_loop_expand_id__)
#define __reflect_static_def_memb_loop_expand_id__() 2
#endif
#if !defined(__reflect_static_def_memb_call_def_class_expand_id__)
#define __reflect_static_def_memb_call_def_class_expand_id__() 2
#endif
#if !defined(__reflect_static_def_memb_call_def_memb_expand_id__)
#define __reflect_static_def_memb_call_def_memb_expand_id__() 3
#endif

/**
 * @brief 定义编译时静态反射，无视访问权限获取成员指针。
 * class_info的变长参数列表为基类名称，基类列表使用__entity__()宏包围的类名称。
 * @detail 用例：
 * namespace ns {
 * class B
 * {
 * };
 * __static_reflect__
 * (
 * 	__refl_class__(__entity__(B))
 * )
 * class A: B
 * {
 * 	void func(int);
 * 	float x;
 * 	int y;
 * 	float operator+(int);
 * };
 * 可以使用以下宏定义静态反射
 * __static_reflect__
 * (
 * 	__refl_class__(__entity__(A), __entity__(B)),
 *  __refl_function__(__entity__(func), void(int)),
 *  __refl_field__(__entity__(x), float),
 *  __refl_field__(__entity__(y), int)
 *  __refl_function__(operator_add, __entity__(operator+), float(int))
 * )
 * 访问x的成员指针的写法为reflect::type_info<ns::A>::field::x::pmemb；
 * 访问func的成员指针的写法为reflect::type_info<ns::A>::function::func::pmemb；
 * 访问operator+的成员指针的写法为reflect::type_info<ns::A>::function::operator_add::pmemb。
 */
#define __static_reflect_def__(class_info, ...)\
	__call_exp__(__reflect_static_def_memb_call_def_class_expand_id__())(__static_reflect_def_intl__, __unpack__(class_info), __VA_ARGS__)

#define __static_reflect_def_intl__(refl_type_class, refl_class_name, class_name, class_base_list, ...)\
	namespace __reflect\
	{\
	template<typename _Class>\
	struct type_info_t;\
	template<>\
	struct type_info_t<__entity_val__(class_name)>\
	{\
		static constexpr const char* refl_name = __entity_str__(refl_class_name);\
		typedef __entity_val__(class_name) type;\
		static constexpr const char* name = __entity_str__(class_name);\
		static constexpr ::reflect::reflect_type refl_type = ::reflect::reflect_type::refl_type_class;\
		typedef __reflect_static_bases_list__(class_base_list) bases;\
		struct field\
		{\
			__reflect_static_def_membs__(type_field, refl_class_name, class_name, __VA_ARGS__)\
		};\
		typedef __reflect_static_membs_list__(type_field, __VA_ARGS__) fields;\
		struct function\
		{\
			__reflect_static_def_membs__(type_function, refl_class_name, class_name, __VA_ARGS__)\
		};\
		typedef __reflect_static_membs_list__(type_function, __VA_ARGS__) functions;\
	};\
	inline type_info_t<__entity_val__(class_name)> __type_info_adl(::tplmp::type_t<__entity_val__(class_name)>) noexcept;\
	}\
	using __reflect::__type_info_adl;

// ----- 定义基类列表 -----

#if !defined(__reflect_static_bases_list_loop_expand_id__)
#define __reflect_static_bases_list_loop_expand_id__() 3
#endif

#define __reflect_static_bases_list_tpl_params_op__(i, begin_idx, end_idx, const_params, base_class_name)\
	__append_comma__(::reflect::type_info<__entity_val__(base_class_name)>)
#define __reflect_static_bases_list_tpl_params__(class_base_list)\
	__strip_trailing_1_comma__(__for_each__(__reflect_static_bases_list_loop_expand_id__())(__reflect_static_bases_list_tpl_params_op__, , __unpack__(class_base_list)))

#define __reflect_static_bases_list__(class_base_list)\
	::tplmp::type_pack<__reflect_static_bases_list_tpl_params__(class_base_list)>

// ----- 成员定义 -----

#define __reflect_static_def_memb_op__(i, begin_idx, end_idx, target_refl_type, refl_class_name, class_name, memb_info)\
	__call_exp__(__reflect_static_def_memb_call_def_memb_expand_id__())(__reflect_static_def_memb_op_intl__, target_refl_type, refl_class_name, class_name, __unpack__(memb_info))

/**
 * 注意使用using decl_type = __entity_val__(memb_decl_type);而不是typedef定义别名。
 * 这是因为如果成员声明类型是函数类型，例如void(int)，那么typedef void(int) decl_type;是非法的会报编译错误，而using decl_type = void(int);是合法的
 */
#define __reflect_static_def_memb_op_intl__(target_refl_type, refl_class_name, class_name, refl_memb_type, refl_memb_name, memb_name, memb_decl_type)\
	__if_intl__(__equal__(refl_memb_type, target_refl_type))\
	(\
		struct __entity_val__(refl_memb_name)\
		{\
			static constexpr const char* refl_decl_class_name = __entity_str__(refl_class_name);\
			typedef __entity_val__(class_name) decl_class;\
			static constexpr const char* decl_class_name = __entity_str__(class_name);\
			static constexpr const char* refl_name = __entity_str__(refl_memb_name);\
			static constexpr const char* name = __entity_str__(memb_name);\
			using decl_type = __entity_val__(memb_decl_type);\
			static constexpr const char* decl_type_name = __entity_str__(memb_decl_type);\
			static constexpr ::reflect::reflect_type refl_type = ::reflect::reflect_type::refl_memb_type;\
			static constexpr typename ::tplmp::ptr_type<__entity_val__(class_name), __entity_val__(memb_decl_type)>::type pmemb = __memb_ptr<__reflect_pmemb_id__(refl_class_name, refl_memb_name)>();\
		};\
	)

#define __reflect_static_def_membs__(target_refl_type, refl_class_name, class_name, ...)\
	__for_each__(__reflect_static_def_memb_loop_expand_id__())(__reflect_static_def_memb_op__, __pack_list__(target_refl_type, refl_class_name, class_name), __VA_ARGS__)

// ----- 定义成员列表，用于编译期遍历 -----

#if !defined(__reflect_static_membs_list_loop_expand_id__)
#define __reflect_static_membs_list_loop_expand_id__() 3
#endif
#if !defined(__reflect_static_membs_list_call_expand_id__)
#define __reflect_static_membs_list_call_expand_id__() 4
#endif

#define __reflect_static_memb_namespace__type_field() field
#define __reflect_static_memb_namespace__type_function() function
#define __reflect_static_memb_namespace__(refl_memb_type) __cat__(2, __reflect_static_memb_namespace__, refl_memb_type)()

#define __reflect_static_membs_list_tpl_params_op__(i, begin_idx, end_idx, target_refl_type, memb_info)\
	__call_exp__(__reflect_static_membs_list_call_expand_id__())(__reflect_static_membs_list_tpl_params_op_intl__, i, begin_idx, end_idx, target_refl_type, __unpack__(memb_info))
#define __reflect_static_membs_list_tpl_params_op_intl__(i, begin_idx, end_idx, target_refl_type, refl_memb_type, refl_memb_name, memb_name, memb_decl_type)\
	__if_intl__(__equal__(refl_memb_type, target_refl_type))\
	(\
		__append_comma__(__reflect_static_memb_namespace__(refl_memb_type)::__entity_val__(refl_memb_name))\
	)
#define __reflect_static_membs_list_tpl_params__(target_refl_type, ...)\
	__strip_trailing_1_comma__(__for_each__(__reflect_static_membs_list_loop_expand_id__())(__reflect_static_membs_list_tpl_params_op__, target_refl_type, __VA_ARGS__))

#define __reflect_static_membs_list__(target_refl_type, ...)\
	::tplmp::type_pack<__reflect_static_membs_list_tpl_params__(target_refl_type, __VA_ARGS__)>

/**
 * @brief 只定义静态反射
 */
#define __static_reflect__(class_info, ...)\
	__reflect_decl_pmemb__(class_info, __VA_ARGS__)\
	__static_reflect_def__(class_info, __VA_ARGS__)
}

#endif//_REFLECT_STATICREFLECT
