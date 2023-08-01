#include <cassert>
#include <vector>
#include <memory>
#include <random>
#include <queue>
#include <iostream>

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

struct node {
	int _key;
	int _priority;
	node *parent;
	std::unique_ptr<node> _left = nullptr;
	std::unique_ptr<node> _right = nullptr;

	node( int k, int p, node *par ) : _key(k), _priority(p), parent( par ) {}

	node *left() const   { return _left.get(); }
	node *right() const  { return _right.get(); }
	int key() const 	   { return _key; }
	int priority() const { return _priority; }

	node *add_node( int k, int p ) {
		if ( k < _key ) {
			_left = std::make_unique<node>( k, p, this );
			return _left.get();
		} else {
			_right = std::make_unique<node>( k, p, this );
			return _right.get();
		}
	}

	std::size_t count() const {
		std::size_t n_count = 1;
		if ( left() )  n_count += left()->count();
		if ( right() ) n_count += right()->count();
		return n_count;
	}

	void copy( std::vector<int> &vec ) const {
		if ( left() ) left()->copy( vec );
		vec.push_back( _key );
		if ( right() ) right()->copy( vec );
	}

	std::unique_ptr<node> duplicate( node *par_dupl ) {
		auto n = std::make_unique<node>( key(), priority(), par_dupl );
		if ( left() )  n->_left  = right()->duplicate( this );
		if ( right() ) n->_right = right()->duplicate( this );
		return n;
	}

	void print() const {
		std::cout << "( k: " << _key << ", p: " << _priority << ", children: ";
		if ( left() )  std::cout << "l ";
		if ( right() ) std::cout << "r ";
		std::cout << ") ";
	}
};

struct treap {
	std::unique_ptr<node> _root = nullptr;

	treap() = default;
	treap( treap &&other ) {
		_root = std::move( other._root );		
	}
	treap( const treap &other ) {
		if ( other.root() ) _root = other.root()->duplicate( nullptr );
	};
	treap &operator=( treap &&other ) {
		_root = std::move( other._root );
		return *this;
	}
	treap &operator=( const treap &other ) {
		if ( other.root() ) _root = other.root()->duplicate( nullptr );
		return *this;
	}
	

	// rOTat e ba nAn a
	node *rotate( node *n ) {
		node *p = n->parent;
		if ( !p || p->priority() >= n->priority() ) return nullptr;
		
		std::unique_ptr<node> &transfer_child = ( n->key() < p->key() ) ? n->_right : n->_left;
		std::unique_ptr<node> &n_ptr = 			( n->key() < p->key() ) ? p->_left : p->_right;
		std::unique_ptr<node> &p_ptr = 			( !p->parent ) ? _root : 
					 		 ( ( p->key() < p->parent->key() ) ? p->parent->_left : p->parent->_right );

		if ( transfer_child ) transfer_child->parent = p_ptr.get();
		n_ptr->parent = p_ptr->parent;
		p_ptr->parent = n_ptr.get();
		
		std::unique_ptr<node> temp = std::move( transfer_child );
		transfer_child = std::move( p_ptr );
		p_ptr = std::move( n_ptr );
		n_ptr = std::move( temp );
		return p_ptr.get();
	}
	
	node *root() const { return _root.get(); }

	//	finds the node with key closest to k that has 1 or 0 children
	node *find_key( int k ) const {
		if ( !root() ) return nullptr;
		node *current = root();
		while( k != current->key() ) {
			if ( k < current->key() && current->left() ) {
				current = current->left();
			} else if ( k > current->key() && current->right() ) {
				current = current->right();
			} else {
				return current;
			}
		}
		return current;
	}
	
	bool insert( int k, int p = rand() ) {
		if ( !root() ) {
			_root = std::make_unique<node>( k, p, nullptr );
			return true;
		}
		node *n = find_key( k );
		if ( n->key() == k ) return false;
		node *new_node = n->add_node( k, p ); 
		new_node = rotate( new_node );
		while( new_node ) {
			new_node = rotate( new_node );
		}
		return true;
	}

	//	deletes node with 1 or 0 children 
	//	and transfers the child pointer ( or nullptr ) to the parent
	node *delete_node( node *n ) {
		node *p = n->parent;

		std::unique_ptr<node> &child = ( n->left() ) ? n->_left : n->_right;
		if ( child ) child->parent = p;
		if ( !p ) {
			_root = (child) ? std::move( child ) : nullptr;
		} else {
			std::unique_ptr<node> &n_ptr = ( n->key() < p->key() ) ? p->_left : p->_right;
			n_ptr = (child) ? std::move( child ) : nullptr;
		}
		return p;
	}
	
	bool erase( int k ) {
		node *n = find_key( k );
		if ( !n || n->key() != k ) return false;
		
		if ( n->left() && n->right() ) {	// node is inner with two children
			node *succ = n->right(); 	// = find_key(k+1);
			while( succ->left() ) { succ = succ->left(); }
			n->_key = succ->key();
			delete_node( succ );
		} else {
			delete_node( n );			
		}
		return true;
	}

	bool contains( int k ) const {
		node *n = find_key( k );
		return ( n && n->key() == k );
	}
	
	int priority( int k ) {
		node *n = find_key( k );
		if ( !n || n->key() != k ) return -1;
		return n->priority();
	}
	
	void clear() { _root = nullptr; }
	
	std::size_t size() const {
		if ( !root() ) return 0;
		return root()->count();
	}
	
	void copy( std::vector<int> &v ) const {
		if ( !root() ) return;
		root()->copy( v );
	}

	void print() const {
		std::queue<node*> layer;
		if ( root() ) layer.push( root() );
		while( !layer.empty() ) {
			std::queue<node*> next_layer;
			while ( !layer.empty() ) {
				auto i = layer.front();
				layer.pop();
				i->print();
				if ( i->left() ) next_layer.push( i->left() );
				if ( i->right() ) next_layer.push( i->right() );
			}
			std::cout << std::endl;
			layer = next_layer;
		}
		std::cout << std::endl;
	}
};

void print( std::vector<int> v ) {
	std::cout << "< ";
	for ( auto i : v ) {
		std::cout << i << " ";
	}
	std::cout << ">" << std::endl;
}

treap make_test_treap() {
	treap t;
	t.insert( 1, rand() % 111 );
	t.insert( 7, rand() % 2000 );
	t.insert( 9, rand() % 2000 );
	t.insert( 2, rand() % 111 );
	t.insert( 3, rand() % 2000 );
	t.insert( 10, rand() % 111 );
	t.insert( 11, rand() % 2000 );
	t.insert( 5, rand() % 111 );
	t.insert( 8, rand() % 2000 );
	t.insert( 6, rand() % 3000 );
	return t;	
}

void test_insert() {
	std::cout << "TEST INSERT" << std::endl;
	treap t;
	std::vector<int> insert_key = { 1, 7, 9, 2, 3, 10, 11, 5, 8, 6 };
	std::vector<int> insert_pr_mod = { 111, 2000, 2000, 111, 2000, 111, 2000, 111, 2000, 3000 };
	for ( std::size_t i = 0; i < insert_key.size(); ++i ) {
		std::cout << "Inserting " << insert_key[i] << std::endl;
		t.insert( insert_key[i], rand() % insert_pr_mod[i] );
		t.print();		
	}

	std::vector<int> v;
	t.copy(v);
	print(v);
}

void test_erase() {
	std::cout << "TEST ERASE" << std::endl;
	treap t = make_test_treap();
	std::vector<int> to_erase = { 7, 1, 3, 11, 5, 8 };
	print( to_erase );
	t.print();
	for ( auto i : to_erase ) {
		std::cout << "Erasing " << i << std::endl;
		t.erase( i );
		t.print();		
	}

	std::vector<int> v;
	t.copy(v);
	print(v);	
}

void test_copy() {
	std::cout << "TEST COPY" << std::endl;
	treap t;
	const treap &ct = t;
	treap new_t = ct;
	assert( !new_t.root() );
	new_t.insert( 1 );
	assert( new_t.contains(1) );

	treap constr_t( new_t );
	assert( constr_t.contains(1) );

	treap temp_t = treap();
	assert( !temp_t.contains(1) ) ;
}

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

    //test_insert();
    test_erase();
    test_copy();

    return 0;
}
