#include "Keyboard.h"
#include <iostream>

using namespace std;

ostream & operator<<( ostream & out, const QWERTY & kb ) {
	out << "Finger statistics:" << endl;
	for( auto f : kb.stats.finger_use ) {
		out << "  " << f.first
			<< ": " << f.second
			<< " -- " << kb.stats.finger_movement[ f.first ].second
			<< endl;
	}
	cout << "Hand-switch percentage: " << ( kb.stats.to_left + kb.stats.to_right ) / ( double ) kb.stats.total << endl;
}