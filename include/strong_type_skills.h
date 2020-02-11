// strong_type_skills.h

#ifndef ROAM_STRONG_TYPE_SKILLS_H
#define ROAM_STRONG_TYPE_SKILLS_H

//-----------------------------------------------------------------------------

#include <cmath>       // for float equality
#include <limits>      // for float equality

#include <cstdint>     // for int operators
#include <type_traits> // is_float

//-----------------------------------------------------------------------------

namespace roam
{
// Base ( utility to grab underlying strong type or value ref )
//=============================================================================
template < typename T, template< typename > class constraint >
struct st_skill
{
    // convert derived skill to underlying strong type
    constexpr T& underlying() { return static_cast< T& >( *this ); }
    constexpr T const& underlying() const { return static_cast< T const& >( *this ); }
    // directly access strong type value
    constexpr auto& value() { return underlying().get(); }
    constexpr auto const& value() const { return underlying().get(); }
};

// Equality ( with safe floating point compare )
//=============================================================================
namespace detail
{
    template < typename T >
    [[nodiscard]] inline constexpr bool st_test_equality( T const& a, T const& b )
    {
        if constexpr ( std::is_floating_point_v< T > )
        {   // safe float equality, not explicitly needed for strong types
            auto constexpr c_base_eps = std::numeric_limits< T >::epsilon();
            auto const scaled_eps = c_base_eps * std::fmax( std::fabs( a ), std::fabs( b ) );
            return std::fabs( a - b ) <= scaled_eps;
        }
        else {
            return a == b;
        }
    }
} // detail

template < typename T >
struct st_eq : public st_skill< T, st_eq >
{
    [[nodiscard]] friend constexpr bool operator==( st_eq< T > const& lhs, st_eq< T > const& rhs )
    {
        return detail::st_test_equality( lhs.value(), rhs.value() );
    }
    [[nodiscard]] friend constexpr bool operator!=( st_eq< T > const& lhs, st_eq< T > const& rhs )
    {
        return !( lhs == rhs );
    }
};

// Comparison
//=============================================================================
template < typename T >
struct st_cmp : public st_eq< T > // comparisons inherit equality functionality
{
    [[nodiscard]] friend constexpr bool operator<( st_cmp< T > const& lhs, st_cmp< T > const& rhs )
    {   // @note: implemented to leverage equality specializations above, e.g. for float types
        return lhs.value() < rhs.value() && !( lhs == rhs );
    }
    [[nodiscard]] friend constexpr bool operator>=( st_cmp< T > const& lhs, st_cmp< T > const& rhs )
    {
        return !( lhs < rhs );
    }
    [[nodiscard]] friend constexpr bool operator>( st_cmp< T > const& lhs, st_cmp< T > const& rhs )
    {
        return rhs < lhs;
    }
    [[nodiscard]] friend constexpr bool operator<=( st_cmp< T > const& lhs, st_cmp< T > const& rhs )
    {
        return !( rhs < lhs );
    }
};

// Arithmetic
//=============================================================================
// return value of op( lhs, rhs )
#define ROAM_OP_VAL( op, op_name ) \
    [[nodiscard]] friend constexpr T operator op( op_name< T > const& lhs, op_name< T > const& rhs ) { \
        return T{ lhs.value() op rhs.value() }; \
    }

// return value of op( lhs, custom rhs )
#define ROAM_OP_VAL_RHS( op, op_name, rhs_type ) \
    [[nodiscard]] friend constexpr T operator op( op_name< T > const& lhs, rhs_type const& rhs ) { \
        return T{ lhs.value() op rhs }; \
    } \
    [[nodiscard]] friend constexpr T operator op( rhs_type const& lhs, op_name< T > const& rhs ) { \
        return T{ lhs op rhs.value() }; \
    }

// return value of op( lhs )
#define ROAM_OP_VAL_UNARY( op, op_name ) \
    [[nodiscard]] friend constexpr T operator op( op_name< T > const& lhs ) { \
        return T{ op lhs.value() }; \
    }

// modify lhs and return ref
// note: lhs.value() may not be mutable, so we can't modify in place, e.g. +=
#define ROAM_OP_REF( op, op_name ) \
    friend constexpr T& operator op( op_name< T >& lhs, op_name< T > const& rhs ) { \
        auto tmp = lhs.value(); \
        tmp op rhs.value(); \
        return lhs.underlying() = T{ tmp }; \
    }

// modify lhs and return ref
#define ROAM_OP_REF_RHS( op, op_name, rhs_type ) \
    friend constexpr T& operator op( op_name< T >& lhs, rhs_type const& rhs ) { \
        auto tmp = lhs.value(); \
        tmp op rhs; \
        return lhs.underlying() = T{ tmp }; \
    }
    // note: explicitly leaving out symmetric version for now: e.g. type += strong_type

template < typename T >
struct st_add : public st_skill< T, st_add >
{
    ROAM_OP_VAL( +, st_add );
    ROAM_OP_REF( +=, st_add );
};

template < typename T >
struct st_sub : public st_skill< T, st_sub >
{
    ROAM_OP_VAL( -, st_sub );
    ROAM_OP_VAL_UNARY( -, st_sub );
    ROAM_OP_REF( -=, st_sub );
};

template < typename T >
struct st_mul_t : public st_skill< T, st_mul_t >
{
    ROAM_OP_VAL( *, st_mul_t );
    ROAM_OP_REF( *=, st_mul_t );
};

template < typename T >
struct st_mul_f : public st_skill< T, st_mul_f >
{
    ROAM_OP_VAL_RHS( *,  st_mul_f, float );
    ROAM_OP_REF_RHS( *=, st_mul_f, float );
    ROAM_OP_VAL_RHS( *,  st_mul_f, double );
    ROAM_OP_REF_RHS( *=, st_mul_f, double );
};

template < typename T >
struct st_mul_i : public st_skill< T, st_mul_i >
{
    ROAM_OP_VAL_RHS( *,  st_mul_i, int32_t );
    ROAM_OP_REF_RHS( *=, st_mul_i, int32_t );
    ROAM_OP_VAL_RHS( *,  st_mul_i, uint32_t );
    ROAM_OP_REF_RHS( *=, st_mul_i, uint32_t );
    ROAM_OP_VAL_RHS( *,  st_mul_i, int64_t );
    ROAM_OP_REF_RHS( *=, st_mul_i, int64_t );
};

template < typename T >
struct st_div_t : public st_skill< T, st_div_t >
{
    ROAM_OP_VAL( /, st_div_t );
    ROAM_OP_REF( /=, st_div_t );
};

template < typename T >
struct st_div_f : public st_skill< T, st_div_f >
{
    ROAM_OP_VAL_RHS( /,  st_div_f, float );
    ROAM_OP_REF_RHS( /=, st_div_f, float );
    ROAM_OP_VAL_RHS( /,  st_div_f, double );
    ROAM_OP_REF_RHS( /=, st_div_f, double );
};

template < typename T >
struct st_div_i : public st_skill< T, st_div_i >
{
    ROAM_OP_VAL_RHS( /,  st_div_i, int32_t );
    ROAM_OP_REF_RHS( /=, st_div_i, int32_t );
    ROAM_OP_VAL_RHS( /,  st_div_i, uint32_t );
    ROAM_OP_REF_RHS( /=, st_div_i, uint32_t );
    ROAM_OP_VAL_RHS( /,  st_div_i, int64_t );
    ROAM_OP_REF_RHS( /=, st_div_i, int64_t );
};

// composites
template < typename T >
struct st_mul : public st_mul_t< T >, public st_mul_f< T >, public st_mul_i< T >
{
};

template < typename T >
struct st_div : public st_div_t< T >, public st_div_f< T >, public st_div_i< T >
{
};

// @note: this currently contains 'mul_f/mul_i' AND 'div_f/div_i', separate if needed
template < typename T >
struct st_math : public st_add< T >, public st_sub< T >, public st_mul< T >, public st_div< T >
{
};

} // roam

//-----------------------------------------------------------------------------

#endif // ROAM_STRONG_TYPE_SKILLS_H
