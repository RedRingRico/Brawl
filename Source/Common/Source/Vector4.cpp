#include <Vector4.hpp>
#include <Arithmetic.hpp>
#include <cmath>

namespace Brawl
{
	Vector4::Vector4( ) :
		m_X( 0.0f ),
		m_Y( 0.0f ),
		m_Z( 0.0f ),
		m_W( 0.0f )
	{
	}

	Vector4::Vector4( Float32 p_X, Float32 p_Y, Float32 p_Z, Float32 p_W ) :
		m_X( p_X ),
		m_Y( p_Y ),
		m_Z( p_Z ),
		m_W( p_W )
	{
	}

	Vector4::~Vector4( )
	{
	}

	void Vector4::Set( Float32 p_X, Float32 p_Y, Float32 p_Z, Float32 p_W )
	{
		m_X = p_X;
		m_Y = p_Y;
		m_Z = p_Z;
		m_W = p_W;
	}

	void Vector4::SetX( Float32 p_X )
	{
		m_X = p_X;
	}

	void Vector4::SetY( Float32 p_Y )
	{
		m_Y = p_Y;
	}

	void Vector4::SetZ( Float32 p_Z )
	{
		m_Z = p_Z;
	}

	void Vector4::SetW( Float32 p_W )
	{
		m_W = p_W;
	}

	Float32 Vector4::GetX( ) const
	{
		return m_X;
	}

	Float32 Vector4::GetY( ) const
	{
		return m_Y;
	}

	Float32 Vector4::GetZ( ) const
	{
		return m_Z;
	}

	Float32 Vector4::GetW( ) const
	{
		return m_W;
	}

	Float32 Vector4::Magnitude( ) const
	{
		return ( sqrtf( ( m_X * m_X ) + ( m_Y * m_Y ) + ( m_Z * m_Z ) +
			( m_W * m_W ) ) );
	}

	Float32 Vector4::MagnitudeSquared( ) const
	{
		return ( ( m_X * m_X ) + ( m_Y * m_Y ) + ( m_Z * m_Z ) +
			( m_W * m_W ) );
	}

	void Vector4::Normalise( )
	{
		Float32 Squared = MagnitudeSquared( );

		if( IsZero( Squared ) )
		{
			Zero( );
		}
		else
		{
			Float32 Factor = InverseSquareRoot( Squared );

			m_X *= Factor;
			m_Y *= Factor;
			m_Z *= Factor;
			m_W *= Factor;
		}
	}

	void Vector4::Zero( )
	{
		m_X = m_Y = m_Z = m_W = 0.0f;
	}

	void Vector4::Clean( )
	{
		if( IsZero( m_X ) )
		{
			m_X = 0.0f;
		}

		if( IsZero( m_Y ) )
		{
			m_Y = 0.0f;
		}

		if( IsZero( m_Z ) )
		{
			m_Z = 0.0f;
		}

		if( IsZero( m_W ) )
		{
			m_W = 0.0f;
		}
	}

	Vector4 Vector4::Cross( const Vector4 &p_Right ) const 
	{
		Vector4 Cross;

		Cross.m_X = ( m_Y * p_Right.m_Z ) - ( m_Z * p_Right.m_Y );
		Cross.m_Y = ( m_Z * p_Right.m_X ) - ( m_X * p_Right.m_Z );
		Cross.m_Z = ( m_X * p_Right.m_Y ) - ( m_Y * p_Right.m_X );

		return Cross;
	}

	Float32 Vector4::Dot( const Vector4 &p_Right ) const
	{
		return ( ( m_X * p_Right.m_X ) + ( m_Y * p_Right.m_Y ) +
			( m_Z * p_Right.m_Z ) + ( m_W * p_Right.m_W ) );
	}

	Bool Vector4::operator==( const Vector4 &p_Right ) const
	{
		if( IsEqual( m_X, p_Right.m_X ) &&
			IsEqual( m_Y, p_Right.m_Y ) &&
			IsEqual( m_Z, p_Right.m_Z ) &&
			IsEqual( m_W, p_Right.m_W ) )
		{
			return True;
		}

		return False;
	}

	Bool Vector4::operator!=( const Vector4 &p_Right ) const
	{
		if( IsEqual( m_X, p_Right.m_X ) ||
			IsEqual( m_Y, p_Right.m_Y ) ||
			IsEqual( m_Z, p_Right.m_Z ) ||
			IsEqual( m_W, p_Right.m_W ) )
		{
			return True;
		}

		return False;
	}

	Vector4 &Vector4::operator-( )
	{
		m_X = -m_X;
		m_Y = -m_Y;
		m_Z = -m_Z;
		m_W = -m_W;

		return *this;
	}

	Vector4 Vector4::operator+( const Vector4 &p_Right ) const
	{
		return Vector4( m_X + p_Right.m_X, m_Y + p_Right.m_Y,
			m_Z + p_Right.m_Z, m_W + p_Right.m_W );
	}

	Vector4 Vector4::operator-( const Vector4 &p_Right ) const
	{
		return Vector4( m_X - p_Right.m_X, m_Y - p_Right.m_Y,
			m_Z - p_Right.m_Z, m_W - p_Right.m_W );
	}

	Vector4 Vector4::operator*( const Vector4 &p_Right ) const
	{
		return Vector4( m_X * p_Right.m_X, m_Y * p_Right.m_Y,
			m_Z * p_Right.m_Z, m_W * p_Right.m_W );
	}

	Vector4 Vector4::operator*( const Float32 p_Scalar ) const
	{
		return Vector4( m_X * p_Scalar, m_Y * p_Scalar, m_Z * p_Scalar,
			m_W * p_Scalar );
	}

	Vector4 &Vector4::operator+=( const Vector4 &p_Right )
	{
		m_X += p_Right.m_X;
		m_Y += p_Right.m_Y;
		m_Z += p_Right.m_Z;
		m_W += p_Right.m_W;

		return *this;
	}

	Vector4 &Vector4::operator-=( const Vector4 &p_Right )
	{
		m_X -= p_Right.m_X;
		m_Y -= p_Right.m_Y;
		m_Z -= p_Right.m_Z;
		m_W -= p_Right.m_W;

		return *this;
	}

	Vector4 &Vector4::operator*=( const Vector4 &p_Right )
	{
		m_X *= p_Right.m_X;
		m_Y *= p_Right.m_Y;
		m_Z *= p_Right.m_Z;
		m_W *= p_Right.m_W;

		return *this;
	}

	Vector4 &Vector4::operator*=( const Float32 p_Scalar )
	{
		m_X *= p_Scalar;
		m_Y *= p_Scalar;
		m_Z *= p_Scalar;
		m_W *= p_Scalar;

		return *this;
	}
}
