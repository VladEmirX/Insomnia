module;
#include <iostream>
#include <print>
#include <random>
#include <bit>
#include <bitset>
#include <cassert>
module User;
import input;
using namespace std;

__inline static bitset<512> sha3(bitset<576> const &) noexcept; //SHA3-512
static bitset<320> generate_salt() noexcept;

[[nodiscard]]
bool User::check_password(string_view pass) const noexcept
{
	bitset<576> hashee{};
	reinterpret_cast<bitset<320> &>(hashee) = _salt;
	memcpy_s(40 + (char *)&hashee, 32, pass.data(), pass.size());
	if (pass.size() < 32) hashee[575] = hashee[576 - 8 * (32 - pass.size())] = 1;
	return _password == sha3(hashee);
}

User &User::set_password(string_view const pass) noexcept
{
	_salt = generate_salt(); 
	bitset<576> hashee{};
	reinterpret_cast<bitset<320> &>(hashee) = _salt;
	memcpy_s(40 + (char *)&hashee, 32, pass.data(), pass.size());
	if (pass.size() < 32) hashee[575] = hashee[576 - 8 * (32 - pass.size())] = true;
	_password = sha3(hashee);
	return *this;
}

void User::from_stream(std::istream & in = cin) try
{ 
	_name = input<string>("Введите ваше имя: ", in);
	_phone = input<string>("Введите ваши контактные данные: ", in);
	for (;;) try
	{
		string str;
		str = input_password("Введите пароль: ", in);
		if (str.size() < 8) 
			{ println("Неверная длина пароля (должен быть от 8 символов)."); continue; }
		set_password(str); 
		bool b = 
		check_password(input_password("Подтвердите пароль: ", in));
		if (b) return;
		println("Пароль неверный, попытайтесь ещё раз");
	}
	catch (std::invalid_argument &)
	{
		println("Пароль нулевой длины запрещён");
	}
	unreachable();
}
catch (...)
{
	_password.reset();
	_salt.reset();
	_name = _phone = {};
	throw;
}

void User::to_stream(std::ostream & out = cout) const
{ 
	println(out, "{} ( {} )", _name, _phone);
}

void User::from_binary(std::istream & in)
{ 
	in.read(&(char &)_password, sizeof _password);
	in.read(&(char &)_salt, sizeof _salt);
	in.read(&(char &)_is_admin, sizeof _is_admin);
	in.read(&(char &)_is_blocked, sizeof _is_blocked);
	size_t name_s, phone_s;
	in.read(&(char &)name_s, sizeof(size_t));
	in.read(&(char &)phone_s, sizeof(size_t));
	_name.resize(name_s);
	_phone.resize(phone_s);
	in.read(_name.data(), name_s);
	in.read(_phone.data(), phone_s);
}

void User::to_binary(std::ostream & out) const
{ 
	out.write((char const *)&_password, sizeof _password);
	out.write((char const *)&_salt, sizeof _salt);
	out.write((char const *)&_is_admin, sizeof _is_admin);
	out.write((char const *)&_is_blocked, sizeof _is_blocked);
	out.write((char const *)&(const size_t &)(_name.size()), sizeof(size_t));
	out.write((char const *)&(const size_t &)(_phone.size()), sizeof(size_t));
	out.write(_name.data(), _name.size());
	out.write(_phone.data(), _phone.size());
}

static bitset<320> generate_salt() noexcept
{
	static mt19937_64 R = mt19937_64( time(0) );
	return bit_cast<bitset<320>, uint64_t[5]>({ R(), R(), R(), R(), R() });
}

union A_type { uint64_t A[5][5]; bitset<320> AB[5]; };

static __inline A_type Tau(const A_type &A) noexcept
{
	const uint64_t 
	C[5] =
	{
		A.A[0][0] xor A.A[0][1] xor A.A[0][2] xor A.A[0][3] xor A.A[0][4],
		A.A[1][0] xor A.A[1][1] xor A.A[1][2] xor A.A[1][3] xor A.A[1][4],
		A.A[2][0] xor A.A[2][1] xor A.A[2][2] xor A.A[2][3] xor A.A[2][4],
		A.A[3][0] xor A.A[3][1] xor A.A[3][2] xor A.A[3][3] xor A.A[3][4],
		A.A[4][0] xor A.A[4][1] xor A.A[4][2] xor A.A[4][3] xor A.A[4][4],
	},
	D[5] =
	{
		C[4] xor rotr(C[1], 1),
		C[0] xor rotr(C[2], 1),
		C[1] xor rotr(C[3], 1),
		C[2] xor rotr(C[4], 1),
		C[3] xor rotr(C[0], 1),
	};
	return
	{
		A.A[0][0] xor D[0], A.A[0][1] xor D[0], A.A[0][2] xor D[0], A.A[0][3] xor D[0], A.A[0][4] xor D[0],  
		A.A[1][0] xor D[1], A.A[1][1] xor D[1], A.A[1][2] xor D[1], A.A[1][3] xor D[1], A.A[1][4] xor D[1],  
		A.A[2][0] xor D[2], A.A[2][1] xor D[2], A.A[2][2] xor D[2], A.A[2][3] xor D[2], A.A[2][4] xor D[2],  
		A.A[3][0] xor D[3], A.A[3][1] xor D[3], A.A[3][2] xor D[3], A.A[3][3] xor D[3], A.A[3][4] xor D[3],  
		A.A[4][0] xor D[4], A.A[4][1] xor D[4], A.A[4][2] xor D[4], A.A[4][3] xor D[4], A.A[4][4] xor D[4],  
	};
}
static __inline A_type Rho(A_type A) noexcept
{
	
	for (uint16_t t = 0, i = 1, j = 0; t != 24; ++t, tie(i, j) = pair(j, (2 * i + 3 * j) % 5))
		A.A[i][j] = rotr(A.A[i][j], (t + 1) * (t + 2) % 5);

	return A;
}
static __inline A_type Pi(const A_type &A) noexcept
{
	return 
	{
		A.A[0][0], A.A[3][0], A.A[1][0], A.A[4][0], A.A[2][0], 
		A.A[1][1], A.A[4][1], A.A[2][1], A.A[0][1], A.A[3][1], 
		A.A[2][2], A.A[0][2], A.A[3][2], A.A[1][2], A.A[4][2], 
		A.A[3][3], A.A[1][3], A.A[4][3], A.A[2][3], A.A[0][3], 
		A.A[4][4], A.A[2][4], A.A[0][4], A.A[3][4], A.A[1][4], 
	};	
}
static __inline A_type Chi(const A_type &A) noexcept
{
	return
	{
		.AB =
		{
			A.AB[0] xor (compl A.AB[1] bitand A.AB[2]),
			A.AB[1] xor (compl A.AB[2] bitand A.AB[3]),
			A.AB[2] xor (compl A.AB[3] bitand A.AB[4]),
			A.AB[3] xor (compl A.AB[4] bitand A.AB[0]),
			A.AB[4] xor (compl A.AB[0] bitand A.AB[1]),
		}
	};
}
static __inline A_type Iota(A_type A, uint8_t& R) noexcept
{
	for (uint_fast8_t i = 0; i != 7; ++i)
	{
		R = ((R << 1) xor (R bitand 128ui8 ? 0b0111'0001ui8 : 0ui8));
		if (R bitand 2)
			A.A[0][0] xor_eq (1ui64 << ((1 << i) - 1));
	}
	return A;
}

static bitset<512> sha3(bitset<576> const &hashee) noexcept
{
	A_type A{ {} };
	uint8_t R = 1;
	reinterpret_cast<bitset<576> &>(A) = hashee;
	for (uint_fast8_t i = 0; i != 24ui8; ++i)
		A = Iota(Chi(Pi(Rho(Tau(A)))), R);
	return reinterpret_cast<bitset<512>&>(A);
}

string input_password(string str, istream &in)
{
	string out;
	exception_ptr ex = nullptr;
	try
	{
		out = input<string>(str, in);
	}
	catch (...)
	{
		ex = current_exception();
	}
	cout.flush();
	print("\033[A\033[{}G\033[90m", str.size() + 1);
	for (auto i = 0; i != out.size(); ++i)
		print("•");
	println("\33[39m\33[K");
	if (ex) rethrow_exception(ex);
	if (out.size() > 32)
	{
		throw invalid_argument("Длина пароля не должна быть больше 32 символов");
	}
	return out;
}

