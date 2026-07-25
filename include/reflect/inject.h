#ifndef _REFLECT_INJECT
#define _REFLECT_INJECT

#include <tplmp/access.h>
#include <ppmp/loop.h>

namespace reflect
{
enum reflect_type
{
	type_primitive, type_class, type_field, type_function
};

/**
 * @brief 反射实体
 * @param refl_type 实体的类型
 * @param [opt_refl_name] 可选，是实际储存信息的结构体名称，如果为空则默认使用name_entity的值。需要注意的是，由于模板中含有<>和,都不是合法标识符，对于模板类型必须要传入一个合法的ID，禁止使用默认的名称（默认名称非法）
 * @param <name_entity> 实体的名称，必须用__entity__()宏包围
 * @param __VA_ARGS__ 附加参数，将被打包作为一个整体
 *
 * 若opt_refl_name以括号包围，则代表无反射key，直接传入了name_entity
 */
#define __refl_info__(refl_type, opt_refl_name, ...)\
	__if_else_intl__(__in_matched_paren__(opt_refl_name))\
	(\
		__refl_info_intl__(refl_type, opt_refl_name, opt_refl_name, __VA_ARGS__),\
		__refl_info_intl__(refl_type, __entity__(opt_refl_name), __VA_ARGS__)\
	)
#define __refl_info_intl__(refl_type, refl_name, name_entity, ...)\
	__pack__(refl_type, refl_name, name_entity, __entity__(__VA_ARGS__))

/**
 * @brief 函数实体
 * @param [opt_refl_name] 函数成员的反射索引key，必须是合法C++标识符
 * @param <memb_name> 函数的名称，必须用__entity__()宏包围
 * @param __VA_ARGS__ 函数声明类型，包含返回值类型和参数类型
 */
#define __refl_function__(opt_refl_name, ...) __refl_info__(type_function, opt_refl_name, __VA_ARGS__)

/**
 * @brief 字段实体
 * @param [opt_refl_name] 字段成员的反射索引key，必须是合法C++标识符
 * @param <memb_name> 字段的名称，必须用__entity__()宏包围
 * @param __VA_ARGS__ 字段声明类型
 */
#define __refl_field__(opt_refl_name, ...) __refl_info__(type_field, opt_refl_name, __VA_ARGS__)

/**
 * @brief 类实体
 * @param [opt_refl_name] 类的反射索引key，必须是合法C++标识符
 * @param <class_name> 类的名称，必须用__entity__()宏包围
 * @param __VA_ARGS__ 类的基类列表，每个基类都以__refl_class__()包裹，在基类列表中的类不需要递归写出它的基类列表
 */
#define __refl_class__(opt_refl_name, ...) __refl_info__(type_class, opt_refl_name, __VA_ARGS__)

#define __equal_def__type_class(x) x
#define __equal_def__type_field(x) x
#define __equal_def__type_function(x) x

/**
 * @brief 储存反射指针的pmenb_id
 * 		  refl_class_name与refl_memb_name均为__entity__()包围的名称
 */
#define __reflect_pmemb_id__(refl_class_name, refl_memb_name) __cat__(4, __reflect_, __entity_val__(refl_class_name), _, __entity_val__(refl_memb_name))

// ----- 定义取成员指针的友元注入相关类 -----

#if !defined(__reflect_decl_pmemb_loop_expand_id__)
#define __reflect_decl_pmemb_loop_expand_id__() 1
#endif
#if !defined(__reflect_decl_pmemb_call_expand_id__)
#define __reflect_decl_pmemb_call_expand_id__() 1
#endif

#define __reflect_decl_pmemb_op__(i, begin_idx, end_idx, class_info, memb_info)\
	__call_exp__(__reflect_decl_pmemb_call_expand_id__())(__reflect_decl_pmemb_op_intl__, i, begin_idx, end_idx, __unpack__(class_info), __unpack__(memb_info))
#define __reflect_decl_pmemb_op_intl__(i, begin_idx, end_idx, refl_type_class, refl_class_name, class_name, class_base_list, refl_type_memb, refl_memb_name, memb_name, memb_decl_type)\
	__decl_pmemb__(__reflect_pmemb_id__(refl_class_name, refl_memb_name), class_name, memb_name, memb_decl_type)\
	__decl_memb_ptr__(__reflect_pmemb_id__(refl_class_name, refl_memb_name), memb_decl_type)

#define __reflect_decl_pmemb__(class_info, ...)\
	namespace __reflect\
	{\
	__for_each__(__reflect_decl_pmemb_loop_expand_id__())(__reflect_decl_pmemb_op__, class_info, __VA_ARGS__)\
	}
}

#endif//_REFLECT_INJECT
