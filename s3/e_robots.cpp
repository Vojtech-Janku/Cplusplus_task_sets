#include <cassert>
#include <cmath>
#include <tuple>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <memory>

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

// ===================== PARSING FUNCTION ============================

struct bad_grammar {}; // TODO: exception

using program_parts = std::tuple<std::string_view,std::string_view,std::string_view>;
program_parts program_split( std::string_view program ) {
	std::size_t with_idx = program.find( "with" );
	std::size_t init_idx = program.find( "init", with_idx );
	std::size_t rep_idx  = program.find( "repeat", init_idx );
	return { program.substr( with_idx+4, init_idx ),
			 program.substr( init_idx+4, rep_idx  ),
			 program.substr( rep_idx+6,  program.size() ) };
}

/*std::string_view next_token( std::string_view program, std::size_t &idx ) {
	while( idx < program.size() && ( program[idx] == ' ' ) ) ++idx;
	return program.substr( idx, program.find( ' ', idx ) );
} */

struct variables {
	std::map<std::string,robot*> vars_rob;
	std::map<std::string,position> vars_cor;
	std::map<std::string,double> vars_num;

	void add_var( std::string key, robot *val ) { vars_rob[key] = val; }
	void add_var( std::string key, position val ) { vars_cor[key] = val; }
	void add_var( std::string key, double val ) { vars_num[key] = val; }
};

position read_coordinates( std::istringstream &line_stream ) {
	double p1, p2, p3;
	line_stream >> p1 >> p2 >> p3; // TODO: add assert				
	return { p1, p2, p3 };
}

// ===================== GAME ============================

struct game {
	std::map<int, std::tuple<int, int, int>> players;
	std::set<std::unique_ptr<robot>> robots;

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
			auto [a_,b_,c_,d_,id] = *it;
			res.push_back(-id);
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
					//r->target = nullptr;
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
		//	std::cout << "STARTING STATE " << std::endl;
		//	for ( auto &r : robots ) { print( r.get() ); }
		while( !game_end() ) {
			tick();
			++ticks;
			//		std::cout << "TICK " << ticks << std::endl;
			//		for ( auto &r : robots ) { print( r.get() ); }
		}
		return { ticks, get_sorted_players() };
	}

	variables game_setup( std::string_view with ) {
		variables vars;
		std::string line, token, var_name, var;
		std::istringstream iss( with );
		while ( getline( iss, line ) ) {
			std::istringstream line_stream( line );
			line >> var_name >> token >> var; assert( token == "=" );
			if ( var == "@" ) {
				auto pos = read_coordinates( line_stream );
				vars.add_var( var_name, pos );				
			} else if ( var == "red" || var == "green" || var == "blue" ) {
				auto r_type = get_type( var );
				int id;
				line_stream >> id >> token; assert( token == "@" );
				auto pos = read_coordinates( line_stream );
				//	vars.add_var( var_name, pos );	// TODO: create robot
			} else if ( var[0] == '-' || std::isdigit(var[0]) || var[0] == '.' ) {
				double num = std::stoi( var ); 		// TODO: fix this - for double
				vars.add_var( var_name, num );
			} else {
				assert( false );
			}
			assert( !line_stream );
		}
		return vars;		
	}
	void game_init( std::string_view init, variables &vars ) {
		std::string line, token, cmd;
		std::istringstream iss( init );
		while ( getline( iss, line ) ) {
			std::istringstream line_stream( line );
			line_stream >> cmd;
			if ( cmd == "let" ) {
				std::string left, action, right;
				line_stream >> left >> action >> right;
				
			} else if ( cmd == "set" ) {
				std::string left, right;
				line_stream >> left >> token >> right; assert( token == ":=" );
								
			} else if ( cmd == "do" ) {

			} else if ( cmd == "if" ) {

			}			
		}		
	}
	void game_tick( std::string_view repeat, variables &vars ) {
		
	}

	std::tuple< int, std::vector<int> > run( std::string_view program ) {
		auto [ with, init, repeat ] = program_split( program );
		game_setup( with );
		game_init( init );
		int ticks = 0;
		while( !game_end() ) {
			game_tick( repeat );
		}
		return { ticks, get_sorted_players() };
	}
};

/* Uvažme hru ‹s2/c_robots› – Vaším úkolem bude nyní naprogramovat
 * jednoduchý interpret, který bude hru řídit. Vstupní program
 * sestává ze tří částí:
 *
 *  1. deklarace, které popisují jak roboty ve hře a jejich
 *     startovní pozice, tak případné pomocné proměnné,
 *  2. příkazy, které se provedou jednou na začátku hry,
 *  3. příkazy, které se provedou každý tik, dokud hra neskončí.
 *
 * Program by mohl vypadat například takto: */

std::string_view example_1 = R"(with
  a  = red   1 @ -5.0 0 0
  b  = red   1 @  5.0 0 0
  c  = red   2 @  0.0 0 0
  g1 = green 2 @ -9.6 0 0
  g2 = green 2 @  9.6 0 0
init
  let g1 chase a
  let g2 chase b
repeat
)";

std::string_view example_2 = R"(with
  r = red   2 @  0.0 0 0
  g = green 2 @  0.0 0 0
  b = blue  1 @ -9.6 0 0
  tick = 0
init
  let r chase g
  let g go_to @ 1.0 0 0
repeat
  if tick > 9
    if g is_alive
       let b chase g
  set tick := tick + 1
)";

/* Následuje gramatika ve formátu EBNF, která popisuje syntakticky
 * platné programy; terminály gramatiky jsou «tokeny», které jsou od
 * sebe vždy odděleny alespoň jednou mezerou, nebo předepsaným
 * koncem řádku.
 *
 *     prog = 'with', { decl },
 *            'init', { stmt },
 *            'repeat', { stmt } ;
 *     decl = ident, '=', init, '\n' ;
 *     init = color, num, coord | coord | num ;
 *
 *     color = 'red' | 'green' | 'blue' ;
 *     coord = '@', num, num, num ;
 *
 *     stmt = cmd, '\n' |
 *            'if', cond, stmt ;
 *     cmd  = 'let', ident, 'chase', ident |
 *            'let', ident, 'go_to', expr |
 *            'set', ident, ':=', expr |
 *            'do', stmt, { stmt }, 'end' ;
 *     cond = atom, '=', atom |
 *            atom, '<', atom |
 *            atom, '>', atom |
 *            ident, 'is_alive' ;
 *     expr = atom |
 *            atom, '+', atom |
 *            atom, '-', atom |
 *            atom, '*', atom |
 *            '[', expr, ']' |
 *            '(', expr, ')' ;
 *     atom = ident | coord | num;
 *
 * Krom terminálních řetězců (‹'red'› a pod.) považujeme za tokeny
 * také symboly ‹num› a ‹ident›, zadané těmito pravidly:
 *
 *     num   = [ '-' ], digit, { digit }, [ '.', { digit } ] ;
 *     ident = letter, { letter | digit }
 *     digit = '0' | '1' | … | '9' ;
 *     letter = 'a' | 'b' | … | 'z' | '_' ;
 *
 * V programu se objevují hodnoty tří typů:
 *
 *  1. čísla (hodnoty typu ‹double›),
 *  2. trojice čísel (reprezentuje pozici nebo směr),
 *  3. odkaz na robota.
 *
 * S hodnotami (a proměnnými, které hodnoty daných typů aktuálně
 * obsahují), lze provádět tyto operace:
 *
 *  1. čísla lze sčítat, odečítat, násobit a srovnávat (neterminály
 *     ‹expr› a ‹cond›),
 *  2. trojice lze sčítat, odečítat a srovnat (ale pouze rovností),
 *  3. roboty lze posílat za jiným robotem nebo na zadané
 *     souřadnice (příkaz ‹let›),
 *  4. operace hranaté závorky hodnotu zjednodušuje:
 *     
 *     ◦ ‹[ robot ]› je aktuální pozice robota (trojice),
 *     ◦ ‹[ point ]› je Euklidovská vzdálenost bodu od počátku, resp.
 *        velikost směrového vektoru (‹[ p₁ - p₂ ]› tak spočítá
 *        vzdálenost bodů ‹p₁› a ‹p₂›.
 *
 * Operace, které nejsou výše popsané (např. pokus o sečtení
 * robotů), nemají určené chování. Totéž platí pro pokus o použití
 * nedeklarované proměnné (včetně přiřazení do ní). Podobně není
 * určeno chování, nevyhovuje-li vstupní program zadané gramatice.
 *
 * Robot, kterému bylo uloženo pronásledovat (chase) jiného robota,
 * bude na tohoto robota zamčen, až než mu bude uložen jiný cíl,
 * nebo cílový robot zanikne. Nemá-li robot žádný jiný příkaz, stojí
 * na místě (bez ohledu na barvu).
 *
 * Program je předán metodě ‹run› třídy ‹game› jako hodnota typu
 * ‹std::string_view›, návratová hodnota i zde nezmíněná pravidla
 * zůstavají v platnosti z příkladu v druhé sadě. */

int main()
{
    game g, h;

    auto [ ticks, players ] = g.run( example_1 );
    TRACE( ticks, players );
    assert( ticks == 15 );
    assert(( players == std::vector{ 1, 2 } ));

    std::tie( ticks, players ) = h.run( example_2 );
    assert( ticks == 49 );
    assert(( players == std::vector{ 2, 1 } ));

    return 0;
}
