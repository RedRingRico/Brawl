#ifndef __BRAWL_VECTOR4_HPP__
#define __BRAWL_VECTOR4_HPP__

#include <DataTypes.hpp>

namespace Brawl
{
	class Matrix4x4;

	class Vector4
	{
	public:
		Vector4( );
		Vector4( Float32 p_X, Float32 p_Y, Float32 p_Z, Float32 p_W );
		~Vector4( );

		void Set( Float32 p_X, Float32 p_Y, Float32 p_Z, Float32 p_W );
		void SetX( Float32 p_X );
		void SetY( Float32 p_Y );
		void SetZ( Float32 p_Z );
		void SetW( Float32 p_W );

		Float32 GetX( ) const;
		Float32 GetY( ) const;
		Float32 GetZ( ) const;
		Float32 GetW( ) const;

		Float32 Magnitude( ) const;
		Float32 MagnitudeSquared( ) const;

		void Normalise( );

		void Zero( );
		void Clean( );

		Vector4 Cross( const Vector4 &p_Right ) const;
		Float32 Dot( const Vector4 &p_Right ) const;

		Bool operator==( const Vector4 &p_Right ) const;
		Bool operator!=( const Vector4 &p_Right ) const;

		Vector4 &operator-( );

		Vector4 operator+( const Vector4 &p_Right ) const;
		Vector4 operator-( const Vector4 &p_Right ) const;
		Vector4 operator*( const Vector4 &p_Right ) const;
		Vector4 operator*( const Float32 p_Scalar ) const;

		Vector4 &operator+=( const Vector4 &p_Right );
		Vector4 &operator-=( const Vector4 &p_Right );
		Vector4 &operator*=( const Vector4 &p_Right );
		Vector4 &operator*=( const Float32 p_Scalar );

	private:
		Float32 m_X, m_Y, m_Z, m_W;
	};
}

#endif // __BRAWL_VECTOR4_HPP__
