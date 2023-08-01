#include <cassert>
#include <cstdint>
#include <vector>
#include <iostream>
#include <algorithm>
#include <tuple>
#include <cmath>

/* Předmětem této úlohy je naprogramovat typ ‹real›, který
 * reprezentuje reálné číslo s libovolnou přesností a rozsahem.
 *
 * Z hodnot:
 *
 *  • ‹a›, ‹b› typu ‹real›,
 *  • ‹k› typu ‹int›
 *
 * nechť je lze utvořit tyto výrazy, které mají vždy přesný
 * výsledek:
 *
 *  • ‹a + b›, ‹a - b›, ‹a * b›, ‹a / b›,
 *  • ‹a += b›, ‹a -= b›, ‹a *= b›, ‹a /= b›,
 *  • ‹a == b›, ‹a != b›, ‹a < b›, ‹a <= b›, ‹a > b›, ‹a >= b›,
 *  • ‹-a› – opačná hodnota,
 *  • ‹a.abs()› – absolutní hodnota,
 *  • ‹a.reciprocal()› – převrácená hodnota (není definováno pro 0),
 *  • ‹a.power( k )› – mocnina (včetně záporné).
 *
 * Výrazy, které nemají přesnou explicitní (číselnou) reprezentaci
 * jsou parametrizované požadovanou přesností ‹p› typu ‹real›:
 *
 *  • ‹a.sqrt( p )› – druhá odmocnina,
 *  • ‹a.exp( p )› – exponenciální funkce (se základem ⟦e⟧),
 *  • ‹a.log1p( p )› – přirozený logaritmus ⟦\ln(1 + a)⟧, kde
 *    ⟦a ∈ (-1, 1)⟧.
 *
 * Přesností se myslí absolutní hodnota rozdílu skutečné (přesné) a
 * reprezentované hodnoty. Pro aproximaci odmocnin je vhodné použít
 * Newtonovu-Raphsonovu metodu (viz ukázka z prvního týdne). Pro
 * aproximaci transcendentálních funkcí (exponenciála a logaritmus)
 * lze s výhodou použít příslušných mocninných řad. Nezapomeňte
 * ověřit, že řady v potřebné oblasti konvergují. Při určování
 * přesnosti (počtu členů, které je potřeba sečíst) si dejte pozor
 * na situace, kdy členy posloupnosti nejprve rostou a až poté se
 * začnou zmenšovat.
 *
 * Konečně je-li ‹d› hodnota typu ‹double›, nechť jsou přípustné
 * tyto konverze:
 *
 *  • ‹real x( d )›, ‹static_cast< real >( d )›,
 *
 * Poznámka: abyste se vyhnuli problémům s nejednoznačnými
 * konverzemi, je vhodné označit konverzní konstruktory a operátory
 * pro hodnoty typu ‹double› klíčovým slovem ‹explicit›. */

// ================ NATURAL =====================

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

// ================ REAL =====================

/* Předmětem této úlohy je naprogramovat typ ‹real›, který
 * reprezentuje reálné číslo s libovolnou přesností a rozsahem.
 *
 * Z hodnot:
 *
 *  • ‹a›, ‹b› typu ‹real›,
 *  • ‹k› typu ‹int›
 *
 * nechť je lze utvořit tyto výrazy, které mají vždy přesný
 * výsledek:
 *
 *  • ‹a + b›, ‹a - b›, ‹a * b›, ‹a / b›,
 *  • ‹a += b›, ‹a -= b›, ‹a *= b›, ‹a /= b›,
 *  • ‹a == b›, ‹a != b›, ‹a < b›, ‹a <= b›, ‹a > b›, ‹a >= b›,
 *  • ‹-a› – opačná hodnota,
 *  • ‹a.abs()› – absolutní hodnota,
 *  • ‹a.reciprocal()› – převrácená hodnota (není definováno pro 0),
 *  • ‹a.power( k )› – mocnina (včetně záporné).
 *
 * Výrazy, které nemají přesnou explicitní (číselnou) reprezentaci
 * jsou parametrizované požadovanou přesností ‹p› typu ‹real›:
 *
 *  • ‹a.sqrt( p )› – druhá odmocnina,
 *  • ‹a.exp( p )› – exponenciální funkce (se základem ⟦e⟧),
 *  • ‹a.log1p( p )› – přirozený logaritmus ⟦\ln(1 + a)⟧, kde
 *    ⟦a ∈ (-1, 1)⟧.
 *
 * Přesností se myslí absolutní hodnota rozdílu skutečné (přesné) a
 * reprezentované hodnoty. Pro aproximaci odmocnin je vhodné použít
 * Newtonovu-Raphsonovu metodu (viz ukázka z prvního týdne). Pro
 * aproximaci transcendentálních funkcí (exponenciála a logaritmus)
 * lze s výhodou použít příslušných mocninných řad. Nezapomeňte
 * ověřit, že řady v potřebné oblasti konvergují. Při určování
 * přesnosti (počtu členů, které je potřeba sečíst) si dejte pozor
 * na situace, kdy členy posloupnosti nejprve rostou a až poté se
 * začnou zmenšovat.
 *
 * Konečně je-li ‹d› hodnota typu ‹double›, nechť jsou přípustné
 * tyto konverze:
 *
 *  • ‹real x( d )›, ‹static_cast< real >( d )›,
 *
 * Poznámka: abyste se vyhnuli problémům s nejednoznačnými
 * konverzemi, je vhodné označit konverzní konstruktory a operátory
 * pro hodnoty typu ‹double› klíčovým slovem ‹explicit›. */

struct real {
	bool _sign = false;		// false = positive, true = negative
	natural _p, _q;

	real( bool sgn, const natural &p, const natural &q ) : _sign( sgn ), _p( p ), _q( q ) {}

	real reciprocal() const { return real( _sign, _q, _p ); }
	real abs() const { 		  return real( false, _p, _q ); }

	friend real operator+( const real &x, const real &y ) {
		
	}
	friend real operator-( const real &x ) { return real( !x._sign, x._p, x._q ); }
	friend real operator-( const real &x, const real &y ) { return x + ( -y ); }
	friend real operator*( const real &x, const real &y ) {
		return real( x._sign != y._sign, x._p * y._p, x._q * y._q );
	}
	friend real operator/( const real &x, const real &y ) {
		return real( x._sign != y._sign, x._p * y._q, x._q * y._p );
	}
	
	void set( bool sgn, const real &p, const real &q ) {
		_sign = sgn; _p = p; _q = q;
	}
	// maybe this first and then a + b  == real(a); a+=b
	real &operator+=( const real &x ) { return *this; }
	real &operator-=( const real &x ) { return *this; }
	real &operator*=( const real &x ) { return *this; }
	real &operator/=( const real &x ) { return *this; }

	friend bool operator==( const real &x, const real &y ) {
		return ( x._sign == y._sign ) && ( x._p * y._q == x._q * y._p );
	}
	friend bool operator!=( const real &x, const real &y ) { return !( x == y ); }
	friend bool operator<( const real &x, const real &y ) {
		return 
	}
	friend bool operator<=( const real &x, const real &y ) { return x < y || x == y; }
	friend bool operator>( const real &x, const real &y )  { return y < x; }
	friend bool operator>=( const real &x, const real &y ) { return y < x || x == y; }

	real power( int k ) const {}


 /* Přesností p se myslí absolutní hodnota rozdílu skutečné (přesné) a
 * reprezentované hodnoty. Pro aproximaci odmocnin je vhodné použít
 * Newtonovu-Raphsonovu metodu (viz ukázka z prvního týdne). Pro
 * aproximaci transcendentálních funkcí (exponenciála a logaritmus)
 * lze s výhodou použít příslušných mocninných řad. Nezapomeňte
 * ověřit, že řady v potřebné oblasti konvergují. Při určování
 * přesnosti (počtu členů, které je potřeba sečíst) si dejte pozor
 * na situace, kdy členy posloupnosti nejprve rostou a až poté se
 * začnou zmenšovat. */
 
 	/*real sqrt( const real &p ) const {
		// TODO: Newtonova-Raphsonova metoda
 	}
 	real exp( const real &p ) const {
 		real r = *this, //r_prev = real( 0 );
		while( abs( r, r_prev ) > p ) {
			r_prev = r;
			// TODO: do magic with r
		}
 		return r; 		
 	}
 	real log1p( const real &p ) const {}*/
};

int main()
{
    real zero = 0;
    real one = 1;
    real ten = 10;
    real half = one / 2;

    real eps = ten.power( -3 );

    real pi( 3.14159265 );
    real sqrt2( 1.41421356 );
    real e( 2.71828182 );
    real l_half( 0.40546511 );

    assert( ( one.exp( eps ) - e ).abs() < eps );
    assert( zero.log1p( eps ).abs() < eps );
    assert( ( half.log1p( eps ) - l_half ).abs() < eps );
    assert( static_cast< real >( 1.0 ) == one );
    assert( one + -one == zero );
    assert( one * ten == ten );

    return 0;
}