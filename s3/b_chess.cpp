#include<vector>
#include<map>
#include<set>
#include<tuple>
#include<optional>
#include<iostream>
#include<cassert>


template< typename T >
int sgn( T val ) {
	return (static_cast<T>(0) < val) - (val < static_cast<T>(0));
}

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

void print( position pos ) {
	std::cout << "< file: " << pos.file << ", rank:" << pos.rank << " >";
}

enum class piece_type
{
    pawn, rook, knight, bishop, queen, king
};

void print( piece_type piece ) {
	switch( piece ) {
		case piece_type::pawn:		std::cout << "P"; break;
		case piece_type::rook:		std::cout << "R"; break;
		case piece_type::knight:	std::cout << "H"; break;
		case piece_type::bishop:	std::cout << "B"; break;
		case piece_type::queen:		std::cout << "Q"; break;
		case piece_type::king:		std::cout << "K"; break;
		default: assert( false );
	}
}

enum class player { white, black };

player get_enemy( player pl ) {
	return ( pl == player::white ) ? player::black : player::white;
}

int direction( player pl ) {
	return ( pl == player::white ) ? 1 : -1;
}

int first_rank( player pl ) {
	return ( pl == player::white ) ? 1 : 8;
}

void print( player pl ) {
	std::cout << ( ( pl == player::white ) ? "w" : "b");
}

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

void print( result res ) {
	switch( res ) {
		case result::capture:		std::cout << "capture"; break;
		case result::ok:			std::cout << "ok"; break;
		case result::no_piece:		std::cout << "no_piece"; break;
		case result::bad_piece:		std::cout << "bad_piece"; break;
		case result::bad_move:		std::cout << "bad_move"; break;
		case result::blocked:		std::cout << "blocked"; break;
		case result::lapsed:		std::cout << "lapsed"; break;
		case result::in_check:		std::cout << "in_check"; break;
		case result::would_check:	std::cout << "would_check"; break;
		case result::has_moved:		std::cout << "has_moved"; break;
		case result::bad_promote:	std::cout << "bad_promote"; break;
		default: assert( false );
	}
}

struct piece
{
    player owner;
    piece_type type;
};

bool operator==( const piece &p1, const piece &p2 ) {
	return p1.owner == p2.owner && p1.type == p2.type;
}

using occupant = std::optional< piece >;

void print( occupant o ) {
	std::cout << "<";
    if ( o ) {
    	print( o->owner );
    	print( o->type );
    } else {
    	std::cout << "  ";
   	}
   	std::cout << ">";
}

class chess
{
	using chessboard = std::map<position, piece>;
	using enpass_pair = std::tuple<position,position>;
	player on_turn = player::white;
	chessboard board;
	position white_king_pos = { 5, 1 }, black_king_pos = { 5, 8 };
	bool was_in_check;
	bool white_moved[3] = {false,false,false}, black_moved[3] = {false,false,false};
	std::optional<position> enpass_pos = {};
	std::set<enpass_pair> 	   enpass_lapsed;
	std::set<position> castling_positions = { {1,1}, {5,1}, {8,1}, {1,8}, {5,8}, {8,8} };
	
	void init_front_row( chessboard &cb, player owner ) {
		int rank = ( owner == player::white ) ? 2 : 7;
		for ( int file = 1; file < 9; ++file ) 
			cb[ {file,rank} ] = { owner, piece_type::pawn };
	}
	void init_back_row( chessboard &cb, player owner ) {
		int rank = ( owner == player::white ) ? 1 : 8;
		cb[ {1,rank} ] = { owner, piece_type::rook };
		cb[ {2,rank} ] = { owner, piece_type::knight };
		cb[ {3,rank} ] = { owner, piece_type::bishop };
		cb[ {4,rank} ] = { owner, piece_type::queen };
		cb[ {5,rank} ] = { owner, piece_type::king };
		cb[ {6,rank} ] = { owner, piece_type::bishop };
		cb[ {7,rank} ] = { owner, piece_type::knight };
		cb[ {8,rank} ] = { owner, piece_type::rook };
	}
	
public:

    /* Sestrojí hru ve výchozí startovní pozici. První volání metody
     * ‹play› po sestrojení hry indikuje tah bílého hráče. */

    chess() {
		init_front_row( board, player::white );
		init_front_row( board, player::black );
		init_back_row(  board, player::white );
		init_back_row(  board, player::black );
  	}

  	chess( const chessboard &c ) : board(c) {}

  	void print_lapsed() {
  		std::cout << "Enpass_lapsed: { ";
  		for ( auto [from, to] : enpass_lapsed ) {
  			print( from );
  			print( to );
  		}
		std::cout << " }" << std::endl;
  	}

    /* Metoda ‹at› vrátí stav zadaného pole. */

    occupant at( position p ) const {
    	return ( board.contains( p ) ) ? occupant(board.at(p)) : std::nullopt;
    }

	void set_square( position p, occupant o ) {
		if ( o ) {
			board[p] = o.value();
		} else {
			board.erase( p );
		}
	}

	position get_king_pos( player owner ) {
		return ( owner == player::white ) ? white_king_pos : black_king_pos;
	}

	void 	 set_king_pos( player owner, position pos ) {
		if ( owner == player::white ) {
			white_king_pos = pos;
		} else {
			black_king_pos = pos;
		}
	}

	bool has_moved( player owner, int file ) {
		int idx = sgn( file - 5 ) + 1;
		return ( owner == player::white ) ? white_moved[idx] : black_moved[idx];
	}

	/*void mark_moved( player owner, int file ) {
		int idx = sgn( file - 5 ) + 1;
		if ( owner == player::white )  {
			white_moved[idx] = true;
		} else {
			black_moved[idx] = true;
		}
	}*/

	void mark_moved( position pos ) {
		if ( !castling_positions.contains( pos ) ) return;
		int idx = sgn( pos.file - 5 ) + 1;
		if ( pos.rank == 1 )  {
			white_moved[idx] = true;
		} else {
			black_moved[idx] = true;
		}
	}

	bool valid_move( position from, position to ) {
		if ( to.file < 1 || to.file > 8 || to.rank < 1 || to.rank > 8 || from == to ) return false;
		occupant attacker = at(from);
		auto file_diff = std::abs( from.file - to.file ), 
			 rank_diff = std::abs( from.rank - to.rank );
		switch( attacker->type ) {
			case piece_type::pawn:
				return ( ( file_diff < 2 ) && ( to.rank == from.rank + direction( attacker->owner ) ) ) ||
					   ( ( file_diff == 0 ) && 
					   	 ( ( attacker->owner == player::white && from.rank == 2 && to.rank == 4 ) ||
					   	   ( attacker->owner == player::black && from.rank == 7 && to.rank == 5 ) ) );
			case piece_type::rook:
				return ( file_diff == 0 || rank_diff == 0 );
			case piece_type::knight:
				return ( ( file_diff == 1 && rank_diff == 2 ) || ( file_diff == 2 && rank_diff == 1 ) );
			case piece_type::bishop:
				return ( file_diff == rank_diff );
			case piece_type::queen:
				return ( file_diff == rank_diff || file_diff == 0 || rank_diff == 0 );
			case piece_type::king:
				if ( file_diff < 2 && rank_diff < 2 ) return true;
				return ( from.rank == first_rank( attacker->owner ) &&
						 from.file == 5 && file_diff == 2 && rank_diff == 0  );
			default: assert( false );
		}
		return true;
	}

	bool blocked( position from, position to ) {
		if ( at(from)->type == piece_type::knight ) {
			return ( at(to) && at(to)->owner == at(from)->owner );
		}
		if ( at(from)->type == piece_type::pawn && from.file == to.file && at(to) ) return true;
		if ( at(to) && at(to)->owner == at(from)->owner ) return true;
		auto [f,r] = from;
		for ( int i = 0; i < std::max( std::abs(to.file-from.file), std::abs(to.rank-from.rank) )-1; ++i ) {
			f += sgn( to.file - from.file );
			r += sgn( to.rank - from.rank );
			if ( at( {f,r} ) ) return true;
		}
		return false;
	}

	bool in_check( player owner ) {
		auto king_pos = get_king_pos( owner );
		for ( auto [pos, piece] : board ) {
			if ( piece.owner == get_enemy( owner ) && 
				 valid_move( pos, king_pos ) && 
				 !blocked( pos, king_pos ) ) return true;
		}
		return false;
	}

	void move( position from, position to ) {
		if ( at(from) && at(from)->type == piece_type::king ) {
			set_king_pos( at(from)->owner, to );
		}
  		set_square( to, at(from) );
  		set_square( from, {} );		
	}
	
  	result try_move( position from, position to ) {
  		if ( !valid_move( from, to ) ) return result::bad_move;				// bad_move
  		if ( blocked( from, to ) ) return result::blocked;					// blocked
  		auto attacker = at(from), target = at(to);
  		if ( attacker->type == piece_type::king && std::abs( to.file - from.file ) == 2 ) {
  			return result::ok; // handle castling outside					// ok -> castling
  		}
  		move( from, to );
  		return ( target ) ? result::capture : result::ok;					// capture, ok
  	}

  	result play_pawn( position from, position to, piece_type promote ) {
  		auto target = at(to);
  		if ( ( std::abs( to.file - from.file ) == 1 && !target ) && 
  			( !at( { to.file, from.rank } ) || 
  			  !enpass_lapsed.contains( {from, { to.file, from.rank }} ) ) ) {
  			return result::bad_move;						// en-passant bad_move
  		}
  		result res = try_move( from, to );					// bad_move, blocked, capture, ok
  		if ( res != result::ok && res != result::capture ) return res;		// all fail results
  			// en passant
  		if ( std::abs( to.file - from.file ) == 1 && !target ) {
  			if ( enpass_pos && enpass_pos->file == to.file && enpass_pos->rank == from.rank ) {
  				auto some_guy = at( enpass_pos.value() );
  				set_square( enpass_pos.value(), {} );
			  	if ( in_check( on_turn) ) {
			  		move( to, from );
			  		set_square( to, target );
			  		set_square( enpass_pos.value(), some_guy );
					return ( was_in_check ) ? result::in_check : result::would_check;  			// in_check, would_check
			  	} 
  				enpass_lapsed.erase( { from, enpass_pos.value() } );
  				enpass_pair other = { { from.file + 2*sgn( to.file - from.file ), enpass_pos->rank  }, enpass_pos.value() };
  				enpass_lapsed.erase( other );
  				return result::capture;								// capture
  			} else {
  				move( to, from );
  				set_square( to, target );
  				return result::lapsed;								// lapsed
  			}
  		}
	  	if ( in_check( on_turn) ) {
	  		move( to, from );
	  		set_square( to, target );
			return ( was_in_check ) ? result::in_check : result::would_check;  			// in_check, would_check
	  	}
  		if ( to.rank == first_rank( get_enemy( on_turn ) ) ) {
  			if ( promote == piece_type::pawn || promote == piece_type::king ) {
  				move( to, from );
  				set_square( to, target );
  				return result::bad_promote;					// bad_promote
  			} 
  			set_square( to, occupant( { on_turn, promote } ) );
  			//return res;
  		}
  		return res;
  	}

  	/*result play_rook( position from, position to ) {
  		result res = try_move( from, to );
  		if ( res != result::ok && res != result::capture ) return res;
  		if ( from.rank == first_rank( on_turn ) && 
  			 ( from.file == 1 || from.file == 8 ) ) mark_moved( on_turn, from.file );
  		return res;
  	}*/
  	
  	result play_king( position from, position to ) {
  		result res = try_move( from, to );
  		if ( res != result::ok || std::abs( to.file - from.file ) != 2 ) return res; 		// everything except ok while move by 2 ( == castling )
  		int rook_file = ( to.file - from.file > 0 ) ? 8 : 1;
  		position rook_pos = { rook_file, from.rank };
  		if ( !at(rook_pos) || at(rook_pos)->owner != on_turn || 
  							  at(rook_pos)->type != piece_type::rook ) return result::bad_move;		// bad_move
  		position mid_pos = { from.file + sgn( to.file - from.file ), from.rank };
  		if ( blocked( rook_pos, mid_pos ) ) return result::blocked;											// blocked
  		if ( has_moved( on_turn, rook_file ) || has_moved( on_turn, from.file ) ) return result::has_moved;		// has_moved

		if ( was_in_check ) return result::in_check;											// in_check
  		// check would_check
		move( from, mid_pos );
	  	if ( in_check( on_turn ) ) {
			move( mid_pos, from );  			
	  		return result::would_check;															// would_check
	  	}
	  	move( mid_pos, to );
	  	if ( in_check( on_turn ) ) {
			move( to, from );		
	  		return result::would_check;															// would_check
	  	}
 		move( rook_pos, mid_pos );
 		//mark_moved( on_turn, rook_file );
 		//mark_moved( on_turn, from.file );
 		mark_moved( rook_pos );
 		mark_moved( from );
		return result::ok;																		// ok
  	}
  	
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
                 piece_type promote = piece_type::pawn ) 
    {
    	occupant attacker = at(from), target = at(to);
    	if ( !attacker ) return result::no_piece;						// no_piece
    	if ( attacker->owner != on_turn ) return result::bad_piece;		// bad_piece
    	was_in_check = in_check( on_turn );
    	result res;
    	switch( attacker->type ) {
    		case piece_type::pawn:		res = play_pawn( from, to, promote ); break;
    		case piece_type::rook:		//res = play_rook( from, to ); break;
    		case piece_type::knight:
    		case piece_type::bishop:
    		case piece_type::queen:		res = try_move( from, to ); break;
    		case piece_type::king:		res = play_king( from, to ); break;
    		default: assert( false );
    	}
		if ( res != result::ok && res != result::capture ) return res;	// all fail results
		if ( in_check( on_turn) ) {
			move( to, from );
			set_square( to, target );
			return ( was_in_check ) ? result::in_check : result::would_check;		// in_check, would_check
		}
		if ( attacker->type == piece_type::pawn && std::abs( to.rank - from.rank ) == 2 ) {
			auto enpass_left = at( { to.file-1, to.rank } ), 
				 enpass_right = at( { to.file+1, to.rank } );
			if ( enpass_left && enpass_left->owner != on_turn && enpass_left->type == piece_type::pawn )
				enpass_lapsed.insert( { { to.file-1, to.rank }, to} );
			if ( enpass_right && enpass_right->owner != on_turn && enpass_right->type == piece_type::pawn )
				enpass_lapsed.insert( { { to.file+1, to.rank }, to} );
			enpass_pos = to;
		} else {
			enpass_pos = {};
			for ( auto it = enpass_lapsed.begin(); it != enpass_lapsed.end(); ) {
				auto [att, tar] = *it;
				if ( from == att || from == tar || to == att || to == tar ) {
					it = enpass_lapsed.erase( it );
				} else {
					++it;
				}
			}
		}
		mark_moved( from );
		mark_moved( to );
		on_turn = get_enemy( on_turn );
		return res;							// all the rest == ok, capture
    }
};

void print( chess &c ) {
    for ( int i = 1; i < 9; ++i ) {
	    for ( int j = 1; j < 9; ++j ) {
	    	print( c.at( {j,i} ) );
	    }
	    std::cout << std::endl; 		
    }
    std::cout << std::endl;
}

// a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8
void test_blocked() {
	std::cout << "Testing blocked:" << std::endl;
	chess c;
	assert( c.play( {1, 2}, {1, 4} ) == result::ok );
	assert( c.play( {4, 7}, {4, 5} ) == result::ok );
	assert( c.play( {1, 1}, {1, 3} ) == result::ok );
	assert( c.play( {3, 8}, {8, 3} ) == result::ok );
	assert( c.play( {7, 1}, {6, 3} ) == result::ok );
	assert( c.play( {4, 8}, {4, 6} ) == result::ok );
	assert( c.play( {1, 3}, {4, 3} ) == result::ok );
	assert( c.play( {5, 8}, {4, 8} ) == result::ok );
	std::cout << "PASSED" << std::endl;
}

// a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8
void test_castling() {
	std::cout << "Testing castling:" << std::endl;
	chess c;
	assert( c.play( {7, 2}, {7, 3} ) == result::ok );
	assert( c.play( {7, 7}, {7, 6} ) == result::ok );
	assert( c.play( {6, 1}, {8, 3} ) == result::ok );
	assert( c.play( {7, 8}, {6, 6} ) == result::ok );
	assert( c.play( {7, 1}, {6, 3} ) == result::ok ); // knight moved
	assert( c.play( {6, 8}, {7, 7} ) == result::ok );
	assert( c.play( {5, 1}, {2, 1} ) == result::bad_move );
	assert( c.play( {5, 1}, {3, 1} ) == result::blocked );
	assert( c.play( {5, 1}, {7, 1} ) == result::ok ); // white castled
	assert( c.play( {8, 8}, {5, 8} ) == result::blocked );
	assert( c.play( {8, 8}, {8, 7} ) == result::blocked );
	assert( c.play( {8, 8}, {6, 8} ) == result::ok );
	assert( c.play( {6, 3}, {4, 2} ) == result::blocked );
	assert( c.play( {6, 3}, {5, 5} ) == result::ok ); // knight moved
	assert( c.play( {6, 8}, {8, 8} ) == result::ok );
	assert( c.play( {6, 2}, {6, 4} ) == result::ok );
	assert( c.play( {5, 8}, {3, 8} ) == result::blocked );
	assert( c.play( {5, 8}, {7, 8} ) == result::has_moved );
	assert( c.play( {4, 7}, {5, 6} ) == result::bad_move );
	assert( c.play( {4, 7}, {4, 6} ) == result::ok );
	assert( c.play( {7, 1}, {6, 2} ) == result::ok );
	assert( c.play( {3, 8}, {5, 6} ) == result::ok );
	assert( c.play( {6, 2}, {5, 1} ) == result::ok );
	assert( c.play( {5, 6}, {1, 2} ) == result::capture );
	assert( c.play( {5, 1}, {8, 1} ) == result::bad_move );
	assert( c.play( {5, 1}, {7, 1} ) == result::blocked );
	assert( c.play( {6, 1}, {8, 1} ) == result::ok );
	assert( c.play( {4, 8}, {4, 7} ) == result::ok );
	assert( c.play( {5, 1}, {7, 1} ) == result::has_moved );
	assert( c.play( {7, 3}, {7, 4} ) == result::ok );
	assert( c.play( {4, 7}, {2, 5} ) == result::ok ); // queen moved
	assert( c.play( {7, 4}, {7, 5} ) == result::ok );
	assert( c.play( {5, 8}, {3, 8} ) == result::blocked );
	assert( c.play( {2, 8}, {1, 6} ) == result::ok );
	assert( c.play( {5, 1}, {6, 2} ) == result::ok ); // w
	assert( c.play( {5, 8}, {3, 8} ) == result::would_check );
	assert( c.play( {3, 7}, {3, 5} ) == result::ok );
	assert( c.play( {8, 3}, {6, 1} ) == result::ok );
	assert( c.play( {2, 7}, {3, 6} ) == result::bad_move ); // black tried en passant on itself
	assert( c.play( {5, 8}, {3, 8} ) == result::ok );	// black castled
	assert( c.play( {6, 2}, {6, 3} ) == result::ok ); // w
	assert( c.play( {6, 6}, {5, 4} ) == result::ok ); // b move horse
	assert( c.play( {8, 2}, {8, 3} ) == result::ok ); // w
	assert( c.play( {8, 7}, {8, 5} ) == result::ok ); // b
	assert( c.play( {8, 3}, {8, 4} ) == result::ok ); // w
	assert( c.play( {6, 7}, {6, 5} ) == result::ok ); // b
	assert( c.play( {7, 5}, {8, 6} ) == result::lapsed );
	assert( c.play( {7, 5}, {6, 6} ) == result::capture );
	assert( c.play( {5, 4}, {4, 2} ) == result::capture ); // b horse, check
	assert( c.play( {4, 1}, {5, 1} ) == result::in_check );
	assert( c.play( {4, 1}, {4, 2} ) == result::capture );
	assert( c.play( {3, 8}, {2, 8} ) == result::ok ); // moved king
	assert( c.play( {6, 6}, {7, 7} ) == result::capture ); // w pawn capture
	assert( c.play( {2, 8}, {1, 8} ) == result::ok ); // moved king
	assert( c.play( {7, 7}, {7, 8} ) == result::bad_promote );
	assert( c.play( {7, 7}, {8, 8} ) == result::bad_promote );
	assert( c.play( {7, 7}, {8, 8}, piece_type::king ) == result::bad_promote );
	assert( c.play( {7, 7}, {8, 8}, piece_type::queen ) == result::capture );
	std::cout << "PASSED" << std::endl;	
}

void test_check() {
	std::cout << "Testing check:" << std::endl;
	std::map<position, piece> board;
	board[ {5, 5} ] = { player::white, piece_type::king };
	board[ {3, 3} ] = { player::white, piece_type::queen };
	board[ {1, 1} ] = { player::black, piece_type::bishop };
	board[ {1, 8} ] = { player::black, piece_type::rook };
	board[ {7, 2} ] = { player::white, piece_type::rook };
	chess c( board );
	assert( c.play( {5,5}, {6,6} ) == result::ok );
	assert( c.play( {1,1}, {1,6} ) == result::ok );
	assert( c.play( {7,2}, {7,8} ) == result::in_check );
	assert( c.play( {4,4}, {3,3} ) == result::in_check );
	assert( c.play( {6,6}, {5,5} ) == result::ok );
	assert( c.play( {1,6}, {1,1} ) == result::ok );
	
	assert( c.play( {3,3}, {3,4} ) == result::would_check );
	assert( c.play( {3,3}, {4,4} ) == result::ok );
	assert( c.play( {1,1}, {5,1} ) == result::ok );
	assert( c.play( {7,2}, {7,8} ) == result::in_check );
	assert( c.play( {4,4}, {3,3} ) == result::in_check );
	assert( c.play( {7,2}, {5,2} ) == result::ok );
	std::cout << "PASSED" << std::endl;
}

void test_priority() {
	std::cout << "Testing priority:" << std::endl;
	std::map<position, piece> board;
	board[ {5, 8} ] = { player::black, piece_type::king };
	board[ {1, 8} ] = { player::black, piece_type::rook };
	board[ {8, 8} ] = { player::black, piece_type::rook };
	board[ {8, 1} ] = { player::white, piece_type::rook };
	board[ {8, 1} ] = { player::white, piece_type::pawn };
	chess c( board );
	assert( c.play( {-1,-1}, {3,3} ) == result::no_piece );
	assert( c.play( {5,8}, {3,8} ) == result::bad_piece );
	
	std::cout << "PASSED" << std::endl;
}

void test_castling2() {
	std::cout << "Testing castling 2:" << std::endl;
	chess c;
	assert( c.play( {-1,-1}, {3,3} ) == result::no_piece );
	assert( c.play( {5,8}, {3,8} ) == result::bad_piece );
	
	std::cout << "PASSED" << std::endl;
}

int main()
{
	test_blocked();
	//test_check();
	test_priority();
	test_castling();
	test_castling2();
	
	
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
