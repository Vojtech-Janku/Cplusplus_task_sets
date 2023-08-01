#include <tuple>
#include <map>
#include <vector>
#include <cassert>
#include <iostream>

/* V tomto úkolu budeme pracovat s dvourozměrnou „mapou města“,
 * kterou reprezentujeme jako čtvercovou síť. Na každém políčku
 * může stát budova (tvaru kvádru), která má barvu a celočíselnou
 * výšku (budova výšky 1 má tvar krychle). Pro práci s mapou si
 * zavedeme:
 *
 *  • typ ‹building›, který reprezentuje budovu,
 *  • typ ‹coordinates›, který určuje pozici budovy a nakonec
 *  • typ ‹city›, který reprezentuje mapu jako celek.
 *
 * Jihozápadní (levý dolní) roh mapy má souřadnice ⟦(0, 0)⟧, ⟦x⟧-ová
 * souřadnice roste směrem na východ, ⟦y⟧-ová směrem na sever. */

struct building
{
    int height;
    int colour;
};

using coordinates = std::tuple< int, int >;
using city = std::map< coordinates, building >;

/* Nejsou-li nějaké souřadnice v mapě přítomny, znamená to, že na
 * tomto místě žádná budova nestojí.
 *
 * Vaším úkolem je podle zadané mapy spočítat pravoúhlý boční pohled
 * na město (panorama), které vznikne při pohledu z jihu, a které
 * bude popsáno typy:
 *
 *  • ‹column›, který reprezentuje jeden sloupec a pro každou
 *    viditelnou jednotkovou krychli obsahuje jedno číslo, které
 *    odpovídá barvě této krychle,
 *  • ‹skyline›, které obsahuje pro každou ‹x›-ovou souřadnici mapy
 *    jednu hodnotu typu ‹column›, kde index příslušného sloupce
 *    odpovídá jeho ‹x›-ové souřadnici. */

using column = std::vector< int >;
using skyline = std::vector< column >;

/* Vstup a odpovídající výstup si můžete představit např. takto:
 *
 *                                   ┌───┐
 *                                   │░░░│ 4
 *                                   ├───┤
 *                                   │░░░│ 3
 *   ┌───┬───┬───┬───┐   ┌───┬───┐   ├───┤
 *   │░1░│░3░│   │░5░│   │▒▒▒│░░░│   │░░░│ 2
 *   ├───┼───┼───┼───┤   ├───┼───┤   ├───┤
 *   │   │ 2 │   │   │   │▒▒▒│   │   │░░░│ 1
 *   ├───┼───┼───┼───┤   ├───┼───┤   ├───┤
 *   │▒3▒│▒1▒│   │   │   │▒▒▒│▒▒▒│   │░░░│ 0
 *   └───┴───┴───┴───┘   └───┴───┴───┴───┘
 *                         0   1   2   3
 *
 * Napište čistou funkci ‹compute_skyline› která výpočet provede.
 * Počet prvků každého sloupce musí být právě výška nejvyšší budovy
 * s danou ⟦x⟧-ovou souřadnicí. */

void print( column col ) {
	std::cout << "[ ";
	for ( auto i : col ) {
		std::cout << i << " ";
	}
	std::cout << "]";
}

void print( skyline sky ) {
	std::cout << "Skyline:" << std::endl;
	for ( auto &col : sky ) {
		print( col );
		std::cout << std::endl;
	}
}

int get_width( const city &daegu ) {
	if ( daegu.empty() ) return 0;
	auto [x,y] = daegu.rbegin()->first;
	return x+1;
}

// The city is called Daegu because I'm there on erasmus right now :)
/* The solution uses the implicit sorting of std::tuple in the keys of std::map
*	- the keys are sorted first by x, then by y
*	i.e. the for cycle goes through the coordinates like
* 		<0,0>, <0,1>, ... <1,0>, <1,1>, <1,2>, ... 
*	For each x we keep track of the maximum level, and if it is exceeded,
* 	we fill the difference with the color of the higher building
*	and we set the level to the new maximum height
*/
skyline compute_skyline( const city &daegu ) {
	if ( daegu.empty() ) return {};
	skyline horizon( get_width( daegu ), column() );
	int x = 0;
	int level = 0;
	for ( const auto &it : daegu ) {
		auto [x_new, y] = it.first;
		const building &skyscraper = it.second;
		if ( x != x_new ) {
			level = 0;
			x = x_new;
		}
		if ( skyscraper.height > level ) {
			for ( int i = level; i < skyscraper.height; i++ ) {
				horizon[x].push_back( skyscraper.colour );
			}
			level = skyscraper.height;
		}
	}
	return horizon;
}

int main()
{
    city example{ { { 0, 0 }, { 3, 0 } },
                  { { 1, 0 }, { 1, 2 } },
                  { { 1, 1 }, { 2, 0 } },
                  { { 0, 2 }, { 1, 2 } },
                  { { 1, 2 }, { 3, 1 } },
                  { { 3, 2 }, { 5, 1 } } };

    skyline expected{ { 0, 0, 0 }, { 2, 0, 1 }, {}, { 1, 1, 1, 1, 1 } };
    assert( compute_skyline( example ) == expected );

    return 0;
}
