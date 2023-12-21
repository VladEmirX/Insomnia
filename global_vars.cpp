module;
#include <fstream>
module global_vars;

using namespace std;
constexpr auto& users_bin = "..\\files\\users.bin";
constexpr auto& tours_bin = "..\\files\\tours.bin";

std::map<uint64_t, std::unique_ptr<User>>
get_users()
{
	std::map<uint64_t, std::unique_ptr<User>> result;
	ifstream in(users_bin, ios::binary);
	do
	{
		if (not in.is_open()) { ofstream(users_bin, ios::binary | ios::noreplace); break; };
		in.exceptions(ios::failbit | ios::badbit);

		while (in.peek() != EOF) try
		{
			uint64_t id;
			in.read((char *)&id, 8);
			auto where = result.emplace_hint(result.cend(), id, make_unique<User>());
			where->second->from_binary(in);
		}
		catch (bad_alloc &)
		{
			println(cerr, "Oshibka: Dannye faila povrezhdeny."); //На этом этапе локаль ещё не установлена
			result.clear();
			//break;
			throw;
		}
		catch (ios::failure &)
		{
			println(cerr, "Oshibka: Dannye faila povrezhdeny.");
			result.clear();
			//break;
			throw;
		}
	}
	while (false);
	if (result.empty())
		result.emplace_hint(result.cend(),
				1,
				(unique_ptr<User>) & (new User(true))->name("-"sv).phone("-"sv).set_password("-"sv)
		);
	return move(result);
}
std::map<uint64_t, std::unique_ptr<Tour>> get_tours()
{
	std::map<uint64_t, std::unique_ptr<Tour>> result;
	ifstream in(tours_bin, ios::binary);

	if (not in.is_open()) { ofstream(tours_bin, ios::binary | ios::noreplace); return result; };
	in.exceptions(ios::failbit | ios::badbit);

	while (in.peek() != EOF) try
	{
		uint64_t id;
		in.read((char *)&id, 8);
		auto where = result.try_emplace(result.end(), id, make_unique<Tour>()); 
		where->second->from_binary(in);
	}
	catch (bad_alloc &)
	{
		println(cerr, "Oshibka: Dannye faila povrezhdeny."); //На этом этапе локаль ещё не установлена
		result.clear();
		//break;
		throw;
	}
	catch (ios::failure &)
	{
		cerr << "Oshibka: Dannye faila povrezhdeny.\n";
		//return {};
		throw;
	}
	return result;
}

namespace global_vars
{
	std::map<uint64_t, std::unique_ptr<User>> users = get_users();
	std::map<uint64_t, std::unique_ptr<Tour>> tours = get_tours();
	std::map<uint64_t, std::unique_ptr<User>>::iterator current_user = users.end();
}

void global_vars::save() noexcept
{
	ofstream us_out(users_bin, ios::binary), to_out(tours_bin, ios::binary);
	if (not us_out.is_open())
	{
		cerr << "Не смог открыть users.bin.\n";
		return;
	}
	if (not to_out.is_open())
	{
		cerr << "Не смог открыть tours.bin.\n";
		return;
	}
	for (const auto &[id, tour] : tours)
	{
		to_out.write((const char *)&id, 8);
		tour->to_binary(to_out);
	}
	for (const auto &[id, user] : users)
	{
		us_out.write((const char *)&id, 8);
		user->to_binary(us_out);
	}
}