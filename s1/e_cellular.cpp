#include <cassert>
#include <vector>
#include <iostream>

/* Vaším úkolem bude naprogramovat jednoduchý simulátor
 * jednorozměrného celulárního automatu. Implementace bude sestávat
 * ze dvou struktur, ‹automaton_state› a ‹automaton›, které jsou
 * popsané níže. Zadané rozhraní je nutné dodržet. */

/* Definujte strukturu, ‹automaton_state›, která reprezentuje stav
 * automatu definovaného na «kružnici», s buňkami číslovanými po
 * směru hodinových ručiček od indexu 0. Stav si můžete představit
 * takto:
 *
 *             ┌───┐
 *         ┌───┤ 1 ├───┐
 *         │ 1 ├───┤ 0 │
 *       ┌─┴─┬─┘ ▲ └─┬─┴─┐
 *       │ 0 │   ●   │ 0 │
 *       └─┬─┴─┐   ┌─┴─┬─┘
 *         │ 1 ├───┤ 1 │
 *         └───┤ 1 ├───┘
 *             └───┘
 *
 * Platným indexem je «libovolné celé číslo» – určuje o kolik
 * políček od indexu 0 se posuneme (kladná čísla po směru, záporná
 * proti směru hodinových ručiček).
 *
 * Jsou-li ‹s›, ‹t› hodnoty typu ‹automaton_state›, dále ‹i›, ‹n›
 * jsou hodnoty typu ‹int› a ‹v› je hodnota typu ‹bool›, tyto
 * výrazy musí být dobře utvořené:
 *
 *  • ‹automaton_state( s )› vytvoří nový stav, který je stejný jako
 *    stav ‹s›,
 *  • ‹automaton_state( n )› vytvoří nový stav o ‹n› buňkách, které
 *    jsou všechny nastaveny na ‹false› (pro ‹n ≤ 0› není
 *    definováno),
 *  • ‹s.size()› vrátí aktuální počet buněk stavu ‹s›,
 *  • ‹s.get( i )› vrátí hodnotu buňky na indexu ‹i›,
 *  • ‹s.set( i, v )› nastaví buňku na indexu ‹i› na hodnotu ‹v›,
 *  • ‹s.extend( n )› vloží ‹n› nových buněk nastavených na hodnotu
 *    ‹false›, a to tak, že nové buňky budou na indexech ⟦-1⟧ až
 *    ⟦-n⟧ (je-li ‹n› záporné, chování není definováno),
 *  • ‹s.reduce( n )› odstraní ‹n› buněk proti směru hodinových
 *    ručiček, počínaje indexem -1 (je-li ‹n ≥ s.size()› nebo je ‹n›
 *    záporné, chování není definováno),
 *  • ‹t = s› upraví stav ‹t› tak, aby byl stejný jako ‹s›,
 *  • ‹t == s› je ‹true› právě když jsou stavy ‹s› a ‹t› stejné,
 *  • ‹t != s› je ‹true› právě když se stavy ‹s› a ‹t› liší,
 *  • ‹t <= s› se vyhodnotí na ‹true› právě když pro všechny indexy
 *    ‹i› platí ‹s.get( i ) || !t.get( i )›,
 *  • ‹t < s› se vyhodnotí na ‹true› právě když ‹t <= s && t != s›.
 *
 * Je-li to možné, výrazy musí pracovat správně i v případech, kdy
 * jsou ‹s› a/nebo ‹t› konstantní. Metody ‹size›, ‹get› a ‹set› musí
 * pracovat v konstantním čase, vše ostatní v čase nejvýše
 * lineárním. */

int int_modulo( int n, int m ) {
	if ( n % m >= 0 ) {
		return (n % m);
	}
	return (n % m) + m; // for negative n % m
}

struct automaton_state {
	std::vector<bool> cells;

	automaton_state( const automaton_state &s ) {
		cells = s.cells;
	}
	automaton_state( int n ) {
		cells = std::vector<bool>(n,false);
	}

	int size() const {
		return cells.size();
	}

	bool get( int i ) const {
		return cells[ int_modulo( i, size() ) ];
	}
	void set( int i, bool v ) {
		cells[ int_modulo( i, size() ) ] = v;
	}

	void extend( int n ) {
		cells.resize( size() + n, false );
	}

	void reduce( int n ) {
		cells.resize( size() - n );
	}

	automaton_state &operator=( const automaton_state &s ) = default;

	bool operator==( const automaton_state &s ) {
		if ( size() != s.size() ) return false;
		for ( int i = 0; i < size(); i++ ) {
			if ( get(i) != s.get(i) ) return false;
		}
		return true;
	}
	bool operator!=( const automaton_state &s ) {
		return !(*this == s);
	}
	bool operator<=( const automaton_state &s ) {
		if ( size() != s.size() ) return false;
		for ( int i = 0; i < size(); i++ ) {
			if ( !get( i ) && s.get( i ) ) return false;
		}
		return true;
	}
	bool operator<( const automaton_state &s ) {
		return (*this <= s) && (*this != s);
	}

	void print() {
		std::cout << "<";
		for ( auto i : cells ) {
			std::cout << ( i ? '*' : ' '); //<< ",";
		}
		std::cout << ">" << std::endl;
	}
};

/* Struktura ‹automaton› reprezentuje samotný automat. Třída si
 * udržuje interní stav, na kterém provádí výpočty (tzn. například
 * volání metody ‹step()› změní tento interní stav).
 *
 * Následovné výrazy musí být dobře utvořené (kde ‹a›, ‹b› jsou
 * hodnoty typu ‹automaton›, ‹s› je hodnota typu ‹automaton_state›,
 * a konečně ‹n› a ‹rule› jsou hodnoty typu ‹int›):
 *
 *  • ‹automaton( rule, n )› sestrojí automat s ‹n› buňkami
 *    nastavenými na ‹false› (chování pro ‹n ≤ 0› není definováno),
 *    a s pravidlem ‹rule› zadaným tzv. Wolframovým kódem (chování
 *    je definováno pouze pro ‹rule› v rozsahu 0 až 255 včetně),
 *  • ‹automaton( rule, s )› sestrojí nový automat a nastaví jeho
 *    vnitřní stav tak, aby byl stejný jako ‹s› (význam parametru
 *    ‹rule› je stejný jako výše),
 *  • ‹a.state()› umožní přístup k internímu stavu, a to tak, že je
 *    možné jej měnit, není-li samotné ‹a› konstantní (např.
 *    ‹a.state().set( 3, true )› nastaví buňku interního stavu
 *    s indexem 3 na hodnotu ‹true›),
 *  • ‹a = b› nastaví automat ‹a› tak, aby byl stejný jako automat
 *    ‹b› (zejména tedy upraví nastavené pravidlo a vnitřní stav),
 *  • ‹a.step()› provede jeden krok výpočtu na vnitřním stavu (jeden
 *    krok nastaví všechny buňky vnitřního stavu na další generaci),
 *  • ‹a.reset( s )› přepíše vnitřní stav kopií stavu ‹s›.
 *
 * Hodnoty, které vstupují do výpočtu nové generace buňky podle
 * zadaného Wolframova kódu, čteme po směru hodinových ručiček (tzn.
 * ve směru rostoucích indexů).
 *
 * Krok výpočtu musí mít nejvýše lineární (časovou i paměťovou)
 * složitost. */

bool compute_cell( const automaton_state &a, int i, int rule ) {
	int bit_idx = 4*a.get(i-1) + 2*a.get(i) + a.get(i+1);
	if ( bit_idx == 0 ) return false;
	return rule & ( 1 << ( bit_idx ) );
}

struct automaton {

	int rule;
	automaton_state aut_state;
	
	automaton( int r, int n ) : rule(r), aut_state( automaton_state( n ) ) {}
	automaton( int r, automaton_state s ) : rule(r), aut_state(s) {}

	automaton_state &state() {
		return aut_state;
	}
	const automaton_state &state() const {
		return aut_state;
	}
	
	void step() {
		auto new_state = aut_state;
		for ( int i = 0; i < aut_state.size(); i++ ) {
			new_state.set( i, compute_cell( aut_state, i, rule ) );
		}
		aut_state = new_state;
	}
	void reset( automaton_state s ) {
		aut_state = s;
	}

	automaton &operator=( const automaton &b ) = default;
};

int main()
{

    automaton_state s( 3 );
    const auto &s_const = s;

    assert( s.size() == 3 );
    assert( !s_const.get( 0 ) );
    s.set( 0, true );
    assert( s_const.get( 0 ) );

    automaton a( 0, s );
    const auto &a_const = a;
    a.step();
    assert( !a_const.state().get( 0 ) );
    a.state().set( 0, true );
    assert( a_const.state().get( 0 ) );
    a.step();
    assert( !a_const.state().get( 0 ) );

    automaton_state s1( 15 );
    //s1.print();
    s1.set( 0, true );
    //s1.print();
	automaton a1( 2, s1 );
	a1.step();
	//a1.state().print();
	assert( !a1.state().get(0) );
	assert( a1.state().get(-1) );
    
    automaton_state unit180( 180 );
    unit180.set( 90, true );
    automaton a_complex( 30, unit180 );
    for ( int i = 0; i < 100; ++i ) {
    	a_complex.state().print();
    	a_complex.step();
    }
    

    return 0;
}
