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
	println(clog, "������: ��� ������ ������!");
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
		println(clog, "������: {}", x.what());
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
		.name = "����� �� ���������",
		.fun = bind(exit, 0),
	},
	{
		.par = 1,
		.name = "��������������",
		.fun = []
		{
			decltype(global_vars::current_user) user;
			for (;;) try
			{
				auto id = input<uint64_t>("������� ��� id (0 ��� ������): "sv);
				if (id == 0) return;
				user = global_vars::users.find(id);
				if (user == global_vars::users.end()) throw logic_error("��� ������ ������������");
				if (user->second->is_blocked()) throw logic_error("���� ������������ ������������");
				break;
			}
			catch (std::logic_error &x)
			{
				println(clog, "������: {}", x.what());
			}
			for (;;) try
			{
				auto str = input_password("������� ������ (\"\" ��� ������): ");
				if (user->second->check_password(str)) [[likely]] break;
				println(clog, "������: ������������ ������");
			}
			catch (std::invalid_argument &)
			{
				return;
			}
			global_vars::current_user = user;
			println("�� ������� ����� � �������!");
			current_ptr = &main_menu;
		}
	},
	{
		.par = 2,
		.name = "������������������",
		.fun = []
		{
			try
			{
				unique_ptr<User> user = make_unique<User>();
				user->
					name(input<string>("������� ��� ������������ (\"\" ��� ������): ")).
					phone(input<string>("������� ���������� ������ (\"\" ��� ������): ")).
					set_password(input_password("������� ������ (\"\" ��� ������): ")).
					is_blocked(true);

				if (not user->check_password(input_password("��������� ������ (\"\" ��� ������): ")))
					[[unlikely]] println(clog, "������: �������� ������.");
				else [[likely]]
				{
					auto id = 
					global_vars::users.try_emplace
					(
						global_vars::users.cend(),
						prev(global_vars::users.cend())->first + 1,
						move(user)
					)->first;
					println(clog, "����������� �������, ��� id: {}", id);
					global_vars::save();
				}
			}
			catch (std::invalid_argument &) { return; }
		}
	},
	{
		.par = 3,
		.name = "���������� ��� �����������",
		.fun = [] { current_ptr = &main_menu; global_vars::current_user = global_vars::users.end(); },
	},
}
	.name("\t������� � �������")
	.on_incorrect(incorrect),

main_menu = Menu<uint32_t, void()>
{
	{
		.par = 0,
		.name = "���� �����������...",
		.fun = [] { if (global_vars::current_user != global_vars::users.end()) global_vars::save(); current_ptr = &autorization_menu; }
	},
	{
		.par = 1,
		.name = "����...",
		.fun = [] { current_ptr = &tour_menu; }
	},
	{
		.par = 2,
		.name = "��������������...",
		.fun = [] { current_ptr = &self_menu; },
		.enable_if = if_user
	},
	{
		.par = 3,
		.name = "������������...",
		.fun = [] { current_ptr = &user_menu; },
		.enable_if = if_admin
	},
}
	.name("\t--������� ����--")
	.on_incorrect(incorrect),

tour_menu = Menu<uint32_t, void()>
{
	{
		.par = 0,
		.name = "�����...",
		.fun = [] { current_ptr = &main_menu; }
	},
	{
		.par = 1,
		.name = "����������� ���������� �� ����",
		.fun = [] 
		{ 
			for (;;) try
			{
				auto id = input<uint64_t>("�������� id ���� (0 ��� ������): ");
				if (id == 0) return;
				try 
				{ 
					cout << *global_vars::tours.at(id); 
					if (global_vars::current_user != global_vars::users.end() and
						global_vars::current_user->second->is_admin())
						if (not global_vars::tours.at(id)->reserves().empty()) [[likely]]
							for (println("������ ������:"); auto [id, res]: global_vars::tours.at(id)->reserves())
								try {println("\tID = {}: {} �������(�), {}; ��������: {}", 
										id, res.count, res.accept ? "���������"sv : "������� �������������"sv, global_vars::users.at(id)->phone());}
								catch (out_of_range &) { }
						else println("������ ������ ����.");
				}
				catch (std::out_of_range &) { println(clog, "������: ��� ������ ����"); }
				return;
			}
			catch (logic_error &x) { println(clog, "������: {}", x.what()); }
			unreachable();
		}
	},
	{
		.par = 2,
		.name = "����������� ������ �����",
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
					.name = "�� id",
					.fun = [] () noexcept { return by_id; }
				},
				{
					.par = 1,
					.name = "�� ��������",
					.fun = [] () noexcept
						{ return [] (Par f, Par s) noexcept { return f.second->my_name() < s.second->my_name(); }; }
				},
				{
					.par = 2,
					.name = "�� ����� �������",
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
					.name = "�� ������� ������ �������",
					.fun = [] () noexcept
						{ return [] (Par f, Par s) noexcept 
						{ 
							return f.second->my_events().begin()->first < 
							s.second->my_events().begin()->first;
						}; }
				},
				{
					.par = 4,
					.name = "�� ������� ����� �������",
					.fun = [] () noexcept
						{ return [] (Par f, Par s) noexcept 
						{ 
							return prev(f.second->my_events().end())->first < 
							prev(s.second->my_events().end())->first;
						}; }
				},
			}.name("����������� ��..."sv)
				.on_incorrect([] () noexcept { return nullptr; });
			
			enum class Order : uint8_t { asc, des, inc };

			const static auto sort_ord_menu = Menu<uint32_t, Order() noexcept>
			{
				{
					.par = 1,
					.name = "�� �����������",
					.fun = [] () noexcept { return Order::asc; }
				},
				{
					.par = 2,
					.name = "�� ��������",
					.fun = [] () noexcept { return Order::des; }
				},
			}.name("����������� � �������..."sv).on_incorrect([] () noexcept { return Order::inc; });
			
			enum class Filter : uint8_t { id_up = 1, id_down = 2, len_up = 4, len_down = 8,
			beg_up = 16, beg_down = 32, end_up = 64, end_down = 128} 
			filter_by = (Filter)0;
			struct filter_stop { };
			struct filter_err { };

			const auto filter_menu = Menu<uint32_t, void()>
			{
				{
					.par = 0,
					.name = "����������...",
					.fun = [] { throw filter_stop{}; },
				},
				{
					.par = 1,
					.name = "�� �������� �������� id",
					.fun = [&filter_by] { (uint8_t &)filter_by or_eq (uint8_t)Filter::id_up; },
					.enable_if = [&filter_by] () noexcept { return not ((uint8_t)Filter::id_up bitand (uint8_t)filter_by); }
				},
				{
					.par = 2,
					.name = "�� ������� �������� id",
					.fun = [&filter_by] { (uint8_t &)filter_by |= (uint8_t)Filter::id_down; },
					.enable_if = [&filter_by] () noexcept { return not ((uint8_t)Filter::id_down bitand (uint8_t)filter_by); }
				},
				{
					.par = 3,
					.name = "�� �������� �������� ������",
					.fun = [&filter_by] { (uint8_t &)filter_by |= (uint8_t)Filter::beg_up; },
					.enable_if = [&filter_by] () noexcept { return not ((uint8_t)Filter::beg_up bitand (uint8_t)filter_by); }
				},
				{
					.par = 4,
					.name = "�� ������� �������� ������",
					.fun = [&filter_by] { (uint8_t &)filter_by |= (uint8_t)Filter::beg_down; },
					.enable_if = [&filter_by] () noexcept { return not ((uint8_t)Filter::beg_down bitand (uint8_t)filter_by); }
				},
				{
					.par = 5,
					.name = "�� �������� �������� �����",
					.fun = [&filter_by] { (uint8_t &)filter_by |= (uint8_t)Filter::end_up; },
					.enable_if = [&filter_by] () noexcept { return not ((uint8_t)Filter::end_up bitand (uint8_t)filter_by); }
				},
				{
					.par = 6,
					.name = "�� ������� �������� �����",
					.fun = [&filter_by] { (uint8_t &)filter_by |= (uint8_t)Filter::end_down; },
					.enable_if = [&filter_by] () noexcept { return not ((uint8_t)Filter::end_down bitand (uint8_t)filter_by); }
				},
			}.name("����������� ��..."sv).on_incorrect([] { throw filter_err{}; });

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
					println(clog, "������: ��� ����� ��������� ����");
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
				println(clog, "������: ��� ����� ��������� ����");
			}


			if ((uint8_t)Filter::id_up bitand (uint8_t)filter_by) for (;;) try
			{
				users = { from_range, users | views::filter([_id = input<uint64_t>("������� ������� ������ id: ")] (Par &u) 
						{ return u.first < _id; }) };
				break;
			} 
			catch (logic_error &err)
			{
				println(clog, "������: {}", err.what());
			}

			if ((uint8_t)Filter::id_down bitand (uint8_t)filter_by) for (;;) try
			{
				users = { from_range, users | views::filter([_id = input<uint64_t>("������� ������ ������ id: ")] (Par &u) 
						{ return u.first >= _id; }) };
				break;
			} 
			catch (logic_error &err)
			{
				println(clog, "������: {}", err.what());
			}

			if ((uint8_t)Filter::beg_up bitand (uint8_t)filter_by) for (;;) try
			{
				users = { from_range, users | views::filter([_t = enter_time("������� ������� ������ ������� ������: ")] (Par &u) 
						{ return u.second->my_events().begin()->first < _t; }) };
				break;
			} 
			catch (logic_error &err)
			{
				println(clog, "������: {}", err.what());
			}

			if ((uint8_t)Filter::beg_down bitand (uint8_t)filter_by) for (;;) try
			{
				users = { from_range, users | views::filter([_t = enter_time("������� ������ ������ ������� ������: ")] (Par &u) 
						{ return u.second->my_events().begin()->first >= _t; }) };
				break;
			} 
			catch (logic_error &err)
			{
				println(clog, "������: {}", err.what());
			}

			if ((uint8_t)Filter::end_up bitand (uint8_t)filter_by) for (;;) try
			{
				users = { from_range, users | views::filter([_t = enter_time("������� ������� ������ ������� �����: ")] (Par &u) 
						{ return prev(u.second->my_events().end())->first < _t; }) };
				break;
			} 
			catch (logic_error &err)
			{
				println(clog, "������: {}", err.what());
			}

			if ((uint8_t)Filter::end_down bitand (uint8_t)filter_by) for (;;) try
			{
				users = { from_range, users | views::filter([_t = enter_time("������� ������ ������ ������� �����: ")] (Par &u) 
						{ return prev(u.second->my_events().end())->first >= _t; }) };
				break;
			} 
			catch (logic_error &err)
			{
				println(clog, "������: {}", err.what());
			}

			table_out(users);
		}
	},
	{
		.par = 3,
		.name = "�������� ���",
		.fun = [] 
		{ 
			try
			{
				uint64_t id = input<uint64_t>("������� id ����: ");
				if (not global_vars::tours.contains(id)) throw logic_error("��� ����� ����");
				uint8_t count = input<uint8_t>("������� ����� ������������� ����: ");
				global_vars::tours.at(id)->reserve(count, global_vars::current_user->first);
				println("�� �������� {} ���� � ���� {:?}. �������� ���� � ���� ��������.", count, global_vars::tours.at(id)->my_name());
			}
			catch (logic_error &x)
			{
				println(clog, "������: {}", x.what());
			}
		},
		.enable_if = if_not_admin ////////////////////
	},
	{
		.par = 4,
		.name = "�����������/��������� ����� ����",
		.fun = [] 
		{
			try
			{
				uint8_t yes = input<uint8_t>("(1 = �����������/0 = ���������): ");
				if (yes > 1) throw logic_error("����� ���� ������ 0 ��� 1");
				uint64_t
					id_tour = input<uint64_t>("������� id ���� (0 ��� ������): "),
					id_user = input<uint64_t>("������� id ������������ (0 ��� ������): ");
				if (not global_vars::tours.contains(id_tour)) throw logic_error("��� ����� ����");
				Tour &tour = *global_vars::tours.at(id_tour);
				if (not tour.reserves().contains(id_user)) throw logic_error("���� ������������ �� ������������ ���");
				if (yes) tour.accept(id_user);
				else tour.unreserve(id_user);
			}
			catch (logic_error &x) { println(clog, "������: {}", x.what()); }
		},
		.enable_if = if_admin ///////////////////////////////
	},
	{
		.par = 5,
		.name = "������� ���",
		.fun = []
		{
			try
			{
				auto id = input<uint64_t>("������� id ���� (0 ��� ������): ");
				if (id) 
					if (global_vars::tours.contains(id)) global_vars::users.erase(id);
					else println(clog, "������: ��� ����� ����"); 
			}
			catch (logic_error& x) { println(clog, "������: {}", x.what()); }
		},
		.enable_if = if_admin
	},
	{
		.par = 6,
		.name = "�������� ���",
		.fun = [] 
		{  
			uint64_t id;
			try { id = input<uint64_t>("������� id ���� (0 ��� ��������������� ���������): "); }
			catch (logic_error &x) { return println(clog, "������: {}", x.what()); }
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
				if (not did) [[unlikely]] return println(clog, "������: {}", "����� ��� ��� ����");
				else [[likely]] println("ID: {}", where->first);
			}
			where->second->from_stream();
		},
		.enable_if = if_admin 
	},
	{
		.par = 7,
		.name = "������������� ���",
		.fun = [] 
		{
			uint64_t id;
			try { id = input<uint64_t>("������� id ����: "); }
			catch (logic_error &x) { return println(clog, "������: {}", x.what()); }
			auto tour = global_vars::tours.find(id);
			if (tour == global_vars::tours.end()) { return println(clog, "������: {}", "��� ����� ����"); }
			
			enum { Nothing, Insert, Remove, RemoveX, Move, MoveX, Name } what;
			
			auto what_menu = Menu<uint32_t, void()>
			{
				{
					.par = 0,
					.name = "������"sv,
					.fun = [&what] { what = Nothing; }
				},
				{
					.par = 1,
					.name = "�������� �������"sv,
					.fun = [&what] { what = Insert; }
				},
				{
					.par = 2,
					.name = "������� �������"sv,
					.fun = [&what] { what = Remove; }
				},
				{
					.par = 3,
					.name = "������� ��������� �������"sv,
					.fun = [&what] { what = RemoveX; }
				},
				{
					.par = 4,
					.name = "����������� �������"sv,
					.fun = [&what] { what = Move; }
				},
				{
					.par = 5,
					.name = "����������� �������"sv,
					.fun = [&what] { what = MoveX; }
				},
				{
					.par = 6,
					.name = "�������������"sv,
					.fun = [&what] { what = Name; }
				},
			}.name("�������� ��������");
			move(what_menu).on_incorrect([&what_menu] { incorrect(); what_menu(); })();
			
			

			auto &_tour = *tour->second;
			switch (string str; what)
			{
				time_t when, when2;
			return; case Nothing: 
			return; case Insert: 
				for (;;) try { str = input<string>("������� �������� �������: "); break; }
				catch (exception &x) { println(clog, "������: {}", x.what()); };
				for (;;) try { _tour.insert_event(enter_time("������� �����/���� �������: "), str); break; }
				catch (exception &x) { println(clog, "������: {}", x.what()); };
			return; case Remove:
				for (;;) try { _tour.remove_event(_tour.my_events().lower_bound(
					enter_time("������� �����/���� ������� ��� ����� ����� ����� ����: "))); break; }
				catch (exception &x) { println(clog, "������: {}", x.what()); };
			return; case RemoveX:
				for (;;) try 
				{ 
					when = enter_time("������� �����/���� ������ ��������� ��������: ");
					when2 = enter_time("������� �����/���� ����� ��������� ��������: ");
					_tour.remove_events(next(_tour.my_events().lower_bound(when)),
										next(_tour.my_events().upper_bound(when2))); 
					break; 
				}
				catch (exception &x) { println(clog, "������: {}", x.what()); };
			return; case Move:
				for (;;) try 
				{ 
					when = enter_time("������� �����/���� ������������� ������� ��� ����� ����� ����� ����: ");
					_tour.shift_event_to(_tour.my_events().lower_bound(when),
										 enter_time("������� ����� �����/���� (������� �������� ������): "));
					break; 
				}
				catch (exception &x) { println(clog, "������: {}", x.what()); };
				
			return; case MoveX:
				for (;;) try 
				{ 
					when = enter_time("������� �����/���� ������ ��������� �����������: ");
					when2 = enter_time("������� �����/���� ����� ��������� �����������: ");
					_tour.shift_events_to(next(_tour.my_events().lower_bound(when)),next(_tour.my_events().upper_bound(when)),
										 enter_time("������� ����� �����/���� ��� ������� ������� (������� �������� ������): "));
					break; 
				}
				catch (exception &x) { println(clog, "������: {}", x.what()); };
			return; case Name:
				for (;;) try { _tour.my_name(input<string>()); break; }
				catch (exception &x) { println(clog, "������: {}", x.what()); };

			}
		},
		.enable_if = if_admin
	},
	
}
	.name("\t--����--")
	.on_incorrect(incorrect),

self_menu = Menu<uint32_t, void()>
{
	{
		.par = 0,
		.name = "�����...",
		.fun = [] { current_ptr = &main_menu; }
	},
	{
		.par = 1,
		.name = "������� ����",
		.fun = []
		{
			for (;;) try
			{
				auto yes = input<uint32_t>("�� �������? (1 = ��/0 = ���): ");
				if (yes > 1) throw logic_error("");
				if (yes)
				{
					current_ptr = &autorization_menu;
					global_vars::users.erase(global_vars::current_user);
				}
			}
			catch (logic_error) { println(clog, "������ �����, ���������� ��� ���. "); }
		},
		.enable_if = if_not_admin
	},
	{
		.par = 2,
		.name = "�������� ���",
		.fun = []
		{
			try
			{
				global_vars::current_user->second->name(input<string>("������� ����� ��� (\"\" ��� ������): "));
			}
			catch (invalid_argument) { }
		}
	},
	{
		.par = 3,
		.name = "�������� ��������",
		.fun = []
		{
			try
			{
				global_vars::current_user->second->phone(input<string>("������� ����� �������� (\"\" ��� ������): "));
			}
			catch (invalid_argument) { }
		}
	},
	{
		.par = 4,
		.name = "�������� ������",
		.fun = []
		{
			for (;;)
			{
					
				string new_password, check_password; 
				try 
				{ 
					new_password = input_password("������� ����� ������ (\"\" ��� ������): "s);
					if (new_password.size() < 8) throw invalid_argument("������ ������ ��������� ��� ������� 8 ��������");
				}
				catch (invalid_argument &x)
				{
					if (new_password == "") return;
					println(clog, "������: {}", x.what());
					continue;
				}
				try { check_password = input_password("����������� ����� ������ (\"\" ��� ������): "s); }
				catch (invalid_argument)
				{
					println(clog, "������ ������� �������. ���������.");
					continue;
				}
				global_vars::current_user->second->set_password(new_password);
				break;
			}
		}
	},
}
	.name("\t--��������������--")
	.on_incorrect(incorrect),

user_menu = Menu<uint32_t, void()>
{
	{
		.par = 0,
		.name = "�����...",
		.fun = [] { current_ptr = &main_menu; }
	},
	{
		.par = 1,
		.name = "������� ������������",
		.fun = []
		{
			for (;;) try
			{
				auto id = input<uint64_t>("������� id ������������ (�� ����): ");
				if (not global_vars::users.contains(id)) 
					{ println(clog, "������: �� ���� ����� ������������"); return; }
				if (id == global_vars::current_user->first) 
					{ println(clog, "������: ������ ������� ����"); return; }
				auto yes = input<uint32_t>("�� �������? (1 = ��/0 = ���): ");
				if (yes > 1) throw logic_error("");
				if (yes)
					global_vars::users.erase(id);
				return;
			}
			catch (logic_error) { println(clog, "������ �����, ���������� ��� ���. "); }
			unreachable();
		}
	},
	{
		.par = 2,
		.name = "������������� ������������",
		.fun = []
		{
			enum What : uint8_t { Name, Phone, Pass, Cancel } what;
			{
				auto what_menu = Menu<uint32_t, void() noexcept>
				{
					{
						.par = 0,
						.name = "������"sv,
						.fun = [&what] () noexcept { what = Cancel; }
					},
					{
						.par = 1,
						.name = "���"sv,
						.fun = [&what] () noexcept { what = Name; }
					},
					{
						.par = 2,
						.name = "��������"sv,
						.fun = [&what] () noexcept { what = Phone; }
					},
				}.name("��� �������������?");
				move(what_menu).on_incorrect([&what_menu] () noexcept { incorrect(); what_menu(); })();
			}
			if (what == Cancel) return;
			uint64_t id;
			for (;;) 
			{
				try { id = input<uint64_t>("������� id ������������ (0 ��� ��������������� ���������): "); }
				catch (logic_error &x) { println(clog, "������: {}", x.what()); continue; }
				if (not global_vars::users.contains(id)) { println(clog, "������: {}", "��� ������ ������������"sv); continue; }
				break;
			}

			try
			{
				switch (what)
				{
				default: unreachable();
				break; case Name: global_vars::users[id]->name(input<string>("������� ����� ��� (\"\" ��� ������): "));
				break; case Phone: global_vars::users[id]->phone(input<string>("������� ����� �������� (\"\" ��� ������): "));
				}
			}
			catch (invalid_argument) { }
		}
	},
	{
		.par = 3,
		.name = "���������������� ������������",
		.fun = []
		{

			uint64_t id;
			try { id = input<uint64_t>("������� id ������������ (0 ��� ��������������� ���������): "); }
			catch (logic_error &x) { println(clog, "������: {}", x.what()); return; }
			if (global_vars::users.contains(id)) { println(clog, "������: {}", "����� ������������ ��� ����"); return; }

			try
			{	
				uint8_t admin;
				try { 
					admin = input<uint8_t>("�������, ����������� �� ������������� (1) ��� ������ (0): "); 
					if (admin != 0 and admin != 1) throw logic_error("��� �� 0 ��� 1");
				}
				catch (logic_error &x) { println(clog, "������: {}", x.what()); return; }
				unique_ptr<User> user = make_unique<User>(admin);
				user->
					name(input<string>("������� ��� ������������ (\"\" ��� ������): ")).
					phone(input<string>("������� ���������� ������ (\"\" ��� ������): ")).
					set_password(input_password("������� ������ (\"\" ��� ������): "));

				if (not user->check_password(input_password("��������� ������ (\"\" ��� ������): ")))
					[[unlikely]] println(clog, "������: �������� ������.");
				else [[likely]] if (not id) [[likely]]
				{
					auto id =
					global_vars::users.try_emplace
					(
						global_vars::users.cend(),
						prev(global_vars::users.cend())->first + 1,
						move(user)
					)->first;
					println("����������� �������, ��� id : {}", id);
				}
				else
				{
					global_vars::users.try_emplace(id, move(user));
					println("����������� �������");
				}
			}
			catch (std::invalid_argument &) { return; }
		}
	},
	{
		.par = 4,
		.name = "����������� ������ �������������",
		.fun = []
		{
			table_out(global_vars::users);
		}
	},
	{
		.par = 5,
		.name = "�������������/�������������� ������������",
		.fun = []
		{
			uint64_t id;
			try { id = input<uint64_t>("������� id ������������: "); }
			catch (logic_error& x)
			{ println(clog, "������: {}", x.what()); return; }
			User* user;
			try { user = global_vars::users.at(id).get();}
			catch (std::out_of_range) { println("������: ��� ������ ������������"); return; }
			try { user->is_blocked(not user->is_blocked()); }
			catch (std::exception) { println("������: ��� �������� ������ ������ � ���������������"); return; }
		}
	},
}
	.name("\t--������������--")
	.on_incorrect(incorrect);