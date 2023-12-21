#include <clocale>
#include <cstdlib>
#include <unordered_map>
#include <memory>
#include <iostream>

import User;
import table_out;
import current_menu;

using namespace std;

[[noreturn]] int main()
{
	system("chcp 1251 & cls");
	setlocale(0, "Russian_BELARUS.1251");
	setlocale(LC_NUMERIC, "US.1251");

	cin.exceptions(ios_base::failbit);
	for (;;) current_menu()();
}