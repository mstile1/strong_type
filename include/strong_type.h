// strong_type.h
//
// How to create a strong_type:
//    1. inherit from strong type
//      : @param crtp_type: derived type
//      : @param ty_t : underlying type
//      : any skills that are needed ( effectively inheriting operators like operator==, operator+, operator*( float ) )
//    2. !!inherit strong type constructors!!
//    3. customize with member functions if needed ( be careful with ctor conflicts with strong_type ctors )
//
//    @example: strong floating point radian type with some inherited skills
//    struct Radian : public roam::strong_type< Radian, double, roam::st_cmp, roam::st_math >
//    {
//        using strong_type::strong_type; // inherit strong_type ctors
//    };

#ifndef ROAM_STRONG_TYPE_H
#define ROAM_STRONG_TYPE_H

//-----------------------------------------------------------------------------

#include "strong_type_skills.h"

#include <type_traits>

//-----------------------------------------------------------------------------

namespace roam
{

// @param crtp_type: the crtp derived class for forwarding to skills
// @param ty_t : base underlying type, e.g. float
template < typename crtp_type, typename ty_t, template< typename > class... skills >
class strong_type : public skills< crtp_type >...
{
    using my_type = strong_type< crtp_type, ty_t, skills... >;

public:
    using value_type = ty_t;

    constexpr strong_type() = default;

    // explicit copy construct
    explicit constexpr strong_type( ty_t const& v ) : v_{ v }
    {
    }
    // allow move from non-references
    explicit constexpr strong_type( ty_t&& v ) noexcept( std::is_nothrow_move_constructible_v< ty_t > ) :
        v_{ std::move( v ) }
    {
    }

    constexpr auto get() noexcept -> ty_t&
    {
        return v_;
    }
    constexpr auto get() const noexcept -> ty_t const&
    {
        return v_;
    }

private:
    ty_t v_{};
};

} // roam

//-----------------------------------------------------------------------------

#endif // ROAM_STRONG_TYPE_H
