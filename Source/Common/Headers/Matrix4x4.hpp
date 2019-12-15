#ifndef __BRAWL_MATRIX4X4_HPP__
#define __BRAWL_MATRIX4X4_HPP__

#include <DataTypes.hpp>
#include <Vector4.hpp>
#include <string>

namespace Brawl
{
	class Matrix4x4
	{
	public:
		Matrix4x4( );
		~Matrix4x4( );

		void SetAsIdentity( );

		Matrix4x4 &Rotate( Float32 p_Angle, const Vector4 &p_Axis );

		void SetRows( const Vector4 &p_Row0, const Vector4 &p_Row1,
			const Vector4 &p_Row2, const Vector4 &p_Row3 );
		void SetColumns( const Vector4 &p_Column0, const Vector4 &p_Column1,
			const Vector4 &p_Column2, const Vector4 &p_Column3 );

		Matrix4x4 &Translate( const Vector4 &p_Translation );

		Matrix4x4 &CreatePerspectiveFOV( Float32 p_FOV, Float32 p_AspectRatio,
			Float32 p_NearPlane, Float32 p_FarPlane );
		Matrix4x4 &CreateViewLookAt( const Vector4 &p_Position,
			const Vector4 &p_Point, const Vector4 &p_WorldUp );
		Matrix4x4 &CreateView3D( const Vector4 &p_Right,
			const Vector4 &p_Up, const Vector4 &p_Direction,
			const Vector4 &p_Position );

		Vector4 operator*( const Vector4 &p_Vector ) const;

		void Print( const std::string &p_Name ) const;

	private:
		Float32	m_M00, m_M01, m_M02, m_M03;
		Float32	m_M10, m_M11, m_M12, m_M13;
		Float32	m_M20, m_M21, m_M22, m_M23;
		Float32	m_M30, m_M31, m_M32, m_M33;
	};
}

#endif // __BRAWL_MATRIX4X4_HPP__
