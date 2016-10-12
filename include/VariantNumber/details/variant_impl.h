#ifndef VARIANT_IMPL_H
#define VARIANT_IMPL_H

#include <type_traits>
#include <limits>
#include <iostream>
#include "VariantNumber/typeid.h"
#include "VariantNumber/details/exceptions.h"

namespace VariantNumber
{

namespace detail{

template <typename T>
using Invoke = typename T::type;


template <typename BoolCondition>
using EnableIf = Invoke<std::enable_if<BoolCondition::value> >;


template <typename T>
struct is_integer : std::integral_constant<bool, std::is_integral<T>::value
        && !std::is_same<T, bool>::value
        && !std::is_same<T, char>::value>
{};

template <typename From, typename To>
struct is_safe_integer_conversion
        : std::integral_constant<bool, detail::is_integer<From>::value
        && detail::is_integer<To>::value
        && sizeof(From) <= sizeof(To)
&& std::is_signed<From>::value == std::is_signed<To>::value>
{};

template <typename From, typename To>
struct unsigned_to_smaller_conversion
        : std::integral_constant<bool, detail::is_integer<From>::value
        && detail::is_integer<To>::value
        && (sizeof(From) > sizeof(To))
&& !std::is_signed<From>::value
&& !std::is_signed<To>::value >
{};

template <typename From, typename To>
struct signed_to_smaller_conversion
        : std::integral_constant<bool, detail::is_integer<From>::value
        && detail::is_integer<To>::value
        && (sizeof(From) > sizeof(To))
&& std::is_signed<From>::value
&& std::is_signed<To>::value >
{};

//---------------------------
template <typename From, typename To>
struct signed_to_smaller_unsigned_conversion
        : std::integral_constant<bool, detail::is_integer<From>::value
        && detail::is_integer<To>::value
        && sizeof(From) >= sizeof(To)
&& std::is_signed<From>::value
&& !std::is_signed<To>::value >
{};

template <typename From, typename To>
struct signed_to_larger_unsigned_conversion
        : std::integral_constant<bool, detail::is_integer<From>::value
        && detail::is_integer<To>::value
        && sizeof(From) < sizeof(To)
&& std::is_signed<From>::value
&& !std::is_signed<To>::value >
{};

template <typename From, typename To>
struct unsigned_to_smaller_signed_conversion
        : std::integral_constant<bool, detail::is_integer<From>::value
        && detail::is_integer<To>::value
        && (sizeof(From) >= sizeof(To))
&& !std::is_signed<From>::value
&& std::is_signed<To>::value >
{};

template <typename From, typename To>
struct unsigned_to_larger_signed_conversion
        : std::integral_constant<bool, detail::is_integer<From>::value
        && detail::is_integer<To>::value
        && sizeof(From) < sizeof(To)
&& !std::is_signed<From>::value
&& std::is_signed<To>::value >
{};

template <typename From, typename To>
struct floating_to_signed_conversion
        : std::integral_constant<bool, std::is_floating_point<From>::value
        && detail::is_integer<To>::value
        && std::is_signed<To>::value >
{};

template <typename From, typename To>
struct floating_to_unsigned_conversion
        : std::integral_constant<bool, std::is_floating_point<From>::value
        && detail::is_integer<To>::value
        && !std::is_signed<To>::value >
{};
}

//----------------------- Implementation ----------------------------------------------

template<typename SRC,typename DST,
         typename detail::EnableIf< detail::is_safe_integer_conversion<SRC, DST>>* = nullptr >
FORCE_INLINE void convert_impl( const SRC& from, DST& target )
{
    //std::cout << "is_safe_integer_conversion" << std::endl;
    target = static_cast<DST>( from);
}


template<typename SRC,typename DST,
         typename detail::EnableIf< detail::unsigned_to_smaller_conversion<SRC, DST>>* = nullptr  >
FORCE_INLINE void convert_impl( const SRC& from, DST& target )
{
    //std::cout << "unsigned_to_smaller_conversion" << std::endl;

    if ( from > static_cast<SRC>(std::numeric_limits<DST>::max()) )
        throw RangeException("Value larger than maximum limit");

    target = static_cast<DST>( from);
}

template<typename SRC,typename DST,
         typename  detail::EnableIf< detail::signed_to_smaller_conversion<SRC, DST>>* = nullptr  >
FORCE_INLINE void convert_impl( const SRC& from, DST& target )
{
    //std::cout << "signed_to_smaller_conversion" << std::endl;

    if ( from < static_cast<SRC>(std::numeric_limits<DST>::min()) )
        throw RangeException("Value smaller than minum limit");

    if ( from > static_cast<SRC>(std::numeric_limits<DST>::max()) )
        throw RangeException("Value larger than maximum limit");

    target = static_cast<DST>( from);
}


template<typename SRC,typename DST,
         typename detail::EnableIf< detail::signed_to_smaller_unsigned_conversion<SRC, DST>>* = nullptr  >
FORCE_INLINE void convert_impl( const SRC& from, DST& target )
{
    //std::cout << "signed_to_smaller_unsigned_conversion" << std::endl;
     if (from < static_cast<SRC>(std::numeric_limits<DST>::min()) )
        throw RangeException("Value is negative and can't be converted to signed");

    if (from > static_cast<SRC>(std::numeric_limits<DST>::max()) )
        throw RangeException("Value larger than maximum limit");

    target = static_cast<DST>( from);
}


template<typename SRC,typename DST,
         typename detail::EnableIf< detail::signed_to_larger_unsigned_conversion<SRC, DST>>* = nullptr   >
FORCE_INLINE void convert_impl( const SRC& from, DST& target )
{
    //std::cout << "signed_to_larger_unsigned_conversion" << std::endl;

    if ( from < 0 )
        throw RangeException("Value is negative and can't be converted to signed");
    target = static_cast<DST>( from);
}

template<typename SRC,typename DST,
         typename detail::EnableIf< detail::unsigned_to_larger_signed_conversion<SRC, DST>>* = nullptr   >
FORCE_INLINE void convert_impl( const SRC& from, DST& target )
{
    //std::cout << "unsigned_to_larger_signed_conversion" << std::endl;
    target = static_cast<DST>( from);
}

template<typename SRC,typename DST,
         typename detail::EnableIf< detail::unsigned_to_smaller_signed_conversion<SRC, DST>>* = nullptr   >
FORCE_INLINE void convert_impl( const SRC& from, DST& target )
{
    //std::cout << "unsigned_to_smaller_signed_conversion" << std::endl;

    if ( from > static_cast<SRC>(std::numeric_limits<DST>::max()) )
        throw RangeException("Value larger than maximum limit");
    target = static_cast<DST>( from);
}

template<typename SRC,typename DST,
         typename detail::EnableIf< detail::floating_to_signed_conversion<SRC, DST>>* = nullptr   >
FORCE_INLINE void convert_impl( const SRC& from, DST& target )
{
    //std::cout << "floating_to_signed_conversion" << std::endl;

    if (from > (std::numeric_limits<DST>::max()) ||
             from < (std::numeric_limits<DST>::min()) )
        throw RangeException("Floating point out of range");

    if( from != static_cast<SRC>(static_cast<DST>( from)))
        throw RangeException("Floating point truncated");

    target = static_cast<DST>( from);
}

template<typename SRC,typename DST,
         typename detail::EnableIf< detail::floating_to_unsigned_conversion<SRC, DST>>* = nullptr   >
FORCE_INLINE void convert_impl( const SRC& from, DST& target )
{
    //std::cout << "floating_to_unsigned_conversion" << std::endl;
    if ( from <0 || from > std::numeric_limits<DST>::max())
        throw RangeException("Value out of range");

    if( from != static_cast<SRC>(static_cast<DST>( from)))
        throw RangeException("Floating point truncated");

    target = static_cast<DST>( from);
}


template<typename DST>
FORCE_INLINE DST VarNumber::convert()
{
    DST target;

    //----------
    switch( _raw_data[8] )
    {
    case Int8:   convert_impl<int8_t,  DST>(*reinterpret_cast<const int8_t*>( _raw_data), target  ); break;
    case Int16:  convert_impl<int16_t, DST>(*reinterpret_cast<const int16_t*>( _raw_data), target  ); break;
    case Int32:  convert_impl<int32_t, DST>(*reinterpret_cast<const int32_t*>( _raw_data), target  ); break;
    case Int64:  convert_impl<int64_t, DST>(*reinterpret_cast<const int64_t*>( _raw_data), target  ); break;

    case UInt8:   convert_impl<uint8_t,  DST>(*reinterpret_cast<const uint8_t*>( _raw_data), target  ); break;
    case UInt16:  convert_impl<uint16_t, DST>(*reinterpret_cast<const uint16_t*>( _raw_data), target  ); break;
    case UInt32:  convert_impl<uint32_t, DST>(*reinterpret_cast<const uint32_t*>( _raw_data), target  ); break;
    case UInt64:  convert_impl<uint64_t, DST>(*reinterpret_cast<const uint64_t*>( _raw_data), target  ); break;

    case Real32:  convert_impl<float, DST>(*reinterpret_cast<const float*>( _raw_data), target  ); break;
    case Real64:  convert_impl<double, DST>(*reinterpret_cast<const double*>( _raw_data), target  ); break;

    default:  throw TypeException("ops"); break;

    }

    return  target;
}

} //end namespace

#endif // VARIANT_H

