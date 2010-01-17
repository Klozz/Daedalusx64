#ifndef VECTOR4_H__
#define VECTOR4_H__

#include "Math/Math.h"
#include "../SysPSP/Utility/pspmath.h"

ALIGNED_TYPE(class, v4, 16)
{
public:
	v4() {}
	v4( float _x, float _y, float _z, float _w ) : x( _x ), y( _y ), z( _z ), w( _w ) {}

	float Normalise()
	{
		float	len( Length() );
		if(len > 0.0001f)
		{
			float r( 1.0f / len );
			x *= r;
			y *= r;
			z *= r;
			w *= r;
		}

		return len;
	}

	v4 operator+( const v4 & v ) const
	{
		return v4( x + v.x, y + v.y, z + v.z, w + v.w );
	}
	v4 operator-( const v4 & v ) const
	{
		return v4( x - v.x, y - v.y, z - v.z, w - v.w );
	}

	v4 operator*( float s ) const
	{
		return v4( x * s, y * s, z * s, w * s );
	}


	float Length() const
	{
		return 1.0f / vfpu_invSqrt( (x*x)+(y*y)+(z*z)+(w*w) );
	}

	float Dot( const v4 & rhs ) const
	{
		return (x*rhs.x) + (y*rhs.y) + (z*rhs.z) + (w*rhs.w);
	}


	float x, y, z, w;
};


#endif // VECTOR4_H__
