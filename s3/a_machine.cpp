#include <cstdint>
#include <limits>
#include <map>
#include <vector>
#include <iostream>
#include <cassert>
#include <functional>

/* V této úloze budete programovat jednoduchý registrový stroj
 * (model počítače). Stroj bude mít libovolný počet celočíselných
 * registrů a paměť adresovatelnou po bajtech. Registry jsou
 * indexované od 1 po ‹INT_MAX›. Každá instrukce jmenuje dva
 * registry a jednu přímo zakódovanou hodnotu (angl. immediate).
 *
 * V každém registru je uložena hodnota typu ‹int32_t›, tzn.
 * velikost strojového slova jsou 4 slabiky (bajty). V paměti jsou
 * slova uložena tak, že nejvýznamnější slabika má nejnižší adresu
 * (tzv. MSB-first). Počáteční hodnoty registrů i paměti jsou nuly.
 *
 * Stroj má následovné instrukce (kdykoliv je ‹reg_X› použito
 * v popisu, myslí se tím samotný registr – jeho hodnota nebo
 * úložiště – nikoliv jeho index; sloupec ‹reg_2› má opačný význam,
 * vztahuje se k indexu uloženému v instrukci).
 *
 *  │ opcode │‹reg_2›│ description                                 │
 *  ├────────┼───────┼◅────────────────────────────────────────────┤
 *  │ ‹mov›  │  ≥ 1  │ kopíruj hodnotu z ‹reg_2› do ‹reg_1›        │
 *  │        │  = 0  │ nastav ‹reg_1› na ‹immediate›               │
 *  │ ‹add›  │  ≥ 1  │ ulož ‹reg_1 + reg_2› do ‹reg_1›             │
 *  │        │  = 0  │ přičti ‹immediate› do ‹reg_1›               │
 *  │ ‹mul›  │  ≥ 1  │ ulož ‹reg_1 * reg_2› do ‹reg_1›             │
 *  │ ‹jmp›  │  = 0  │ skoč na adresu uloženou v ‹reg_1›           │
 *  │        │  ≥ 1  │ skoč na ‹reg_1› je-li ‹reg_2› nenulové      │
 *  │ ‹load› │  ≥ 1  │ načti hodnotu z adresy ‹reg_2› do ‹reg_1›   │
 *  │ ‹stor› │  ≥ 1  │ zapiš hodnotu ‹reg_1› na adresu ‹reg_2›     │
 *  │ ‹halt› │  = 0  │ zastav stroj s výsledkem ‹reg_1›            │
 *  │        │  ≥ 1  │ totéž, ale pouze je-li ‹reg_2› nenulový     │
 *
 * Každá instrukce je v paměti uložena jako 4 slova (adresy slov
 * rostou zleva doprava). Vykonání instrukce, která není skokem,
 * zvýší programový čítač o 4 slova.
 *
 *  ┌────────┬───────────┬───────┬───────┐
 *  │ opcode │ immediate │ reg_1 │ reg_2 │
 *  └────────┴───────────┴───────┴───────┘
 *
 * Vykonání jednotlivé instrukce by mělo zabrat konstantní čas.
 * Paměť potřebná pro výpočet by měla být v nejhorším případě úměrná
 * součtu nejvyšší použité adresy a nejvyššího použitého indexu
 * registru. */

enum class opcode { mov, add, mul, jmp, load, stor, hlt };

struct machine
{
	std::vector<std::int32_t> registers;
	std::vector<std::uint8_t>  memory;
	std::int32_t _immediate;
	bool halt_flag, jump_flag;

	using op_func = std::function<void( std::int32_t, std::int32_t )>;
	std::map<opcode,op_func> func_map {
		{ opcode::mov, 
			[&]( std::int32_t reg1, std::int32_t reg2 ) {
				if ( reg2 == 0 ) {
					set_reg( reg1, _immediate );
				} else if ( reg2 >= 1 ) {
					set_reg( reg1, get_reg( reg2 ) );
				}
			}
		},
		{ opcode::add, 
			[&]( std::int32_t reg1, std::int32_t reg2 ) {
				if ( reg2 == 0 ) {
					set_reg( reg1, get_reg(reg1) + _immediate );
				} else if ( reg2 >= 1 ) {
					set_reg( reg1, get_reg(reg1) + get_reg(reg2) );
				}				
			}
		},
		{ opcode::mul, 
			[&]( std::int32_t reg1, std::int32_t reg2 ) {
				if ( reg2 >= 1 ) 
					set_reg( reg1, get_reg(reg1) * get_reg(reg2) );
			}
		},
		{ opcode::jmp, 
			[&]( std::int32_t, std::int32_t reg2 ) {
				if ( reg2 == 0 || ( reg2 >= 1 && get_reg( reg2 ) != 0 ) )
					jump_flag = true;	
			}
		},
		{ opcode::load, 
			[&]( std::int32_t reg1, std::int32_t reg2 ) {
				if ( reg2 >= 1 )
					set_reg( reg1, get( get_reg( reg2 ) ) );				
			}
		},
		{ opcode::stor, 
			[&]( std::int32_t reg1, std::int32_t reg2 ) {
				if ( reg2 >= 1 )
					set( get_reg( reg2 ), get_reg( reg1 ) );
			}
		},
		{ opcode::hlt, 
			[&]( std::int32_t, std::int32_t reg2 ) {
				if ( reg2 == 0 || ( reg2 >= 1 && get_reg( reg2 ) != 0 ) ) {
					halt_flag = true;
				}
			}
		}
	};

	void check_mem_size( std::int32_t idx ) {
		if ( idx >= 0 && static_cast<std::size_t>(idx+3) >= memory.size() ) memory.resize(idx+4);
	}
	void check_reg_size( std::int32_t idx ) {
		if ( idx >= 0 && static_cast<std::size_t>(idx) >= registers.size() ) registers.resize(idx+1);
	}
	
    /* Čtení a zápis paměti po jednotlivých slovech. */
    std::int32_t get( std::int32_t addr ) const {
    	std::int32_t val = 0;
    	for ( std::size_t i = 0; i < 4; ++i ) {
	    	val = ( val << 8 ) + ( ( addr+i < memory.size() ) ? memory[addr+i] : 0 );   		
    	}
		return val;
    }
    void         set( std::int32_t addr, std::int32_t v ) {
    	check_mem_size( addr );
    	std::int32_t val = v;
    	for ( std::size_t i = 0; i < 4; ++i ) {
	    	memory[addr+(3-i)] = val & 255;
	    	val = val >> 8;    		
    	}
    }

    std::int32_t get_reg( std::int32_t addr ) const {
	    return ( static_cast<std::size_t>(addr) < registers.size() ) ? registers[addr] : 0;    
    }
    void         set_reg( std::int32_t addr, std::int32_t v ) {
    	check_reg_size( addr );
		registers[addr] = v;
    }

    void print_mem() const {
    	std::cout << "< ";
    	for ( auto i : memory ) {
    		std::cout << static_cast<int>(i) << " ";
    	}
    	std::cout << ">" << std::endl;
    }

    /* Spuštění programu, počínaje adresou nula. Vrátí hodnotu
     * uloženou v ‹reg_1› zadaném instrukcí ‹hlt›, která výpočet
     * ukončila. */

    std::int32_t run() {
    		//print_mem();
		int current_address = 0;
		halt_flag = false;
		opcode op;
		std::int32_t op_val, reg1, reg2;
		while ( !halt_flag ) {
			
			jump_flag = false;
			op_val = get( current_address );
			_immediate = get( current_address+1*4 );
			reg1 = get( current_address+2*4 );
			reg2 = get( current_address+3*4 );

				/*std::cout << "Address: " << current_address << std::endl;
				std::cout << "op: " << op_val << std::endl;
				std::cout << "immediate: " << _immediate << std::endl;
				std::cout << "reg_1: " << reg1 << std::endl;
				std::cout << "reg_2: " << reg2 << std::endl << std::endl;*/

			if ( op_val >= 0 && op_val < 7 ) {
				op = static_cast< opcode >( op_val );
				func_map[op]( reg1, reg2 );				
			}
			
			if ( jump_flag ) {
				current_address = get_reg( reg1 );
			} else {
				current_address += 16;
			}
		}
    	return get_reg( reg1 );
    }
};

void test_fmap() {
	std::cout << "Testing func_map" << std::endl;
	machine m;
    m.set_reg( 0x10, 7 );
    m.func_map[ opcode::hlt ]( 0x10, 0 );
    std::cout << "halt result: " << m.get_reg(0x10) << std::endl;
    assert( m.get_reg(0x10) == 7 );
    //std::cout << "Mov op index: " << static_cast< std::int32_t >( opcode::mov ) << std::endl;
}

void test_iota() {
	machine m;
	m.set(0,	0);
	m.set(4,	48);
	m.set(8,	1);
	m.set(12,	0);
	m.set(16,	0);
	m.set(20,	12);
	m.set(24,	2);
	m.set(28,	0);
	m.set(32,	0);
	m.set(36,	268);
	m.set(40,	4);
	m.set(44,	0);
	m.set(48,	1);
	m.set(52,	-1);
	m.set(56,	2);
	m.set(60,	0);
	m.set(64,	1);
	m.set(68,	1);
	m.set(72,	3);
	m.set(76,	0);
	m.set(80,	1);
	m.set(84,	-1);
	m.set(88,	4);
	m.set(92,	0);
	m.set(96,	5);
	m.set(100,	0);
	m.set(104,	3);
	m.set(108,	4);
	m.set(112,	3);
	m.set(116,	0);
	m.set(120,	1);
	m.set(124,	2);
	m.set(128,	6);
	m.set(132,	0);
	m.set(136,	4);
	m.set(140,	0);
	auto res = m.run();
	//std::cout << "Result: " << res << std::endl;
	assert( res == 256 );
}

void test_loop() {
	machine m;
	m.set(0,	0);
	m.set(4,	12);
	m.set(8,	2);
	m.set(12,	0);
	m.set(16,	0);
	m.set(20,	16);
	m.set(24,	1);
	m.set(28,	0);
	m.set(32,	1);
	m.set(36,	-1);
	m.set(40,	2);
	m.set(44,	0);
	m.set(48,	3);
	m.set(52,	0);
	m.set(56,	1);
	m.set(60,	2);
	m.set(64,	6);
	m.set(68,	0);
	m.set(72,	2);
	m.set(76,	0);
	auto res = m.run();
	//std::cout << "Result: " << res << std::endl;
	assert( res == 0 );
}

void test_memory() {
	machine m;
	m.set(0,	0);
	m.set(4,	1701);
	m.set(8,	1);
	m.set(12,	0);
	m.set(16,	0);
	m.set(20,	1705);
	m.set(24,	2);
	m.set(28,	0);
	m.set(32,	0);
	m.set(36,	11);
	m.set(40,	1);	// sus
	m.set(44,	0);
	m.set(48,	0);
	m.set(52,	12);
	m.set(56,	4);
	m.set(60,	0);
	m.set(64,	4);
	m.set(68,	0);
	m.set(72,	3);
	m.set(76,	2);
	m.set(80,	1);
	m.set(84,	-1);
	m.set(88,	3);
	m.set(92,	0);
	m.set(96,	1);
	m.set(100,	1);
	m.set(104,	5);
	m.set(108,	0);
	m.set(112,	5);
	m.set(116,	0);
	m.set(120,	3);
	m.set(124,	2);
	m.set(128,	4);
	m.set(132,	0);
	m.set(136,	4);
	m.set(140,	1);
	m.set(144,	3);
	m.set(148,	0);
	m.set(152,	4);
	m.set(156,	3);
	m.set(160,	6);
	m.set(164,	0);
	m.set(168,	5);
	m.set(172,	0);
	auto res = m.run();
	//std::cout << "Result: " << res << std::endl;
	assert( res == 13 );
}

void test_selfmod() {
	machine m;
	m.set(0,	0);
	m.set(4,	6);
	m.set(8,	1);
	m.set(12,	0);
	m.set(16,	0);
	m.set(20,	96);
	m.set(24,	2);
	m.set(28,	0);
	m.set(32,	5);
	m.set(36,	0);
	m.set(40,	1);
	m.set(44,	2);
	m.set(48,	0);
	m.set(52,	2);
	m.set(56,	1);
	m.set(60,	0);
	m.set(64,	0);
	m.set(68,	104);
	m.set(72,	2);
	m.set(76,	0);
	m.set(80,	5);
	m.set(84,	0);
	m.set(88,	1);
	m.set(92,	2);
	auto res = m.run();
	//std::cout << "Result: " << res << std::endl;
	assert( res == 104 );
}

void test_unaligned() {
	machine m;
	m.set(0,	0);
	m.set(4,	73);
	m.set(8,	1);
	m.set(12,	0);
	m.set(16,	0);
	m.set(20,	5);
	m.set(24,	2);
	m.set(28,	0);
	m.set(32,	3);
	m.set(36,	0);
	m.set(40,	1);
	m.set(44,	0);
	m.set(73,	4);
	m.set(77,	0);
	m.set(81,	1);
	m.set(85,	2);
	m.set(89,	6);
	m.set(93,	0);
	m.set(97,	1);
	m.set(101,	0);
	auto res = m.run();
	//std::cout << "Result: " << res << std::endl;
	assert( res == 73 << 8 );
}
	
int main()
{
	test_iota();
	//test_fmap();
	test_loop();
	//test_memory();
	test_selfmod();
	test_unaligned();
	
    machine m;
    m.set( 0x00, static_cast< std::int32_t >( opcode::mov ) );
    assert( m.get( 0x00 ) == static_cast< std::int32_t >( opcode::mov ) );
    m.set( 0x04, 7 );
    assert( m.get( 0x04 ) == 7 );
    m.set( 0x08, 1 );
    assert( m.get( 0x08 ) == 1 );
    m.set( 0x10, static_cast< std::int32_t >( opcode::hlt ) );
    assert( m.get( 0x10 ) == static_cast< std::int32_t >( opcode::hlt ) );
    m.set( 0x18, 1 );
    assert( m.get( 0x18 ) == 1 );
    //m.print_mem();
    assert( m.run() == 7 );
    return 0;
}
