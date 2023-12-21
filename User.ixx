module;
#include <bitset>
export module User;
export import Data;
import <string>;
import <array>;
import <vector>;
import <istream>;

#pragma warning(push) 
#pragma warning(disable: 4455) 
//warning C4455: "operator ""sv": идентификаторы литеральных суффиксов, которые начинаются не с символа подчеркивания, зарезервированы

using 
	std::string, 
	std::move, 
	std::string_view, 
	std::operator""sv,
	std::operator""s;

#pragma warning(pop)

export extern string input_password(string, std::istream & = std::cin);

export class User final : public Data
{
	std::bitset<512> _password;
	std::bitset<320> _salt;
	string _name{};
	string _phone{};
	bool _is_admin = false, _is_blocked = false;
public:
	explicit User(bool is_admin = false) : _is_admin(is_admin) {}
	bool is_admin() const noexcept { return _is_admin; }

	bool is_blocked() const noexcept { return _is_blocked; }
	User &is_blocked(bool what) & { _is_admin ? throw std::exception{} : _is_blocked = what; return *this; }
	User &&is_blocked(bool what) && { _is_admin ? throw std::exception{} : _is_blocked = what; return move(*this); }

	string const &name() const noexcept { return _name; }
	User &&name(string_view name)&& noexcept { _name = name; return move(*this); }
	User &name(string_view name)& noexcept { _name = name; return *this; }

	string const &phone() const noexcept { return _phone; }
	User &&phone(string_view phone) && noexcept { _phone = phone; return move(*this); }
	User &phone(string_view phone) & noexcept { _phone = phone; return *this; }

	bool check_password(string_view password) const noexcept;
	User &set_password(string_view new_password) noexcept;
	bool set_password(string_view new_password, string_view old_password) noexcept
	{
		if (not check_password(old_password)) return false;
		set_password(new_password);
		return true;
	}

	virtual void from_stream(std::istream &) override;
	virtual void to_stream(std::ostream &) const override;
	virtual void from_binary(std::istream &) override;
	virtual void to_binary(std::ostream &) const override;

	constexpr static std::array<std::streamoff, 4> table_sizes_mem = { 19, 80, 20, 20 };
	constexpr static std::array table_names_mem = { "ID"sv ,"Имя"sv, "Контактные данные"sv, "Статус"sv };
	static_assert(size(table_sizes_mem) == size(table_names_mem));

	std::vector<std::string> table_members() const noexcept
	{
		return { ""s, _name, _phone, _is_admin ? "Администратор"s : _is_blocked ? "Заблокирован"s : "Турист"s };
	}
	static std::span<const std::string_view> table_names() noexcept
	{
		return table_names_mem;
	}
	static std::span<const std::streamoff> table_sizes() noexcept 
	{
		return table_sizes_mem;
	}
};
