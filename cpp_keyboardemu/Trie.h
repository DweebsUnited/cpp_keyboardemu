#pragma once
#include <vector>
#include <memory>
#include <optional>
#include <string>

typedef unsigned long long int uint;

class Node {

	friend class Trie;

protected:
  char c;
  uint used;
  uint eowc;

  std::vector<std::unique_ptr<Node>> children;

	Node( char c, uint used = 0, uint eowc = 0 ) : c( c ), used( used ), eowc( eowc ), children( ) { };

	// Query child
	std::optional<Node *> get_child( char c );
	// Add child if it does not exist
	Node * get_or_add_child( char c );

	// Recursively add string
	void add( std::string::iterator begin, std::string::iterator end );

	// Recursively query string
	std::optional<uint> query( std::string::iterator begin, std::string::iterator end );

};

class Trie {

	std::unique_ptr<Node> root;

public:

};

impl Trie {

	// Create a Trie
	pub fn new( ) -> Trie {
		Trie {
			root: Node::new( '\0' )
		}
	}

	// Add new word
	pub fn add( &mut self, word: &str ) {

		self.root.add( word );

	}

	// Query a word starting at root -> eowc
	pub fn query( &self, word: &str ) -> Option<u64> {

		self.root.query( word )

	}

	pub fn walk<P: FnMut( &str, u64, u64 ) -> bool>( &self, pred: &mut P ) {

		let mut word: String = String::new( );
		self.root.walk( &mut word, pred, false );

	}

}

pub mod io {

	use crate::{ Trie, Node };

	use std::fs::File;
	use std::path::Path;
	use std::io::{ self, BufRead, Result, Error, ErrorKind };
	use std::iter;

	pub fn from_wordlist<P: AsRef<Path>>( path: P ) -> Result<Trie> {

		let f: File = File::open( path )?;
		let lines: io::Lines<io::BufReader<File>> = io::BufReader::new( f ).lines( );

		let mut t: Trie = Trie::new( );

		for line in lines {

			if let Ok( word ) = line {

				t.add( &word );

			}

		}

		Ok( t )

	}

	pub fn from_wordlist_if<P: AsRef<Path>, B: Fn( &str ) -> bool>( path: P, pred: B ) -> Result<Trie> {

		let f: File = File::open( path )?;
		let lines: io::Lines<io::BufReader<File>> = io::BufReader::new( f ).lines( );

		let mut t: Trie = Trie::new( );

		for line in lines {

			if let Ok( word ) = line {

				if pred( &word ) {

					t.add( &word );

				}

			}

		}

		Ok( t )

	}

	pub fn write_text<P: AsRef<Path>>( t: &Trie, path: P ) -> Result<()> {

		let f: File = File::create( path )?;
		let mut w: io::BufWriter<File> = io::BufWriter::new( f );

		t.write( &mut w )

	}

	pub fn read_text<P: AsRef<Path>>( path: P ) -> Result<Trie> {

		let f: File = File::open( path )?;
		let lines: io::Lines<io::BufReader<File>> = io::BufReader::new( f ).lines( );

		let mut t: Trie = Trie::new( );

		t.read( lines )?;

		Ok( t )

	}

	impl Trie {

		fn write<W: io::Write>( &self, w: &mut W ) -> Result<()> {

			self.root.write( w )?;
			w.flush( )

		}

		fn read<L>( &mut self, mut lines: L ) -> Result<()>
		where L: iter::Iterator<Item = Result<String>>,
		{

			self.root.read( &mut lines )

		}

	}

	impl Node {

		fn write<W: io::Write>( &self, w: &mut W ) -> Result<()> {

			write!( w, "{}\x1F{}\x1F{}\x1F{}\n", self.c, self.used, self.eowc, self.children.len( ) )?;

			for child in &self.children {

				child.write( w )?;

			}

			Ok(())

		}

		fn read<L>( &mut self, lines: &mut L ) -> Result<()>
		where L: iter::Iterator<Item = Result<String>>,
		{

			// Get root stats then start loop
			let line: String = lines.next( ).ok_or( Error::new( ErrorKind::Other, "No more lines while building node!" ) )??;
			let mut stats: std::str::Split<char> = line.trim( ).split( '\x1F' );

			let c: &str = stats.next( ).ok_or( Error::new( ErrorKind::Other, "Failed to get character from line split" ) )?;
			if c.len( ) != 1 {
				return Err( Error::new( ErrorKind::Other, "Parsed node character is wrong length!" ) );
			} else {
				self.c = c.chars( ).nth( 0 ).unwrap( );
			}

			let u: &str = stats.next( ).ok_or( Error::new( ErrorKind::Other, "Failed to get used from line split" ) )?;
			if let Ok( u ) = u.parse::<u64>( ) {
				self.used = u;
			} else {
				return Err( Error::new( ErrorKind::Other, "Could not parse used value to u64!" ) );
			}

			let e: &str = stats.next( ).ok_or( Error::new( ErrorKind::Other, "Failed to get eowc from line split" ) )?;
			if let Ok( e ) = e.parse::<u64>( ) {
				self.eowc = e;
			} else {
				return Err( Error::new( ErrorKind::Other, "Could not parse eowc value to u64!" ) );
			}

			let c_cnt_str: &str = stats.next( ).ok_or( Error::new( ErrorKind::Other, "Failed to get child count from line split" ) )?;
			let c_cnt: u64;
			if let Ok( c_cnt_prs ) = c_cnt_str.parse::<u64>( ) {
				c_cnt = c_cnt_prs;
			} else {
				return Err( Error::new( ErrorKind::Other, "Could not parse child count value to u64!" ) );
			}

			for _ in 0..c_cnt {

				let mut c: Node = Node::new( '\0' );
				c.read( lines )?;
				self.children.push( c );

			}

			Ok(())

		}

	}

}