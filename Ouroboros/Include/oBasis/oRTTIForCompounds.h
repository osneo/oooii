// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oRTTIForCompounds_h
#define oRTTIForCompounds_h

#include <oBase/version.h>

enum oRTTI_COMPOUND_ATTR_FLAGS
{
	oRTTI_COMPOUND_ATTR_REGULAR					= 0x0000,
	oRTTI_COMPOUND_ATTR_HIDDEN					= 0x0001,
	oRTTI_COMPOUND_ATTR_READ_ONLY				= 0x0002,
	oRTTI_COMPOUND_ATTR_DONT_SERIALIZE			= 0x0010,
	oRTTI_COMPOUND_ATTR_FAIL_IF_MISSING			= 0x0020,
	oRTTI_COMPOUND_ATTR_OPTIONAL				= 0x0040,
	oRTTI_COMPOUND_ATTR_XML_STYLE_RAW_ARRAY		= 0x0080,
	oRTTI_COMPOUND_ATTR_XML_STYLE_NODE			= 0x0100,
	oRTTI_COMPOUND_ATTR_XML_STYLE_NODE_EMBEDDED = 0x0200 | oRTTI_COMPOUND_ATTR_XML_STYLE_NODE,
	oRTTI_COMPOUND_ATTR_PRIVATE					= oRTTI_COMPOUND_ATTR_HIDDEN | oRTTI_COMPOUND_ATTR_DONT_SERIALIZE,
};

class oRTTI_OBJECT;

template <typename T>
struct oRTTI_DATA_COMPOUND // : oRTTI
{
	struct BASE
	{
		oRTTI* RTTI;
		uint Offset;
	};

	struct ATTR
	{
		inline bool IsGroup() const { return Type == nullptr; }
		inline bool IsVirtual() const { return Offset == ouro::invalid; }
		inline bool DontSerialize() const { return (Flags & oRTTI_COMPOUND_ATTR_DONT_SERIALIZE) == oRTTI_COMPOUND_ATTR_DONT_SERIALIZE; }

		const oRTTI* RTTI;
		const char* Name;
		uint Offset;
		uint Size;
		uint Flags;

		// Pointers to member functions can have a different size based on whether the type has
		// multiple-inheritance or not. If you use a forward declared type (like oRTTI_OBJECT)
		// it assumes multiple-inheritance. We exploit this property to make sure our template
		// leaves enough room to support both cases. Note that the size difference shows only
		// in alignment, not actual sizeof().
		#pragma warning (disable: 4121)
		class dummy;
		union {
			void (T::*GetValueFn)(void* _pValue);
			void (dummy::*GetValueFn2)(void* _pValue);
		};
		union {
			void (T::*SetValueFn)(const void* _pValue);
			void (dummy::*SetValueFn2)(const void* _pValue);
		};
		#pragma warning (default: 4121)
	};

	typedef void (T::*oRTTICompoundAttrChanged)(const oRTTI_DATA_COMPOUND<oRTTI_OBJECT>::ATTR* _pAttr);
	typedef bool (T::*oRTTICompoundFromString)(const char* _StrSource);
	typedef char* (T::*oRTTICompoundToString)(char* _StrDestination, size_t _SizeofStrDestination) const;

	uchar Type;
	uchar NumBases;
	uchar NumAttrs;
//	uchar NumFunctions;
	ouro::version Version;
	uint Size;
	const char* TypeName;
	uint TypeNameHash;
	const BASE* Bases;
	const ATTR* Attrs;
//	const oRTTI_FUNCTION<T>* Functions;
	oRTTIConstructor Constructor;
	oRTTIDestructor Destructor;
	oRTTICompoundAttrChanged AttrChanged;
	oRTTICompoundFromString FromString;
	oRTTICompoundToString ToString;
};

// Declaration macros

#define oRTTI_COMPOUND_DECLARATION(rtti_add_caps, compound_type) \
	extern oRTTI_DATA_COMPOUND<compound_type> oRTTI_##compound_type; \
	rtti_add_caps(DECLARATION, compound_type)

// Compound type has to be canonical, which in our case means that it can't
// have spaces or colons in it, e.g. "Type::SubClass" or "unsigned int"
// So you can use this macro to declare a compound that's non canonical
// for all the other RTTI macros then use the canonical compound type.
// Example: oRTTI_COMPOUND_DECLARATION_NON_CANONICAL(RTTI_CAPS_NONE, Class::SubClass, Class_SubClass)
#define oRTTI_COMPOUND_DECLARATION_NON_CANONICAL(rtti_add_caps, non_canonical_compound_type, compound_type) \
	typedef non_canonical_compound_type compound_type; \
	oRTTI_COMPOUND_DECLARATION(rtti_add_caps, compound_type)


// Internal helpers to support not having to describe when something is not there (base classes, attributes, override functions, callback functions)
#define oRTTI_COMPOUND_VERSION_DEFAULT(major, minor, build, revision) \
	template<typename T> struct oRTTI_COMPOUND_VERSION_HELPER_IS_VALID { enum { VALID = 0 }; }; \
	template<typename T, bool U> struct oRTTI_COMPOUND_VERSION_HELPER { }; \
	template<typename T> struct oRTTI_COMPOUND_VERSION_HELPER<T, false> { static const unsigned short Major = major; static const unsigned short Minor = minor; static const unsigned short Build = build; static const unsigned short Revision = revision; };

#define oRTTI_COMPOUND_SET_VERSION(compound_type, major, minor, build, revision) \
	template<> struct oRTTI_COMPOUND_VERSION_HELPER_IS_VALID<compound_type> { enum { VALID = 1 }; }; \
	template<> struct oRTTI_COMPOUND_VERSION_HELPER<compound_type, true> { static const unsigned short Major = major; static const unsigned short Minor = minor; static const unsigned short Build = build; static const unsigned short Revision = revision; };

#define oRTTI_COMPOUND_GET_VERSION(compound_type) \
	ouro::version(	oRTTI_COMPOUND_VERSION_HELPER<compound_type, oRTTI_COMPOUND_VERSION_HELPER_IS_VALID<compound_type>::VALID != 0>::Major, \
				oRTTI_COMPOUND_VERSION_HELPER<compound_type, oRTTI_COMPOUND_VERSION_HELPER_IS_VALID<compound_type>::VALID != 0>::Minor, \
				oRTTI_COMPOUND_VERSION_HELPER<compound_type, oRTTI_COMPOUND_VERSION_HELPER_IS_VALID<compound_type>::VALID != 0>::Build, \
				oRTTI_COMPOUND_VERSION_HELPER<compound_type, oRTTI_COMPOUND_VERSION_HELPER_IS_VALID<compound_type>::VALID != 0>::Revision)


#define oRTTI_COMPOUND_FUNCTION_TEMPLATE_SET_NULLPTR_DEFAULT(function_label) \
	template<typename T> struct oRTTICompoundFunction##function_label##IsValid { enum { VALID = 0 }; }; \
	template<typename T, bool U> struct oRTTICompoundFunction##function_label { }; \
	template<typename T> struct oRTTICompoundFunction##function_label<T, false> { static const int POINTER = 0; };

#define oRTTI_COMPOUND_FUNCTION_TEMPLATE_SET_POINTER(compound_type, function_label, function_pointer) \
	template<> struct oRTTICompoundFunction##function_label##IsValid<compound_type> { enum { VALID = 1 }; }; \
	template<> struct oRTTICompoundFunction##function_label<compound_type, true> { static const oRTTI_DATA_COMPOUND<compound_type>::oRTTICompound##function_label POINTER; }; \
	const oRTTI_DATA_COMPOUND<compound_type>::oRTTICompound##function_label oRTTICompoundFunction##function_label<compound_type, true>::POINTER = function_pointer;

#define oRTTI_COMPOUND_FUNCTION_TEMPLATE_GET_POINTER(compound_type, function_label) \
	(const oRTTI_DATA_COMPOUND<compound_type>::oRTTICompound##function_label)(oRTTICompoundFunction##function_label<compound_type, oRTTICompoundFunction##function_label##IsValid<compound_type>::VALID != 0>::POINTER)


#define oRTTI_COMPOUND_ARRAY_TEMPLATE_SET_EMPTY_DEFAULT(array_element_type, array_label) \
	template<typename T> struct oRTTICompoundArray##array_label##Count { enum { COUNT = 0 }; }; \
	template<typename T, bool U> struct oRTTICompoundArray##array_label##Pointer { }; \
	template<typename T> struct oRTTICompoundArray##array_label##Pointer<T, false> { enum { s##array_label = 0 }; };

#define oRTTI_COMPOUND_ARRAY_TEMPLATE_SET_POINTER_AND_COUNT(compound_type, array_element_type, array_label) \
	template<> \
	struct oRTTICompoundArray##array_label##Count<compound_type> { enum { COUNT = sizeof(oRTTI##array_label##compound_type) / sizeof(oRTTI_DATA_COMPOUND<compound_type>::array_element_type) }; }; \
	template<> struct oRTTICompoundArray##array_label##Pointer<compound_type, true>	{ static const oRTTI_DATA_COMPOUND<compound_type>::array_element_type * const s##array_label; }; \
	const oRTTI_DATA_COMPOUND<compound_type>::array_element_type * const oRTTICompoundArray##array_label##Pointer<compound_type, true>::s##array_label = oRTTI##array_label##compound_type;

#define oRTTI_COMPOUND_ARRAY_TEMPLATE_GET_COUNT(compound_type, array_label) \
	oRTTICompoundArray##array_label##Count<compound_type>::COUNT

#define oRTTI_COMPOUND_ARRAY_TEMPLATE_GET_POINTER(compound_type, array_element_type, array_label) \
	(const oRTTI_DATA_COMPOUND<compound_type>::array_element_type *)(oRTTICompoundArray##array_label##Pointer<compound_type, oRTTICompoundArray##array_label##Count<compound_type>::COUNT != 0>::s##array_label)


// Description macros

#define oRTTI_COMPOUND_BEGIN_DESCRIPTION(rtti_add_caps, compound_type) \
	rtti_add_caps(DESCRIPTION, compound_type)

#define oRTTI_COMPOUND_END_DESCRIPTION(compound_type) \
	oRTTI_DATA_COMPOUND<compound_type> oRTTI_##compound_type = { \
		oRTTI_TYPE_COMPOUND, \
		oRTTI_COMPOUND_ARRAY_TEMPLATE_GET_COUNT(compound_type, Bases), \
		oRTTI_COMPOUND_ARRAY_TEMPLATE_GET_COUNT(compound_type, Attrs), \
		oRTTI_COMPOUND_GET_VERSION(compound_type), \
		sizeof(compound_type), \
		#compound_type, \
		ouro::invalid, \
		oRTTI_COMPOUND_ARRAY_TEMPLATE_GET_POINTER(compound_type, BASE, Bases), \
		oRTTI_COMPOUND_ARRAY_TEMPLATE_GET_POINTER(compound_type, ATTR, Attrs), \
		oRTTIConstruct##compound_type, \
		oRTTIDestruct##compound_type, \
		oRTTI_COMPOUND_FUNCTION_TEMPLATE_GET_POINTER(compound_type, AttrChanged), \
		oRTTI_COMPOUND_FUNCTION_TEMPLATE_GET_POINTER(compound_type, FromString), \
		oRTTI_COMPOUND_FUNCTION_TEMPLATE_GET_POINTER(compound_type, ToString), \
	};

// Describe base classes

#define oRTTI_COMPOUND_BASES_BEGIN(compound_type) \
	static oRTTI_DATA_COMPOUND<compound_type>::BASE oRTTIBases##compound_type[] = {

#define oRTTI_COMPOUND_BASES_END(compound_type) \
	}; \
	oRTTI_COMPOUND_ARRAY_TEMPLATE_SET_POINTER_AND_COUNT(compound_type, BASE, Bases)

#define oRTTI_COMPOUND_BASES_BASE(compound_type, compound_base_type) \
	{ \
		(oRTTI*)&oRTTI_##compound_base_type, \
		((uint)((size_t)((compound_base_type *)((compound_type *)1)) - 1)) \
	},

#define oRTTI_COMPOUND_BASES_SINGLE_BASE(compound_type, compound_base_type) \
	oRTTI_COMPOUND_BASES_BEGIN(compound_type) \
	oRTTI_COMPOUND_BASES_BASE(compound_type, compound_base_type) \
	oRTTI_COMPOUND_BASES_END(compound_type)

// Describe compound type

#define oRTTI_COMPOUND_CONCRETE(compound_type) \
	static void oRTTIConstruct##compound_type##Impl(const oRTTI& _RTTI, void* _pData) \
	{ \
		new (_pData) compound_type; \
	} \
	static void oRTTIDestruct##compound_type##Impl(const oRTTI& _RTTI, void* _pData) \
	{ \
		((compound_type *)_pData)->~compound_type(); \
	} \
	static const oRTTIConstructor oRTTIConstruct##compound_type = oRTTIConstruct##compound_type##Impl; \
	static const oRTTIDestructor oRTTIDestruct##compound_type  = oRTTIDestruct##compound_type##Impl;

#define oRTTI_COMPOUND_ABSTRACT(compound_type) \
	static const oRTTIConstructor oRTTIConstruct##compound_type = nullptr; \
	static const oRTTIDestructor oRTTIDestruct##compound_type = nullptr;

// Describe compound version

#define oRTTI_COMPOUND_VERSION(compound_type, major, minor, build, revision) \
	oRTTI_COMPOUND_SET_VERSION(compound_type, major, minor, build, revision)

// Describe compound custom functions

#define oRTTI_COMPOUND_CUSTOM_FROMSTRING(compound_type, compound_fromstring) \
	oRTTI_COMPOUND_FUNCTION_TEMPLATE_SET_POINTER(compound_type, FromString, &compound_type::compound_fromstring)

#define oRTTI_COMPOUND_CUSTOM_TOSTRING(compound_type, compound_tostring) \
	oRTTI_COMPOUND_FUNCTION_TEMPLATE_SET_POINTER(compound_type, ToString, &compound_type::compound_tostring)

// Describe compound callbacks

#define oRTTI_COMPOUND_ON_ATTR_CHANGED(compound_type, compound_attrchanged) \
	oRTTI_COMPOUND_FUNCTION_TEMPLATE_SET_POINTER(compound_type, AttrChanged, &compound_type::compound_attrchanged)

// Describe attributes

#define oRTTI_COMPOUND_ATTRIBUTES_BEGIN(compound_type) \
	static oRTTI_DATA_COMPOUND<compound_type>::ATTR oRTTIAttrs##compound_type[] = {

#define oRTTI_COMPOUND_ATTRIBUTES_END(compound_type) \
	}; \
	oRTTI_COMPOUND_ARRAY_TEMPLATE_SET_POINTER_AND_COUNT(compound_type, ATTR, Attrs)

// Returns the size of a member
#define oRTTI_SIZEOF(s, m) sizeof(((s*)0)->m)

#define oRTTI_COMPOUND_ATTR(compound_type, attr_label, attr_rtti, attr_name, attr_flags) \
	{ \
		&attr_rtti, \
		attr_name, \
		offsetof(compound_type, attr_label), \
		oRTTI_SIZEOF(compound_type, attr_label), \
		attr_flags, \
		nullptr, \
		nullptr, \
	},

#define oRTTI_COMPOUND_ATTR_VIRTUAL(compound_type, attr_rtti, attr_name, attr_get, attr_set, attr_flags) \
	{ \
		&attr_rtti, \
		attr_name, \
		ouro::invalid, \
		ouro::invalid, \
		attr_flags, \
		(void (compound_type::*)(void*))&compound_type::attr_get, \
		(void (compound_type::*)(const void*))&compound_type::attr_set, \
	},

#define oRTTI_COMPOUND_ATTR_GROUP(attr_group_name) \
	{ \
		nullptr, \
		attr_group_name, \
		0, \
		0, \
		0, \
		nullptr, \
		nullptr, \
	},


// Default values for anything that is not described for compound types

oRTTI_COMPOUND_VERSION_DEFAULT(0,1,0,0)

oRTTI_COMPOUND_FUNCTION_TEMPLATE_SET_NULLPTR_DEFAULT(AttrChanged)
oRTTI_COMPOUND_FUNCTION_TEMPLATE_SET_NULLPTR_DEFAULT(FromString)
oRTTI_COMPOUND_FUNCTION_TEMPLATE_SET_NULLPTR_DEFAULT(ToString)

oRTTI_COMPOUND_ARRAY_TEMPLATE_SET_EMPTY_DEFAULT(ATTR, Attrs)
oRTTI_COMPOUND_ARRAY_TEMPLATE_SET_EMPTY_DEFAULT(BASE, Bases)
//oRTTI_COMPOUND_ARRAY_TEMPLATE_SET_EMPTY_DEFAULT(oRTTI_FUNCTION<T>, Functions)

#endif
