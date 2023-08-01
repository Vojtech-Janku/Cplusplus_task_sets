#include <memory>
#include <string_view>
#include <string>
#include <map>
#include <vector>
#include <stack>
#include <cassert>
#include <iostream>
#include <stdexcept>

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

void print( json_type type ) {
	std::cout << "Val_type: ";
	switch( type ) {
		case json_type::integer:	std::cout << "integer"; break;
		case json_type::array:		std::cout << "array"; break; 
		case json_type::object:		std::cout << "object"; break; 
		default: assert( false );
	}
	std::cout << std::endl;	
}

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

struct json_integer : public json_value
{
public:
	int value;

	json_integer( int val = 0 ) : json_value(), value( val ) {}
	
    json_type type() const override { return json_type::integer; }
    int int_value() const override { return value; }
    json_ref item_at( int ) const override { throw std::out_of_range( "sir, this is integer" ); }
    json_ref item_at( std::string_view ) const override { throw std::out_of_range( "sir, this is integer" ); }
    std::string key_at( int ) const override { throw std::out_of_range( "sir, this is integer" ); }
    int length() const override { return 0; }	
};

struct json_array : public json_value
{
public:
	std::vector<json_ptr> arr;

	json_array() : json_value() {}
	
    json_type type() const override { return json_type::array; }
    int int_value() const override { return 0; }
    json_ref item_at( int idx ) const override {
    	if ( idx < 0 || idx >= length() ) throw std::out_of_range( "we don't have that number of books, good sir" );
    	return *arr[idx];
    }
    json_ref item_at( std::string_view ) const override { throw std::out_of_range( "no book here with that title, good sir" ); }
    std::string key_at( int ) const override { throw std::out_of_range( "can't place that there m'lord" ); }
    int length() const override { return arr.size(); }

    void add_val( json_ptr val ) {
    	arr.push_back( std::move( val ) );
    }		
};

struct json_object : public json_value
{
public:
	std::map<std::string,json_ptr> obj_map;

	json_object() : json_value() {}

    json_type type() const override { return json_type::object; }
    int int_value() const override { return 0; }

	auto iter_at( int idx ) const {
    	if ( idx < 0 || idx >= length() ) throw std::out_of_range( "here there be dragons" );
    	int count = 0;
    	for ( auto it = obj_map.begin(); it != obj_map.end(); ++it ) {
    		if ( count == idx ) return it;
    		++count;
    	}
    	assert( false );		
	}
    
    json_ref item_at( std::string_view view ) const override { 
		std::string key( view );
		auto it = obj_map.find( key );
		if ( it == obj_map.end() ) throw std::out_of_range( "apologies, our cartographer couldn't find that name on our map, sir" );
		return *(it->second);
		
    }
    json_ref item_at( int idx ) const override {
		return *iter_at( idx )->second;
    }
    std::string key_at( int idx ) const override {
    	return iter_at( idx )->first;
    }
    int length() const override { return obj_map.size(); }

    void add_keyval( std::string key, json_ptr val ) {
    	obj_map[key] = std::move( val );
    }
};


/* Čistá funkce ‹json_parse› analyzuje dokument a vytvoří odpovídající
 * stromovou strukturu, nebo skončí výjimkou ‹json_error›:
 *
 *  • nevyhovuje-li řetězec zadané gramatice gramatice,
 *  • objeví-li se v kterémkoliv objektu zdvojený klíč. */


std::size_t skip_spaces( std::string_view &view, std::size_t start ) {
	std::size_t idx = start;
	while( idx < view.size() && isspace( view[idx] ) ) { ++idx; }
	return idx;
}

std::string get_value( std::string_view &view, std::size_t &start ) {
	std::size_t idx = skip_spaces( view, start );
	while( idx < view.size() && !isspace( view[idx] ) && 
			view[idx] != ':' && view[idx] != '[' && view[idx] != '{' && 
			view[idx] != ',' && view[idx] != ']' && view[idx] != '}' ) { ++idx; }
	std::string res( view.substr( start, idx-start ) );
	start = idx;
	return res;
}

bool begins_new( char c ) {
	return c == '[' || c == '{' || c == '-' || isdigit( c );
}

json_ptr parse_val( std::string_view view, std::size_t &idx );

using j_array_ptr = std::unique_ptr<json_array>;
using j_object_ptr = std::unique_ptr<json_object>;

json_ptr parse_array( std::string_view view, std::size_t &idx ) {
	j_array_ptr res = std::make_unique<json_array>();
	idx = skip_spaces( view, idx );
	while ( idx < view.size() && view[idx] != ']' ) {
		if ( view[idx] == ',' ) {
			++idx;
			idx = skip_spaces( view, idx );
		}
		if ( begins_new( view[idx] ) ) {
			json_ptr val = parse_val( view, idx );
			res->add_val( std::move(val) );
			idx = skip_spaces( view, idx );
		}
	}
	++idx;
	return res;
}

json_ptr parse_object( std::string_view view, std::size_t &idx ) {
	j_object_ptr res = std::make_unique<json_object>();
	std::string key;
	idx = skip_spaces( view, idx );
	while ( idx < view.size() && view[idx] != '}' ) {
		if ( view[idx] == ',' ) {
			++idx;
			idx = skip_spaces( view, idx );
		}
		key = get_value( view, idx );
		if ( res->obj_map.count(key) ) throw json_error();
		idx = skip_spaces( view, idx );
		assert( view[idx] == ':' );
		++idx;
		idx = skip_spaces( view, idx );
		json_ptr val = parse_val( view, idx );
		res->add_keyval( key, std::move(val) );
		idx = skip_spaces( view, idx );
	}
	++idx;	
	return res;
}

json_ptr parse_val( std::string_view view, std::size_t &idx ) {
	json_ptr res;
	idx = skip_spaces( view, idx );
	if ( view[idx] == '-' || isdigit( view[idx] ) ) {
		std::string token = get_value( view, idx );
		int val = stoi(token);
		return std::make_unique<json_integer>( val );
	} else if ( view[idx] == '[' ) {
		++idx;
		return parse_array( view, idx );
	} else if ( view[idx] == '{' ) {
		++idx;
		return parse_object( view, idx );
	} else {
		throw json_error();
	}
}

/* Konečně čistá funkce ‹json_validate› rozhodne, je-li vstupní
 * dokument správně utvořený (tzn. odpovídá zadané gramatice). Tato
 * funkce nesmí skončit výjimkou (krom ‹std::bad_alloc› v situaci,
 * kdy během analýzy dojde paměť). */

bool json_validate( std::string_view );

json_ptr json_parse( std::string_view view ) {
	//if ( !json_validate( view ) ) throw json_error();
	std::size_t idx = 0;
	return parse_val( view, idx );
}

/*bool check_value( std::string_view &view, std::size_t &idx, std::stack<char> &brackets );

bool check_integer( std::string_view &view, std::size_t &idx ) {
	if ( view[idx] == '0' ) {
		++idx;
		return true;	// its outside's problem now
	}
	if ( view[idx] == '-' ) ++idx;
	if ( idx < view.size() && isdigit( view[idx] ) && view[idx] != '0' ) {
		++idx;
	} else {
		return false;
	}
	while ( idx < view.size() && isdigit( view[idx] ) ) {
		++idx;
	}
	return true;
}

bool check_bracket( char br, std::stack<char> &brackets ) {
	if ( brackets.empty() ) return false;
	char top_br = brackets.top();
	brackets.pop();
	return br == top_br;
}

bool check_array(  std::string_view &view, std::size_t &idx, std::stack<char> &brackets ) {
	if ( idx >= view.size() ) return false;
	if ( view[idx] == ']' ) {
		if ( !check_bracket( view[idx], brackets ) ) return false;
		++idx;
		return true;
	}
	if ( !check_value( view, idx, brackets ) ) return false;
	while ( idx < view.size() && view[idx] != ']' ) {
		if ( view[idx] != ',' ) return false;
		idx++;
		if ( !check_value( view, idx, brackets ) ) return false;
	}
	if ( idx >= view.size() || !check_bracket( view[idx], brackets ) ) return false;
	++idx;
	return true;
}

bool check_key( std::string_view &view, std::size_t &idx ) {
	idx = skip_spaces( view, idx );
	if ( idx >= view.size() || !isalpha( view[idx] ) ) return false;
	while ( idx < view.size() && isalpha( view[idx] ) ) { ++idx; }
	idx = skip_spaces( view, idx );
	if ( idx >= view.size() || view[idx] != ':' ) return false;
	++idx;
	return true;
}

bool check_object( std::string_view &view, std::size_t &idx, std::stack<char> &brackets ) {
	if ( idx >= view.size() ) return false;
	if ( view[idx] == '}' ) {
		if ( !check_bracket( view[idx], brackets ) ) return false;
		++idx;
		return true;
	}
	// first keyval
	if ( !check_key( view, idx ) ) return false;
	if ( !check_value( view, idx, brackets ) ) return false;

	// all keyvals
	while ( idx < view.size() && view[idx] != '}' ) {
		if ( view[idx] != ',' ) return false;
		++idx;
		if ( !check_key( view, idx ) ) return false;
		if ( !check_value( view, idx, brackets ) ) return false;
	}
	if ( idx >= view.size() || !check_bracket( view[idx], brackets ) ) return false;
	++idx;
	return true;
}

bool check_value( std::string_view &view, std::size_t &idx, std::stack<char> &brackets ) {
	bool res;
	idx = skip_spaces( view, idx );
	if ( idx >= view.size() ) return false;
	if ( view[idx] == '-' || isdigit( view[idx] ) ) {	// integer
		res = check_integer( view, idx );			
	} else if ( view[idx] == '[' ) {					// array begin
		brackets.emplace( ']' );
		++idx;
		res = check_array( view, idx, brackets );
	} else if ( view[idx] == '{' ) {					// object begin
		brackets.emplace( '}' );
		++idx;
		res = check_object( view, idx, brackets );
	} else {
		return false;
	}
	idx = skip_spaces( view, idx );
	
	return res;
} */

bool json_validate( std::string_view view ) {
	try { 
		json_parse( view );
		return true;
	} catch ( json_error e ) {
		return false;
	}
	/*std::stack<char> brackets;
	std::size_t idx = 0;
	return check_value( view, idx, brackets ) && idx >= view.size() && brackets.empty(); */
}

void test_array() {
	assert( !json_validate( "[ ]" ) );
	auto a = json_parse( "[ 1 ,2, 8]" );
	auto b = json_parse( "[ [[  []]] ,2, [ [5] , 9 ]]  " );
	auto c = json_parse( " [ [[  [[[[]]]]]] ,2, [ [5] , 9 ]]" );
	auto d = json_parse( " [ { gman: -57, freeman :9000} , [], 7 ]  " );
	auto emp = json_parse( " []  " );

	assert( b->length() == 3 );
	assert( c->length() == 3 );
	assert( d->length() == 3 );
	assert( d->item_at(0).length() == 2 );
	std::string_view kkk( "gman" );
	assert( d->item_at(0).item_at( kkk ).int_value() == -57 );
	assert( d->item_at(2).length() == 0 );
	assert( emp->length() == 0 );

	assert( a->type() == json_type::array );
	assert( a->length() == 3 );
	assert( a->item_at(0).type() == json_type::integer );
	assert( a->item_at(0).int_value() == 1 );
	assert( a->item_at(1).int_value() == 2 );
	assert( a->item_at(2).int_value() == 8 );
	try {
		assert( a->item_at(3).int_value() == 1 );
		assert( false );
	} catch ( std::out_of_range &e ) {}
	try {
		assert( a->item_at(-1).int_value() == 1 );
		assert( false );
	} catch ( std::out_of_range &e ) {}
	
	const auto &con = c->item_at(0);
	for ( int i = 0; i < 3; ++i ) {
		const auto &cc = con;
		const auto &con = cc.item_at(0);
		assert( con.type() == json_type::array );
	}
}

void test_object() {
	assert( !json_validate( " { }  " ) );
	assert( !json_validate( " { : 4 }  " ) );
	assert( !json_validate( " { gh: }  " ) );
	assert( json_validate( " { a:5,b:[7,2],c:{} }  " ) );
	json_parse( " { a:5,b:[7,2],c:{ c : {c:{a:7 ,c:{}} } }}  " );
	try {
		json_parse( "{  a: }  " );
		assert( false );		
	} catch ( json_error &e ) {}
	try {
		json_parse( "{   c:1, e : [ 1, 2] ,f:7,c:{}  }  " );
		assert( false );
	} catch ( json_error &e ) {}
	try {
		json_parse( "{  a:  [ 1 ,2 , { a:8, t:[], a:8 , 1] }  " );
		assert( false );		
	} catch ( json_error &e ) {}
	
}

void test_object_properties() {
	auto o = json_parse( " { b:[7,2],clem:{},alphons:5 }  " );
	assert( o->type() == json_type::object );
	assert( o->length() == 3 );
	assert( o->item_at( 0 ).type() == json_type::integer );
	assert( o->item_at( 1 ).type() == json_type::array );
	assert( o->item_at( 2 ).type() == json_type::object );
	assert( o->item_at( "alphons" ).int_value() == 5 );
	assert( o->item_at( "b" ).length() == 2 );
	assert( o->item_at( "clem" ).length() == 0 );
	assert( o->key_at( 1 ) == "b" );
	assert( o->key_at( 0 ) == "alphons" );
	try {
		o->key_at( -1 );
		assert( false );
	} catch ( std::out_of_range &e ) {}
	try {
		o->key_at( 3 );
		assert( false );
	} catch ( std::out_of_range &e ) {}
	try {
		o->item_at( -1 );
		assert( false );
	} catch ( std::out_of_range &e ) {}
	try {
		o->item_at( 3 );
		assert( false );
	} catch ( std::out_of_range &e ) {}
	try {
		o->item_at( "alfons" );
		assert( false );
	} catch ( std::out_of_range &e ) {}
	try {
		o->item_at( 2 ).item_at(0);
		assert( false );
	} catch ( std::out_of_range &e ) {}
}

int main()
{
	test_array();
	test_object();
	test_object_properties();

	try {
		json_parse( "-1" );
		json_parse( "0" );
		json_parse( "-99801" );
		json_parse( "50000" );
	} catch ( json_error &e ) {
		assert( false );
	}
    assert( !json_validate( "x" ) );
    assert( json_validate( "{}" ) );
    assert( json_validate( "[ 1 ]" ) );
	try {
		auto bad = json_parse( "[ 1 ,2, 1, [] , 1, 2, 1 ]  " );
	} catch ( json_error &e ) {
		assert( false );
	}

    auto t = json_parse( "{}" );
    auto a = json_parse( "[ 1 ,2, 8]" );
    assert( t->type() == json_type::object );

	assert( a->type() == json_type::array );
	assert( a->item_at(2).type() == json_type::integer );
	
    return 0;
}
