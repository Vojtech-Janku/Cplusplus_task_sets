#include <cassert>
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include <stdexcept>

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


/*enum node_type { BOOL, INT, ARRAY, OBJECT };
void print( node_type type ) {
	switch ( type ) {
		case node_type::BOOL:	std::cout << "BOOL";   break;
		case node_type::INT:	std::cout << "INT";    break;
		case node_type::ARRAY:	std::cout << "ARRAY";  break;
		case node_type::OBJECT:	std::cout << "OBJECT"; break;
		default: std::cout << "ERR"; break;
	}
}*/

class node;
class tree;



class node {
public:
	virtual bool is_bool() const { return false; }
	virtual bool is_int() const { return false; }
	virtual bool is_array() const { return false; }
	virtual bool is_object() const { return false; }
	
	virtual std::size_t size() const { return 0; }
	virtual bool as_bool() const = 0;
	virtual int as_int() const { throw std::domain_error( "node cannot be converted to int" ); return 0; }

	virtual node &get( int ) { throw std::out_of_range( "index not found" ); }
	virtual const node &get( int ) const { throw std::out_of_range( "index not found" ); }
	virtual tree copy( int ) const = 0; //{ throw std::out_of_range( "index not found" ); }

	virtual void set( int, const tree & ) { throw std::domain_error( "node cannot have children" ); }
	virtual void take( int, tree & ) { throw std::domain_error( "node cannot have children" ); }
	virtual void take( int, tree && ) { throw std::domain_error( "node cannot have children" ); }
	
	virtual std::unique_ptr<node> duplicate() const = 0;

	virtual ~node() = default;
};

class tree {
public:
	std::unique_ptr<node> _root = nullptr;

	tree() = default;
	tree( std::unique_ptr<node> p ) {
		if (p) _root = std::move(p);
	}
	tree( const tree &other ) { if ( !other.is_null() ) _root = other._root->duplicate(); }
	tree( tree &&other ) 	  { if ( !other.is_null() ) _root = std::move( other._root ); }

	tree &operator=( const tree &other )  { if ( !other.is_null() ) _root = other._root->duplicate(); return *this; }
	tree &operator=( tree &&other ) 	  { if ( !other.is_null() ) _root = std::move( other._root ); return *this; }
	
	bool is_null() const { return !_root; }

	node &operator*() {
		if ( is_null() ) throw std::runtime_error( "cannot dereference null tree" );
		return *_root;
	}
	const node &operator*() const {
		if ( is_null() ) throw std::runtime_error( "cannot dereference null tree" );
		return *_root;
	}	
};

class node_bool : public node {
public:
	bool val;

	node_bool() : val(false) {}
	node_bool( bool v ) : val(v) {}
	
	bool is_bool() const override { return true; }
	bool as_bool() const override { return val; }
	int  as_int()  const override { return (val) ? 1 : 0; }

	tree copy( int ) const override { throw std::out_of_range( "index not found" ); }

	std::unique_ptr<node> duplicate() const override { return std::make_unique<node_bool>( val ); }
};

class node_int : public node {
public:
	int val;

	node_int() : val(0) {}
	node_int( int v ) : val(v) {}

	bool is_int() const override { return true; }
	bool as_bool() const override { return val != 0; }
	int  as_int()  const override { return val; }

	tree copy( int ) const override { throw std::out_of_range( "index not found" ); }

	std::unique_ptr<node> duplicate() const override { return std::make_unique<node_int>( val ); }
};

class node_array : public node {
public:
	std::vector<std::unique_ptr<node>> children;

	bool is_array() const override { return true; }
	std::size_t size() const override { return children.size(); } 		// size of vector or not nulls???
	bool as_bool() const override { return size() > 0; }

	std::unique_ptr<node> duplicate() const override {
		auto new_node = std::make_unique<node_array>();
		new_node->children.resize( size() );
		for ( std::size_t i = 0; i < size(); ++i ) {
			if ( children[i] ) new_node->children[i] = children[i]->duplicate();
		}
		return new_node;
	}

	node &get( int idx ) override {
		if ( idx < 0 || static_cast<std::size_t>(idx) >= size() || !children[idx].get() ) throw std::out_of_range( "index not found" );
		return *(children[idx]);
	}
	const node &get( int idx ) const override {
		if ( idx < 0 || static_cast<std::size_t>(idx) >= size() || !children[idx].get() ) throw std::out_of_range( "index not found" );
		return *(children[idx]);
	}
	tree copy( int idx ) const override {
		if ( idx < 0 || static_cast<std::size_t>(idx) >= size() ) throw std::out_of_range( "index not found" );
		if ( !children[idx] ) return {};
		return { children[idx]->duplicate() };
	}

	void set( int idx, const tree &t ) override {
		auto node_copy = ( t.is_null() ) ? nullptr : t._root->duplicate();
		if ( static_cast<std::size_t>(idx) >= size() ) children.resize( idx+1 );
		children[idx] = std::move(node_copy);
	}
	void take( int idx, tree &t ) override {
		if ( static_cast<std::size_t>(idx) >= size() ) children.resize( idx+1 );
		if ( t.is_null() ) {
			children[idx] = nullptr;
		} else {
			children[idx] = std::move( t._root );
		}
	}
	void take( int idx, tree &&t ) override {
		if ( static_cast<std::size_t>(idx) >= size() ) children.resize( idx+1 );
		if ( t.is_null() ) {
			children[idx] = nullptr;
		} else {
			children[idx] = std::move( t._root );
		}
	}	
};

class node_object : public node {
public:
	std::map<int, std::unique_ptr<node>> children;

	bool is_object() const override { return true; }
	std::size_t size() const override { return children.size(); }
	bool as_bool() const override { return size() > 0; }

	std::unique_ptr<node> duplicate() const override {
		auto new_node = std::make_unique<node_object>();
		for ( auto &[i, p] : children ) {
			new_node->children[i] = p->duplicate();
		}
		return new_node;
	}

	node &get( int idx ) override {
		if ( children.find(idx) == children.end() ) throw std::out_of_range( "index not found" );
		return *(children[idx]);
	}
	const node &get( int idx ) const override {
		if ( children.find(idx) == children.end() ) throw std::out_of_range( "index not found" );
		return *(children.at(idx));
	}
	tree copy( int idx ) const override { return { get(idx).duplicate() }; }

	void set( int idx, const tree &t ) override {
		if ( t.is_null() ) {
			children.erase(idx);
		} else {
			auto node_copy = t._root->duplicate();
			children[idx] = std::move(node_copy);
		}
	}
	void take( int idx, tree &t ) override {
		if ( t.is_null() ) {
			children[idx] = nullptr;
		} else {
			children[idx] = std::move( t._root );
		}
	}
	void take( int idx, tree &&t ) override {
		if ( t.is_null() ) {
			children[idx] = nullptr;
		} else {
			children[idx] = std::move( t._root );
		}
	}	
	
};

tree make_bool( bool val = false ) { 
	tree t;
	t._root = std::make_unique<node_bool>( val );
	return t;
}
tree make_int( int val = 0 ) { 
	tree t;
	t._root = std::make_unique<node_int>( val );
	return t;
}
tree make_array() { 
	tree t;
	t._root = std::make_unique<node_array>();
	return t;
}
tree make_object() { 
	tree t;
	t._root = std::make_unique<node_object>();
	return t;
}

std::tuple<int, tree> enum_tree( int id ) {
	tree t;
	auto res = std::tuple{ id, t };
	return res;
}

void test_pairs() {
	std::cout << "TEST PAIRS" << std::endl;
	tree t{};
	t = make_object();
	auto [child_id, child_tree] = enum_tree( 1 );
}

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

    tree tnull;
    na.set( 5, tnull );
    assert( tnull.is_null() );
    assert( (*c_ta).size() == 6 );
    auto cp_null = na.copy( 4 );
    assert( cp_null.is_null() );

    test_pairs();
    
    return 0;
}
