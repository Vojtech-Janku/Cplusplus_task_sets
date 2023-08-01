#include <string_view>

/* Uvažme hru ‹s2/c_robots› – Vaším úkolem bude nyní naprogramovat
 * jednoduchý interpret, který bude hru řídit. Vstupní program
 * sestává ze tří částí:
 *
 *  1. deklarace, které popisují jak roboty ve hře a jejich
 *     startovní pozice, tak případné pomocné proměnné,
 *  2. příkazy, které se provedou jednou na začátku hry,
 *  3. příkazy, které se provedou každý tik, dokud hra neskončí.
 *
 * Program by mohl vypadat například takto: */

std::string_view example_1 = R"(with
  a  = red   1 @ -5.0 0 0
  b  = red   1 @  5.0 0 0
  c  = red   2 @  0.0 0 0
  g1 = green 2 @ -9.6 0 0
  g2 = green 2 @  9.6 0 0
init
  let g1 chase a
  let g2 chase b
repeat
)";

std::string_view example_2 = R"(with
  r = red   2 @  0.0 0 0
  g = green 2 @  0.0 0 0
  b = blue  1 @ -9.6 0 0
  tick = 0
init
  let r chase g
  let g go_to @ 1.0 0 0
repeat
  if tick > 9
    if g is_alive
       let b chase g
  set tick := tick + 1
)";

/* Následuje gramatika ve formátu EBNF, která popisuje syntakticky
 * platné programy; terminály gramatiky jsou «tokeny», které jsou od
 * sebe vždy odděleny alespoň jednou mezerou, nebo předepsaným
 * koncem řádku.
 *
 *     prog = 'with', { decl },
 *            'init', { stmt },
 *            'repeat', { stmt } ;
 *     decl = ident, '=', init, '\n' ;
 *     init = color, num, coord | coord | num ;
 *
 *     color = 'red' | 'green' | 'blue' ;
 *     coord = '@', num, num, num ;
 *
 *     stmt = cmd, '\n' |
 *            'if', cond, stmt ;
 *     cmd  = 'let', ident, 'chase', ident |
 *            'let', ident, 'go_to', expr |
 *            'set', ident, ':=', expr |
 *            'do', stmt, { stmt }, 'end' ;
 *     cond = atom, '=', atom |
 *            atom, '<', atom |
 *            atom, '>', atom |
 *            ident, 'is_alive' ;
 *     expr = atom |
 *            atom, '+', atom |
 *            atom, '-', atom |
 *            atom, '*', atom |
 *            '[', expr, ']' |
 *            '(', expr, ')' ;
 *     atom = ident | coord | num;
 *
 * Krom terminálních řetězců (‹'red'› a pod.) považujeme za tokeny
 * také symboly ‹num› a ‹ident›, zadané těmito pravidly:
 *
 *     num   = [ '-' ], digit, { digit }, [ '.', { digit } ] ;
 *     ident = letter, { letter | digit }
 *     digit = '0' | '1' | … | '9' ;
 *     letter = 'a' | 'b' | … | 'z' | '_' ;
 *
 * V programu se objevují hodnoty tří typů:
 *
 *  1. čísla (hodnoty typu ‹double›),
 *  2. trojice čísel (reprezentuje pozici nebo směr),
 *  3. odkaz na robota.
 *
 * S hodnotami (a proměnnými, které hodnoty daných typů aktuálně
 * obsahují), lze provádět tyto operace:
 *
 *  1. čísla lze sčítat, odečítat, násobit a srovnávat (neterminály
 *     ‹expr› a ‹cond›),
 *  2. trojice lze sčítat, odečítat a srovnat (ale pouze rovností),
 *  3. roboty lze posílat za jiným robotem nebo na zadané
 *     souřadnice (příkaz ‹let›),
 *  4. operace hranaté závorky hodnotu zjednodušuje:
 *     
 *     ◦ ‹[ robot ]› je aktuální pozice robota (trojice),
 *     ◦ ‹[ point ]› je Euklidovská vzdálenost bodu od počátku, resp.
 *        velikost směrového vektoru (‹[ p₁ - p₂ ]› tak spočítá
 *        vzdálenost bodů ‹p₁› a ‹p₂›.
 *
 * Operace, které nejsou výše popsané (např. pokus o sečtení
 * robotů), nemají určené chování. Totéž platí pro pokus o použití
 * nedeklarované proměnné (včetně přiřazení do ní). Podobně není
 * určeno chování, nevyhovuje-li vstupní program zadané gramatice.
 *
 * Robot, kterému bylo uloženo pronásledovat (chase) jiného robota,
 * bude na tohoto robota zamčen, až než mu bude uložen jiný cíl,
 * nebo cílový robot zanikne. Nemá-li robot žádný jiný příkaz, stojí
 * na místě (bez ohledu na barvu).
 *
 * Program je předán metodě ‹run› třídy ‹game› jako hodnota typu
 * ‹std::string_view›, návratová hodnota i zde nezmíněná pravidla
 * zůstavají v platnosti z příkladu v druhé sadě. */

int main()
{
    game g, h;

    auto [ ticks, players ] = g.run( example_1 );
    TRACE( ticks, players );
    assert( ticks == 15 );
    assert(( players == std::vector{ 1, 2 } ));

    std::tie( ticks, players ) = h.run( example_2 );
    assert( ticks == 49 );
    assert(( players == std::vector{ 2, 1 } ));

    return 0;
}
