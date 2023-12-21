module;
export module Menu;
import <functional>;
import <map>;
import <string_view>;
import <ranges>;
import <tuple>;
import <print>; 
import <iostream>;
import input;


using 
	std::move_only_function,
	std::pair,
	std::string_view,
	std::move,
	std::get,
	std::println;


export template <typename parT, typename funT>
	class Menu
{
public:
	using fun_type = move_only_function<funT>;
	struct mapped_type { mutable fun_type fun; string_view name; mutable move_only_function<bool()> yes; };
	using dic_type = std::map<parT, mapped_type>;
	struct init_type
	{
		parT par;
		string_view name = "";
		mutable fun_type fun = [] { return typename fun_type::result_type(); };
		mutable move_only_function<bool() noexcept> enable_if = [] () noexcept { return true; };
	};
private:
	string_view _head{};
	dic_type _funcs;
	mutable fun_type _default{};
public:
	Menu(std::initializer_list<init_type> list)
		: _funcs
		{
			std::from_range_t{},
			list | std::views::transform([] (const init_type &arg) -> dic_type::value_type
										{
											return { arg.par, mapped_type
													(move(arg.fun), arg.name, move(arg.enable_if))
													};
										})
		}
	{ };
	Menu &&name(string_view name) && { _head = name; return move(*this); }
	Menu &&on_incorrect(fun_type def) && { _default = move(def); return move(*this); };

	template <class... argsT>
	fun_type::result_type operator()(argsT&&... args) const
	{
		println("{}", _head);
		for (auto& [key, val] : _funcs)
			if (val.yes()) [[likely]] println("{} => {}", key, val.name);

		parT key;

		for (;;) try
		{
			key = input<parT>("Выберите: ");
			break;
		}
		catch (std::logic_error)
		{
			println(std::clog, "Ошибка ввода. Попробуйте ещё раз.");
		}
		try
		{
			auto& [fun, name, yes] = _funcs.at(key);
			if (not yes()) [[unlikely]] throw std::out_of_range{ "" };
			return fun();
		}
		catch (std::out_of_range)
		{
			return _default(std::forward<argsT>(args)...);
		}
	}
};
