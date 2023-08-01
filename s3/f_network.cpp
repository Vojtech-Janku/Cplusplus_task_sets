#include <algorithm>
#include <cassert>
#include <memory>
#include <set>
#include <vector>
#include <list>
#include <map>
#include <tuple>
#include <string>
#include <string_view>
#include <iostream>
#include <sstream>

enum node_type { ENDPOINT, BRIDGE, ROUTER }; 

class network;
class node;

struct node_cmp {
	bool operator()( const node *n1, const node *n2 ) const;
};

class node {
protected:
	network *_segment;
	node_type _type;
	std::set<node*> connected;
public:
	node( network *seg, node_type type ) : _segment( seg ), _type( type ) {}

	node_type get_type() const { return _type; }
	const network *get_segment() const { return _segment; }
	virtual std::string get_id() const { return ""; }
	virtual std::size_t get_capacity() const = 0;
	

	virtual bool can_add( node *n ) const = 0;
	void add( node *n ) 			 { connected.insert( n ); }
	bool can_remove( node *n ) const { return ( n && connected.contains(n) ); }
	void remove( node *n ) 			 { connected.erase( connected.find(n) ); }

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

	virtual const std::set<node*> &get_connected() const { return connected; };

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


bool node_cmp::operator()( const node *n1, const node *n2 ) const {
	if ( n1 == n2 ) return false;
	if ( n1->get_type() != n2->get_type() ) return n1->get_type() < n2->get_type();
	// node types are equal
	if ( n1->get_type() != node_type::ENDPOINT )
		return	n1->get_id().compare( n2->get_id() ) < 0;	// id should be unique for net and type
	// nodes are endpoints
	if ( n1->get_connected().size() != n2->get_connected().size() ) 
		return n1->get_connected().size() < n2->get_connected().size();
	// both endpoints have same number of connections
	if ( !n1->get_connected().empty() ) {
		auto it1 = n1->get_connected().begin();
		auto it2 = n2->get_connected().begin();
		if ( (*it1)->get_type() != (*it2)->get_type() ) return (*it1)->get_type() < (*it2)->get_type();
	}
	return ( std::less<const node*>{}( n1, n2 ) );
}

class endpoint : public node {
public:
	endpoint( network *seg ) : node( seg, node_type::ENDPOINT ) {}

	std::size_t get_capacity() const override { return 1; }
	bool can_add( node *n ) const override { return n && connected.empty(); }
};

class bridge : public node {
	std::size_t capacity;
	std::string _id;
public:
	bridge( network *seg, int p, std::string_view id ) 
	 : node( seg, node_type::BRIDGE ), capacity( p ), _id( id ) {}

	std::string get_id() const override { return _id; }
	std::size_t get_capacity() const override { return capacity; }
	
	bool can_add( node *n ) const override {
		if ( !n ) return false;
		for ( auto i : connected ) { if ( i == n ) return false; }
		return ( connected.size() < capacity );
	}
};

class router : public node {
	std::size_t capacity;
	std::string _id;
public:
	router( network *seg, int i, std::string_view id ) 
	 : node( seg, node_type::ROUTER ), capacity( i ), _id( id ) {}

	std::string get_id() const override { return _id; }
	std::size_t get_capacity() const override { return capacity; }

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
	bridge *add_bridge( int p, std::string_view id ) { 
		auto [it,_] = nodes.insert( std::make_unique<bridge>( this, p, id ) );
		return dynamic_cast<bridge *>( it->get() );
	}
	router *add_router( int i, std::string_view id ) { 
		auto [it,_] = nodes.insert( std::make_unique<router>( this, i, id ) );
		return dynamic_cast<router *>( it->get() );		
	}

	std::vector<node*> get_node_vec( node_type type ) const {
		std::set<node*,node_cmp> node_set;
		for ( auto &n : nodes ) {
			if ( n->get_type() == type ) node_set.insert( n.get() );
		}
		return { node_set.begin(), node_set.end() };
	}
	std::vector<node*> endpoints() const { return get_node_vec( node_type::ENDPOINT ); }
	std::vector<node*> bridges() const 	 { return get_node_vec( node_type::BRIDGE ); }
	std::vector<node*> routers() const 	 { return get_node_vec( node_type::ROUTER ); }

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


/* Navrhněte textový formát pro ukládání informací o sítích tak, jak
 * byly definované v příkladu ‹s2/e_network›, který bude mít tyto
 * vlastnosti:
 *
 *  • do jednoho řetězce musí být možno uložit libovolný počet sítí,
 *    které mohou být vzájemně propojené směrovači,
 *  • výsledek načtení z řetězce nesmí být možné odlišit (použitím
 *    veřejného rozhraní) od hodnot, kterých uložením řetězec
 *    vznikl,
 *  • obsah řetězce musí být plně určen aktuálním stavem vstupních
 *    sítí, bez ohledu na posloupnost operací, kterými vznikly –
 *    zejména tedy nezmí záležet na pořadí přidávání a propojování
 *    (případně rozpojování) uzlů,¹
 *  • jako speciální případ předchozího, načtení a následovné
 *    uložení sítě musí být idempotentní (výsledný řetězec je
 *    identický jako ten, se kterým jsme začali).
 *
 * Rozhraní je dané těmito dvěma čistými funkcemi (zejména se žádná
 * síť nesmí změnit použitím funkce ‹serialize›): */

template< typename C, typename T = typename C::value_type >
int get_index( const C &cont, T elem ) {
	auto it = std::find( cont.begin(), cont.end(), elem );
	if ( it == cont.end() ) return -1;
	return std::distance( cont.begin(), it );
}

std::string get_id_prefix( const node *n ) {
	switch( n->get_type() ) {
		case node_type::ENDPOINT: return "e_";
		case node_type::BRIDGE: return "b_";
		case node_type::ROUTER: return "r_";
		default: assert( false );
	}	
}

int count_endpoints( node *n ) {
	int count = 0;
	for ( auto c : n->get_connected() ) { 
		if ( c->get_type() == node_type::ENDPOINT ) ++count; 
	}
	return count;
}

bool connect_endpoint( node *n, std::vector<node*> &endpts ) {
	for ( auto e : endpts ) { 
		if ( n != e && e->get_connected().empty() ) {
			n->connect( e );
			return true; 
		}
	}
	return false;
}

std::string node_to_string( const node *n, const std::vector<network*> &system ) {
	std::ostringstream oss;
	oss << "node{ type= ";
	switch( n->get_type() ) {
		case node_type::ENDPOINT:
			oss << "endpoint e_count= "; 
			if ( !n->get_connected().empty() && 
				  (* n->get_connected().begin() )->get_type() == node_type::ENDPOINT ) {
				oss << 1 << " }"; 	
			} else {
				oss << 0 << " }";
			}
			return oss.str();
		case node_type::BRIDGE:
			oss << "bridge"; break;
		case node_type::ROUTER:
			oss << "router"; break;
		default:
			assert( false );
	}
	oss << " id= ";
	oss << n->get_id();
	oss << " capacity= " << n->get_capacity();
	oss << " connected=( ";
	int e_count = 0;
	std::set<node*,node_cmp> node_set = { n->get_connected().begin(), n->get_connected().end() };
	for ( auto c : node_set ) {
		if ( c->get_type() == node_type::ENDPOINT ) {
			++e_count; 
			continue;
		}
		if ( n->get_type() == node_type::ROUTER ) {
			oss << get_index( system, c->get_segment() ) << ":";
		}
		oss << get_id_prefix( c ) << c->get_id();
		oss << " ";
	}
	oss << ") e_count= " << e_count << " }";
	return oss.str();
}

using node_connection = std::tuple< node*, std::string >; // node, connected

void read_node( std::string node_str, network &net, std::set<node_connection> &connects ) {
	std::string token, n_type, id;
	int cap, epts;
	std::istringstream iss( node_str );
	iss >> token; 				assert( token == "type=" );
	iss >> n_type;
	node *new_node;

	if ( n_type == "endpoint" ) {
		new_node = net.add_endpoint();
		iss >> token;			assert( token == "e_count=" );
		iss >> epts;
		if ( epts ) connects.insert( { new_node, "e_1" } );
		return;
	}
	iss >> token;				assert( token == "id=" );
	iss >> id >> token;			assert( token == "capacity=" );
	iss >> cap >> token;		assert( token == "connected=(" );
	
	if ( n_type == "bridge" ) {
		new_node = net.add_bridge( cap, id );
	} else if ( n_type == "router" ) {
		new_node = net.add_router( cap, id );
	} else { 
		assert( false );
	}
	iss >> token;
	while ( token != ")" ) {
		connects.insert( {new_node, token} );
		iss >> token;
	}
	iss >> token; 				assert( token == "e_count=" );
	iss >> epts;
	if ( epts )	connects.insert( { new_node, "e_" + std::to_string( epts ) } );
}

std::string net_to_string( const network &net, const std::vector<network*> &system ) {
	std::vector<node*> endpoints = net.endpoints(), bridges = net.bridges(), routers = net.routers();
	std::ostringstream oss;
	oss << "network[ ";
	oss << "endpoints=( ";
	for ( auto n : endpoints ) { oss << node_to_string( n, system ) << " "; }
	oss << ")" << std::endl;
	oss << "bridges=( ";
	for ( auto n : bridges ) { oss << node_to_string( n, system ) << " "; }
	oss << ")" << std::endl;
	oss << "routers=( ";
	for ( auto n : routers ) { oss << node_to_string( n, system ) << " "; }
	oss << ") ]" << std::endl;
	return oss.str();
}

void read_node_list( std::istringstream &iss, network &net, std::set<node_connection> &net_connects ) {
	std::string token, node_str;
	iss >> token;
	while( token != ")" ) {
		assert( token == "node{" );
		getline( iss, node_str, '}' );
		read_node( node_str, net, net_connects );
		iss >> token;
	}	
}

std::set<node_connection> read_net( std::string net_str, std::list<network> &nets ) {
	network &net = nets.emplace_back();
	std::string token, node_str;
	std::istringstream iss( net_str );
	
	std::set<node_connection> net_connects;
	iss >> token; assert( token == "endpoints=(" );
	read_node_list( iss, net, net_connects );
	iss >> token; assert( token == "bridges=(" );
	read_node_list( iss, net, net_connects );
	iss >> token; assert( token == "routers=(" );
	read_node_list( iss, net, net_connects );

	return net_connects;
}

std::string serialize( std::list< network > &nets ) {
	std::ostringstream oss;
	oss << "net_series{" << std::endl;
	std::vector<network*> p_nets;
	for( auto &net : nets ) { p_nets.push_back( &net ); }
	for( auto &net : nets ) {
		oss << net_to_string( net, p_nets );
	}
	oss << "}ser_end" << std::endl;
	return oss.str();
}

using net_id_ptrs = std::map<std::string,node*>;

std::vector<net_id_ptrs> get_structure( const std::list< network > &nets ) {
	std::vector<net_id_ptrs> series;
	for ( auto &n : nets ) {
		net_id_ptrs ids;
		int i = 0;
		for ( auto e : n.endpoints() ) {
			ids[ get_id_prefix(e) + std::to_string(i) ] = e;
			++i;
		}
		for ( auto b : n.bridges() ) {
			ids[ get_id_prefix(b) + b->get_id() ] = b;
		}
		for ( auto r : n.routers() ) {
			ids[ get_id_prefix(r) + r->get_id() ] = r;
		}
		series.emplace_back( std::move( ids ) );
	}
	return series;
}

std::list< network > deserialize( std::string_view view ) {
	std::list<network> nets;
	std::vector<std::set<node_connection>> cons;

	std::string token, net_str;
	std::istringstream iss{ std::string(view) };
	getline( iss, token ); assert( token == "net_series{" );
	iss >> token;
	while( token == "network[" ) {
		getline( iss, net_str, ']' );
		cons.push_back( read_net( net_str, nets ) );
		iss >> token;
	}
	auto structure = get_structure( nets );
	int target_net;
	char sep;
	std::string node_id;
	for( std::size_t i = 0; i < cons.size(); ++i ) {
		for ( auto [ node_p, id ] : cons[i] ) {
			if ( id[0] == 'e' ) {
				int e_count = std::stoi( id.substr(2) );
				auto epts = node_p->get_segment()->endpoints();
				for ( int j = count_endpoints( node_p); j < e_count; ++j ) {
					assert( connect_endpoint( node_p, epts ) );
				}
			} else {
				target_net = i;
				std::istringstream is( id );
				if ( node_p->get_type() == node_type::ROUTER ) {
					is >> target_net >> sep; 	assert( sep == ':' );
				}
				is >> node_id;
				node_p->connect( structure[target_net][node_id] );				
			}
		}
	}
	return nets;
}

/* Aby se Vám serializace snáze implementovala, přidejte metodám
 * ‹add_bridge› a ‹add_router› parametr typu ‹std::string_view›,
 * který reprezentuje neprázdný identifikátor sestavený z číslic a
 * anglických písmen. Identifikátor je unikátní pro danou síť a typ
 * uzlu.
 *
 * Konečně aby bylo možné s načtenou sítí pracovat, přidejte metody
 * ‹endpoints›, ‹bridges› a ‹routers›, které vrátí vždy
 * ‹std::vector› ukazatelů vhodného typu. Konkrétní pořadí uzlů
 * v seznamu není určeno. */

/* ¹ Samozřejmě záleží na pořadí, ve kterém jsou sítě předány
 *   serializační funkci – serializace sítí ‹a, b› se může obecně
 *   lišit od serializace ‹b, a›. */

void test_reachable() {
	std::cout << "Testing reachable:" << std::endl;
    std::list<network> nets;
    auto &net0 = nets.emplace_back();
    auto &net1 = nets.emplace_back();
	auto b_1 = net0.add_bridge( 5, "ufo" );
    auto e_2 = net0.add_endpoint();
	auto r_3 = net0.add_router( 2, "r3" );
    auto e_4 = net1.add_endpoint();
    auto e_5 = net1.add_endpoint();
    assert( b_1->connect( e_2 ) );
    assert( b_1->connect( r_3 ) );
    assert( e_4->connect( e_5 ) );
	std::string res = serialize( nets );
	//std::cout << res;
	auto nets_copy = deserialize( res );
	std::string res2 = serialize( nets_copy );
	//std::cout << res2;
	assert( res == res2 );
	auto epts_2 = nets_copy.back().endpoints();
	assert( epts_2[0]->reachable( epts_2[1] ) );
	assert( epts_2[1]->reachable( epts_2[0] ) );
	assert( epts_2[0]->reachable( epts_2[0] ) );
	assert( epts_2[1]->reachable( epts_2[1] ) );
	std::cout << "PASSED:" << std::endl;  	
}

void test_disconnect_sanity() {
	std::cout << "Testing disconnect (sanity):" << std::endl;
    std::list<network> nets;
    auto &net0 = nets.emplace_back();
    auto &net1 = nets.emplace_back();
	auto b_A1 = net0.add_bridge( 2, "A" );
    auto e_2 = net0.add_endpoint();
    auto e_4 = net0.add_endpoint();
	auto b_A2 = net1.add_bridge( 2, "A" );
	auto r_A = net1.add_router( 2, "A" );
	auto e_7 = net1.add_endpoint();
    assert( b_A1->connect( e_2 ) );
    assert( b_A1->connect( e_4 ) );
    assert( b_A2->connect( r_A ) );
    assert( b_A2->connect( e_7 ) );
    auto str = serialize( nets );
    //std::cout << str;
    assert( !b_A1->connect( e_2 ) );
    assert( b_A1->reachable( e_2 ) );
    assert( b_A1->disconnect( e_2 ) );
    assert( e_2->connect( b_A1 ) );
    assert( b_A1->disconnect( e_4 ) );
    assert( e_4->connect( b_A1 ) );
    
    assert( b_A2->disconnect( r_A ) );
    assert( r_A->connect( b_A2 ) );
    assert( b_A2->disconnect( e_7 ) );
    assert( e_7->connect( b_A2 ) );
    std::cout << "PASSED:" << std::endl; 
}

void test_disconnect() {
	std::cout << "Testing disconnect:" << std::endl;
    std::list<network> nets;
    auto &net1 = nets.emplace_back();
    auto b_A = net1.add_bridge( 2, "A" );
    auto b_B = net1.add_bridge( 2, "B" );
    auto b_C = net1.add_bridge( 2, "C" );
    assert( b_A->connect( b_B ) );
    assert( b_A->connect( b_C ) );
    assert( b_B->connect( b_C ) );
    auto str = serialize( nets );
    //std::cout << str;
		// A-B
	assert( b_A->disconnect( b_B ) );
	assert( b_B->connect( b_A ) );
	auto res_1 = serialize( nets );
	//std::cout << res_1;
	assert( str == res_1 );
		// A-C
	assert( b_A->disconnect( b_C ) );
	assert( b_C->connect( b_A ) );
	auto res_2 = serialize( nets );
	//std::cout << res_2;
	assert( str == res_2 );
		// B-C
	assert( b_B->disconnect( b_C ) );
	assert( b_C->connect( b_B ) );
	auto res_3 = serialize( nets );
	//std::cout << res_3;
	assert( str == res_3 );
	std::cout << "PASSED:" << std::endl;  	
}

void test_cmp() {
	std::cout << "Testing comparator:" << std::endl;
    std::list<network> nets;
    auto &net1 = nets.emplace_back();
    auto e_ref = net1.add_endpoint();
    std::set<node*> epts;
    for( int i = 0; i < 20; ++i ) {
    	auto a = net1.add_endpoint();
    	epts.insert(a);
    }
    assert( epts.size() == 20 );
    node_cmp cmp;
    for ( auto e : epts ) {
    	assert( !( cmp( e_ref, e ) == cmp( e, e_ref ) ) );
    }
  	std::cout << "PASSED:" << std::endl;  	
}

int main()
{
	test_reachable();
	test_disconnect_sanity();
	test_disconnect();
	test_cmp();
	
    std::list< network > sys_1;

    auto &n = sys_1.emplace_back();
    auto &m = sys_1.emplace_back();

    auto e1 = n.add_endpoint(),
         e2 = n.add_endpoint(),
         e3 = m.add_endpoint();
    auto b = n.add_bridge( 2, "ufo" );
    auto r1 = n.add_router( 2, "r1" );
    auto r2 = m.add_router( 2, "r2" );

    assert( n.bridges().size() == 1 );
    assert( n.routers().size() == 1 );
    assert( n.endpoints().size() == 2 );

    assert( b->connect( e1 ) );
    assert( b->connect( r1 ) );
    assert( r1->connect( r2 ) );
    assert( r2->connect( e3 ) );

    assert( e1->reachable( e3 ) );
    assert( !e1->reachable( e2 ) );

    std::string str = serialize( sys_1 );
    std::list< network > sys_2 = deserialize( str );
    assert( sys_2.size() == 2 );
    assert( serialize( sys_2 ) == str );

    const network &nn = sys_2.front(),
                  &mm = sys_2.back();

    auto nn_e = nn.endpoints();
    auto mm_e = mm.endpoints();

    assert( nn_e.size() == 2 );
    assert( mm_e.size() == 1 );

    assert( nn_e.front()->reachable( mm_e.front() ) ||
            nn_e.back()->reachable( mm_e.front() ) );

    return 0; 
}