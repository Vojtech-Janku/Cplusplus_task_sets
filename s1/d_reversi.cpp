#include <tuple>
#include <cassert>

/* Předmětem tohoto úkolu je hra Reversi (známá také jako Othello),
 * avšak ve třírozměrné verzi. Hra se tedy odehrává v kvádru, který
 * se skládá ze sudého počtu polí (krychlí) v každém ze tří
 * základních směrů (podle os ⟦x⟧, ⟦y⟧ a ⟦z⟧). Dvě taková pole můžou
 * sousedit stěnou (6 směrů), hranou (12 směrů) nebo jediným
 * vrcholem (8 směrů). Pole může být prázdné, nebo může obsahovat
 * černý nebo bílý hrací kámen.
 *
 * Hru hrají dva hráči (černý a bílý, podle barvy kamenů, které jim
 * patří) a pravidla hry jsou přímočarým rozšířením těch klasických
 * dvourozměrných:
 *
 *  • každý hráč má na začátku 4 kameny, rozmístěné kolem
 *    prostředního bodu kvádru (jedná se tedy o 8 polí, které
 *    tento bod sdílí), a to tak, že žádná dvě obsazená pole
 *    stejné barvy nesdílí stěnu, přičemž pole s nejmenšími
 *    souřadnicemi ve všech směrech obsahuje bílý kámen,
 *  • hráči střídavě pokládají nový kámen do volného pole; je-li na
 *    tahu bílý hráč, pokládá bílý kámen do pole, které musí být
 *    nepřerušeně spojeno¹ černými kameny s alespoň jedním
 *    stávajícím bílým kamenem (černý hráč hraje analogicky),
 *  • po položení nového kamene se barva všech kamenů, které leží
 *    na libovolné takové spojnici, změní na opačnou (tzn. přebarví
 *    se na barvu právě položeného kamene).
 *
 * Začíná bílý hráč. Hra končí, není-li možné položit nový kámen
 * (ani jedné barvy). Vyhrává hráč s více kameny na ploše. */

struct reversi
{
    /* Metoda ‹start› začne novou hru na ploše zadané velikosti.
     * Případná rozehraná partie je tímto voláním zapomenuta. Po
     * volání ‹start› je na tahu bílý hráč. */

    void start( int x_size, int y_size, int z_size );

    /* Metoda ‹size› vrátí aktuální velikost hrací plochy. */

    std::tuple< int, int, int > size() const;

    /* Metoda ‹play› položí kámen na souřadnice zadané parametrem.
     * Barva kamene je určena tím, který hráč je právě na tahu.
     * Byl-li tah přípustný, metoda vrátí ‹true› a další volání
     * položí kámen opačné barvy. V opačném případě se hrací plocha
     * nezmění a stávající hráč musí provést jiný tah. */

    /* Není určeno, co se má stát v případě, že hra ještě nezačala,
     * nebo již skončila (tzn. nebyla zavolána metoda ‹start›, nebo
     * by metoda ‹finished› vrátila ‹true›). */

    bool play( int x, int y, int z );

    /* Nemůže-li aktivní hráč provést platný tah, zavolá metodu
     * ‹pass›. Tato vrátí ‹true›, jedná-li se o korektní přeskočení
     * tahu (má-li hráč k dispozici jakýkoliv jiný platný tah,
     * «musí» nějaký provést – volání ‹pass› v takovém případě vrátí
     * ‹false› a aktivní hráč se nemění).
     *
     * Platí stejná omezení na stav hry jako u metody ‹play›. */

    bool pass();

    /* Metoda-predikát ‹finished› vrací ‹true› právě tehdy,
     * nemůže-li ani jeden z hráčů provést platný tah a hra tedy
     * skončila. Výsledek volání není určen pro hru, která dosud
     * nezačala (nedošlo k volání metody ‹start›). */

    bool finished() const;

    /* Metodu ‹result› je povoleno zavolat pouze v případě, že hra
     * skončila (tzn. volání ‹finished› by vrátilo ‹true›). Její
     * návratovou hodnotou je rozdíl v počtu kamenů mezi bílým a
     * černým hráčem – kladné číslo značí výhru bílého hráče,
     * záporné výhru černého hráče a nula značí remízu. */

    int result() const;
};

/* ¹ Uvažujme dvojicí polí (krychlí) ⟦A⟧, ⟦B⟧ a úsečku ⟦u⟧, která
 *   spojuje jejich středy, a která prochází středem stěny, hrany
 *   nebo vrcholem pole ⟦A⟧. Nepřerušeným spojením myslíme všechna
 *   pole, které úsečka ⟦u⟧ protíná, vyjma ⟦A⟧ a ⟦B⟧ samotných.
 *   Dvojici polí, pro které potřebná úsečka ⟦u⟧ neexistuje, nelze
 *   nepřerušeně spojit. */

int main()
{
    reversi r;
    r.start( 4, 4, 4 );
    assert( !r.play( 2, 2, 2 ) );
    assert( r.play( 3, 3, 3 ) );
    assert( !r.play( 3, 3, 3 ) );
    return 0;
}
