#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include <fstream>

typedef unsigned long long int uint;

// Utility to lowercase keys, returning 0 if we don't know them
std::map<char, char> key_lower_map( {
	{ '{', '[' },
	{ '[', '[' },
	{ '}', ']' },
	{ ']', ']' },
	{ ':', ';' },
	{ ';', ';' },
	{ '"', '\'' },
	{ '\'', '\'' },
	{ '<', ',' },
	{ ',', ',' },
	{ '>', '.' },
	{ '.', '.' },
	{ '?', '/' },
	{ '/', '/' },
	{ '|', '\\' },
	{ '\\', '\\' },
	{ '!', '1' },
	{ '1', '1' },
	{ '@', '2' },
	{ '2', '2' },
	{ '#', '3' },
	{ '3', '3' },
	{ '$', '4' },
	{ '4', '4' },
	{ '%', '5' },
	{ '5', '5' },
	{ '^', '6' },
	{ '6', '6' },
	{ '&', '7' },
	{ '7', '7' },
	{ '*', '8' },
	{ '8', '8' },
	{ '(', '9' },
	{ '9', '9' },
	{ ')', '0' },
	{ '0', '0' },
	{ '_', '-' },
	{ '-', '-' },
	{ '+', '=' },
	{ '=', '=' }
	} );
inline char key_lower( char c ) {

	if( c >= 'a' && c <= 'z' )
		return c;
	else if( c >= 'A' && c <= 'Z' )
		return c + 32;

	// Special chars take a lookup in the map, but we expect these very infrequently
	auto l = key_lower_map.find( c );
	if( l == key_lower_map.end( ) )
		return 0;
	else
		return l->second;

}

inline double dist( double sx, double ex, double sy, double ey ) {
	return sqrt( ( ex - sx ) * ( ex - sx ) + ( ey - sy ) * ( ey - sy ) );
}

struct Key {

	float x, y;
	short finger;

	Key( float x, float y, short finger ) : x( x ), y( y ), finger( finger ) { };
	Key( ) : x( 0 ), y( 0 ), finger( 1024 ) { };

};

struct Finger {

	float x, y;
	float sx, sy;
	bool left_hand;

	Finger( float sx, float sy, bool left_hand ) : x( sx ), y( sy ), sx( sx ), sy( sy ), left_hand( left_hand ) { };
	Finger( ) : x( 0 ), sx( 0 ), y( 0 ), sy( 0 ), left_hand( true ) { };

};

template<class S>
class Keyboard {
protected:
	S stats;

	std::map<char, Key> keys;
	std::map<short, Finger> fingers;

	// Reset any other internal state needed
	virtual void reset_state( ) {
		for( auto iter : this->fingers ) {
			iter.second.x = iter.second.sx;
			iter.second.y = iter.second.sy;
		}
	};

	// Consume one char, updating estimate of state
	virtual void consume_char( char c ) = 0;

public:
	virtual std::string name( ) = 0;

	// Full reset
	virtual void reset( ) {
		this->stats = std::move( S( ) );
		this->reset_state( );
	}

	virtual void estimate( std::string word ) {
		for( char c : word )
			this->consume_char( c );
	};
	virtual void estimate_from_file( std::string path ) {
		std::ifstream infile( path );

		char c;
		while( infile.get( c ) )
			this->consume_char( c );

		infile.close( );
	};

	virtual S get_stats( ) {
		return this->stats;
	}

	Keyboard( std::initializer_list<std::pair<const char, Key>> keys,
		std::initializer_list<std::pair<const short, Finger>> fingers )
		: keys( keys ), fingers( fingers ) { };
	//Keyboard( std::map<char, Key> && keys, std::map<char, Finger> && fingers ) :
		//keys( std::move( keys ) ),
		//fingers( std::move( fingers ) ) { };

	virtual std::ostream & print( std::ostream & out ) = 0;
};


// QWERTY!
// Here are the stats we care about:
//   Per-finger usage
//   L-to-R percentage
//   Per-finger movement
struct QWERTY_Estimate {
	std::map<short, uint> finger_use;
	uint to_left;
	uint to_right;
	uint total;
	std::map<short, double> finger_movement;
};

class QWERTY : public Keyboard<QWERTY_Estimate> {
private:
	char last = 0;
	bool last_left = false;

protected:
	void consume_char( char c ) {

		c = key_lower( c );

		// Skip spaces -- Not really interested for QWERTY
		if( c == ' ' || c == '\n' || c == '\t' )
			return;

		// Skip keys we don't know about
		// TODO: Should probably handle some of these better...
		if( c == 0 )
			return;
		
		// Get the key object
		Key & k = this->keys[ c ];
		
		// Increment finger usage
		this->stats.finger_use[ k.finger ] += 1;

		// Get the finger
		Finger & f = this->fingers[ k.finger ];
		
		// Accum the distance, update the finger
		this->stats.finger_movement[ k.finger ] += dist( f.x, k.x, f.y, k.y );
		f.x = k.x;
		f.y = k.y;

		// If we have a last char
		if( this->last != 0 ) {
			// If moving from right to left
			if( f.left_hand && !this->last_left )
				this->stats.to_left += 1;
			// Else if moving right to left
			else if( this->last_left && !f.left_hand )
				this->stats.to_right += 1;
		}
		// Save this char as the last
		this->last = c;
		this->last_left = f.left_hand;

		this->stats.total += 1;
	}

public:
	std::ostream & print( std::ostream & out ) {
		out << "Finger statistics:" << std::endl;
		for( auto f : this->stats.finger_use ) {
			out << "  " << f.first
				<< ": " << f.second
				<< " -- " << this->stats.finger_movement[ f.first ]
				<< std::endl;
		}
		out << "Hand-switch percentage: " << ( this->stats.to_left + this->stats.to_right ) / ( double ) this->stats.total << std::endl;
		return out;
	}

	std::string name( ) {
		return "QWERTY";
	}

	QWERTY( ) : Keyboard( {
			{ '1', Key(-0.50, 3.0, -4 ) },
			{ '2', Key( 0.50, 3.0, -3 ) },
			{ '3', Key( 1.50, 3.0, -2 ) },
			{ '4', Key( 2.50, 3.0, -1 ) },
			{ '5', Key( 3.50, 3.0, -1 ) },
			{ '6', Key( 4.50, 3.0,  1 ) },
			{ '7', Key( 5.50, 3.0,  1 ) },
			{ '8', Key( 6.50, 3.0,  2 ) },
			{ '9', Key( 7.50, 3.0,  3 ) },
			{ '0', Key( 8.50, 3.0,  4 ) },
			{ '-', Key( 9.50, 3.0,  4 ) },
			{ '=', Key(10.50, 3.0,  4 ) },
			{ 'q', Key( 0.00, 2.0, -4 ) },
			{ 'w', Key( 1.00, 2.0, -3 ) },
			{ 'e', Key( 2.00, 2.0, -2 ) },
			{ 'r', Key( 3.00, 2.0, -1 ) },
			{ 't', Key( 4.00, 2.0, -1 ) },
			{ 'y', Key( 5.00, 2.0,  1 ) },
			{ 'u', Key( 6.00, 2.0,  1 ) },
			{ 'i', Key( 7.00, 2.0,  2 ) },
			{ 'o', Key( 8.00, 2.0,  3 ) },
			{ 'p', Key( 9.00, 2.0,  4 ) },
			{ '[', Key(10.00, 2.0,  4 ) },
			{ ']', Key(11.00, 2.0,  4 ) },
			{'\\', Key(12.00, 2.0,  4 ) },
			{ 'a', Key( 0.25, 1.0, -4 ) },
			{ 's', Key( 1.25, 1.0, -3 ) },
			{ 'd', Key( 2.25, 1.0, -2 ) },
			{ 'f', Key( 3.25, 1.0, -1 ) },
			{ 'g', Key( 4.25, 1.0, -1 ) },
			{ 'h', Key( 5.25, 1.0,  1 ) },
			{ 'j', Key( 6.25, 1.0,  1 ) },
			{ 'k', Key( 7.25, 1.0,  2 ) },
			{ 'l', Key( 8.25, 1.0,  3 ) },
			{ ';', Key( 9.25, 1.0,  4 ) },
			{'\'', Key(10.25, 1.0,  4 ) },
			{ 'z', Key( 0.75, 0.0, -4 ) },
			{ 'x', Key( 1.75, 0.0, -3 ) },
			{ 'c', Key( 2.75, 0.0, -2 ) },
			{ 'v', Key( 3.75, 0.0, -1 ) },
			{ 'b', Key( 4.75, 0.0, -1 ) },
			{ 'n', Key( 5.75, 0.0,  1 ) },
			{ 'm', Key( 6.75, 0.0,  1 ) },
			{ ',', Key( 7.75, 0.0,  2 ) },
			{ '.', Key( 8.75, 0.0,  3 ) },
			{ '/', Key( 9.75, 0.0,  4 ) }
		}, {
			{ -4, Finger( 0.25, 1.0, true ) },
			{ -3, Finger( 1.25, 1.0, true ) },
			{ -2, Finger( 2.25, 1.0, true ) },
			{ -1, Finger( 3.25, 1.0, true ) },
			{  1, Finger( 6.25, 1.0, false ) },
			{  2, Finger( 7.25, 1.0, false ) },
			{  3, Finger( 8.25, 1.0, false ) },
			{  4, Finger( 9.25, 1.0, false ) }
		} ) { };

	QWERTY( std::initializer_list<std::pair<const char, Key>> keys,
		std::initializer_list < std::pair<const short, Finger>> fingers ) :
		Keyboard( keys, fingers ) { };
	//QWERTY( std::map<char, Key> && keys, std::map<char, Finger> && fingers ) : Keyboard( move( keys ), move( fingers ) ) { };

};

class Colemak : public QWERTY {

public:
	std::string name( ) {
		return "Colemak";
	}

	Colemak( ) : QWERTY( {
			{ '1', Key( -0.50, 3.0, -4 ) },
			{ '2', Key( 0.50, 3.0, -3 ) },
			{ '3', Key( 1.50, 3.0, -2 ) },
			{ '4', Key( 2.50, 3.0, -1 ) },
			{ '5', Key( 3.50, 3.0, -1 ) },
			{ '6', Key( 4.50, 3.0,  1 ) },
			{ '7', Key( 5.50, 3.0,  1 ) },
			{ '8', Key( 6.50, 3.0,  2 ) },
			{ '9', Key( 7.50, 3.0,  3 ) },
			{ '0', Key( 8.50, 3.0,  4 ) },
			{ '-', Key( 9.50, 3.0,  4 ) },
			{ '=', Key( 10.50, 3.0,  4 ) },
			{ 'q', Key( 0.00, 2.0, -4 ) },
			{ 'w', Key( 1.00, 2.0, -3 ) },
			{ 'f', Key( 2.00, 2.0, -2 ) },
			{ 'p', Key( 3.00, 2.0, -1 ) },
			{ 'g', Key( 4.00, 2.0, -1 ) },
			{ 'j', Key( 5.00, 2.0,  1 ) },
			{ 'l', Key( 6.00, 2.0,  1 ) },
			{ 'u', Key( 7.00, 2.0,  2 ) },
			{ 'y', Key( 8.00, 2.0,  3 ) },
			{ ';', Key( 9.00, 2.0,  4 ) },
			{ '[', Key( 10.00, 2.0,  4 ) },
			{ ']', Key( 11.00, 2.0,  4 ) },
			{'\\', Key( 12.00, 2.0,  4 ) },
			{ 'a', Key( 0.25, 1.0, -4 ) },
			{ 'r', Key( 1.25, 1.0, -3 ) },
			{ 's', Key( 2.25, 1.0, -2 ) },
			{ 't', Key( 3.25, 1.0, -1 ) },
			{ 'd', Key( 4.25, 1.0, -1 ) },
			{ 'h', Key( 5.25, 1.0,  1 ) },
			{ 'n', Key( 6.25, 1.0,  1 ) },
			{ 'e', Key( 7.25, 1.0,  2 ) },
			{ 'i', Key( 8.25, 1.0,  3 ) },
			{ 'o', Key( 9.25, 1.0,  4 ) },
			{'\'', Key( 10.25, 1.0,  4 ) },
			{ 'z', Key( 0.75, 0.0, -4 ) },
			{ 'x', Key( 1.75, 0.0, -3 ) },
			{ 'c', Key( 2.75, 0.0, -2 ) },
			{ 'v', Key( 3.75, 0.0, -1 ) },
			{ 'b', Key( 4.75, 0.0, -1 ) },
			{ 'k', Key( 5.75, 0.0,  1 ) },
			{ 'm', Key( 6.75, 0.0,  1 ) },
			{ ',', Key( 7.75, 0.0,  2 ) },
			{ '.', Key( 8.75, 0.0,  3 ) },
			{ '/', Key( 9.75, 0.0,  4 ) }
		}, {
			{ -4, Finger( 0.25, 1.0, true ) },
			{ -3, Finger( 1.25, 1.0, true ) },
			{ -2, Finger( 2.25, 1.0, true ) },
			{ -1, Finger( 3.25, 1.0, true ) },
			{  1, Finger( 6.25, 1.0, false ) },
			{  2, Finger( 7.25, 1.0, false ) },
			{  3, Finger( 8.25, 1.0, false ) },
			{  4, Finger( 9.25, 1.0, false ) }
		} ) { };

};

class Dvorak : public QWERTY {
public:
	std::string name( ) {
		return "Dvorak";
	}

	Dvorak( ) : QWERTY( {
			{ '1', Key( -0.50, 3.0, -4 ) },
			{ '2', Key( 0.50, 3.0, -3 ) },
			{ '3', Key( 1.50, 3.0, -2 ) },
			{ '4', Key( 2.50, 3.0, -1 ) },
			{ '5', Key( 3.50, 3.0, -1 ) },
			{ '6', Key( 4.50, 3.0,  1 ) },
			{ '7', Key( 5.50, 3.0,  1 ) },
			{ '8', Key( 6.50, 3.0,  2 ) },
			{ '9', Key( 7.50, 3.0,  3 ) },
			{ '0', Key( 8.50, 3.0,  4 ) },
			{ '[', Key( 9.50, 3.0,  4 ) },
			{ ']', Key( 10.50, 3.0,  4 ) },
			{'\'', Key( 0.00, 2.0, -4 ) },
			{ ',', Key( 1.00, 2.0, -3 ) },
			{ '.', Key( 2.00, 2.0, -2 ) },
			{ 'p', Key( 3.00, 2.0, -1 ) },
			{ 'y', Key( 4.00, 2.0, -1 ) },
			{ 'f', Key( 5.00, 2.0,  1 ) },
			{ 'd', Key( 6.00, 2.0,  1 ) },
			{ 'c', Key( 7.00, 2.0,  2 ) },
			{ 'r', Key( 8.00, 2.0,  3 ) },
			{ 'l', Key( 9.00, 2.0,  4 ) },
			{ '/', Key( 10.00, 2.0,  4 ) },
			{ '=', Key( 11.00, 2.0,  4 ) },
			{'\\', Key( 12.00, 2.0,  4 ) },
			{ 'a', Key( 0.25, 1.0, -4 ) },
			{ 'o', Key( 1.25, 1.0, -3 ) },
			{ 'e', Key( 2.25, 1.0, -2 ) },
			{ 'u', Key( 3.25, 1.0, -1 ) },
			{ 'i', Key( 4.25, 1.0, -1 ) },
			{ 'd', Key( 5.25, 1.0,  1 ) },
			{ 'h', Key( 6.25, 1.0,  1 ) },
			{ 't', Key( 7.25, 1.0,  2 ) },
			{ 'n', Key( 8.25, 1.0,  3 ) },
			{ 's', Key( 9.25, 1.0,  4 ) },
			{ '-', Key( 10.25, 1.0,  4 ) },
			{ ';', Key( 0.75, 0.0, -4 ) },
			{ 'q', Key( 1.75, 0.0, -3 ) },
			{ 'j', Key( 2.75, 0.0, -2 ) },
			{ 'k', Key( 3.75, 0.0, -1 ) },
			{ 'x', Key( 4.75, 0.0, -1 ) },
			{ 'b', Key( 5.75, 0.0,  1 ) },
			{ 'm', Key( 6.75, 0.0,  1 ) },
			{ 'w', Key( 7.75, 0.0,  2 ) },
			{ 'v', Key( 8.75, 0.0,  3 ) },
			{ 'z', Key( 9.75, 0.0,  4 ) }
		}, {
			{ -4, Finger( 0.25, 1.0, true ) },
			{ -3, Finger( 1.25, 1.0, true ) },
			{ -2, Finger( 2.25, 1.0, true ) },
			{ -1, Finger( 3.25, 1.0, true ) },
			{  1, Finger( 6.25, 1.0, false ) },
			{  2, Finger( 7.25, 1.0, false ) },
			{  3, Finger( 8.25, 1.0, false ) },
			{  4, Finger( 9.25, 1.0, false ) }
		} ) { };
};

/* TODO
class GKOS : public Keyboard {

};
*/