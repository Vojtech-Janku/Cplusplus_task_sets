#include <cassert>

/* V této úloze budete programovat jednoduchou hru, ve které se ve
 * volném třírozměrném prostoru pohybují robotické entity tří barev:
 *
 *  1. červený robot (třída ‹robot_red›):
 *     
 *     ◦ není-li uzamčený a na hrací ploše je alespoň jeden cizí
 *       zelený robot, uzamkne se na ten nejbližší, jinak stojí na
 *       místě,
 *     ◦ je-li na nějaký robot uzamčený, přibližuje se přímo k němu
 *       (tzn. směr pohybu je vždy v aktuálním směru tohoto robotu),
 *     
 *  2. zelený robot (třída ‹robot_green›):
 *     
 *     ◦ je-li nějaký cizí modrý robot ve vzdálenosti do 10 metrů,
 *       směruje přímo k tomu nejbližšímu,
 *     ◦ je-li nejbližší takový robot dále než 10 metrů, zelený
 *       robot se teleportuje do místa, které leží na jejich
 *       spojnici, 8 metrů od cílového robotu na jeho «vzdálenější»
 *       straně,
 *     ◦ jinak stojí na místě.
 *     
 *  3. modrý robot (třída ‹robot_blue›):
 *     
 *     ◦ směruje k nejbližšímu cizímu červenému robotu, existuje-li
 *       nějaký,
 *     ◦ jinak se «poloviční rychlostí» pohybuje po přímce ve směru,
 *       který měl naposledy,
 *     ◦ na začátku hry je otočen směrem k začátku souřadnicového
 *       systému (je-li přímo v počátku, chování není určeno).
 *
 * Roboty se pohybují rychlostí 15 m/s. Dostanou-li se dva roboty
 * různých barev a různých vlastníků do vzdálenosti 1 metr nebo
 * menší, jeden z nich zanikne podle pravidla:
 *
 *  • červený vítězí nad zeleným,
 *  • zelený vítězí nad modrým a konečně
 *  • modrý vítězí nad červeným.
 *
 * Hra jako celek nechť je zapouzdřená ve třídě ‹game›. Bude mít
 * tyto metody:
 *
 *  • metoda ‹tick› posune čas o 1/60 sekundy, a provede tyto akce:
 *    
 *    a. všechny roboty se posunou na své nové pozice «zároveň»,
 *       tzn. dotáže-li se nějaký robot na aktuální pozici jiného
 *       robotu, dostane souřadnice, které měl tento na konci
 *       předchozího tiku,
 *    b. vzájemné ničení robotů, které proběhne opět zároveň –
 *       sejdou-li se v dostatečné blízkosti roboty všech tří barev,
 *       zaniknou všechny.
 *    
 *  • metoda ‹run› simuluje hru až do jejího konce,
 *    
 *    ◦ tzn. do chvíle, kdy nemůže zaniknout žádný další robot a
 *    ◦ vrátí dvojici (počet tiků, hráči),
 *    ◦ kde „hráči“ je vektor identifikátorů hráčů, seřazený podle
 *      počtu zbývajících robotů; je-li více hráčů se stejným počtem
 *      robotů, první bude ten s větším počtem červených, dále
 *      zelených a nakonec modrých robotů; je-li stále hráčů více,
 *      budou uspořádáni podle svého identifikátoru,
 *      
 *  • metody ‹add_X› pro ‹X› = ‹red›, ‹green› nebo ‹blue›, které
 *    přidají do hry robota odpovídající barvy, a které mají dva
 *    parametry:
 *    
 *    ◦ počáteční pozici, jako trojici hodnot typu ‹double› (zadané
 *      v metrech v kartézské soustavě),
 *    ◦ nenulový celočíselný identifikátor hráče-vlastníka. */

struct robot_red;
struct robot_green;
struct robot_blue;

struct game;

int main()
{
    game g;

    auto [ ticks, players ] = g.run();
    assert( ticks == 0 );
    assert( players.empty() );

    g.add_red( { -5, 0, 0 }, 1 );
    g.add_red( {  5, 0, 0 }, 1 );
    g.add_red( {  0, 0, 0 }, 2 );

    std::tie( ticks, players ) = g.run();
    assert( ticks == 0 );
    assert(( players == std::vector{ 1, 2 } ));

    g.add_green( {  9.6, 0, 0 }, 2 );
    g.add_green( { -9.6, 0, 0 }, 2 );

    std::tie( ticks, players ) = g.run();
    assert(( players == std::vector{ 1, 2 } ));
    assert( ticks == 15 );

    return 0;
}
