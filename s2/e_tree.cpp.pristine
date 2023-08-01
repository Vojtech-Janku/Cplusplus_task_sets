#include <cassert>

/* Uvažujme stromovou strukturu, která má 4 typy uzlů, a která
 * představuje zjednodušený JSON:
 *
 *  • ‹node_bool› – listy typu ‹bool›,
 *  • ‹node_int› – listy typu ‹int›,
 *  • ‹node_array› – indexované celými čísly souvisle od nuly,
 *  • ‹node_object› – klíčované libovolnými celými čísly.
 *
 * Typ ‹tree› pak reprezentuje libovolný takový strom (včetně
 * prázdného a jednoprvkového). Pro hodnoty ‹t› typu ‹tree›, ‹n›
 * libovolného výše uvedeného typu ‹node_X› a ‹idx› typu ‹int›, jsou
 * všechny níže uvedené výrazy dobře utvořené.
 *
 * Práce s hodnotami typu ‹tree›:
 *
 *  • ‹t.is_null()› – vrátí ‹true› reprezentuje-li tato hodnota
 *    «prázdný strom»,
 *  • ‹*t› – platí-li ‹!t.is_null()›, jsou ‹(*t)› a ‹n› záměnné,
 *    jinak není definováno,
 *  • implicitně sestrojená hodnota reprezentuje prázdný strom,
 *  • hodnoty typu ‹tree› lze také vytvořit volnými funkcemi
 *    ‹make_X›, kde výsledkem je vždy strom s jediným uzlem typu
 *    ‹node_X› (v případě ‹make_bool› resp. ‹make_int› s hodnotou
 *    ‹false› resp. ‹0›, není-li v parametru uvedeno jinak).
 *
 * Hodnoty typu ‹node_X› lze sestrojit implicitně, a reprezentují
 * ‹false›, ‹0› nebo prázdné pole (objekt).
 *
 * Skalární operace (výsledkem je zabudovaný skalární typ):
 *
 *  • ‹n.is_X()› – vrátí ‹true›, je-li typ daného uzlu ‹node_X›
 *    (např. ‹is_bool()› určí, je-li uzel typu ‹node_bool›),
 *  • ‹n.size()› vrátí počet potomků daného uzlu (pro listy 0),
 *  • ‹n.as_bool()› vrátí ‹true› je-li ‹n› uzel typu ‹node_bool› a
 *    má hodnotu ‹true›, nebo je to uzel typu ‹node_int› a má
 *    nenulovou hodnotu, nebo je to neprázdný uzel typu ‹node_array›
 *    nebo ‹node_object›,
 *  • ‹n.as_int()› vrátí 1 nebo 0 pro uzel typu ‹node_bool›, hodnotu
 *    uloženou n uzlu ‹node_int›, nebo skončí výjimkou
 *    ‹std::domain_error›.
 *
 * Operace přístupu k potomkům:
 *
 *  • ‹n.get( idx )› vrátí odkaz (referenci) na potomka:
 *    ◦ s indexem (klíčem) ‹idx› vhodného typu tak, aby
 *      s ní bylo možné pracovat jako s libovolnou hodnotou typu
 *      ‹node_X›, nebo
 *    ◦ skončí výjimkou ‹std::out_of_range› když takový potomek
 *      neexistuje,
 *  • ‹n.copy( idx )› vrátí potomka na indexu (s klíčem) ‹idx› jako
 *    «hodnotu» typu ‹tree›, nebo skončí výjimkou
 *    ‹std::out_of_range› neexistuje-li takový.
 *
 * Operace, které upravují existující strom:
 *
 *  • ‹n.set( idx, t )› nastaví potomka na indexu nebo u klíče ‹i› na
 *    hodnotu ‹t›, přitom samotné ‹t› není nijak dotčeno, přitom:
 *    ◦ je-li ‹n› typu ‹node_array›, je rozšířeno dle potřeby tak,
 *      aby byl ‹idx› platný index, přitom takto vytvořené indexy
 *      jsou «prázdné»),
 *    ◦ je-li ‹n› typu ‹node_bool› nebo ‹node_int›, skončí
 *      s výjimkou ‹std::domain_error›,
 *  • ‹n.take( idx, t )› totéž jako ‹set›, ale podstrom je z ‹t›
 *    přesunut, tzn. metoda ‹take› nebude žádné uzly kopírovat a po
 *    jejím skončení bude platit ‹t.is_null()›.
 *
 * Všechny metody a návratové hodnoty referenčních typů musí správně
 * pracovat s kvalifikací ‹const›. Vytvoříme-li kopii hodnoty typu
 * ‹tree›, tato bude obsahovat kopii celého stromu. Je-li umožněno
 * kopírování jednotlivých uzlů, nemá určeno konkrétní chování. */

class node_bool;
class node_int;
class node_array;
class node_object;
class tree;

int main()
{
    tree tt = make_bool( true ),
         tf = make_bool(),
         ta = make_array(),
         to = make_object();

    const tree &c_tt = tt,
               &c_ta = ta,
               &c_to = to;

    auto &na = *ta;
    const auto &c_na = *c_ta;
    auto &no = *to;
    const auto &c_no = no;

    assert( (*c_tt).as_bool() );
    assert( !(*tf).as_bool() );
    assert( !c_na.as_bool() );

    na.set( 0, ta );
    na.take( 1, make_bool() );

    assert( !ta.is_null() );
    assert( !c_ta.is_null() );
    assert( !c_to.is_null() );

    no.set( 1, ta );
    na.take( 1, to );

    assert( to.is_null() );
    assert( !(*ta).get( 0 ).as_bool() );
    assert( !(*c_ta).get( 0 ).as_bool() );
    assert( c_no.get( 1 ).size() == 2 );

    return 0;
}

// clang-tidy: -clang-analyzer-cplusplus.Move
