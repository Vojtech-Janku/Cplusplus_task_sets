#include <cassert>
#include <cstdint>
#include <vector>
#include <iostream>

/* Vaším úkolem je tentokrát naprogramovat strukturu, která bude
 * reprezentovat libovolně velké přirozené číslo (včetně nuly). Tyto
 * hodnoty musí být možné:
 *
 *  • sčítat (operátorem ‹+›),
 *  • odečítat (‹x - y› je ovšem definováno pouze za předpokladu ‹x ≥ y›),
 *  • násobit (operátorem ‹*›),
 *  • libovolně srovnávat (operátory ‹==›, ‹!=›, ‹<›, atd.),
 *  • mocnit na kladný exponent typu ‹int› metodou ‹power›,
 *  • sestrojit z libovolné nezáporné hodnoty typu ‹int›.
 *
 * Implicitně sestrojená hodnota typu ‹natural› reprezentuje nulu.
 * Všechny operace krom násobení musí být nejvýše lineární vůči
 * «počtu dvojkových cifer» většího z reprezentovaných čísel.
 * Násobení může mít v nejhorším případě složitost přímo úměrnou
 * součinu ⟦m⋅n⟧ (kde ⟦m⟧ a ⟦n⟧ jsou počty cifer operandů). */

int int_bytes( int v ) {
	assert( v >= 0 );
	if ( v >= (1<<24) ) return 4;
	if ( v >= (1<<16) ) return 3;
	if ( v >= (1<<8) ) return 2;
	return 1;
}

struct natural {
	std::vector< std::uint8_t > digits;

	natural() : digits( 1, 0 ) {}
	natural( int val ) : digits( int_bytes(val), 0 ) {
		assert( val >= 0 );
		for ( int i = 0; i < int_bytes(val); i++ ) {
			digits[i] = ( ( 255 << (8*i) ) & val) >> (8*i);
		}
	}
	natural( int n, int val ) : natural( val ) {
		digits.resize( n );
	}

	std::size_t digit_count() const {
		return digits.size();
	}

	std::uint8_t &operator[]( int i ) {
		return digits[i];
	}
	const std::uint8_t &operator[]( int i ) const {
		return digits[i];
	}

	void add_digits( int n ) {
		digits.resize( digits.size() + n );
	}

	void remove_zero_digits() {
		int zero_digs = 0;
		for ( std::size_t i = digits.size(); i > 1; --i ) {
			if ( digits[i-1] ) break;
			++zero_digs;
		}
		digits.resize( digits.size() - zero_digs );
	}

	natural power( int p );
};

void print( const natural &n ) {
	std::cout << "[ ";
	for ( std::size_t i = 0; i < n.digit_count(); ++i ) {
		std::cout << n[i] << " ";
	}
	std::cout << "]" << std::endl;
}

bool operator==( const natural &a, const natural &b ) {
	if ( a.digit_count() != b.digit_count() ) return false;
	return a.digits == b.digits;
}
bool operator!=( const natural &a, const natural &b ) {
	return !( a == b );
}

bool less_than( const natural &a, const natural &b, bool eq ) {
	if ( a.digit_count() != b.digit_count() ) {
		return a.digit_count() < b.digit_count();
	}
	for ( std::size_t i = a.digit_count(); i > 0; --i ) {
		if ( a[i-1] != b[i-1] ) {
			return a[i-1] < b[i-1];
		}
	}
	return eq;
}
bool operator<( const natural &a, const natural &b ) {
	return less_than( a, b, false );
}
bool operator>( const natural &a, const natural &b ) {
	return less_than( b, a, false );
}
bool operator<=( const natural &a, const natural &b ) {
	return less_than( a, b, true );
}
bool operator>=( const natural &a, const natural &b ) {
	return less_than( b, a, true );
}

natural add( const natural &a, const natural &b, bool subtract ) {
	if ( subtract ) assert ( a >= b );
	std::size_t res_size = std::max( a.digit_count(), b.digit_count() );
	natural res( res_size, 0 );
	std::int16_t a_val = 0, b_val = 0, dig_val = 0, carry = 0;
	for ( std::size_t i = 0; i < res.digit_count(); ++i ) {
		a_val = ( i < a.digit_count() ) ? a[i] : 0;
		b_val = ( i < b.digit_count() ) ? b[i] : 0;
		dig_val = ( subtract ) ? (a_val - b_val - carry) : (a_val + b_val + carry);
		if ( 	 !subtract && (dig_val >> 8) ) {
			carry = 1;
			res[i] = dig_val & 255;
		}
		else if ( subtract && (dig_val < 0) ) {
			carry = 1;
			res[i] = ( 1UL << 8 ) + dig_val;
		} else {
			carry = 0;
			res[i] = dig_val;
		}
	}
	if ( !subtract && carry ) {
		res.add_digits( 1 );
		res[ res.digit_count()-1 ] = carry;
	}
	res.remove_zero_digits();
	return res;	
}

natural operator+( const natural &a, const natural &b ) { return add( a, b, false ); }
natural operator-( const natural &a, const natural &b ) { return add( a, b, true ); }

natural operator*( const natural &a, const natural &b ) {
	std::size_t res_size = a.digit_count() + b.digit_count();
		// we have to create separate number for overflows because 
		// uint8*uint8 + uint8 could overflow uint16
	natural zeros( res_size, 0 ), res( res_size, 0 ), 
			dig_res( res_size, 0 ), carry( res_size, 0 );
	std::uint16_t a_val, b_val, dig_val = 0;
	for ( std::size_t i = 0; i < a.digit_count(); ++i ) {
		dig_res = zeros;
		carry = zeros;
		for ( std::size_t j = 0; j < b.digit_count(); ++j ) {
			a_val = a[i];
			b_val = b[j];
			dig_val = a_val * b_val;
			carry[ i + j + 1 ] = dig_val >> 8;
			dig_res[ i + j ] = dig_val & ( (1UL << 8) - 1 );
		}
		res = res + dig_res + carry;
	}	
	res.remove_zero_digits();
	return res;	
}

// implementing the exponentiation by squaring algorithm
natural natural::power( int p ) { 
	natural base = *this;
	natural res(1);
	if ( p == 0 ) return res;
	while ( p > 1 ) {
		if ( p % 2 == 0 ) {
			p = p/2;
		} else {
			res = base * res;
			p = (p-1)/2;
		}
		base = base * base;
	}
	return base*res;
}

int main()
{
    natural zero;
    assert( zero + zero == zero );
    assert( zero * zero == zero );
    assert( zero - zero == zero );
    natural one( 1 ), two( 2 ), three( 3 ), four( 4 ), five( 5 ), 
    		six( 6 ), seven( 7 ), eight( 32 ), nine( 9 ), ten( 10 );
    natural three_ten( 59049 ), ten_three( 1000 ), fortynine( 49 );
    assert( one + zero == one );
    assert( one - zero == one );
    assert( one - one == zero );
    assert( two * five == ten );
    assert( one.power( 2 ) == one );
    assert( three.power( 10 ) == three_ten );
    assert( ten.power( 3 ) == ten_three );
    assert( seven.power( 2 ) == fortynine );

	natural a(13), b(9);
	for ( int i = 1; i < 5; ++i ) {
		natural iter(i);
		b = b * b + a + iter;
		a = a * a + b;
		//std::cout << i << std::endl;
		//print( a );
		//print( b );
		//print( a-b );
		assert( a - b > b );
	}

    return 0;
}
