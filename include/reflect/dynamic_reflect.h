#ifndef _REFLECT_DYNAMICREFLECT
#define _REFLECT_DYNAMICREFLECT

#include <string.h>
#include <unordered_map>

#include <ppmp/base.h>
#include <ppmp/list_op_step.h>
#include <ppmp/loop.h>

#include <tplmp/stl_util.h>

#include "inject.h"

/**
 * 动态反射
 */

namespace reflect
{
struct type_info_rt_t;

/**
 * @brief 定义memb_info_rt除成员指针外的信息成员
 * @param refl_decl_class_name 定义本成员的类的反射key
 * @param decl_class_name 定义本成员的类的名称
 * @param refl_name 成员的反射key
 * @param name 成员名称
 * @param decl_type_name 成员声明类型的名称
 */
#define __def_memb_info_rt_info_memb__()\
	const char* refl_decl_class_name;\
	const char* decl_class_name;\
	const ::reflect::type_info_rt_t* decl_class;\
	const char* refl_name;\
	const char* name;\
	const char* decl_type_name;\
	const ::reflect::type_info_rt_t* decl_type;\
	const ::reflect::reflect_type refl_type;

/**
 * 运行时访问成员信息的接口，禁止直接构造本类，必须通过__memb_info_rt_t<>对象指针进行强转得到
 */
struct memb_info_rt
{
	__def_memb_info_rt_info_memb__()
	/**
	 * @brief 成员指针的大小
	 */
	const size_t pmemb_size;
	const unsigned char pmemb_val[1];

	memb_info_rt() = delete;
	memb_info_rt(const memb_info_rt&) = delete;
	memb_info_rt(memb_info_rt&&) = delete;

	/**
	 * @brief 成员指针值，长度为pmemb_size
	 */
	template<typename _Class, typename _T>
	inline typename tplmp::ptr_type<_Class, _T>::type pmemb() const noexcept
	{
		return *(typename tplmp::ptr_type<_Class, _T>::type*)pmemb_val;
	}

	// 成员字段操作

	template<typename _Class, typename _T>
	inline void store(_Class* pobj, const _T& val) const
			noexcept(noexcept(pobj->*(pmemb<_Class, _T>()) = val))
	{
		pobj->*(pmemb<_Class, _T>()) = val;
	}

	template<typename _Class, typename _T>
	inline void store(_Class& obj, const _T& val) const
			noexcept(noexcept(obj.*(pmemb<_Class, _T>()) = val))
	{
		obj.*(pmemb<_Class, _T>()) = val;
	}

	template<typename _T, typename _Class>
	inline const _T& load(const _Class* pobj) const noexcept
	{
		return pobj->*(pmemb<_Class, _T>());
	}

	template<typename _T, typename _Class>
	inline _T& load(_Class& obj) const noexcept
	{
		return obj.*(pmemb<_Class, _T>());
	}

	template<typename _Class, typename _T>
	inline void load(const _Class* pobj, _T& val) const
			noexcept(noexcept(val = pobj->*(pmemb<_Class, _T>())))
	{
		val = pobj->*(pmemb<_Class, _T>());
	}

	template<typename _Class, typename _T>
	inline void load(const _Class& obj, _T& val) const
			noexcept(noexcept(val = obj.*(pmemb<_Class, _T>())))
	{
		val = obj.*(pmemb<_Class, _T>());
	}

	// 成员函数调用

	template<typename _RetType, typename _Class, typename ..._ArgTypes>
	inline auto call(_Class* pobj, _ArgTypes&& ... args) const
			noexcept(noexcept((pobj->*(pmemb<_Class, _RetType(_ArgTypes...)>()))(tplmp::forward<_ArgTypes>(args)...)))
	-> decltype((pobj->*(pmemb<_Class, _RetType(_ArgTypes...)>()))(tplmp::forward<_ArgTypes>(args)...))
	{
		return (pobj->*(pmemb<_Class, _RetType(_ArgTypes...)>()))(tplmp::forward<_ArgTypes>(args)...);
	}

	template<typename _RetType, typename _Class, typename ..._ArgTypes>
	inline auto call(_Class& obj, _ArgTypes&& ... args) const
			noexcept(noexcept((obj.*(pmemb<_Class, _RetType(_ArgTypes...)>()))(tplmp::forward<_ArgTypes>(args)...)))
	-> decltype((obj.*(pmemb<_Class, _RetType(_ArgTypes...)>()))(tplmp::forward<_ArgTypes>(args)...))
	{
		return (obj.*(pmemb<_Class, _RetType(_ArgTypes...)>()))(tplmp::forward<_ArgTypes>(args)...);
	}
};

/**
 * @brief 实际储存成员信息的模板，使用聚合初始化对__def_memb_info_rt_info_memb__()进行赋值
 */
template<typename _pMembType, _pMembType _pMemb>
struct __memb_info_rt_t
{
	__def_memb_info_rt_info_memb__()
	const size_t pmemb_size = sizeof(_pMembType);
	const _pMembType pmemb_val = _pMemb;

	inline operator const memb_info_rt&() const noexcept
	{
		return *(const memb_info_rt*)this;
	}
};

struct memb_info_rt_list
{
	const size_t size;
	const size_t memb_info_size;
	const unsigned char memb_info[1];

	inline const memb_info_rt* at(size_t idx) const noexcept
	{
		return (const memb_info_rt*)(memb_info + idx * memb_info_size);
	}

	inline const memb_info_rt* by_refl_name(const char* refl_name) const noexcept
	{
		for(size_t i = 0; i < size; ++i)
		{
			const memb_info_rt* memb = at(i);
			if(!strcmp(memb->refl_name, refl_name))
				return memb;
		}
		return nullptr;
	}

	inline const memb_info_rt* by_name(const char* name) const noexcept
	{
		for(size_t i = 0; i < size; ++i)
		{
			const memb_info_rt* memb = at(i);
			if(!strcmp(memb->name, name))
				return memb;
		}
		return nullptr;
	}
};

struct base_info_rt_list;

struct type_info_rt_t
{
	const char* refl_name;
	const char* name;
	const reflect::reflect_type refl_type = reflect::reflect_type::type_class;
	const base_info_rt_list* bases;
	const memb_info_rt_list* fields;
	const memb_info_rt_list* functions;

	inline const type_info_rt_t* base(size_t idx) const noexcept;

	inline const memb_info_rt* field(size_t idx) const noexcept
	{
		return fields->at(idx);
	}

	inline const memb_info_rt* field_by_refl_name(const char* refl_name) const noexcept
	{
		return fields->by_refl_name(refl_name);
	}

	inline const memb_info_rt* field_by_name(const char* name) const noexcept
	{
		return fields->by_name(name);
	}

	inline const memb_info_rt* function(size_t idx) const noexcept
	{
		return functions->at(idx);
	}

	inline const memb_info_rt* function_by_refl_name(const char* refl_name) const noexcept
	{
		return functions->by_refl_name(refl_name);
	}

	inline const memb_info_rt* function_by_name(const char* name) const noexcept
	{
		return functions->by_name(name);
	}
};

struct base_info_rt_list
{
	const size_t size;
	const unsigned char type_info[1];

	inline const type_info_rt_t* at(size_t idx) const noexcept
	{
		return *(const type_info_rt_t**)(type_info + idx * sizeof(type_info_rt_t*));
	}
};

inline const type_info_rt_t* type_info_rt_t::base(size_t idx) const noexcept
{
	return bases->at(idx);
}

typedef std::unordered_map<const char*, type_info_rt_t*, tplmp::cstr_djb2_hash_op, tplmp::cstr_equal_op> type_info_map;

inline type_info_map& __type_info_map() noexcept
{
	static type_info_map map;
	return map;
}

/**
 * @brief 查询指定名称的
 */
inline type_info_rt_t* type_info_rt(const char* refl_name) noexcept
{
	return __type_info_map()[refl_name];
}

template<typename _T>
inline type_info_rt_t* __type_info_rt_adl(tplmp::type_t<_T>) noexcept
{
	return nullptr;
}

template<typename _T>
inline type_info_rt_t* type_info_rt() noexcept
{
	return __type_info_rt_adl(tplmp::type_t<_T>());
}

// 定义基本类型
#define __static_reflect_def_primitive_intl__(i, begin_idx, end_idx, const_params, type)\
	template<typename _T>\
	inline ::reflect::type_info_rt_t* type_info_rt() noexcept;\
	template<typename _T>\
	struct __type_info_rt_t;\
	template<>\
	struct __type_info_rt_t<type>\
	{\
		const char* refl_name = __str__(type);\
		const char* name = __str__(type);\
		const ::reflect::reflect_type refl_type = ::reflect::reflect_type::type_primitive;\
		const ::reflect::base_info_rt_list* bases = nullptr;\
		const ::reflect::memb_info_rt_list* fields= nullptr;\
		const ::reflect::memb_info_rt_list* functions = nullptr;\
	private:\
		friend ::reflect::type_info_rt_t* type_info_rt<type>() noexcept;\
		__type_info_rt_t()\
		{\
			::reflect::__type_info_map()[__str__(type)] = (::reflect::type_info_rt_t*)this;\
		}\
		static const __type_info_rt_t<type> instance;\
	};\
	const __type_info_rt_t<type> __type_info_rt_t<type>::instance{};\
	template<>\
	inline ::reflect::type_info_rt_t* type_info_rt<type>() noexcept\
	{\
		return (::reflect::type_info_rt_t*)&__type_info_rt_t<type>::instance;\
	}

#define __static_reflect_def_primitive__(...)\
	__for_each__(0)(__static_reflect_def_primitive_intl__, , __VA_ARGS__)

#if __cplusplus < 202002L
__static_reflect_def_primitive__(bool, char, char16_t, char32_t, wchar_t, short,
		int, long, long long, float, double, long double, unsigned char, unsigned short,
		unsigned int, unsigned long, unsigned long long, void)
#else
__static_reflect_def_primitive__(bool, char, char8_t, char16_t, char32_t, wchar_t, short,
		int, long, long long, float, double, long double, unsigned char, unsigned short,
		unsigned int, unsigned long, unsigned long long, void)
#endif

#undef __static_reflect_def_primitive__
#undef __static_reflect_def_primitive_intl__

// ----- 动态反射类定义 -----
#if !defined(__reflect_dynamic_def_memb_loop_expand_id__)
#define __reflect_dynamic_def_memb_loop_expand_id__() 2
#endif
#if !defined(__reflect_dynamic_def_memb_call_def_class_expand_id__)
#define __reflect_dynamic_def_memb_call_def_class_expand_id__() 2
#endif
#if !defined(__reflect_dynamic_def_memb_call_def_memb_expand_id__)
#define __reflect_dynamic_def_memb_call_def_memb_expand_id__() 3
#endif

/**
 * @brief 定义编译时动态反射，无视访问权限获取成员指针。
 * class_info的变长参数列表为基类名称，基类列表使用__entity__()宏包围的类名称。
 * @detail 用例：
 * namespace ns {
 * class B
 * {
 * };
 * __dynamic_reflect__
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
 * 可以使用以下宏定义动态反射
 * __dynamic_reflect__
 * (
 * 	__refl_class__(__entity__(A), __entity__(B)),
 *  __refl_function__(__entity__(func), void(int)),
 *  __refl_field__(__entity__(x), float),
 *  __refl_field__(__entity__(y), int)
 *  __refl_function__(operator_add, __entity__(operator+), float(int))
 * )
 * 运行时访问type_info_rt的写法为reflect::type_info_rt<ns::A>()或reflect::type_info_rt("A")；
 * 访问x的成员信息的写法为type_info_rt->field_by_name("x")；
 * 访问func的成员信息的写法为type_info_rt->function_by_name("func")；
 * 访问operator+的成员信息的写法为type_info_rt->function_by_name("operator+")或type_info_rt->function_by_refl_name("operator_add")；
 */
#define __dynamic_reflect_def__(class_info, ...)\
	__call_exp__(__reflect_dynamic_def_memb_call_def_class_expand_id__())(__dynamic_reflect_def_intl__, __unpack__(class_info), __VA_ARGS__)

#define __dynamic_reflect_def_intl__(refl_type_class, refl_class_name, class_name, class_base_list, ...)\
	namespace __reflect\
	{\
	inline ::reflect::type_info_rt_t* __type_info_rt_adl(::tplmp::type_t<__entity_val__(class_name)>) noexcept;\
	template<typename _Class>\
	struct __type_info_rt_t;\
	template<>\
	struct __type_info_rt_t<__entity_val__(class_name)>\
	{\
		const char* refl_name = __entity_str__(refl_class_name);\
		const char* name = __entity_str__(class_name);\
		const ::reflect::reflect_type refl_type = ::reflect::reflect_type::refl_type_class;\
		void* bases;\
		void* fields;\
		void* functions;\
		const struct\
		{\
			const size_t size = __sizeof__(__unpack__(class_base_list));\
			__reflect_dynamic_bases_list__(class_base_list)\
		} __bases_list;\
		const struct\
		{\
			const size_t size = __reflect_dynamic_def_membs_size__(type_field, __VA_ARGS__);\
			const size_t memb_info_size = sizeof(::reflect::memb_info_rt) + sizeof(int __entity_val__(class_name)::*);\
			__reflect_dynamic_def_membs__(type_field, refl_class_name, class_name, __VA_ARGS__)\
		} __fields_list;\
		const struct\
		{\
			const size_t size = __reflect_dynamic_def_membs_size__(type_function, __VA_ARGS__);\
			const size_t memb_info_size = sizeof(::reflect::memb_info_rt) + sizeof(void(__entity_val__(class_name)::*)());\
			__reflect_dynamic_def_membs__(type_function, refl_class_name, class_name, __VA_ARGS__)\
		} __functions_list;\
	private:\
		friend inline ::reflect::type_info_rt_t* __type_info_rt_adl(::tplmp::type_t<__entity_val__(class_name)>) noexcept;\
		__type_info_rt_t()\
		{\
			bases = (void*)&this->__bases_list;\
			fields = (void*)&this->__fields_list;\
			functions = (void*)&this->__functions_list;\
			::reflect::__type_info_map()[__entity_str__(refl_class_name)] = (::reflect::type_info_rt_t*)this;\
		}\
		static const __type_info_rt_t<__entity_val__(class_name)> instance;\
	};\
	const __type_info_rt_t<__entity_val__(class_name)> __type_info_rt_t<__entity_val__(class_name)>::instance{};\
	inline ::reflect::type_info_rt_t* __type_info_rt_adl(::tplmp::type_t<__entity_val__(class_name)>) noexcept\
	{\
		return (::reflect::type_info_rt_t*)&__type_info_rt_t<__entity_val__(class_name)>::instance;\
	}\
	}\
	using __reflect::__type_info_rt_adl;

// ----- 定义基类列表 -----

#if !defined(__reflect_dynamic_bases_list_loop_expand_id__)
#define __reflect_dynamic_bases_list_loop_expand_id__() 3
#endif

#define __reflect_dynamic_bases_list_def_elements_op__(i, begin_idx, end_idx, const_params, base_class_name)\
	const ::reflect::type_info_rt_t* _##i = ::reflect::type_info_rt<__entity_val__(base_class_name)>();

#define __reflect_dynamic_bases_list__(class_base_list)\
	__for_each__(__reflect_dynamic_bases_list_loop_expand_id__())(__reflect_dynamic_bases_list_def_elements_op__, , __unpack__(class_base_list))

// ----- 成员定义 -----

#define __reflect_dynamic_def_memb_op__(i, begin_idx, end_idx, target_refl_type, refl_class_name, class_name, memb_info)\
	__call_exp__(__reflect_dynamic_def_memb_call_def_memb_expand_id__())(__reflect_dynamic_def_memb_op_intl__, target_refl_type, refl_class_name, class_name, __unpack__(memb_info))

/**
 * @brief 定义__memb_info_rt_t<>类型的成员作为字段，成员信息的实际储存位置
 */
#define __reflect_dynamic_def_memb_op_intl__(target_refl_type, refl_class_name, class_name, refl_memb_type, refl_memb_name, memb_name, memb_decl_type)\
	__if_intl__(__equal__(refl_memb_type, target_refl_type))\
	(\
		const ::reflect::__memb_info_rt_t<typename ::tplmp::ptr_type<__entity_val__(class_name), __entity_val__(memb_decl_type)>::type, __memb_ptr<__reflect_pmemb_id__(refl_class_name, refl_memb_name)>()>\
		__entity_val__(refl_memb_name)\
		{\
			__entity_str__(refl_class_name),\
			__entity_str__(class_name),\
			::reflect::type_info_rt<__entity_val__(class_name)>(),\
			__entity_str__(refl_memb_name),\
			__entity_str__(memb_name),\
			__entity_str__(memb_decl_type),\
			::reflect::type_info_rt<__entity_val__(memb_decl_type)>(),\
			::reflect::reflect_type::refl_memb_type\
		};\
	)

#define __reflect_dynamic_def_membs__(target_refl_type, refl_class_name, class_name, ...)\
	__for_each__(__reflect_dynamic_def_memb_loop_expand_id__())(__reflect_dynamic_def_memb_op__, __pack_list__(target_refl_type, refl_class_name, class_name), __VA_ARGS__)

/**
 * @brief 匹配target_refl_type的成员的数量
 */
#define __reflect_dynamic_def_memb_size_op__(i, begin_idx, end_idx, target_refl_type, memb_info)\
	__call_exp__(__reflect_dynamic_def_memb_call_def_memb_expand_id__())(__reflect_dynamic_def_memb_size_op_intl__, target_refl_type, __unpack__(memb_info))

#define __reflect_dynamic_def_memb_size_op_intl__(target_refl_type, refl_memb_type, refl_memb_name, memb_name, memb_decl_type)\
	__if_intl__(__equal__(refl_memb_type, target_refl_type))\
	(\
		__append_comma__(1)\
	)

#define __reflect_dynamic_def_membs_size__(target_refl_type, ...)\
	__sizeof__(__strip_trailing_1_comma__(__for_each__(__reflect_dynamic_def_memb_loop_expand_id__())(__reflect_dynamic_def_memb_size_op__, target_refl_type, __VA_ARGS__)))

/**
 * @brief 只定义动态反射
 */
#define __dynamic_reflect__(class_info, ...)\
	__reflect_decl_pmemb__(class_info, __VA_ARGS__)\
	__dynamic_reflect_def__(class_info, __VA_ARGS__)
}

#endif//_REFLECT_DYNAMICREFLECT
