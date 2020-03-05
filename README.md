# strong_type
Strong typing via inheritance, with the ability to also inherit 'skills'/operations.

A canonical example is math functions that desire a floating point angle as a parameter.
Is this angle in degrees or radians? Probably radians.
But it's often common for data driven "angles" to be specified and stored as degrees as they're easier to visualize.

```
struct degree;
struct radian : public roam::strong_type< radian, double, roam::st_cmp, roam::st_math >
{
    using strong_type::strong_type;
    operator degree() const; // implicit conversion to degree
};

struct degree : public roam::strong_type< degree, double, roam::st_cmp, roam::st_math >
{
    using strong_type::strong_type;
    operator radian() const; // implicit conversion to radian
};

inline constexpr c_pi = 3.1415926536;
inline degree::operator radian() const
{
    return radian{ this->get() / 180 * c_pi };
}
inline radian::operator degree() const
{
    return degree{ this->get() / c_pi * 180 };
}
```
```
// common function signature
//--------------------------------------------------------
void apply_some_kind_of_rotation( float angle ); // please pass me radians
apply_some_kind_of_rotation( 90.0 ); // whoops
```
```
// stronger function signature
//--------------------------------------------------------
void apply_some_kind_of_rotation( radian angle );

double my_angle = 90.0;
apply_some_kind_of_rotation( my_angle ); // doesn't compile

radian my_angle{ c_pi / 2 };
apply_some_kind_of_rotation( my_angle ); // great

degree my_angle{ 90.0 };
apply_some_kind_of_rotation( my_angle ); // great, implicitly converts to radian
```
