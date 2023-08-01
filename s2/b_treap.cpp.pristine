#include <cassert>
#include <vector>

/* Datová struktura «treap» kombinuje binární vyhledávací strom a
 * binární haldu – hodnotu, vůči které tvoří vyhledávací strom
 * budeme nazývat «klíč» a hodnotu, vůči které tvoří haldu budeme
 * nazývat «priorita». Platí pak:
 *
 *  • «klíč» v každém uzlu je větší než klíč v jeho levém potomkovi,
 *    a zároveň je menší, než klíč v pravém potomkovi,
 *  • «priorita» je v každém uzlu větší nebo stejná, jako priority
 *    obou potomků.
 *
 * Smyslem haldové části struktury je udržet strom přibližně
 * vyvážený. Algoritmus vložení prvku pracuje takto:
 *
 *  1. na základě klíče vložíme uzel na vhodné místo tak, aby nebyla
 *     porušena vlastnost vyhledávacího stromu,
 *  2. je-li porušena vlastnost haldy, budeme «rotacemi» přesouvat
 *     nový uzel směrem ke kořenu, a to až do chvíle, než se tím
 *     vlastnost haldy obnoví.
 *
 * Budou-li priority přiděleny náhodně, vložení uzlu do větší
 * hloubky vede i k vyšší pravděpodobnosti, že tím bude vlastnost
 * haldy porušena; navíc rotace, které obnovují vlastnost haldy,
 * zároveň snižují maximální hloubku stromu. */

/* Implementujte typ ‹treap›, který bude reprezentovat množinu
 * pomocí datové struktury «treap» a poskytovat tyto operace (‹t› je
 * hodnota typu ‹treap›, ‹k›, ‹p› jsou hodnoty typu ‹int›):
 *
 *  • implicitně sestrojená instance ‹treap› reprezentuje prázdnou
 *    množinu,
 *  • ‹t.insert( k, p )› vloží klíč ‹k› s prioritou ‹p› (není-li
 *    uvedena, použije se náhodná); metoda vrací ‹true› pokud byl
 *    prvek skutečně vložen (dosud nebyl přítomen),
 *  • ‹t.erase( k )› odstraní klíč ‹k› a vrátí ‹true› byl-li
 *    přítomen,
 *  • ‹t.contains( k )› vrátí ‹true› je-li klíč ‹k› přítomen,
 *  • ‹t.priority( k )› vrátí prioritu klíče ‹k› (není-li přítomen,
 *    chování není definováno),
 *  • ‹t.clear()› smaže všechny přítomné klíče,
 *  • ‹t.size()› vrátí počet uložených klíčů,
 *  • ‹t.copy( v )›, kde ‹v› je reference na ‹std::vector< int >›,
 *    v lineárním čase vloží na konec ‹v› všechny klíče z ‹t› ve
 *    vzestupném pořadí,
 *  • metodu ‹t.root()›, které výsledkem je ukazatel ‹p›, pro který:
 *    ◦ ‹p->left()› vrátí obdobný ukazatel na levý podstrom,
 *    ◦ ‹p->right()› vrátí ukazatel na pravý podstrom,
 *    ◦ ‹p->key()› vrátí klíč uložený v uzlu reprezentovaném ‹p›,
 *    ◦ ‹p->priority()› vrátí prioritu uzlu ‹p›,
 *    ◦ je-li příslušný strom (podstrom) prázdný, ‹p› je ‹nullptr›.
 *  • konečně hodnoty typu ‹treap› nechť je možné přesouvat,
 *    kopírovat a také přiřazovat (a to i přesunem).¹
 *
 * Metody ‹insert›, ‹erase› a ‹contains› musí mít složitost lineární
 * k «výšce» stromu (při vhodné volbě priorit tedy očekávaně
 * logaritmickou k počtu klíčů). Metoda ‹erase› nemusí nutně
 * zachovat vazbu mezi klíči a prioritami (tzn. může přesunout klíč
 * do jiného uzlu aniž by zároveň přesunula prioritu). */

struct treap;

/* ¹ Verze s přesunem můžete volitelně vynechat (v takovém případě
 *   je označte jako smazané). Budou-li přítomny, budou testovány.
 *   Implementace přesunu je podmínkou hodnocení kvality známkou A. */

int main()
{
    treap t;
    std::vector< int > vec;

    assert( t.size() == 0 );
    assert( t.insert( 1, 0 ) );
    assert( !t.insert( 1, 1 ) );

    t.copy( vec );
    assert(( vec == std::vector{ 1 } ));
    assert( t.root() != nullptr );
    assert( t.root()->left() == nullptr );
    assert( t.root()->right() == nullptr );
    assert( t.root()->key() == 1 );
    assert( t.root()->priority() == 0 );

    assert( t.contains( 1 ) );
    assert( !t.contains( 2 ) );
    assert( t.erase( 1 ) );
    assert( !t.erase( 1 ) );
    assert( !t.contains( 1 ) );

    return 0;
}
