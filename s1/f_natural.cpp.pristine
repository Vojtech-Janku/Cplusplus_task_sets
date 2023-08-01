#include <cassert>

/* Vaším úkolem je tentokrát naprogramovat strukturu, která bude
 * reprezentovat libovolně velké přirozené číslo (včetně nuly). Tyto
 * hodnoty musí být možné:
 *
 *  • sčítat (operátorem ‹+›),
 *  • odečítat (‹x - y› je ovšem definováno pouze za předpokladu ‹x ≥ y›),
 *  • násobit (operátorem ‹*›),
 *  • libovolně srovnávat (operátory ‹==›, ‹!=›, ‹<›, atd.),
 *  • mocnit na kladný exponent typu ‹int› metodou ‹power›,
 *  • sestrojit z libovolné nezáporné hodnoty typu ‹int›.
 *
 * Implicitně sestrojená hodnota typu ‹natural› reprezentuje nulu.
 * Všechny operace krom násobení musí být nejvýše lineární vůči
 * «počtu dvojkových cifer» většího z reprezentovaných čísel.
 * Násobení může mít v nejhorším případě složitost přímo úměrnou
 * součinu ⟦m⋅n⟧ (kde ⟦m⟧ a ⟦n⟧ jsou počty cifer operandů). */

struct natural;

int main()
{
    natural zero;
    assert( zero + zero == zero );
    assert( zero * zero == zero );
    assert( zero - zero == zero );
    natural one( 1 );
    assert( one + zero == one );
    assert( one.power( 2 ) == one );

    return 0;
}
