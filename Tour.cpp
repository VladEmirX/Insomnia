module;
#include <ctime>
module Tour;
import input;
import <format>;
import <iostream>;
import <chrono>;
using namespace std;

bool operator==(Tour::Reservation f, Tour::Reservation s) noexcept
{ 
	return f.accept == s.accept and f.count == s.count;
}

static const time_t time_diff = []() noexcept //Разница между часовыми поясами
{
	time_t local{1000000000};
	tm tm;
	gmtime_s(&tm, &local);
	time_t utc = mktime(&tm);
	return local - utc;
}();

std::string time_to_str(time_t time)
{
	return format("{:%R/%d.%m.%y}", chrono::system_clock::from_time_t(time + time_diff));
}

time_t str_to_time(std::string_view str, time_t relative) try
{
	istringstream ostr((string)str);
	tm tm;
	localtime_s(&tm, &relative);
	ostr >> std::get_time(&tm, "%R%n/%n%d%n.%n%m%n.%n%y%n");
	if (not ostr.eof()) throw invalid_argument{ "Ввод времени неправильный." };
	return mktime(&tm);
}
catch (std::ios::failure const &)
{
	throw invalid_argument{ "" };
}

void Tour::from_stream(std::istream &in)
{ 
	auto events_end = events.cend();
	name = input<string>("Введите название тура: ", in);
	for (;;)
	{
		auto last_time = [&e = this->events]() noexcept
			{
				if (e.empty()) [[unlikely]]
					return time(0);
				else [[likely]] return (--e.cend())->first;
			};
		const auto time_text = 
			format("Введите время/дату начала события: "
				   "\33""7""\33[90m"
				   "00:00/{:%d.%m.%y}"
				   "\33[39m""\33""8", chrono::current_zone()->to_local(chrono::system_clock::from_time_t(last_time())));;
		string event_name;
		time_t event_time;
		try
		{
			event_name = input<string>("Введите название события (или \"Конец\"): "sv);
			event_time = str_to_time(input<string>(time_text), last_time());
		}
		catch (std::exception &x)
		{
			clog << "Ошибка: " << x.what() << '\n';
			continue;
		}

		if (last_time() >= event_time)
		{
			cerr << "Событие введено не по очереди\n";
			continue;
		}
		events.emplace_hint(events.cend(), event_time, event_name);

		if (event_name == "Конец"sv) [[unlikely]]
		{
			cout << endl;
			return;
		}
	}
	unreachable();
}

void Tour::to_stream(std::ostream & out) const
{ 
	out << name << '\n';
	for (auto &[time, name] : events) out << '\t' << time_to_str(time) << " -> " << name << '\n';
	out << endl;
}

#define C (char*)&
void Tour::from_binary(std::istream & in)
{ 
	decltype(size(name)) size;
	in.read(C size, sizeof size);
	name.resize(size);
	in.read(name.data(), size);
	while (true)
	{
		time_t time;
		in.read(C time, sizeof time);
		if (time == (time_t)-1) break;
		in.read(C size, sizeof size);
		auto where = events.try_emplace(events.cend(), time, size, 0);
		in.read(where->second.data(), size);
	}
	while (true)
	{
		decltype(reservations)::value_type val;
		in.read(C val, sizeof val);
		if (val == decltype(val){}) return;
		reservations.emplace_hint(reservations.cend(), val);
	}
}
#undef C
#define C (const char*)&
void Tour::to_binary(std::ostream & out) const
{
	out.write(C (const size_t &)size(name), sizeof size(name));
	out.write(name.data(), size(name));
	for (const auto &[time, name] : events) out
		.write(C time, sizeof time)
		.write(C(const size_t &)size(name), sizeof size(name))
		.write(name.data(), size(name));
	out.write(C(const time_t &)(time_t) - 1, sizeof(time_t));
	for (const auto &res : reservations)
		out.write(C res, sizeof res);
	out.write(C (decltype(reservations)::value_type const&) decltype(reservations)::value_type{},
			  sizeof(decltype(reservations)::value_type));

}
#undef C

void Tour::reserve(uint8_t count, uint64_t id) noexcept
{ 
	if (count == 0) [[unlikely]] return unreserve(id);
	reservations[id] = { count };
}

void Tour::unreserve(uint64_t id) noexcept
{ 
	reservations.erase(id);
}

void Tour::accept(uint64_t id, bool yes)
{ 
	reservations.at(id).accept = yes;
}

void Tour::remove_events(event_iter first, event_iter last)
{ 
	if (last == events.cend() and first != events.cend()) throw invalid_argument("Нельзя удалять конец тура");
	if (last->first < first->first) throw invalid_argument("Диапазон задан в неправильном направлении");
	events.erase(first, last);
}

void Tour::remove_event(event_iter element)
{ 
	if (element == --events.cend()) throw invalid_argument("Нельзя удалять конец тура");
	if (element == events.cend()) throw invalid_argument("Тут нечего удалять");
	events.erase(element);
}

void Tour::insert_event(time_t time, std::string_view name)
{ 
	if (time >= (--events.cend())->first) throw invalid_argument("Нельзя добавлять событие после конца тура");
	auto [_, did] = events.emplace(pair(time, name));
	if (not did) [[unlikely]] throw invalid_argument("Это время уже занято");
}

void Tour::shift_events(event_iter first, event_iter last, time_t diff)
{ 
	if (diff >= 0)
	{
		if
			(
			last != events.end() and
			last->first <= diff + prev(last)->first
			)
			throw invalid_argument("Нельзя переупорядочивать события");
		for (--last, --first; last != first; ----last)
		{
			pair<time_t, string> event = move(*last);
			last = events.erase(last);
			event.first += diff;
			events.emplace_hint(last, move(event)); 
		}
	}
	else
	{	
		if 
			(
			first != events.begin() and
			prev(first)->first >= diff + first->first
			)
			throw invalid_argument("Нельзя переупорядочивать события");
		while (first != last)
		{
			pair<time_t, string> event = move(*first);
			first = events.erase(first);
			event.first += diff;
			events.emplace_hint(first, move(event));
		}
	}
}

void Tour::shift_event(event_iter event, time_t diff)
{ 
	if 
		(
			diff >= 0 and
			event != prev(events.end()) and
			next(event)->first <= diff + event->first
		or
			diff < 0 and
			event != events.begin() and
			prev(event)->first >= diff + event->first
		)
		throw invalid_argument("Нельзя переупорядочивать события");
	std::pair e = move(*event);
	event = events.erase(event);
	events.try_emplace(event, e.first + diff, move(e.second));
}
