#include <cassert>
#include <cstdint>
#include <vector>
#include <iostream>
#include <algorithm>
#include <tuple>
#include <cmath>

/* Tento úkol rozšiřuje ‹s1/f_natural› o tyto operace (hodnoty ‹m› a
 * ‹n› jsou typu ‹natural›):
 *
 *  • konstruktor, který přijme nezáporný parametr typu ‹double› a
 *    vytvoří hodnotu typu ‹natural›, která reprezentuje dolní
 *    celou část parametru,
 *  • operátory ‹m / n› a ‹m % n› (dělení a zbytek po dělení;
 *    chování pro ‹n = 0› není definované),
 *  • metodu ‹m.digits( n )› která vytvoří ‹std::vector›, který bude
 *    reprezentovat hodnotu ‹m› v soustavě o základu ‹n› (přitom
 *    nejnižší index bude obsahovat nejvýznamnější číslici),
 *  • metodu ‹m.to_double()› která vrátí hodnotu typu ‹double›,
 *    která co nejlépe aproximuje hodnotu ‹m› (je-li ⟦l = log₂(m) -
 *    52⟧ a ‹d = m.to_double()›, musí platit ‹m - 2ˡ ≤ natural( d )›
 *    a zároveň ‹natural( d ) ≤ m + 2ˡ›; je-li ‹m› příliš velké, než
 *    aby šlo typem ‹double› reprezentovat, chování je
 *    nedefinované).
 *
 * Převody z/na typ ‹double› musí být lineární v počtu bitů
 * operandu. Dělení může mít složitost nejvýše kvadratickou v počtu
 * bitů levého operandu. Metoda ‹digits› smí vůči počtu bitů ‹m›,
 * ‹n› provést nejvýše lineární počet «aritmetických operací» (na
 * hodnotách ‹m›, ‹n›). */

int int_bytes( int v ) {
	assert( v >= 0 );
	if ( v >= (1<<24) ) return 4;
	if ( v >= (1<<16) ) return 3;
	if ( v >= (1<<8) ) return 2;
	return 1;
}

struct natural {
	std::vector< std::uint8_t > _digits;

	natural() : _digits( 1, 0 ) {}
	natural( int val ) : _digits( int_bytes(val), 0 ) {
		assert( val >= 0 );
		for ( int i = 0; i < int_bytes(val); i++ ) {
			_digits[i] = ( ( 255 << (8*i) ) & val) >> (8*i);
		}
	}
	natural( int n, int val ) : natural( val ) {
		_digits.resize( n );
	}
	natural( double d ) {
		double dint = std::trunc(d);
		while( dint > 0 ) {
			_digits.push_back( std::fmod( dint, 256 ) );
			dint = std::trunc( dint / 256 );
		}
		if ( _digits.empty() ) _digits.push_back( 0 );
	}
	natural( std::vector<std::uint8_t> vec ) : _digits(vec) {}

	std::size_t digit_count() const {
		return _digits.size();
	}
	
	double to_double() {
		double val = 0;
		for ( std::size_t i = 0; i < digit_count(); ++i ) {
			val += ( _digits[i] * std::pow(2, 8*i) );
		}
		return val;
	}
	
	std::uint8_t &operator[]( int i ) {
		return _digits[i];
	}
	const std::uint8_t &operator[]( int i ) const {
		return _digits[i];
	}

	void add_digits( int n ) {
		_digits.resize( _digits.size() + n );
	}

	void remove_zero_digits() {
		int zero_digs = 0;
		for ( std::size_t i = _digits.size(); i > 1; --i ) {
			if ( _digits[i-1] ) break;
			++zero_digs;
		}
		_digits.resize( _digits.size() - zero_digs );
	}
	
	std::vector<natural> digits( const natural &n );
	natural power( int p );
};

void print( const natural &n ) {
	std::cout << "[ ";
	for ( std::size_t i = 0; i < n.digit_count(); ++i ) {
		std::cout << static_cast<int>( n[i] ) << " ";
	}
	std::cout << "] ";
}
void print( std::vector<int> v ) {
	std::cout << "< ";
	for ( int i : v ) {
		std::cout << i << " ";
	}
	std::cout << "> ";
}

bool operator==( const natural &a, const natural &b ) {
	if ( a.digit_count() != b.digit_count() ) return false;
	return a._digits == b._digits;
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

void operator<<=( natural &n, int off ) {
	n.add_digits(1);
	for ( auto i = n.digit_count(); i > 1; --i ) {
		n[i-1] = ( n[i-1] << off ) | ( n[i-2] >> (8-off) );	
	}
	n[0] = n[0] << off;
	n.remove_zero_digits();
}
void operator>>=( natural &n, int off ) {
	for ( std::size_t i = 0; i < n.digit_count()-1; ++i ) {
		n[i] = ( n[i] >> off ) | ( n[i+1] << (8-off) );
	}
	n[ n.digit_count()-1 ] = n[ n.digit_count()-1 ] >> off;
	n.remove_zero_digits();
}

std::uint8_t short_div( auto &it_start, auto &it_end, const natural &denom ) {
	natural den = denom;
	std::vector<std::uint8_t> nomvec(it_start, it_end);
	std::reverse( nomvec.begin(), nomvec.end() );
	natural nom( nomvec );
	if ( nom < denom ) {
		return 0;
	}
	std::uint8_t res = 0;
	den <<= 7;
	for ( int bit_off = 0; bit_off < 8; ++bit_off ) {
		if ( den <= nom ) {
			nom = nom - den;
			res = res | ( 1 << (7-bit_off) );
		}
		den >>= 1;
	}

	auto it = it_start;
	for ( std::size_t i = it_end-it_start; i > 0 ; --i ) {
		*it = ( i > nom.digit_count() ) ? 0 : nom[ i-1 ];
		++it;
	}
	return res;
}

std::tuple<natural,natural> divide( const natural &num, const natural &denom ) {
	if ( num < denom ) return { natural(), num };
	natural div( num.digit_count() - denom.digit_count() + 1, 0 );
	natural rem = num;
	auto it_start = rem._digits.rbegin();
	auto it_end = rem._digits.rbegin() + denom.digit_count();
	for ( std::size_t i = div.digit_count(); i > 0; --i ) {
		div[i-1] = short_div( it_start, it_end, denom );
		while( it_start != it_end && *it_start == 0 ) { ++it_start; }
		++it_end;
	}
	div.remove_zero_digits();
	rem.remove_zero_digits();
	return { div, rem };
}

natural operator/( const natural &a, const natural &b ) {
	auto [div, rem] = divide( a, b );
	return div;
}

natural operator%( const natural &a, const natural &b ) {
	auto [div, rem] = divide( a, b );
	return rem;
}

std::vector<natural> natural::digits( const natural &n ) {
	natural t = *this;
	std::vector<natural> res;
	while( t != natural(0) ) {
		auto [div, rem] = divide( t, n );
		res.push_back( rem );
		t = div;
	}
	std::reverse(res.begin(), res.end());
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

void test_division() {
	std::cout << "TEST DIVISION" << std::endl;
	natural m( 789123 ), n( 45621 ), o(53);
	assert( m/n == natural(17) );
	assert( m%n == natural(13566) );	
	assert( m/o == natural(14889) );
	assert( m%o == natural(6) );

	natural zero(0), one(1);
	assert( m/m == one );
	assert( m%m == zero );
	assert( n/n == one );
	assert( n%n == zero );
	assert( o/o == one );
	assert( o%o == zero );	
}

void test_digits() {
	std::cout << "TEST DIGITS" << std::endl;
	natural m( 541 );
	std::vector<natural> m_digits_base3 = { natural(2), natural(0), natural(2), 
											natural(0), natural(0), natural(1) };
	std::vector<natural> m_digits_base10 = { natural(5), natural(4), natural(1) };
	std::vector<natural> m_digits_base16 = { natural(2), natural(1), natural(13) };
	assert( m.digits( natural(3) ) == m_digits_base3 );
	assert( m.digits( natural(10) ) == m_digits_base10 );
	assert( m.digits( natural(16) ) == m_digits_base16 );
}

void test_double() {
	std::cout << "TEST DOUBLE" << std::endl;
	natural m( std::pow( 2, 130) );
	double dist{ m.to_double() - std::pow( 2, 130 ) };
	assert( std::fabs( dist ) <= std::pow( 2, 130-52) );
	
	natural three_over( 3.0000001 ), three_under( 2.9999999 );
	assert( three_over != three_under );
}

int main()
{
    natural m( 2.1 ), n( 2.9 );
    assert( m == n );
    assert( m / n == 1 );
    assert( m % n == 0 );
    assert( m.digits( 10 ).size() == 1 );
    assert( m.to_double() == 2.0 );

	test_division();
    test_digits();
    test_double();

    return 0;
}