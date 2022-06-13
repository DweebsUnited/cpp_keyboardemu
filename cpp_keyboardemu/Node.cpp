#include "Trie.h"
#include <vector>
#include <memory>
#include <optional>

// Query for a child
std::optional<Node *> Node::get_child( char c ) {

	if let Ok( cdx ) = self.children.binary_search_by_key( &c, | n: &Node | n.c ) {
		Some( &self.children[ cdx ] )
	} else {
		None
	}

};

// Get child if exists, or add a new one
Node * Node::get_or_add_child( char c ) {

	let cdx_opt = self.children.binary_search_by_key( &c, | n: &Node | n.c );

	match cdx_opt {
		Ok( cdx ) => {
			self.children.get_mut( cdx ).expect( "Couldn't query the child we just found..." )
		},
		Err( cdx ) => {
			// Insert sorted
			self.children.insert( cdx, Node::new( c ) );
			// Have to query it to return it
			self.children.get_mut( cdx ).expect( "Couldn't query the child we just inserted..." )
		}
	}

};

// Add a new word
void Node::add( std::string::iterator begin, std::string::iterator end ) {

	self.used += 1;

	if word.len( ) == 0 {
		self.eowc += 1;
		return;
	}

	let c: char = word.chars( ).nth( 0 ).unwrap( );
	let child: &mut Node = self.get_or_add_child( c );
	child.add( &word[ 1.. ] );

};

// Recursively query a word
std::optional<uint> query( std::string::iterator begin, std::string::iterator end ) {

	if word.len( ) == 0 {
		if self.eowc > 0 {
			return Some( self.eowc );
		} else {
			return None;
		}
	}

	let c = word.chars( ).nth( 0 ).unwrap( );

	if let Some( child ) = self.get_child( c ) {
		child.query( &word[ 1.. ] )
	} else {
		None
	}

};