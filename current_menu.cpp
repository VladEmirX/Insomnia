module;
#include <ctime>
module current_menu;
import input;
import global_vars;
import table_out;
import <string>;
import <iostream>;
import <print>;
import <algorithm>;
import <ranges>;
import <vector>;
import <chrono>;

using namespace std;

extern const Menu<uint32_t, void()>
	main_menu,
	autorization_menu,
	user_menu,
	self_menu,
	tour_menu;

static const Menu<uint32_t, void()> *current_ptr = &autorization_menu;
const Menu<uint32_t, void()> &current_menu() noexcept { return *current_ptr; }

static void incorrect() noexcept
{
	println(clog, "Ошибка: нет такого пункта!");
}

static auto enter_time (const string &str) noexcept
{
	const auto _str = 
		format("{}"
			    "\33[90m"
				"00:00/{:%d.%m.%y}"
				"\33[39m\33[{}G", str, chrono::current_zone()->to_local(chrono::system_clock::now()), str.size() + 1);
	for (;;) try
	{
		cout.flush();
		return str_to_time(input<string>(_str));
	}
	catch (std::exception &x)
	{
		println(clog, "Ошибка: {}", x.what());
	}
};

static bool if_admin() noexcept
{
	return global_vars::current_user != global_vars::users.end() and global_vars::current_user->second->is_admin();
}
static bool if_not_admin() noexcept
{
	return global_vars::current_user != global_vars::users.end() and not global_vars::current_user->second->is_admin();
}
static bool if_user() noexcept
{
	return global_vars::current_user != global_vars::users.end();
}
static bool weak_if_admin() noexcept
{
	return global_vars::current_user->second->is_admin();
}
static bool weak_if_not_admin() noexcept
{
	return not global_vars::current_user->second->is_admin();
}

const Menu<uint32_t, void()> 
autorization_menu = Menu<uint32_t, void()>
{
	{
		.par = 0,
		.name = "Выйти из программы",
		.fun = bind(exit, 0),
	},
	{
		.par = 1,
		.name = "Авторизоваться",
		.fun = []
		{
			decltype(global_vars::current_user) user;
			for (;;) try
			{
				auto id = input<uint64_t>("Введите ваш id (0 для отмены): "sv);
				if (id == 0) return;
				user = global_vars::users.find(id);
				if (user == global_vars::users.end()) throw logic_error("Нет такого пользователя");
				if (user->second->is_blocked()) throw logic_error("Этот пользователь заблокирован");
				break;
			}
			catch (std::logic_error &x)
			{
				println(clog, "Ошибка: {}", x.what());
			}
			for (;;) try
			{
				auto str = input_password("Введите пароль (\"\" для отмены): ");
				if (user->second->check_password(str)) [[likely]] break;
				println(clog, "Ошибка: неправильный пароль");
			}
			catch (std::invalid_argument &)
			{
				return;
			}
			global_vars::current_user = user;
			println("Вы успешно вошли в систему!");
			current_ptr = &main_menu;
		}
	},
	{
		.par = 2,
		.name = "Зарегистрироваться",
		.fun = []
		{
			try
			{
				unique_ptr<User> user = make_unique<User>();
				user->
					name(input<string>("Введите имя пользователя (\"\" для отмены): ")).
					phone(input<string>("Введите контактные данные (\"\" для отмены): ")).
					set_password(input_password("Введите пароль (\"\" для отмены): ")).
					is_blocked(true);

				if (not user->check_password(input_password("Повторите пароль (\"\" для отмены): ")))
					[[unlikely]] println(clog, "Ошибка: неверный пароль.");
				else [[likely]]
				{
					auto id = 
					global_vars::users.try_emplace
					(
						global_vars::users.cend(),
						prev(global_vars::users.cend())->first + 1,
						move(user)
					)->first;
					println(clog, "Регистрация успешна, ваш id: {}", id);
					global_vars::save();
				}
			}
			catch (std::invalid_argument &) { return; }
		}
	},
	{
		.par = 3,
		.name = "Продолжить без авторизации",
		.fun = [] { current_ptr = &main_menu; global_vars::current_user = global_vars::users.end(); },
	},
}
	.name("\tВойдите в систему")
	.on_incorrect(incorrect),

main_menu = Menu<uint32_t, void()>
{
	{
		.par = 0,
		.name = "Меню авторизации...",
		.fun = [] { if (global_vars::current_user != global_vars::users.end()) global_vars::save(); current_ptr = &autorization_menu; }
	},
	{
		.par = 1,
		.name = "Туры...",
		.fun = [] { current_ptr = &tour_menu; }
	},
	{
		.par = 2,
		.name = "Персонализация...",
		.fun = [] { current_ptr = &self_menu; },
		.enable_if = if_user
	},
	{
		.par = 3,
		.name = "Пользователи...",
		.fun = [] { current_ptr = &user_menu; },
		.enable_if = if_admin
	},
}
	.name("\t--Главное меню--")
	.on_incorrect(incorrect),

tour_menu = Menu<uint32_t, void()>
{
	{
		.par = 0,
		.name = "Назад...",
		.fun = [] { current_ptr = &main_menu; }
	},
	{
		.par = 1,
		.name = "Просмотреть информацию по туру",
		.fun = [] 
		{ 
			for (;;) try
			{
				auto id = input<uint64_t>("Выберите id тура (0 для отмены): ");
				if (id == 0) return;
				try 
				{ 
					cout << *global_vars::tours.at(id); 
					if (global_vars::current_user != global_vars::users.end() and
						global_vars::current_user->second->is_admin())
						if (not global_vars::tours.at(id)->reserves().empty()) [[likely]]
							for (println("Список броней:"); auto [id, res]: global_vars::tours.at(id)->reserves())
								try {println("\tID = {}: {} человек(а), {}; Контакты: {}", 
										id, res.count, res.accept ? "Подтвеждён"sv : "Ожидает подтверждения"sv, global_vars::users.at(id)->phone());}
								catch (out_of_range &) { }
						else println("Список броней пуст.");
				}
				catch (std::out_of_range &) { println(clog, "Ошибка: нет такого тура"); }
				return;
			}
			catch (logic_error &x) { println(clog, "Ошибка: {}", x.what()); }
			unreachable();
		}
	},
	{
		.par = 2,
		.name = "Просмотреть список туров",
		.fun = []
		{ 
			using Par = pair<uint64_t, const Tour*>;

			constexpr static auto by_id = [] (Par, Par) noexcept -> bool
				{ unreachable(); }; 

			
			const static auto sort_par_menu = 
				Menu<uint32_t, auto () noexcept -> auto (*) (Par, Par) noexcept -> bool>
			{
				{
					.par = 0,
					.name = "По id",
					.fun = [] () noexcept { return by_id; }
				},
				{
					.par = 1,
					.name = "По названию",
					.fun = [] () noexcept
						{ return [] (Par f, Par s) noexcept { return f.second->my_name() < s.second->my_name(); }; }
				},
				{
					.par = 2,
					.name = "По длине поездки",
					.fun = [] () noexcept
						{ return [] (Par f, Par s) noexcept 
						{ 
							return f.second->my_events().begin()->first 
							- prev(f.second->my_events().end())->first >= 
							s.second->my_events().begin()->first
							- prev(s.second->my_events().end())->first;
						}; }
				},
				{
					.par = 3,
					.name = "По времени начала поездки",
					.fun = [] () noexcept
						{ return [] (Par f, Par s) noexcept 
						{ 
							return f.second->my_events().begin()->first < 
							s.second->my_events().begin()->first;
						}; }
				},
				{
					.par = 4,
					.name = "По времени конца поездки",
					.fun = [] () noexcept
						{ return [] (Par f, Par s) noexcept 
						{ 
							return prev(f.second->my_events().end())->first < 
							prev(s.second->my_events().end())->first;
						}; }
				},
			}.name("Сортировать по..."sv)
				.on_incorrect([] () noexcept { return nullptr; });
			
			enum class Order : uint8_t { asc, des, inc };

			const static auto sort_ord_menu = Menu<uint32_t, Order() noexcept>
			{
				{
					.par = 1,
					.name = "По возрастанию",
					.fun = [] () noexcept { return Order::asc; }
				},
				{
					.par = 2,
					.name = "По убыванию",
					.fun = [] () noexcept { return Order::des; }
				},
			}.name("Сортировать в порядке..."sv).on_incorrect([] () noexcept { return Order::inc; });
			
			enum class Filter : uint8_t { id_up = 1, id_down = 2, len_up = 4, len_down = 8,
			beg_up = 16, beg_down = 32, end_up = 64, end_down = 128} 
			filter_by = (Filter)0;
			struct filter_stop { };
			struct filter_err { };

			const auto filter_menu = Menu<uint32_t, void()>
			{
				{
					.par = 0,
					.name = "Продолжить...",
					.fun = [] { throw filter_stop{}; },
				},
				{
					.par = 1,
					.name = "По верхнему значению id",
					.fun = [&filter_by] { (uint8_t &)filter_by or_eq (uint8_t)Filter::id_up; },
					.enable_if = [&filter_by] () noexcept { return not ((uint8_t)Filter::id_up bitand (uint8_t)filter_by); }
				},
				{
					.par = 2,
					.name = "По нижнему значению id",
					.fun = [&filter_by] { (uint8_t &)filter_by |= (uint8_t)Filter::id_down; },
					.enable_if = [&filter_by] () noexcept { return not ((uint8_t)Filter::id_down bitand (uint8_t)filter_by); }
				},
				{
					.par = 3,
					.name = "По верхнему значению начала",
					.fun = [&filter_by] { (uint8_t &)filter_by |= (uint8_t)Filter::beg_up; },
					.enable_if = [&filter_by] () noexcept { return not ((uint8_t)Filter::beg_up bitand (uint8_t)filter_by); }
				},
				{
					.par = 4,
					.name = "По нижнему значению начала",
					.fun = [&filter_by] { (uint8_t &)filter_by |= (uint8_t)Filter::beg_down; },
					.enable_if = [&filter_by] () noexcept { return not ((uint8_t)Filter::beg_down bitand (uint8_t)filter_by); }
				},
				{
					.par = 5,
					.name = "По верхнему значению конца",
					.fun = [&filter_by] { (uint8_t &)filter_by |= (uint8_t)Filter::end_up; },
					.enable_if = [&filter_by] () noexcept { return not ((uint8_t)Filter::end_up bitand (uint8_t)filter_by); }
				},
				{
					.par = 6,
					.name = "По нижнему значению конца",
					.fun = [&filter_by] { (uint8_t &)filter_by |= (uint8_t)Filter::end_down; },
					.enable_if = [&filter_by] () noexcept { return not ((uint8_t)Filter::end_down bitand (uint8_t)filter_by); }
				},
			}.name("Фильтровать по..."sv).on_incorrect([] { throw filter_err{}; });

			auto users = vector<Par>
				(from_range, global_vars::tours | 
				 std::views::transform([] (auto const &user) 
									   { return Par(user.first, user.second.get()); }));
			
			bool (*by_what) (Par, Par) noexcept;
			Order in_order;
			for (;;)
			{
				by_what = sort_par_menu();
				in_order = sort_ord_menu();
				if (not by_what or in_order == Order::inc) [[unlikely]]
				{
					println(clog, "Ошибка: нет этого параметра меню");
					continue;
				}
				break;
			}
			
			if (in_order == Order::asc)
				if (by_what != by_id) [[likely]] std::stable_sort(users.begin(), users.end(), by_what);
				else;
			else 
				if (by_what == by_id) users = { users.rbegin(), users.rend() };
				else [[likely]] std::stable_sort(users.rbegin(), users.rend(), by_what);

			for (;;) try
			{
				filter_menu();
			}
			catch (filter_stop)
			{
				break;
			}
			catch (filter_err)
			{
				println(clog, "Ошибка: нет этого параметра меню");
			}


			if ((uint8_t)Filter::id_up bitand (uint8_t)filter_by) for (;;) try
			{
				users = { from_range, users | views::filter([_id = input<uint64_t>("Введите верхний предел id: ")] (Par &u) 
						{ return u.first < _id; }) };
				break;
			} 
			catch (logic_error &err)
			{
				println(clog, "Ошибка: {}", err.what());
			}

			if ((uint8_t)Filter::id_down bitand (uint8_t)filter_by) for (;;) try
			{
				users = { from_range, users | views::filter([_id = input<uint64_t>("Введите нижний предел id: ")] (Par &u) 
						{ return u.first >= _id; }) };
				break;
			} 
			catch (logic_error &err)
			{
				println(clog, "Ошибка: {}", err.what());
			}

			if ((uint8_t)Filter::beg_up bitand (uint8_t)filter_by) for (;;) try
			{
				users = { from_range, users | views::filter([_t = enter_time("Введите верхний предел времени начала: ")] (Par &u) 
						{ return u.second->my_events().begin()->first < _t; }) };
				break;
			} 
			catch (logic_error &err)
			{
				println(clog, "Ошибка: {}", err.what());
			}

			if ((uint8_t)Filter::beg_down bitand (uint8_t)filter_by) for (;;) try
			{
				users = { from_range, users | views::filter([_t = enter_time("Введите нижний предел времени начала: ")] (Par &u) 
						{ return u.second->my_events().begin()->first >= _t; }) };
				break;
			} 
			catch (logic_error &err)
			{
				println(clog, "Ошибка: {}", err.what());
			}

			if ((uint8_t)Filter::end_up bitand (uint8_t)filter_by) for (;;) try
			{
				users = { from_range, users | views::filter([_t = enter_time("Введите верхний предел времени конца: ")] (Par &u) 
						{ return prev(u.second->my_events().end())->first < _t; }) };
				break;
			} 
			catch (logic_error &err)
			{
				println(clog, "Ошибка: {}", err.what());
			}

			if ((uint8_t)Filter::end_down bitand (uint8_t)filter_by) for (;;) try
			{
				users = { from_range, users | views::filter([_t = enter_time("Введите нижний предел времени конца: ")] (Par &u) 
						{ return prev(u.second->my_events().end())->first >= _t; }) };
				break;
			} 
			catch (logic_error &err)
			{
				println(clog, "Ошибка: {}", err.what());
			}

			table_out(users);
		}
	},
	{
		.par = 3,
		.name = "Заказать тур",
		.fun = [] 
		{ 
			try
			{
				uint64_t id = input<uint64_t>("Введите id тура: ");
				if (not global_vars::tours.contains(id)) throw logic_error("Нет этого тура");
				uint8_t count = input<uint8_t>("Введите число резервируемых мест: ");
				global_vars::tours.at(id)->reserve(count, global_vars::current_user->first);
				println("Вы заказали {} мест в туре {:?}. Ожидайте пока с вами свяжутся.", count, global_vars::tours.at(id)->my_name());
			}
			catch (logic_error &x)
			{
				println(clog, "Ошибка: {}", x.what());
			}
		},
		.enable_if = if_not_admin ////////////////////
	},
	{
		.par = 4,
		.name = "Подтвердить/отклонить заказ тура",
		.fun = [] 
		{
			try
			{
				uint8_t yes = input<uint8_t>("(1 = подтвердить/0 = отклонить): ");
				if (yes > 1) throw logic_error("Нужно было ввести 0 или 1");
				uint64_t
					id_tour = input<uint64_t>("Введите id тура (0 для отмены): "),
					id_user = input<uint64_t>("Введите id пользователя (0 для отмены): ");
				if (not global_vars::tours.contains(id_tour)) throw logic_error("Нет этого тура");
				Tour &tour = *global_vars::tours.at(id_tour);
				if (not tour.reserves().contains(id_user)) throw logic_error("Этот пользователь не резервировал тур");
				if (yes) tour.accept(id_user);
				else tour.unreserve(id_user);
			}
			catch (logic_error &x) { println(clog, "Ошибка: {}", x.what()); }
		},
		.enable_if = if_admin ///////////////////////////////
	},
	{
		.par = 5,
		.name = "Удалить тур",
		.fun = []
		{
			try
			{
				auto id = input<uint64_t>("Введите id тура (0 для отмены): ");
				if (id) 
					if (global_vars::tours.contains(id)) global_vars::users.erase(id);
					else println(clog, "Ошибка: Нет этого тура"); 
			}
			catch (logic_error& x) { println(clog, "Ошибка: {}", x.what()); }
		},
		.enable_if = if_admin
	},
	{
		.par = 6,
		.name = "Добавить тур",
		.fun = [] 
		{  
			uint64_t id;
			try { id = input<uint64_t>("Введите id тура (0 для автоматического выведения): "); }
			catch (logic_error &x) { return println(clog, "Ошибка: {}", x.what()); }
			decltype(global_vars::tours)::iterator where;
			if (not id) [[likely]]
			{
				where =
					global_vars::tours.try_emplace
					(
						global_vars::tours.cend(),
						global_vars::tours.empty() ? 1 : prev(global_vars::tours.cend())->first + 1,
						make_unique<Tour>()
					);
				println("ID: {}", where->first);
			}
			else
			{
				bool did;
				tie(where, did) = global_vars::tours.try_emplace(id, make_unique<Tour>());
				if (not did) [[unlikely]] return println(clog, "Ошибка: {}", "Такой тур уже есть");
				else [[likely]] println("ID: {}", where->first);
			}
			where->second->from_stream();
		},
		.enable_if = if_admin 
	},
	{
		.par = 7,
		.name = "Редактировать тур",
		.fun = [] 
		{
			uint64_t id;
			try { id = input<uint64_t>("Введите id тура: "); }
			catch (logic_error &x) { return println(clog, "Ошибка: {}", x.what()); }
			auto tour = global_vars::tours.find(id);
			if (tour == global_vars::tours.end()) { return println(clog, "Ошибка: {}", "Нет этого тура"); }
			
			enum { Nothing, Insert, Remove, RemoveX, Move, MoveX, Name } what;
			
			auto what_menu = Menu<uint32_t, void()>
			{
				{
					.par = 0,
					.name = "Отмена"sv,
					.fun = [&what] { what = Nothing; }
				},
				{
					.par = 1,
					.name = "Добавить событие"sv,
					.fun = [&what] { what = Insert; }
				},
				{
					.par = 2,
					.name = "Удалить событие"sv,
					.fun = [&what] { what = Remove; }
				},
				{
					.par = 3,
					.name = "Удалить несколько событий"sv,
					.fun = [&what] { what = RemoveX; }
				},
				{
					.par = 4,
					.name = "Передвинуть событие"sv,
					.fun = [&what] { what = Move; }
				},
				{
					.par = 5,
					.name = "Передвинуть события"sv,
					.fun = [&what] { what = MoveX; }
				},
				{
					.par = 6,
					.name = "Переименовать"sv,
					.fun = [&what] { what = Name; }
				},
			}.name("Выберите операцию");
			move(what_menu).on_incorrect([&what_menu] { incorrect(); what_menu(); })();
			
			

			auto &_tour = *tour->second;
			switch (string str; what)
			{
				time_t when, when2;
			return; case Nothing: 
			return; case Insert: 
				for (;;) try { str = input<string>("Введите название события: "); break; }
				catch (exception &x) { println(clog, "Ошибка: {}", x.what()); };
				for (;;) try { _tour.insert_event(enter_time("Введите время/дату события: "), str); break; }
				catch (exception &x) { println(clog, "Ошибка: {}", x.what()); };
			return; case Remove:
				for (;;) try { _tour.remove_event(_tour.my_events().lower_bound(
					enter_time("Введите время/дату события или время сразу после него: "))); break; }
				catch (exception &x) { println(clog, "Ошибка: {}", x.what()); };
			return; case RemoveX:
				for (;;) try 
				{ 
					when = enter_time("Введите время/дату начала диапазона удаления: ");
					when2 = enter_time("Введите время/дату конца диапазона удаления: ");
					_tour.remove_events(next(_tour.my_events().lower_bound(when)),
										next(_tour.my_events().upper_bound(when2))); 
					break; 
				}
				catch (exception &x) { println(clog, "Ошибка: {}", x.what()); };
			return; case Move:
				for (;;) try 
				{ 
					when = enter_time("Введите время/дату перемещаемого события или время сразу после него: ");
					_tour.shift_event_to(_tour.my_events().lower_bound(when),
										 enter_time("Введите новую время/дату (порядок изменять нельзя): "));
					break; 
				}
				catch (exception &x) { println(clog, "Ошибка: {}", x.what()); };
				
			return; case MoveX:
				for (;;) try 
				{ 
					when = enter_time("Введите время/дату начала диапазона перемещения: ");
					when2 = enter_time("Введите время/дату конца диапазона перемещения: ");
					_tour.shift_events_to(next(_tour.my_events().lower_bound(when)),next(_tour.my_events().upper_bound(when)),
										 enter_time("Введите новую время/дату для первого события (порядок изменять нельзя): "));
					break; 
				}
				catch (exception &x) { println(clog, "Ошибка: {}", x.what()); };
			return; case Name:
				for (;;) try { _tour.my_name(input<string>()); break; }
				catch (exception &x) { println(clog, "Ошибка: {}", x.what()); };

			}
		},
		.enable_if = if_admin
	},
	
}
	.name("\t--Туры--")
	.on_incorrect(incorrect),

self_menu = Menu<uint32_t, void()>
{
	{
		.par = 0,
		.name = "Назад...",
		.fun = [] { current_ptr = &main_menu; }
	},
	{
		.par = 1,
		.name = "Удалить себя",
		.fun = []
		{
			for (;;) try
			{
				auto yes = input<uint32_t>("Вы уверены? (1 = да/0 = нет): ");
				if (yes > 1) throw logic_error("");
				if (yes)
				{
					current_ptr = &autorization_menu;
					global_vars::users.erase(global_vars::current_user);
				}
			}
			catch (logic_error) { println(clog, "Ошибка ввода, попробуйте ещё раз. "); }
		},
		.enable_if = if_not_admin
	},
	{
		.par = 2,
		.name = "Изменить имя",
		.fun = []
		{
			try
			{
				global_vars::current_user->second->name(input<string>("Введите новое имя (\"\" для отмены): "));
			}
			catch (invalid_argument) { }
		}
	},
	{
		.par = 3,
		.name = "Изменить контакты",
		.fun = []
		{
			try
			{
				global_vars::current_user->second->phone(input<string>("Введите новые контакты (\"\" для отмены): "));
			}
			catch (invalid_argument) { }
		}
	},
	{
		.par = 4,
		.name = "Изменить пароль",
		.fun = []
		{
			for (;;)
			{
					
				string new_password, check_password; 
				try 
				{ 
					new_password = input_password("Введите новый пароль (\"\" для отмены): "s);
					if (new_password.size() < 8) throw invalid_argument("Пароль должен содержать как минимум 8 символов");
				}
				catch (invalid_argument &x)
				{
					if (new_password == "") return;
					println(clog, "Ошибка: {}", x.what());
					continue;
				}
				try { check_password = input_password("Подтвердите новый пароль (\"\" для отмены): "s); }
				catch (invalid_argument)
				{
					println(clog, "Пароль повторён неверно. Повторите.");
					continue;
				}
				global_vars::current_user->second->set_password(new_password);
				break;
			}
		}
	},
}
	.name("\t--Персонализация--")
	.on_incorrect(incorrect),

user_menu = Menu<uint32_t, void()>
{
	{
		.par = 0,
		.name = "Назад...",
		.fun = [] { current_ptr = &main_menu; }
	},
	{
		.par = 1,
		.name = "Удалить пользователя",
		.fun = []
		{
			for (;;) try
			{
				auto id = input<uint64_t>("Введите id пользователя (не свой): ");
				if (not global_vars::users.contains(id)) 
					{ println(clog, "Ошибка: не было этого пользователя"); return; }
				if (id == global_vars::current_user->first) 
					{ println(clog, "Ошибка: Нельзя удалять себя"); return; }
				auto yes = input<uint32_t>("Вы уверены? (1 = да/0 = нет): ");
				if (yes > 1) throw logic_error("");
				if (yes)
					global_vars::users.erase(id);
				return;
			}
			catch (logic_error) { println(clog, "Ошибка ввода, попробуйте ещё раз. "); }
			unreachable();
		}
	},
	{
		.par = 2,
		.name = "Редактировать пользователя",
		.fun = []
		{
			enum What : uint8_t { Name, Phone, Pass, Cancel } what;
			{
				auto what_menu = Menu<uint32_t, void() noexcept>
				{
					{
						.par = 0,
						.name = "Отмена"sv,
						.fun = [&what] () noexcept { what = Cancel; }
					},
					{
						.par = 1,
						.name = "Имя"sv,
						.fun = [&what] () noexcept { what = Name; }
					},
					{
						.par = 2,
						.name = "Контакты"sv,
						.fun = [&what] () noexcept { what = Phone; }
					},
				}.name("Что редактировать?");
				move(what_menu).on_incorrect([&what_menu] () noexcept { incorrect(); what_menu(); })();
			}
			if (what == Cancel) return;
			uint64_t id;
			for (;;) 
			{
				try { id = input<uint64_t>("Введите id пользователя (0 для автоматического выведения): "); }
				catch (logic_error &x) { println(clog, "Ошибка: {}", x.what()); continue; }
				if (not global_vars::users.contains(id)) { println(clog, "Ошибка: {}", "Нет такого пользователя"sv); continue; }
				break;
			}

			try
			{
				switch (what)
				{
				default: unreachable();
				break; case Name: global_vars::users[id]->name(input<string>("Введите новое имя (\"\" для отмены): "));
				break; case Phone: global_vars::users[id]->phone(input<string>("Введите новые контакты (\"\" для отмены): "));
				}
			}
			catch (invalid_argument) { }
		}
	},
	{
		.par = 3,
		.name = "Зарегистрировать пользователя",
		.fun = []
		{

			uint64_t id;
			try { id = input<uint64_t>("Введите id пользователя (0 для автоматического выведения): "); }
			catch (logic_error &x) { println(clog, "Ошибка: {}", x.what()); return; }
			if (global_vars::users.contains(id)) { println(clog, "Ошибка: {}", "Такой пользователь уже есть"); return; }

			try
			{	
				uint8_t admin;
				try { 
					admin = input<uint8_t>("Введите, добавляется ли администратор (1) или клиент (0): "); 
					if (admin != 0 and admin != 1) throw logic_error("Это не 0 или 1");
				}
				catch (logic_error &x) { println(clog, "Ошибка: {}", x.what()); return; }
				unique_ptr<User> user = make_unique<User>(admin);
				user->
					name(input<string>("Введите имя пользователя (\"\" для отмены): ")).
					phone(input<string>("Введите контактные данные (\"\" для отмены): ")).
					set_password(input_password("Введите пароль (\"\" для отмены): "));

				if (not user->check_password(input_password("Повторите пароль (\"\" для отмены): ")))
					[[unlikely]] println(clog, "Ошибка: неверный пароль.");
				else [[likely]] if (not id) [[likely]]
				{
					auto id =
					global_vars::users.try_emplace
					(
						global_vars::users.cend(),
						prev(global_vars::users.cend())->first + 1,
						move(user)
					)->first;
					println("Регистрация успешна, ваш id : {}", id);
				}
				else
				{
					global_vars::users.try_emplace(id, move(user));
					println("Регистрация успешна");
				}
			}
			catch (std::invalid_argument &) { return; }
		}
	},
	{
		.par = 4,
		.name = "Просмотреть список пользователей",
		.fun = []
		{
			table_out(global_vars::users);
		}
	},
	{
		.par = 5,
		.name = "Заблокировать/разблокировать пользователя",
		.fun = []
		{
			uint64_t id;
			try { id = input<uint64_t>("Введите id пользователя: "); }
			catch (logic_error& x)
			{ println(clog, "Ошибка: {}", x.what()); return; }
			User* user;
			try { user = global_vars::users.at(id).get();}
			catch (std::out_of_range) { println("Ошибка: нет такого пользователя"); return; }
			try { user->is_blocked(not user->is_blocked()); }
			catch (std::exception) { println("Ошибка: это действие нельзя делать с администратором"); return; }
		}
	},
}
	.name("\t--Пользователи--")
	.on_incorrect(incorrect);