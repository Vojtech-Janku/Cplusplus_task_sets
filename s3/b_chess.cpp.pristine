/* Cílem tohoto úkolu je naprogramovat běžná pravidla šachu.
 * Předepsané typy ‹position›, ‹piece_type›, ‹player› ani ‹result›
 * není dovoleno upravovat. */

struct position
{
    int file; /* sloupec („písmeno“) – a = 1, b = 2, ... */
    int rank; /* řádek („číslo“) – 1, 2, …, 8 */

    position( int file, int rank ) : file( file ), rank( rank ) {}

    bool operator== ( const position &o ) const = default;
    auto operator<=>( const position &o ) const = default;
};

enum class piece_type
{
    pawn, rook, knight, bishop, queen, king
};

enum class player { white, black };

/* Metoda ‹play› může mít jeden z následujících výsledků. Možnosti
 * jsou uvedeny v prioritním pořadí, tzn. skutečným výsledkem je
 * vždy první aplikovatelná možnost.
 *
 * ├┄┄┄┄┄┄┄┄┄┄┄┄┄┄▻┼◅┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┤
 * │ ‹capture›     │ tah byl platný a sebral soupeřovu figuru     │
 * │ ‹ok›          │ tah byl platný                               │
 * │ ‹no_piece›    │ na pozici ‹from› není žádná figura           │
 * │ ‹bad_piece›   │ figura na pozici ‹from› nepatří hráči        │
 * │ ‹bad_move›    │ tah není pro tuto figuru platný              │
 * │ ‹blocked›     │ tah je blokován jinou figurou                │
 * │ ‹lapsed›      │ braní mimochodem již nelze provést           │
 * │ ‹has_moved›   │ některá figura rošády se už hýbala           │
 * │ ‹in_check›    │ hráč byl v šachu a tah by jej neodstranil    │
 * │ ‹would_check› │ tah by vystavil hráče šachu                  │
 * │ ‹bad_promote› │ pokus o proměnu na pěšce nebo krále          │
 *
 * Pokus o braní mimochodem v situaci, kdy jsou figury ve špatné
 * pozici, je ‹bad_move›. Krom výsledku ‹has_moved› může pokus
 * o rošádu skončit těmito chybami:
 *
 *  • ‹blocked› – v cestě je nějaká figura,
 *  • ‹in_check› – král je v šachu,
 *  • ‹would_check› – král by prošel nebo skončil v šachu. */

enum class result
{
    capture, ok, no_piece, bad_piece, bad_move, blocked, lapsed,
    in_check, would_check, has_moved, bad_promote
};

struct piece
{
    player owner;
    piece_type type;
};

using occupant = std::optional< piece >;

class chess
{
public:

    /* Sestrojí hru ve výchozí startovní pozici. První volání metody
     * ‹play› po sestrojení hry indikuje tah bílého hráče. */

    chess();

    /* Metoda ‹play› přesune figuru z pole ‹from› na pole ‹to›:
     *
     *  • umístí-li tah pěšce do jeho poslední řady (řada 8 pro
     *    bílého, resp. 1 pro černého), je proměněn na figuru
     *    zadanou parametrem ‹promote› (jinak je tento argument
     *    ignorován),
     *  • rošáda je zadána jako pohyb krále o více než jedno pole,
     *  • je-li výsledkem chyba (cokoliv krom ‹capture› nebo ‹ok›),
     *    stav hry se nezmění a další volání ‹play› provede tah
     *    stejného hráče. */

    result play( position from, position to,
                 piece_type promote = piece_type::pawn );

    /* Metoda ‹at› vrátí stav zadaného pole. */

    occupant at( position ) const;
};

int main()
{
    chess c;

    assert( c.play( { 1, 3 }, { 1, 4 } ) == result::no_piece );
    assert( c.play( { 1, 1 }, { 1, 2 } ) == result::blocked );
    assert( c.play( { 1, 7 }, { 1, 6 } ) == result::bad_piece );
    assert( c.play( { 1, 2 }, { 1, 4 } ) == result::ok );
    assert( c.play( { 1, 2 }, { 1, 4 } ) == result::no_piece );

    assert( c.at( { 1, 1 } ) );
    assert( c.at( { 1, 1 } )->type == piece_type::rook );
    assert( c.at( { 1, 1 } )->owner == player::white );
    assert( !c.at( { 1, 3 } ) );

    return 0;
}
