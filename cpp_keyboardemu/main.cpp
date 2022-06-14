#include "Keyboard.h"
#include <iostream>

using namespace std;

// Test a keyboard
template<class T>
void test( Keyboard<T> & k ) {

	std::cout << "Test keyboard: " << k.name( ) << std::endl;

	// Start clean
	k.reset( );

	// Estimate with a dictionary
	k.estimate_from_file( "words_alpha.txt" );
	// Print and reset
	std::cout << "Dictionary: " << std::endl;
	k.print( std::cout );
	std::cout << std::endl;
	k.reset( );

	// Test with a large english sample text
	k.estimate_from_file( "books_concat.txt" );
	std::cout << "Sample text: " << std::endl;
	k.print( std::cout );
	std::cout << std::endl;

	// Reset before we return
	k.reset( );

}

int main( ) {

	// Test QWERTY
	QWERTY k_q;
	// Curious, the templated type cannot be deduced here, I wonder why..
	//test( (Keyboard<QWERTY_Estimate> & )k_q );

	Colemak k_c;
	//test( ( Keyboard<QWERTY_Estimate> & )k_c );

	Dvorak k_d;
	//test( ( Keyboard<QWERTY_Estimate> & )k_d );

	GKOS k_g;
	test( ( Keyboard<GKOS_Estimate> & )k_g );

}