#include <memory>
#include <string_view>

/* Naprogramujte syntaktický analyzátor pro zjednodušený JSON:
 * v naší verzi nebudeme vůbec uvažovat „uvozovkované“ řetězce –
 * skaláry budou pouze čísla, klíče budou slova bez uvozovek (a tedy
 * například nebudou moct obsahovat mezery). Celý dokument se tedy
 * skládá z objektů (mapování klíč-hodnota ve složených závorkách),
 * polí (seznamů hodnot v hranatých závorkách) a celých čísel.
 *
 * Gramatika ve formátu EBNF:
 * 
 *     (* toplevel elements *)
 *     value   = blank, ( integer | array | object ), blank ;
 *     integer = [ '-' ], digits  | '0' ;
 *     array   = '[', valist, ']' | '[]' ;
 *     object  = '{', kvlist, '}' | '{}' ;
 *     
 *     (* compound data *)
 *     valist  = value,  { ',', value } ;
 *     kvlist  = kvpair, { ',', kvpair } ;
 *     kvpair  = blank, key, blank, ':', value ;
 *     
 *     (* lexemes *)
 *     digits  = nonzero, { digit } ;
 *     nonzero = '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' ;
 *     digit   = '0' | nonzero ;
 *     key     = keychar, { keychar } ;
 *     keychar = ? ASCII upper- or lower-case alphabetical character ? ;
 *     blank   = { ? ASCII space, tab or newline character ? } ;
 * 
 * Pro implementaci neterminálu ‹blank› můžete použít funkci
 * ‹std::isspace›. Rozhraní nechť je nasledovné: */

struct json_value;
using json_ptr = std::unique_ptr< json_value >;
using json_ref = const json_value &;

enum class json_type { integer, array, object };

struct json_error
{
    const char *what() const;
};

/* Typ ‹json_value› reprezentuje načtenou stromovou strukturu
 * dokumentu. Klademe na něj tyto požadavky:
 *
 *  • metoda ‹item_at› nechť skončí výjimkou ‹std::out_of_range›
 *    neexistuje-li požadovaný prvek,
 *  • je-li metodě ‹item_at› objektu typu ‹json_type::object›
 *    předáno číslo ⟦n⟧, vrátí ⟦n⟧-tou hodnotu v abecedním pořadí
 *    klíčů, přitom odpovídající klíč lze získat metodou ‹key_at›,
 *  • metoda ‹length› neselhává (pro celočíselné uzly vrátí
 *    nulu). */

struct json_value
{
    virtual json_type type() const = 0;
    virtual int int_value() const = 0;
    virtual json_ref item_at( int ) const = 0;
    virtual json_ref item_at( std::string_view ) const = 0;
    virtual std::string key_at( int i ) const = 0;
    virtual int length() const = 0;
    virtual ~json_value() = default;
};

/* Čistá funkce ‹json_parse› analyzuje dokument a vytvoří odpovídající
 * stromovou strukturu, nebo skončí výjimkou ‹json_error›:
 *
 *  • nevyhovuje-li řetězec zadané gramatice gramatice,
 *  • objeví-li se v kterémkoliv objektu zdvojený klíč. */

json_ptr json_parse( std::string_view );

/* Konečně čistá funkce ‹json_validate› rozhodne, je-li vstupní
 * dokument správně utvořený (tzn. odpovídá zadané gramatice). Tato
 * funkce nesmí skončit výjimkou (krom ‹std::bad_alloc› v situaci,
 * kdy během analýzy dojde paměť). */

bool json_validate( std::string_view );

int main()
{
    assert( !json_validate( "x" ) );
    assert( json_validate( "{}" ) );
    assert( json_validate( "[ 1 ]" ) );

    auto t = json_parse( "{}" );
    assert( t->type() == json_type::object );

    return 0;
}
