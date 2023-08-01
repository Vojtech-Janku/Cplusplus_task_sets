#include <vector>
#include <set>
#include <cstdint>
#include <cassert>
#include <cmath>
#include <iostream>
#include <tuple>
#include <memory>

/* Magický čtverec je čtvercová síť o rozměru ⟦n × n⟧, kde
 *
 *  1. každé políčko obsahuje jedno z čísel ⟦1⟧ až ⟦n²⟧ (a to tak,
 *     že se žádné z nich neopakuje), a
 *  2. má tzv. «magickou vlastnost»: součet každého sloupce, řádku a
 *     obou diagonál je stejný. Tomuto součtu říkáme „magická
 *     konstanta“.
 *
 * Částečný čtverec je takový, ve kterém mohou (ale nemusí) být
 * některá pole prázdná. Vyřešením částečného čtverce pak myslíme
 * doplnění případných prázdných míst ve čtvercové síti tak, aby měl
 * výsledný čtverec obě výše uvedené vlastnosti. Může se samozřejmě
 * stát, že síť takto doplnit nelze. */

using magic = std::vector< std::int16_t >;

/* Vaším úkolem je naprogramovat backtrackující solver, který
 * čtverec doplní (je-li to možné), nebo rozhodne, že takové
 * doplnění možné není.
 *
 * Napište podprogram ‹magic_solve›, o kterém platí:
 *
 *  • návratová hodnota (typu ‹bool›) indikuje, bylo-li možné
 *    vstupní čtverec doplnit,
 *  • parametr ‹in› specifikuje částečný čtverec, ve kterém jsou
 *    prázdná pole reprezentována hodnotou 0, a který je
 *    uspořádaný po řádcích a na indexu 0 je levý horní roh,
 *  • je-li výsledkem hodnota ‹true›, zapíše zároveň doplněný
 *    čtverec do výstupního parametru ‹out› (v opačném případě
 *    parametr ‹out› nezmění),
 *  • vstupní podmínkou je, že velikost vektoru ‹in› je druhou
 *    mocninou, ale o stavu předaného vektoru ‹out› nic předpokládat
 *    nesmíte.
 *
 * Složitost výpočtu může být až exponenciální vůči počtu prázdných
 * polí, ale solver nesmí prohledávat stavy, o kterých lze v čase
 * ⟦O(n²)⟧ rozhodnout, že je doplnit nelze. Prázdná pole vyplňujte
 * počínaje levým horním rohem po řádcích (alternativou je zajistit,
 * že výpočet v jiném pořadí nebude výrazně pomalejší). */

void print( const magic &sol, std::size_t dim ) {
	for ( std::size_t row = 0; row < dim; row++ ) {
		for ( std::size_t col = 0; col < dim; col++ ) {
			std::cout << sol[ row*dim + col ] << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

void print( const std::set< std::int16_t > &u ) {
	std::cout << "Set: < ";
	for ( auto i : u ) {
		std::cout << i << " ";
	}
	std::cout << ">" << std::endl;
}

int get_sum( const magic &square, int idx, std::size_t dim, bool row ) {
	int sum = 0;
	std::size_t pos;
	for ( std::size_t i = 0; i < dim; i++ ) {
		pos = ( row ) ? (idx*dim + i) : (i*dim + idx);
		sum += square[ pos ];
	}
	return sum;
}

int count_zeros( const magic &square, std::size_t idx, std::size_t dim, bool row ) {
	int count = 0;
	std::size_t pos;
	for ( std::size_t i = 0; i < dim; i++ ) {
		pos = ( row ) ? (idx*dim + i) : (i*dim + idx);
		if ( !square[ pos ] ) ++count;
	}
	return count;
}

using coordinates = std::tuple< std::size_t, std::size_t >;

/* struct sums remembers the sums of all rows, columns, and both diagonals
* 	It also remembers the number of empty (zero) spots inside each of them for optimization 
* 	(for example if we fill the last zero in a column, we check its sum right away)
*	 	- another optimization would be to check the empty spots of diagonals, but that's
*		  too much work and it would make the code less pretty
*/
struct sums {
	std::vector<int> row_sums;
	std::vector<int> col_sums;
	std::vector<int> row_zeros;
	std::vector<int> col_zeros;
	int main_diag_sum 	= 0;
	int sec_diag_sum 	= 0;
	std::size_t dim 	= 0;
	int magic_constant 	= 0;
	
	sums( const magic &square, std::size_t dim ) 
	: row_sums( dim, 0 ), col_sums( dim, 0 ), 
	  row_zeros( dim, 0 ), col_zeros( dim, 0 ), dim( dim )
	{
		magic_constant = dim*( std::pow(dim, 2) + 1 ) / 2;
		for ( std::size_t n = 0; n < dim; n++ ) {
			row_sums[n] = get_sum( square, n, dim, true );
			col_sums[n]	= get_sum( square, n, dim, false );
			row_zeros[n] = count_zeros( square, n, dim, true );
			col_zeros[n] = count_zeros( square, n, dim, false );
			main_diag_sum += square[ n*dim + n ];
			sec_diag_sum  += square[ (dim-1)*(n+1) ];
		}	
	}

	bool can_insert( std::int16_t val, coordinates cor ) const {
		auto [x, y] = cor;
		if ( row_zeros[y] == 1 && row_sums[y] + val != magic_constant ) return false;
		if ( col_zeros[x] == 1 && col_sums[x] + val != magic_constant ) return false;
		return ( row_sums[y] + val <= magic_constant &&
			 	 col_sums[x] + val <= magic_constant &&
			 	( x != y 		|| main_diag_sum + val <= magic_constant ) &&
			 	( y != dim-1-x 	|| sec_diag_sum + val <= magic_constant ) );
	}

	void insert_val( std::int16_t val, coordinates cor ) {
		auto [x, y] = cor;
		row_sums[y] += val;
		col_sums[x] += val;
		row_zeros[y] += ( val > 0) ? -1 : 1 ;
		col_zeros[x]  += ( val > 0) ? -1 : 1 ;
		if ( x == y ) {
			main_diag_sum += val;
		}
		if ( y == dim-1 - x ) {
			sec_diag_sum += val;
		}
	}

	bool is_valid() const {
		for ( size_t i = 0; i < dim; i++ ) {
			if ( row_sums[i] != magic_constant ||
				 col_sums[i] != magic_constant )
				 return false;
		}
		return ( main_diag_sum == magic_constant &&
				 sec_diag_sum  == magic_constant );
	}
};

// returns the next coordinate with zero value (empty spot in the sol square)
// 			or a pair of SIZE_MAX if the end is reached
coordinates next_cor( const magic &sol, coordinates start, std::size_t dim ) {
	auto [x,y] = start;
	while ( sol[ y*dim + x ] != 0 ) {
		if ( x == dim-1 ) {
			if ( y == dim-1 ) {
				return { SIZE_MAX, SIZE_MAX };
			}
			++y;
			x = 0;
		} else {
			++x;
		}
	}
	return {x,y};
}

bool solve_rec( magic &sol, const std::set< std::int16_t > &unused, 
				sums &ss, coordinates cor ) {
	auto [x,y] = next_cor( sol, cor, ss.dim );
	if ( x == SIZE_MAX ) {
		return ss.is_valid();
	}

	auto u = unused;
	for ( auto val : unused ) {
		if ( !ss.can_insert( val, {x,y} ) ) continue;
		sol[ y*(ss.dim) + x ] = val;
		u.erase( val );
		ss.insert_val( val, {x,y} );
		if ( solve_rec( sol, u, ss, {x,y} ) ) return true;
		  // solve_rec failed, reversing the changes
		sol[ y*(ss.dim) + x ] = 0;
		u.insert( val );
		ss.insert_val( -val, {x,y} );;		
	}
	return false;
}

// returns the set of numbers from 1 to size of the quare, 
//	which do not appear in the square (i.e. all the missing numbers)
std::set<std::int16_t> get_unused( const magic &in ) {
	std::set<std::int16_t> unused;
	for ( std::size_t i = 1; i < in.size()+1; i++ ) {
		unused.insert(i);
	}
	for ( auto i : in ) { unused.erase( i ); }	
	return unused;
}

bool magic_solve( const magic &in, magic &out ) {
	std::size_t dim = std::sqrt( in.size() );
	auto unused = get_unused( in );
	magic solution = in;
	sums ss( solution, dim );
	coordinates cor = {0,0};
	
	if ( solve_rec( solution, unused, ss, cor ) ) {
		out = std::move( solution );
		return true;
	}
	return false;
}

int main()
{
    magic in{ 0 }, out;
    assert( magic_solve( in, out ) );
    assert( out.size() == 1 );
    assert( out[ 0 ] == 1 );
    
   	magic bad{ 1, 2,
   			   3, 4 };
   	assert( !magic_solve( bad, out ) );
   	
    magic slv{ 2,  16, 0, 3,
    		   11, 5,  8, 0,
    		   7,  9, 12, 6,
    		   14, 4,  1, 15 };
   	assert( magic_solve( slv, out ) );

    magic zeros3{ 0, 0, 0,
    		      0, 0, 0,
    		      0, 0, 0, };
   	assert( magic_solve( zeros3, out ) );

    magic c_size_8{ 1, 63, 62, 4, 0, 59, 58, 8,
    		   56, 10, 11, 53, 52, 14, 15, 49,
    		   0, 18, 19, 45, 44, 22, 23, 41,
    		   25, 39, 38, 28, 0, 35, 34, 32,
    		   33, 31, 30, 36, 37, 27, 26, 40,
    		   0, 42, 43, 21, 20, 46, 47, 17,
    		   16, 50, 51, 13, 12, 54, 55, 9,
    		   57, 7, 6, 60, 61, 3, 2, 64 };
   	assert( magic_solve( c_size_8, out ) ); 

    magic c_size_9{ 0, 63, 62, 4, 5, 59, 58, 8,
    		   0, 10, 11, 53, 52, 14, 15, 49,
    		   0, 18, 19, 45, 44, 22, 23, 41,
    		   0, 39, 38, 28, 29, 35, 34, 32,
    		   33, 31, 30, 36, 37, 27, 26, 40,
    		   24, 42, 43, 21, 20, 46, 47, 17,
    		   16, 50, 51, 13, 12, 54, 55, 9,
    		   57, 7, 6, 60, 61, 3, 2, 64 };
   	assert( magic_solve( c_size_9, out ) );   	

    return 0;
}
