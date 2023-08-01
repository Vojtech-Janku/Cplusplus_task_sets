#include <cassert>
#include <memory>
#include <set>
#include <vector>
#include <iostream>
#include <tuple>

/* Vaším úkolem bude tentokrát naprogramovat simulátor počítačové
 * sítě, s těmito třídami, které reprezentují propojitelné síťové
 * uzly:
 *
 *  • ‹endpoint› – koncový uzel, má jedno připojení k libovolnému
 *    jinému uzlu,
 *  • ‹bridge› – propojuje 2 nebo více dalších uzlů,
 *  • ‹router› – podobně jako bridge, ale každé připojení musí být
 *    v jiném segmentu.
 *
 * Dále bude existovat třída ‹network›, která reprezentuje síťový
 * segment jako celek. Každý uzel patří právě jednomu segmentu.
 * Je-li segment zničen, musí být zničeny (a odpojeny) i všechny
 * jeho uzly.
 *
 * Třída ‹network› bude mít tyto metody pro vytváření uzlů:
 *
 *  • ‹add_endpoint()› – vytvoří nový (zatím nepřipojený) koncový
 *    uzel, převezme jeho vlastnictví a vrátí vhodný ukazatel na
 *    něj,
 *  • ‹add_bridge( p )› – podobně pro ‹p›-portový bridge,
 *  • ‹add_router( i )› – podobně pro směrovač s ‹i› rozhraními.
 *
 * Jsou-li ‹m› a ‹n› libovolné typy uzlů, musí existovat vhodné
 * metody:
 *
 *  • ‹m->connect( n )› – propojí 2 uzly. Metoda je symetrická v tom
 *    smyslu, že ‹m->connect( n )› a ‹n->connect( m )› mají tentýž
 *    efekt. Metoda vrátí ‹true› v případě, že se propojení podařilo
 *    (zejména musí mít oba uzly volný port).
 *  • ‹m->disconnect( n )› – podobně, ale uzly rozpojí (vrací ‹true›
 *    v případě, že uzly byly skutečně propojené).
 *  • ‹m->reachable( n )› – ověří, zda může uzel ‹m› komunikovat
 *    s uzlem ‹n› (na libovolné vrstvě, tzn. včetně komunikace skrz
 *    routery; jedná se opět o symetrickou vlastnost; vrací hodnotu
 *    typu ‹bool›).
 *
 * Konečně třída ‹network› bude mít tyto metody pro kontrolu (a
 * případnou opravu) své vnitřní struktury:
 *
 *  • ‹has_loops()› – vrátí ‹true› existuje-li v síti cyklus,
 *  • ‹fix_loops()› – rozpojí uzly tak, aby byl výsledek acyklický,
 *    ale pro libovolné uzly, mezi kterými byla před opravou cesta,
 *    musí platit, že po opravě budou nadále vzájemně dosažitelné.
 *
 * Cykly, které prochází více sítěmi (a tedy prohází alespoň dvěma
 * směrovači), neuvažujeme. */

class network;

class node {
protected:
	network *_segment;
	std::set<node*> connected;
public:
	node( network *seg ) : _segment( seg ) {}

	const network *get_segment() const { return _segment; }

	virtual bool can_add( node *n ) const = 0;
	virtual void add( node *n ) 			 { connected.insert( n ); }
	virtual bool can_remove( node *n ) const { return ( n && connected.count(n) ); }
	virtual void remove( node *n ) 			 { connected.erase( n ); }

	bool connect( node *n ) {
		if ( !can_add( n ) || !n->can_add( this ) ) return false;
		add( n );
		n->add( this );
		return true;
	}
	
	bool disconnect( node *n ) {
		if ( !can_remove( n ) || !n->can_remove( this ) ) return false;
		remove( n );
		n->remove( this );
		return true;
	}

	virtual std::set<node*> get_connected() const { return connected; };

	bool find_node( node *n, std::set<node*> &visited ) const {
		for ( auto i : get_connected() ) {
			if ( !visited.count( i ) ) {
				visited.insert( i );
				if ( i == n || i->find_node( n, visited ) ) return true;
			}
		}
		return false;
	}
	
	bool reachable( node *n ) const {
		if ( n == this ) return true;
		std::set<node*> visited = {};
		return find_node( n, visited );
	}

	virtual ~node() {
		for ( auto n : connected ) { n->remove( this ); }
	}
};

class endpoint : public node {
public:
	endpoint( network *seg ) : node( seg ) {}

	bool can_add( node * ) const override { return connected.empty(); }
};

class bridge : public node {
	std::size_t capacity;
public:
	bridge( network *seg, int p ) : node( seg ), capacity( p ) {}

	bool can_add( node *n ) const override {
		if ( !n ) return false;
		for ( auto i : connected ) { if ( i == n ) return false; }
		return ( connected.size() < capacity );
	}
};

class router : public node {
	std::size_t capacity;
public:
	router( network *seg, int i ) : node( seg ), capacity( i ) {}

	bool can_add( node *n ) const override {
		if ( !n ) return false;
		for ( auto i : connected ) { 
			if ( i->get_segment() == n->get_segment() ) return false;
		}
		return ( connected.size() < capacity );
	}
};

class network {
	std::set<std::unique_ptr<node>> nodes;
public:
	endpoint *add_endpoint() {
		auto [it,_] = nodes.insert( std::make_unique<endpoint>( this ) );
		return dynamic_cast<endpoint *>( it->get() );
	}
	bridge *add_bridge( int p ) { 
		auto [it,_] = nodes.insert( std::make_unique<bridge>( this, p ) );
		return dynamic_cast<bridge *>( it->get() );
	}
	router *add_router( int i ) { 
		auto [it,_] = nodes.insert( std::make_unique<router>( this, i ) );
		return dynamic_cast<router *>( it->get() );		
	}

	bool find_cycle( std::set<node*> &visited, node *from, node *current, 
										std::tuple<node*,node*> &cycle_pair ) const {
		for ( auto i : current->get_connected() ) {
			if ( i->get_segment() != current->get_segment() ) continue;
			if ( visited.count( i ) ) {
				if ( i != from ) {
					cycle_pair = { current, i };
					return true;
				}
			} else {
				visited.insert( i );
				if ( find_cycle( visited, current, i, cycle_pair ) ) return true;
			}
		}
		return false;
	}

	bool has_loops() const {
		std::set<node*> all_nodes;
		std::set<node*> visited = {};
		std::tuple<node*, node*> p;
		for ( auto &i : nodes ) { all_nodes.insert( i.get() ); }
		for ( auto node_ptr : all_nodes ) {
			if ( !visited.count( node_ptr ) ) {
				visited.insert( node_ptr );
				if ( find_cycle( visited, nullptr, node_ptr, p ) ) return true;
			}
		}
		return false;
	}

	void fix_loops() {
		std::set<node*> all_nodes;
		std::set<node*> visited = {};
		std::tuple<node*, node*> p;
		for ( auto &i : nodes ) { all_nodes.insert( i.get() ); }
		for ( auto node_ptr : all_nodes ) {
			if ( !visited.count( node_ptr ) ) {
				std::set<node*> component_set = { node_ptr };
				bool cycle_found = find_cycle( component_set, nullptr, node_ptr, p );
				while ( cycle_found ) {
					auto [ node_x, node_y ] = p;
					node_x->disconnect( node_y );
					component_set = { node_ptr };
					cycle_found = find_cycle( component_set, nullptr, node_ptr, p );
				}
				visited.insert( component_set.begin(), component_set.end() );
			}
		}
	}
};

void test_multinet() {
	network net1, net2, net3;
	auto e11 = net1.add_endpoint(),
		 e12 = net1.add_endpoint(),
		 e13 = net1.add_endpoint();
	auto b11 =  net1.add_bridge( 3 ),
		 b12 =  net1.add_bridge( 10 );
	auto r1 =  net1.add_router( 3 );
	e11->connect( b11 );
	e12->connect( b11 );
	b11->connect( r1 );
	assert( !b11->connect( e13 ) );
	assert( !r1->connect( b12 ) );
	assert( !b12->connect( r1 ) );

	auto e21 = net2.add_endpoint(),
		 e22 = net2.add_endpoint();
	auto b2 =  net2.add_bridge( 3 );
	e21->connect( b2 );
	b2->connect( e22 );

	auto e31 = net3.add_endpoint(),
		 e32 = net3.add_endpoint(),
		 e33 = net3.add_endpoint();
	auto r3 =  net3.add_router( 3 );
	auto b3 =  net3.add_bridge( 2 );
	e31->connect( r3 );
	e32->connect( r3 );
	r3->connect( r1 );
	assert( !b3->connect( r1 ) );
	assert( !r1->connect( e33 ) );

	assert( b2->connect( r1 ) );
}

void test_components() {
	network net;
	auto b1 =  net.add_bridge( 10 ),
		 b2 =  net.add_bridge( 10 ),
		 b3 =  net.add_bridge( 10 ),
		 b4 =  net.add_bridge( 10 ),
		 b5 =  net.add_bridge( 10 ),
		 b6 =  net.add_bridge( 10 ),
		 b7 =  net.add_bridge( 10 ),
		 b8 =  net.add_bridge( 10 ),
		 b9 =  net.add_bridge( 10 );
	net.add_endpoint();
	b1->connect( b2 );
	b2->connect( b3 );
	b3->connect( b1 );
	
	b4->connect( b5 );
	b5->connect( b6 );
	b6->connect( b7 );
	b7->connect( b4 );
	b4->connect( b6 );
	b5->connect( b7 );

	b8->connect( b9 );

	assert( net.has_loops() );
	net.fix_loops();
	assert( !net.has_loops() );
}

void test_sigsegv() {
	network net0, net1;
	auto e01 = net0.add_endpoint(),
		 e11 = net1.add_endpoint();
	/*auto b11 =  net1.add_bridge( 3 ),
		 b12 =  net1.add_bridge( 10 );*/
	auto r0 =  net0.add_router( 2 ),
		 r1 =  net1.add_router( 2 );
	e01->connect(r0);
	e11->connect(r1);
	assert( !r0->connect(e01) );
	assert( !r0->connect(e11) );
	assert( r0->connect(r1) );

	assert( e01->reachable(e01) );
	assert( e11->reachable(e11) );
	assert( r0->reachable(r0) );
	assert( r1->reachable(r1) );
	
	assert( e01->reachable(r0) );
	assert( r0->reachable(e01) );
	assert( e01->reachable(r1) );
	assert( r1->reachable(e01) );
	assert( r0->reachable(r1) );
	assert( r1->reachable(r0) );
	assert( e01->reachable(e11) );
	assert( e11->reachable(e01) );
	
}

int main()
{
    network n;
    auto e1 = n.add_endpoint(),
         e2 = n.add_endpoint();
    
    auto b = n.add_bridge( 2 );
    auto r = n.add_router( 2 );
	
    assert( e1->connect( b ) );
    assert( b->connect( e2 ) );
    assert( e1->disconnect( b ) );
    assert( !e1->connect( e2 ) );
    assert( e2->reachable( b ) );
    assert( !n.has_loops() );
    n.fix_loops();
    assert( b->reachable( e2 ) );
    assert( r->connect( b ) );
    assert( !r->connect( e1 ) );

    test_multinet();
    test_components();
    test_sigsegv();

    return 0;
}
