#include <cstdint>
#include <cassert>
#include <iostream>

/* V této úloze budete programovat řešení tzv. problému osmi
 * královen (osmi dam). Vaše řešení bude predikát, kterého vstupem
 * bude jediné 64-bitové bezznaménkové číslo (použijeme typ
 * ‹uint64_t›), které popisuje vstupní stav šachovnice: šachovnice
 * 8×8 má právě 64 polí, a pro reprezentaci každého pole nám stačí
 * jediný bit, který určí, je-li na tomto políčku umístěna královna.
 *
 * Políčka šachovnice jsou uspořádána počínaje levým horním rohem
 * (nejvyšší, tedy 64. bit) a postupují zleva doprava (druhé pole
 * prvního řádku je uloženo v 63. bitu, tj. druhém nejvyšším) po
 * řádcích zleva doprava (první pole druhého řádku je 56. bit),
 * atd., až po nejnižší (první) bit, který reprezentuje pravý dolní
 * roh.
 *
 * Predikát nechť je pravdivý právě tehdy, není-li žádná královna na
 * šachovnici ohrožena jinou. Program musí pracovat správně i pro
 * případy, kdy je na šachovnici jiný počet královen než 8.
 * Očekávaná složitost je v řádu 64² operací – totiž O(n²) kde ⟦n⟧
 * představuje počet políček.
 *
 * Poznámka: preferované řešení používá pro manipulaci se šachovnicí
 * pouze bitové operace a zejména nepoužívá standardní kontejnery.
 * Řešení, které bude nevhodně používat kontejnery (spadá sem např.
 * jakékoliv použití ‹std::vector›) nemůže získat známku A. */

void print( std::uint64_t board ) {
	std::uint64_t pos = 1;
	for ( std::size_t i = 0; i < 8; i++ ) {
		for ( std::size_t j = 0; j < 8; j++ ) {
			if ( board & pos ) {
				std::cout << 1 << " ";
			} else {
				std::cout << 0 << " ";
			}
			pos = (pos << 1);
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

/* Solution is based on shifting the whole chessboard in all the directions 
*  that a queen can move. If a shifted chessboard "intersects" with the original one
*  somewhere (i.e. they both have a queen on the same spot), it means that
*  two queens attack each other in the direction of the chessboard shift.
*/

// functions for shifting the chessboard in 4 different directions
//  - we get diagonal shifts by combining a horizontal and vertical shift of the same size 
std::uint64_t shift_up( std::uint64_t board, int n ) {
	return ( board << (8*n) );
}
std::uint64_t shift_down( std::uint64_t board, int n ) {
	return ( board >> (8*n) );
}
std::uint64_t shift_left( std::uint64_t board, int n ) {
	std::uint64_t row = ( 255 >> n );
	std::uint64_t res = 0;
	for ( std::size_t i = 0; i < 8; i++ ) {
		res += ( ( board & row ) << n );
		row = ( row << 8 );
	}
	return res;
}
std::uint64_t shift_right( std::uint64_t board, int n ) {
	std::uint64_t row = ( 255 >> n ) << n;
	std::uint64_t res = 0;
	for ( std::size_t i = 0; i < 8; i++ ) {
		res += ( ( board & row ) >> n );
		row = ( row << 8 );
	}
	return res;
}

// shifts board in all 8 directions by n and compares with the original board
// returns true if any shift collides with the original board (= 2 queens attack each other)
bool shifts_collide( std::uint64_t board, int n ) {
	return ( board & shift_up( 	  board, n ) ||	// vertical shifts
			 board & shift_down(  board, n ) ||
			 board & shift_left(  board, n ) ||	// horizontal shifts
			 board & shift_right( board, n ) ||
			 board & shift_up( 	 shift_left(  board, n ), n ) ||	// diagonal shifts
			 board & shift_down( shift_left(  board, n ), n ) ||
			 board & shift_up( 	 shift_right( board, n ), n ) ||
			 board & shift_down( shift_right( board, n ), n ) );
}

// This function shifts the chessboard in all valid ways and compares with the original
// We only need to shift in each direction up to 7 times
bool queens( std::uint64_t board ) {
	for ( int i = 1; i < 8; i++ ) {
		if ( shifts_collide( board, i ) ) return false;
	}
	return true;
}

int main()
{
	/*
	std::uint64_t brd = 1170937021957408770;
	print( brd );
	print( shift_up(brd, 1) );
	print( shift_down(brd, 1) );
	print( shift_left(brd, 1) );
	print( shift_right(brd, 1) );
	*/
	
    assert( queens( 0 ) );
    assert( !queens( 3 ) );
    assert( queens( 1170937021957408770 ) );
    return 0;
}
