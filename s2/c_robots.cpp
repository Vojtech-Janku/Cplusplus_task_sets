#include <cassert>
#include <cmath>
#include <tuple>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <memory>

/* V této úloze budete programovat jednoduchou hru, ve které se ve
 * volném třírozměrném prostoru pohybují robotické entity tří barev:
 *
 *  1. červený robot (třída ‹robot_red›):
 *     
 *     ◦ není-li uzamčený a na hrací ploše je alespoň jeden cizí
 *       zelený robot, uzamkne se na ten nejbližší, jinak stojí na
 *       místě,
 *     ◦ je-li na nějaký robot uzamčený, přibližuje se přímo k němu
 *       (tzn. směr pohybu je vždy v aktuálním směru tohoto robotu),
 *     
 *  2. zelený robot (třída ‹robot_green›):
 *     
 *     ◦ je-li nějaký cizí modrý robot ve vzdálenosti do 10 metrů,
 *       směruje přímo k tomu nejbližšímu,
 *     ◦ je-li nejbližší takový robot dále než 10 metrů, zelený
 *       robot se teleportuje do místa, které leží na jejich
 *       spojnici, 8 metrů od cílového robotu na jeho «vzdálenější»
 *       straně,
 *     ◦ jinak stojí na místě.
 *     
 *  3. modrý robot (třída ‹robot_blue›):
 *     
 *     ◦ směruje k nejbližšímu cizímu červenému robotu, existuje-li
 *       nějaký,
 *     ◦ jinak se «poloviční rychlostí» pohybuje po přímce ve směru,
 *       který měl naposledy,
 *     ◦ na začátku hry je otočen směrem k začátku souřadnicového
 *       systému (je-li přímo v počátku, chování není určeno).
 *
 * Roboty se pohybují rychlostí 15 m/s. Dostanou-li se dva roboty
 * různých barev a různých vlastníků do vzdálenosti 1 metr nebo
 * menší, jeden z nich zanikne podle pravidla:
 *
 *  • červený vítězí nad zeleným,
 *  • zelený vítězí nad modrým a konečně
 *  • modrý vítězí nad červeným.
 *
 * Hra jako celek nechť je zapouzdřená ve třídě ‹game›. Bude mít
 * tyto metody:
 *
 *  • metoda ‹tick› posune čas o 1/60 sekundy, a provede tyto akce:
 *    
 *    a. všechny roboty se posunou na své nové pozice «zároveň»,
 *       tzn. dotáže-li se nějaký robot na aktuální pozici jiného
 *       robotu, dostane souřadnice, které měl tento na konci
 *       předchozího tiku,
 *    b. vzájemné ničení robotů, které proběhne opět zároveň –
 *       sejdou-li se v dostatečné blízkosti roboty všech tří barev,
 *       zaniknou všechny.
 *    
 *  • metoda ‹run› simuluje hru až do jejího konce,
 *    
 *    ◦ tzn. do chvíle, kdy nemůže zaniknout žádný další robot a
 *    ◦ vrátí dvojici (počet tiků, hráči),
 *    ◦ kde „hráči“ je vektor identifikátorů hráčů, seřazený podle
 *      počtu zbývajících robotů; je-li více hráčů se stejným počtem
 *      robotů, první bude ten s větším počtem červených, dále
 *      zelených a nakonec modrých robotů; je-li stále hráčů více,
 *      budou uspořádáni podle svého identifikátoru,
 *      
 *  • metody ‹add_X› pro ‹X› = ‹red›, ‹green› nebo ‹blue›, které
 *    přidají do hry robota odpovídající barvy, a které mají dva
 *    parametry:
 *    
 *    ◦ počáteční pozici, jako trojici hodnot typu ‹double› (zadané
 *      v metrech v kartézské soustavě),
 *    ◦ nenulový celočíselný identifikátor hráče-vlastníka. */

void print( std::vector<int> vec ) {
	std::cout << "< ";
	for ( auto v : vec ) {
		std::cout << v << " ";
	}
	std::cout << ">";
}

double DEATH_DIST = 1;
double TICK_SIZE = 1.0/60.0;
double EPSILON = 0.0001;

using position = std::tuple<double,double,double>;

void print( position p ) {
	auto [a,b,c] = p;
	std::cout << "[ " << a << ", " << b << ", " << c << " ]";
}

position operator+( position a, position b ) {
	auto [m,n,o] = a;
	auto [x,y,z] = b;
	return { m+x, n+y, o+z };
}
position operator-( position a, position b ) {
	auto [m,n,o] = a;
	auto [x,y,z] = b;
	return { m-x, n-y, o-z };
}
position operator*( position p, double scale ) {
	auto [x,y,z] = p;
	return { x*scale, y*scale, z*scale };
}

double euclid_dist( position a, position b ) {
	auto [m,n,o] = a;
	auto [x,y,z] = b;
	return std::sqrt( std::pow(x-m, 2) + std::pow(y-n, 2) + std::pow(z-o, 2) );
}

position unit_direction( position start, position target ) {
	auto [m,n,o] = start;
	auto [x,y,z] = target;
	double dist = euclid_dist(start, target);
	if ( std::fabs( dist ) < EPSILON ) return { 0, 0, 0 };
	return { (x-m)/dist, (y-n)/dist, (z-o)/dist };
}

enum Robot_type { RED, GREEN, BLUE };
void print( Robot_type t ) {
	switch( t ) {
		case Robot_type::RED:	std::cout << "RED"; break;
		case Robot_type::GREEN:	std::cout << "GREEN"; break;
		case Robot_type::BLUE:	std::cout << "BLUE"; break;
		default: assert(0);
	}
}

Robot_type get_enemy_type( Robot_type rt ) {
	switch( rt ) {
		case Robot_type::RED: 	return Robot_type::GREEN;
		case Robot_type::GREEN: return Robot_type::BLUE;
		case Robot_type::BLUE: 	return Robot_type::RED;
		default: assert(0);
	}
}

struct robot {
	Robot_type type;
	int player_id;
	position pos, next_pos;
	robot *target = nullptr;
	double base_speed = 15;

	robot( int player, position start ) 
		: player_id( player ), pos( start ), next_pos( start ) {}
	
	virtual void get_next_pos() = 0;

	void move() { 
		pos = next_pos; 
	}

	bool can_attack( const robot *r ) const {
		return r->type == get_enemy_type( type ) && player_id != r->player_id;
	}

	virtual void find_target( std::set<std::unique_ptr<robot>> &robots ) {
		target = nullptr;
		for ( auto &r : robots ) {
			if ( can_attack( r.get() ) && ( !target || euclid_dist( pos, r->pos ) < euclid_dist( pos, target->pos ) ) ) {
				target = r.get();
			}
		}
	}

	virtual ~robot() = default;
};

void print( const robot *r ) {
	std::cout << "Robot: { " << r->player_id << ", ";
	print( r->type );
	std::cout << ", ";
	print( r->pos );
	if ( r->target ) {
		std::cout << ", target: ";
		print( r->target->pos );
		std::cout << " - distance: " << euclid_dist( r->pos, r->target->pos );
	}
	std::cout << " }" << std::endl;
}

struct robot_red : robot {
	robot_red( int player, position start ) : robot( player, start ) {
		type = Robot_type::RED;
	}

	void find_target( std::set<std::unique_ptr<robot>> &robots ) override {
		if ( target ) return;
		for ( auto &r : robots ) {
			if ( can_attack( r.get() ) && ( !target || euclid_dist( pos, r->pos ) < euclid_dist( pos, target->pos ) ) ) {
				target = r.get();
			}
		}
	}
	
	void get_next_pos() override {
		if ( !target ) return;
		next_pos = pos + unit_direction( pos, target->pos )*base_speed*TICK_SIZE;
	}
};
struct robot_green : robot {
	robot_green( int player, position start ) : robot( player, start )  {
		type = Robot_type::GREEN;
	}
	
	void get_next_pos() override {
		if ( !target ) return;
		if ( euclid_dist( pos, target->pos ) > 10 ) {
			next_pos = target->pos + unit_direction( pos, target->pos )*8;
		} else {
			next_pos = pos + unit_direction( pos, target->pos )*base_speed*TICK_SIZE;
		}
	}
};
struct robot_blue : robot {
	position last_direction = unit_direction( pos, {0,0,0} );

	robot_blue( int player, position start ) : robot( player, start ) {
		type = Robot_type::BLUE;
	}
	
	void get_next_pos() override {
		if ( !target ) {
			next_pos = pos + last_direction*(base_speed/2.0)*TICK_SIZE;
		} else {
			last_direction = unit_direction( pos, target->pos );
			next_pos = pos + last_direction*base_speed*TICK_SIZE;
		}
	}	
};

struct game {
	std::map<int, std::tuple<int, int, int>> players;
	std::set<std::unique_ptr<robot>> robots;

	bool game_end() {
		for ( auto &r : robots ) {
			for ( auto &o : robots ) {
				if ( r->can_attack( o.get() ) ) return false;
			}
		}
		return true;
	}

	std::vector<int> get_sorted_players() {
		std::set< std::tuple<int,int,int,int,int> > sorter;
		for ( auto p : players ) {
			auto [id,owns] = p;
			auto [r,g,b] = owns;
			sorter.insert( { r+g+b, r, g, b, -id } );
		}
		std::vector<int> res;
		for ( auto it = sorter.rbegin(); it != sorter.rend(); ) {
			auto [a_,b_,c_,d_,neg_id] = *it;
			res.push_back(-neg_id);
			++it;
		}
		return res;
	}

	int &get_robot_count( int player_id, Robot_type type ) {
		if ( type == Robot_type::RED ) 	 return std::get<0>( players[player_id] );
		if ( type == Robot_type::GREEN ) return std::get<1>( players[player_id] );
		return std::get<2>( players[player_id] );
	}

	void destroy_robots() {
		std::set<robot*> to_destroy;
		for ( auto &r : robots ) {
			for ( auto &o : robots ) {
				if ( r->can_attack( o.get() ) && euclid_dist( r->pos, o->pos ) <= DEATH_DIST ) {
					to_destroy.insert( o.get() );
				}
			}
		}
		for ( auto &r : robots ) {
			if ( to_destroy.count( r->target ) ) r->target = nullptr;
		}
		for ( auto it = robots.begin(); it != robots.end(); ) {
			if ( to_destroy.count( it->get() ) ) {
				--get_robot_count( (*it)->player_id, (*it)->type );
				it = robots.erase( it );
			} else {
				++it;
			}
		}
	}

	void tick() {
		for ( auto &r : robots ) {
			r->find_target( robots );
			r->get_next_pos();
		}
		for ( auto &r : robots ) { r->move(); }
		destroy_robots();
	}

	std::tuple< int, std::vector<int> > run() {
		if ( game_end() ) return { 0, get_sorted_players() };		
		int ticks = 0;
		while( !game_end() ) {
			tick();
			++ticks;
		}
		return { ticks, get_sorted_players() };
	}

	void add_red( 	position start, int player_id ) {
		robots.insert( std::make_unique<robot_red>( player_id, start ) );
		++std::get<0>( players[player_id] );
	}
	void add_green( position start, int player_id ) {
		robots.insert( std::make_unique<robot_green>( player_id, start ) );
		++std::get<1>( players[player_id] );
	}
	void add_blue( 	position start, int player_id ) {
		robots.insert( std::make_unique<robot_blue>( player_id, start ) );
		++std::get<2>( players[player_id] );
	}
};

void test_small() {
	std::cout << "TEST: SMALL" << std::endl;
	game bots{};
	bots.add_red( {15,0,0}, 1 );
	bots.add_red( {-15,0,0}, 2 );
	bots.add_green( {15,0,0}, 1 );
	bots.add_green( {-15,0,0}, 2 );
	bots.add_blue( {15,0,0}, 1 );
	bots.add_blue( {-15,0,0}, 2 );
	auto [ticks, players] = bots.run();
	std::cout << "[ " << ticks << ", ";
	print(players);
	std::cout << " ]" << std::endl;
    assert(( players == std::vector{ 1, 2 } ));
    assert( ticks == 130 );
}

void test_small_3pl() {
	std::cout << "TEST: SMALL 3 PLAYERS" << std::endl;
	game bots{};
	bots.add_red( {15,0,0}, 3 );
	bots.add_red( {-15,0,0}, 2 );
	bots.add_green( {15,0,0}, 3 );
	bots.add_green( {-15,0,0}, 2 );
	bots.add_blue( {15,0,0}, 1 );
	bots.add_blue( {-15,0,0}, 1 );
	auto [ticks, players] = bots.run();
	std::cout << "[ " << ticks << ", ";
	print(players);
	std::cout << " ]" << std::endl;
    assert(( players == std::vector{ 2, 3, 1 } ));
    assert( ticks == 1 );
}

void test_small_multidirect() {
	std::cout << "TEST: SMALL MULTIDIRECTIONAL" << std::endl;
	game bots{};
	bots.add_red( 	{10, 0, 0},	1 );
	bots.add_red( 	{-10,0, 0}, 2 );
	bots.add_green( {0,  10,0}, 1 );
	bots.add_green( {0, -10,0}, 2 );
	bots.add_blue( 	{0,  0, -10},  1 );
	bots.add_blue( 	{0,  0, 10}, 2 );
	auto [ticks, players] = bots.run();
	std::cout << "[ " << ticks << ", ";
	print(players);
	std::cout << " ]" << std::endl;
    assert(( players == std::vector{ 1, 2 } ));
    assert( ticks == 81 );
}

void test_large() {
	std::cout << "TEST: LARGE" << std::endl;
	game bots{};
	bots.add_red( {150,0,0}, 1 );
	bots.add_red( {-150,0,0}, 2 );
	bots.add_green( {150,0,0}, 1 );
	bots.add_green( {-150,0,0}, 2 );
	bots.add_blue( {150,0,0}, 1 );
	bots.add_blue( {-150,0,0}, 2 );
	auto [ticks, players] = bots.run();
	std::cout << "[ " << ticks << ", ";
	print(players);
	std::cout << " ]" << std::endl;
    assert(( players == std::vector{ 1, 2 } ));
    assert( ticks == 1210 );
}

void test_3ticks() {
	std::cout << "TEST: 3 ticks" << std::endl;
	game bots{};
	bots.add_red( 	{1, -1, 1},	0 );
	bots.add_green( {1, -1, -1}, -1 );
	bots.add_blue( 	{1, -1, 0}, 0 );
	auto [ticks, players] = bots.run();
	std::cout << "[ " << ticks << ", ";
	print(players);
	std::cout << " ]" << std::endl;
    assert(( players == std::vector{ 0, -1 } ));
    assert( ticks == 3 );
}

void test_verity_large() {
	std::cout << "TEST: VERITY LARGE" << std::endl;
	game bots{};
	bots.add_red( 	{1, -1, 1}, -1 );
	bots.add_green( {-119,-1,-1}, 0 );
	bots.add_green( {1,-1,-1}, 0 );
	bots.add_blue( {1,-1,0}, -1 );
	auto [ticks, players] = bots.run();
	std::cout << "[ " << ticks << ", ";
	print(players);
	std::cout << " ]" << std::endl;
    assert(( players == std::vector{ -1, 0 } ));
    assert( ticks == 32 );
}

void test_verity_small() {
	std::cout << "TEST: VERITY SMALL" << std::endl;
	game bots{};
	bots.add_red( 	{1, -1, 1}, -1 );
	bots.add_green( {-14,-1,-1}, 0 );
	bots.add_green( {1,-1,-1}, 0 );
	bots.add_blue( {1,-1,0}, -1 );
	auto [ticks, players] = bots.run();
	std::cout << "[ " << ticks << ", ";
	print(players);
	std::cout << " ]" << std::endl;
    assert(( players == std::vector{ -1, 0 } ));
    assert( ticks == 31 );
}

void test_small_114() {
	std::cout << "TEST: SMALL 114" << std::endl;
	game bots{};
	bots.add_red( {-14,-1,1}, -1 );
	bots.add_green( {-14,-1,-1}, 0 );
	bots.add_green( {1,-1,-1}, 0 );
	bots.add_green( {1,-1,-1}, -1 );
	bots.add_blue( {1,-1,0}, -1 );
	bots.add_blue( {1,-1,0}, -1 );
	auto [ticks, players] = bots.run();
	std::cout << "[ " << ticks << ", ";
	print(players);
	std::cout << " ]" << std::endl;
    assert(( players == std::vector{ -1, 0 } ));
    assert( ticks == 114 );
}

int main()
{
    /*
    game g;

    auto [ ticks, players ] = g.run();
    assert( ticks == 0 );
    assert( players.empty() );

    g.add_red( { -5, 0, 0 }, 1 );
    g.add_red( {  5, 0, 0 }, 1 );
    g.add_red( {  0, 0, 0 }, 2 );

    std::tie( ticks, players ) = g.run();
    assert( ticks == 0 );
    //print( players );
    assert(( players == std::vector{ 1, 2 } ));

    g.add_green( {  9.6, 0, 0 }, 2 );
    g.add_green( { -9.6, 0, 0 }, 2 );

    std::tie( ticks, players ) = g.run();
    //print( players );
    assert(( players == std::vector{ 1, 2 } ));
    assert( ticks == 15 );	
    */

	test_small_114();
	
	test_small_3pl();
	test_small();
	test_large();
	test_small_multidirect();
	test_3ticks();
	test_verity_large();
	test_verity_small();
	
    return 0;
}
