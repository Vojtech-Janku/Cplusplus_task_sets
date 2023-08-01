#include <vector>
#include <cstdint>
#include <cassert>

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

bool magic_solve( const magic &in, magic &out );

int main()
{
    magic in{ 0 }, out;
    assert( magic_solve( in, out ) );
    assert( out.size() == 1 );
    assert( out[ 0 ] == 1 );

    return 0;
}
