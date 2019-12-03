#include <iostream>
using namespace std;

#include "initializer.hh"
using namespace archimedes::common;

static int n{0};
static auto dummy1 = Initializer::Register("hello", [n = ++n] {
	cout << "normal: " << n << endl;
	return true;
});
static auto dummy2 = Initializer::Register("hello", [n = ++n] {
	cout << "normal: " << n << endl;
	return true;
});
static auto dummy3 = Initializer::Register("hello", [n = ++n] {
	cout << "normal: " << n << endl;
	return true;
});

static auto dummy = Initializer::Register(
    "hello",
    [n = ++n] {
	    cout << "high: " << n << endl;
	    return true;
    },
    Initializer::Priority::kHigh);

int main()
{
	RUN_INIT();
	return 0;
}
