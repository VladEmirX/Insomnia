module;
//Хотел использовать здесь <chrono>, но она забаговалась:( Так что через time_t
#include <array>
export module Tour;
export import Data;
import <list>;
import <map>;
import <streambuf>;
export import <string>;


#pragma warning(push) 
#pragma warning(disable: 4455) 
//warning C4455: "operator ""sv": идентификаторы литеральных суффиксов, которые начинаются не с символа подчеркивания, зарезервированы

using std::operator""sv;

#pragma warning(pop)

export extern std::string time_to_str(time_t);
export extern time_t str_to_time(std::string_view, time_t = time(0));

export class Tour final : public Data
{
public:
	struct alignas(uint16_t) Reservation
	{
		uint8_t count; bool accept = false; friend bool operator==(Reservation, Reservation) noexcept;
	};
	using event_iter = std::map<std::time_t, std::string>::const_iterator;
private:
	std::string name{};
	std::map<std::time_t, std::string> events{};
	std::map<uint64_t, Reservation> reservations{};
public:
	explicit Tour(std::string_view name = {}) noexcept : name(name) { }

	void from_stream(std::istream & = std::cin) override;
	void to_stream(std::ostream & = std::cout) const override;
	void from_binary(std::istream &) override;
	void to_binary(std::ostream &) const override;

	std::string_view my_name() const { return name; }
	void my_name(std::string_view name) { this->name = name; }

	void reserve(uint8_t, uint64_t) noexcept;
	void unreserve(uint64_t) noexcept;
	void accept(uint64_t, bool = true);
	const auto &reserves() const { return reservations; };

	void remove_events(event_iter, event_iter);
	void remove_event(event_iter);
	void insert_event(time_t, std::string_view);
	void shift_events(event_iter, event_iter, time_t);
	void shift_events_to(event_iter begin, event_iter end, time_t tp)
		{ return shift_events(begin, end, tp - begin->first); };
	void shift_event(event_iter, time_t);
	void shift_event_to(event_iter which, time_t tp)
		{ return shift_event(which, tp - which->first); };
	const auto &my_events() const { return events; }

	constexpr static std::array<std::streamoff, 4> table_sizes_mem = { 19, 60, 30, 30 };
	constexpr static std::array table_names_mem = { "ID"sv, "Название"sv, "Начало"sv, "Конец"sv };
	static_assert(size(table_sizes_mem) == size(table_names_mem));

	std::vector<std::string> table_members() const noexcept
	{
		return { "", name, time_to_str(events.begin()->first), time_to_str((--events.end())->first) };
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
