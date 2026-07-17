#ifndef _REFLECT_BASE
#define _REFLECT_BASE

#include <tplmp/access.h>
#include <ppmp/deferred_loop.h>

namespace reflect
{
enum reflect_type
{
	type_class, type_field, type_function
};

/**
 * @brief 反射实体
 * @param refl_type 实体的类型
 * @param refl_name refl_name是实际储存信息的结构体名称，如果为空则默认使用name_entity的值。需要注意的是，由于模板中含有<>和,都不是合法标识符，对于模板类型必须要传入一个合法的ID，禁止使用默认的名称（默认名称非法）
 * @param name_entity 实体的名称，必须用__entity__()宏包围
 * @param __VA_ARGS__ 附加参数，将被打包作为一个整体
 */
#define __refl_info__(refl_type, refl_name, name_entity, ...)\
	__if_else_intl__(__is_empty__(refl_name))\
	(\
		__pack__(refl_type, __entity_val__(name_entity), name_entity, __entity__(__VA_ARGS__)),\
		__pack__(refl_type, refl_name, name_entity, __entity__(__VA_ARGS__))\
	)

/**
 * @brief 函数实体
 * @param refl_memb_name 函数成员的反射索引key
 * @param memb_name 函数的名称，必须用__entity__()宏包围
 * @param __VA_ARGS__ 函数声明类型，包含返回值类型和参数类型
 */
#define __refl_function__(refl_memb_name, memb_name, ...) __refl_info__(type_function, refl_memb_name, memb_name, __VA_ARGS__)

/**
 * @brief 字段实体
 * @param refl_memb_name 字段成员的反射索引key
 * @param memb_name 字段的名称，必须用__entity__()宏包围
 * @param __VA_ARGS__ 字段声明类型
 */
#define __refl_field__(refl_memb_name, memb_name, ...) __refl_info__(type_field, refl_memb_name, memb_name, __VA_ARGS__)

/**
 * @brief 类实体
 * @param refl_memb_name 类的反射索引key
 * @param class_name 类的名称，必须用__entity__()宏包围
 * @param __VA_ARGS__ 类的基类列表，每个基类都以__refl_class__()包裹，在基类列表中的类不需要递归写出它的基类列表
 */
#define __refl_class__(refl_class_name, class_name, ...) __refl_info__(type_class, refl_class_name, class_name, __pack__(__VA_ARGS__))

#define __equal_def__type_class(x) x
#define __equal_def__type_field(x) x
#define __equal_def__type_function(x) x

/**
 * @brief 储存反射指针的pmenb_id
 */
#define __reflect_pmemb_id__(refl_class_name, refl_memb_name) __cat__(4, __reflect_, refl_class_name, _, refl_memb_name)

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
	__for_each_deferred__(__reflect_decl_pmemb_loop_expand_id__())(__reflect_decl_pmemb_op__, class_info, __VA_ARGS__)

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

#define __reflect_static_def_class__(class_info, ...)\
	__call_exp__(__reflect_static_def_memb_call_def_class_expand_id__())(__reflect_static_def_class_intl__, __unpack__(class_info), __VA_ARGS__)

#define __reflect_static_def_class_intl__(refl_type_class, refl_class_name, class_name, class_base_list, ...)\
	template<typename _Class>\
	struct type_info;\
	template<>\
	struct type_info<__entity_val__(class_name)>\
	{\
		struct field\
		{\
			__reflect_static_def_membs__(type_field, refl_class_name, class_name, __VA_ARGS__)\
		};\
		struct function\
		{\
			__reflect_static_def_membs__(type_function, refl_class_name, class_name, __VA_ARGS__)\
		};\
	};

// ----- 静态反射成员定义 -----

#define __reflect_static_def_memb_op__(i, begin_idx, end_idx, target_refl_type, refl_class_name, class_name, memb_info)\
	__call_exp__(__reflect_static_def_memb_call_def_memb_expand_id__())(__reflect_static_def_memb_op_intl__, target_refl_type, refl_class_name, class_name, __unpack__(memb_info))

/**
 * 注意使用using decl_type = __entity_val__(memb_decl_type);而不是typedef定义别名。
 * 这是因为如果成员声明类型是函数类型，例如void(int)，那么typedef void(int) decl_type;是非法的会报编译错误，而using decl_type = void(int);是合法的
 */
#define __reflect_static_def_memb_op_intl__(target_refl_type, refl_class_name, class_name, refl_memb_type, refl_memb_name, memb_name, memb_decl_type)\
	__if_intl__(__equal__(refl_memb_type, target_refl_type))\
	(\
		struct refl_memb_name\
		{\
			static constexpr const char* refl_decl_class_name = __str__(refl_class_name);\
			typedef __entity_val__(class_name) decl_class;\
			static constexpr const char* decl_class_name = __entity_str__(class_name);\
			static constexpr const char* refl_name = __str__(refl_memb_name);\
			static constexpr const char* name = __entity_str__(memb_name);\
			using decl_type = __entity_val__(memb_decl_type);\
			static constexpr const char* decl_type_name = __entity_str__(memb_decl_type);\
			static constexpr ::reflect::reflect_type refl_type = ::reflect::reflect_type::refl_memb_type;\
			static constexpr typename ::tplmp::ptr_type<__entity_val__(class_name), __entity_val__(memb_decl_type)>::type pmemb = __memb_ptr<__reflect_pmemb_id__(refl_class_name, refl_memb_name)>();\
		};\
	)

#define __reflect_static_def_membs__(target_refl_type, refl_class_name, class_name, ...)\
	__for_each_deferred__(__reflect_static_def_memb_loop_expand_id__())(__reflect_static_def_memb_op__, __pack_list__(target_refl_type, refl_class_name, class_name), __VA_ARGS__)


/**
 * @brief 定义编译时静态反射，无视访问权限获取成员指针。
 * class_info的变长参数列表为基类名称，基类列表为同样使用__refl_class__()表示，但不需要写继承的基类。
 * @detail 用例：
 * namespace ns {
 * class A
 * {
 * 	float x;
 * 	int y;
 * 	void func(int);
 * };
 * 可以使用以下宏定义静态反射
 * __static_reflect__
 * (
 * 	__refl_class__(, __entity__(A)),
 *  __refl_function__(, __entity__(func), void(int)),
 *  __refl_field__(, __entity__(x), float),
 *  __refl_field__(, __entity__(y), int)
 * )
 * 访问x的成员指针的写法为ns::reflect::type_info<ns::A>::field::x::pmemb；
 * 访问func的成员指针的写法为ns::reflect::type_info<ns::A>::function::func::pmemb。
 */
#define __static_reflect__(class_info, ...)\
	namespace reflect\
	{\
		__reflect_decl_pmemb__(class_info, __VA_ARGS__)\
		__reflect_static_def_class__(class_info, __VA_ARGS__)\
	}

}

#endif//_REFLECT_BASE
