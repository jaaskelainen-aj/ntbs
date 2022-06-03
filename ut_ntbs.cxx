/* clang -Wall -std=c++17 -lc++ -o ut_ntbs ut_ntbs.cxx
*/

#include <iostream>
using namespace std;

#define NTBS_DEBUG
#include "ntbs.hpp"
#include "ntbs.cpp"

bool trim()
{
	ntbs ts(50);

	ts = "   zzz   ";
	cout << ts.get() << '|';
	ts.trim();
	cout << ts.get()  << "|\n";

	ts = "     ";
	cout << ts.get() << '|';
	ts.trim();
	cout << ts.get()  << "|\n";

	ts = "   zzz   ";
	cout << ts.get() << '|';
	ts.trim(ntbs::RIGHT);
	cout << ts.get()  << "|\n";

	ts = "   zzz   ";
	cout << ts.get() << '|';
	ts.trim(ntbs::LEFT);
	cout << ts.get()  << "|\n";
	return true;
}

int main(int argc, char const *argv[])
{
	NTBS(na, 20);
	ntbs nb(20);
	ntbs nc;
	ntbs nd("const value");
	ntbs ne("const copy", ntbs::ALLOC);

	if (trim())
		return 0;

	na.dump(cout);
	nb.dump(cout);
	nc.dump(cout);
	nd.dump(cout);
	ne.dump(cout);

	cout << endl;
	cout << "Normal copy:\n";
	na = "Hello world";
	na.dump(cout);

	cout << "Copy constructor:\n";
	ntbs foo(na);
	foo.dump(cout);

	cout << "Realloc:\n";
	na = "Long string to force realloc";
	na.dump(cout);

	cout << "Sprint to empty:\n";
	nc.sprint("My original size was %ld.", nc.size());
	nc.dump(cout);

	cout << "Append print to previous:\n";
	nc.addprint(" %d is number of life.", 42);
	nc.dump(cout);

	return 0;
}