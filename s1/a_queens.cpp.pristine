#include <cstdint>
#include <cassert>

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

bool queens( std::uint64_t board );

int main()
{
    assert( queens( 0 ) );
    assert( !queens( 3 ) );

    return 0;
}
