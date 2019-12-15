#include <Matrix4x4.hpp>
#include <Arithmetic.hpp>
#include <cmath>
#include <iostream>

namespace Brawl
{
	Matrix4x4::Matrix4x4( )
	{
		SetAsIdentity( );
	}

	Matrix4x4::~Matrix4x4( )
	{
	}

	void Matrix4x4::SetAsIdentity( )
	{
		m_M10 = m_M20 = m_M30 = m_M01 = m_M21 = m_M31 = m_M02 = m_M12 = m_M32 =
			m_M03 = m_M13 = m_M23 = 0.0f;
		m_M00 = m_M11 = m_M22 = m_M33 = 1.0f;
	}

	Matrix4x4 &Matrix4x4::Rotate( Float32 p_Angle, const Vector4 &p_Axis )
	{
		Float32 Sin = 0.0f, Cos = 0.0f, Tan = 0.0f;
		SineCosine( p_Angle, Sin, Cos );

		Tan = 1.0f - Cos;

		m_M00 = ( Tan * p_Axis.GetX( ) * p_Axis.GetX( ) ) + Cos;
		m_M10 = ( Tan * p_Axis.GetX( ) * p_Axis.GetY( ) ) +
			( Sin * p_Axis.GetZ( ) );
		m_M20 = ( Tan * p_Axis.GetX( ) * p_Axis.GetZ( ) ) -
			( Sin * p_Axis.GetY( ) );

		m_M01 = ( Tan * p_Axis.GetX( ) * p_Axis.GetY( ) ) -
			( Sin * p_Axis.GetZ( ) );
		m_M11 = ( Tan * p_Axis.GetY( ) * p_Axis.GetY( ) ) + Cos;
		m_M21 = ( Tan * p_Axis.GetY( ) * p_Axis.GetZ( ) ) +
			( Sin * p_Axis.GetX( ) );

		m_M02 = ( Tan * p_Axis.GetX( ) * p_Axis.GetZ( ) ) +
			( Sin * p_Axis.GetY( ) );
		m_M12 = ( Tan * p_Axis.GetY( )* p_Axis.GetZ( ) ) -
			( Sin * p_Axis.GetX( ) );
		m_M22 = ( Tan * p_Axis.GetZ( ) * p_Axis.GetZ( ) ) + Cos;

		m_M30 = m_M31 = m_M32 = m_M03 = m_M13 = m_M23 = 0.0f;
		m_M33 = 1.0f;

		return *this;
	}

	void Matrix4x4::SetRows( const Vector4 &p_Row0, const Vector4 &p_Row1,
			const Vector4 &p_Row2, const Vector4 &p_Row3 )
	{
		m_M00 = p_Row0.GetX( );
		m_M01 = p_Row0.GetY( );
		m_M02 = p_Row0.GetZ( );
		m_M03 = p_Row0.GetW( );

		m_M10 = p_Row1.GetX( );
		m_M11 = p_Row1.GetY( );
		m_M12 = p_Row1.GetZ( );
		m_M13 = p_Row1.GetW( );

		m_M20 = p_Row2.GetX( );
		m_M21 = p_Row2.GetY( );
		m_M22 = p_Row2.GetZ( );
		m_M23 = p_Row2.GetW( );

		m_M30 = p_Row3.GetX( );
		m_M31 = p_Row3.GetY( );
		m_M32 = p_Row3.GetZ( );
		m_M33 = p_Row3.GetW( );
	}

	void Matrix4x4::SetColumns( const Vector4 &p_Column0,
		const Vector4 &p_Column1, const Vector4 &p_Column2,
		const Vector4 &p_Column3 )
	{
		m_M00 = p_Column0.GetX( );
		m_M10 = p_Column0.GetY( );
		m_M20 = p_Column0.GetZ( );
		m_M30 = p_Column0.GetW( );

		m_M01 = p_Column1.GetX( );
		m_M11 = p_Column1.GetY( );
		m_M21 = p_Column1.GetZ( );
		m_M31 = p_Column1.GetW( );

		m_M02 = p_Column2.GetX( );
		m_M12 = p_Column2.GetY( );
		m_M22 = p_Column2.GetZ( );
		m_M32 = p_Column2.GetW( );

		m_M03 = p_Column3.GetX( );
		m_M13 = p_Column3.GetY( );
		m_M23 = p_Column3.GetZ( );
		m_M33 = p_Column3.GetW( );
	}

	Matrix4x4 &Matrix4x4::Translate( const Vector4 &p_Translation )
	{
		SetAsIdentity( );

		m_M03 = p_Translation.GetX( );
		m_M13 = p_Translation.GetY( );
		m_M23 = p_Translation.GetZ( );

		return *this;
	}

	Matrix4x4 &Matrix4x4::CreatePerspectiveFOV( Float32 p_FOV,
		Float32 p_AspectRatio, Float32 p_NearPlane, Float32 p_FarPlane )
	{
		if( Absolute( p_FarPlane - p_NearPlane ) < kEpsilon )
		{
			return *this;
		}

		SetAsIdentity( );

		Float32 TanHalfFOV = tanf( p_FOV * 0.5f );

		m_M00 = 1.0f / ( p_AspectRatio *  TanHalfFOV );
		m_M11 = ( 1.0f / TanHalfFOV ) * -1.0f;
		m_M22 = ( p_FarPlane / ( p_FarPlane - p_NearPlane ) ) * -1.0f;
		m_M23 = -1.0f;
		m_M32 = -( p_FarPlane * p_NearPlane ) / ( p_FarPlane - p_NearPlane );
		m_M33 = 0.0f;


		return *this;
	}

	Matrix4x4 &Matrix4x4::CreateViewLookAt( const Vector4 &p_Position,
		const Vector4 &p_Point, const Vector4 &p_WorldUp )
	{
		Vector4 Right, Up, Direction;

		Direction = p_Position - p_Point;
		Direction.Normalise( );

		Right = p_WorldUp.Cross( Direction );
		Right.Normalise( );

		Up = Direction.Cross( Right );
		Up.Normalise( );

		Matrix4x4 Upper3x3;

		Vector4 Zero( 0.0f, 0.0f, 0.0f, 1.0f );
		Upper3x3.SetRows( Right, Up, Direction, Zero );

		Vector4 Position = ( Upper3x3 * p_Position );

		CreateView3D( Right, Up, Direction, -Position );

		return *this;
	}

	Matrix4x4 &Matrix4x4::CreateView3D( const Vector4 &p_Right,
		const Vector4 &p_Up, const Vector4 &p_Direction,
		const Vector4 &p_Position )
	{
		// R U D 0
		// R U D 0
		// R U D 0
		// P P P 1
		
		m_M00 = p_Right.GetX( );
		m_M10 = p_Right.GetY( );
		m_M20 = p_Right.GetZ( );
		m_M30 = p_Position.GetX( );

		m_M01 = p_Up.GetX( );
		m_M11 = p_Up.GetY( );
		m_M21 = p_Up.GetZ( );
		m_M31 = p_Position.GetY( );

		m_M02 = p_Direction.GetX( );
		m_M12 = p_Direction.GetY( );
		m_M22 = p_Direction.GetZ( );
		m_M32 = p_Position.GetZ( );

		m_M03 = 0.0f;
		m_M13 = 0.0f;
		m_M23 = 0.0f;
		m_M33 = 1.0f;

		return *this;
	}

	Vector4 Matrix4x4::operator*( const Vector4 &p_Vector ) const
	{
		Vector4 Multiply;

		Multiply.SetX(	( m_M00 * p_Vector.GetX( ) ) +
						( m_M01 * p_Vector.GetY( ) ) +
						( m_M02 * p_Vector.GetZ( ) ) +
						( m_M03 * p_Vector.GetW( ) ) );

		Multiply.SetY(	( m_M10 * p_Vector.GetX( ) ) +
						( m_M11 * p_Vector.GetY( ) ) +
						( m_M12 * p_Vector.GetZ( ) ) +
						( m_M13 * p_Vector.GetW( ) ) );

		Multiply.SetZ(	( m_M20 * p_Vector.GetX( ) ) +
						( m_M21 * p_Vector.GetY( ) ) +
						( m_M22 * p_Vector.GetZ( ) ) +
						( m_M23 * p_Vector.GetW( ) ) );

		Multiply.SetW(	( m_M30 * p_Vector.GetX( ) ) +
						( m_M31 * p_Vector.GetY( ) ) +
						( m_M32 * p_Vector.GetZ( ) ) +
						( m_M33 * p_Vector.GetW( ) ) );

		return Multiply;
	}

	void Matrix4x4::Print( const std::string &p_Name ) const
	{
		std::cout << p_Name << std::endl;
		std::cout << "| " << m_M00 << " " << m_M01 << " " << m_M02 << " " <<
			m_M03 << " |" << std::endl;
		std::cout << "| " << m_M10 << " " << m_M11 << " " << m_M12 << " " <<
			m_M13 << " |" << std::endl;
		std::cout << "| " << m_M20 << " " << m_M21 << " " << m_M22 << " " <<
			m_M23 << " |" << std::endl;
		std::cout << "| " << m_M30 << " " << m_M31 << " " << m_M32 << " " <<
			m_M33 << " |" << std::endl;
	}

}
