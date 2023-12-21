export module table_out;

//Линкер плохо работает с шаблонами... (((

import <print>; 
import <string>; 
import <span>;
//Не смог импортировать <ranges> из-за бага

#pragma warning(push) 
#pragma warning(disable: 4455) 
//warning C4455: "operator ""sv": идентификаторы литеральных суффиксов, которые начинаются не с символа подчеркивания, зарезервированы
using
std::print, std::putchar, std::streamoff, std::string, std::size_t, std::println, 
std::pair, std::vector, std::size, std::operator""sv, std::remove_cvref_t, std::to_string;

#pragma warning(pop)

__forceinline void nextline() noexcept { putchar(10); };

void row(std::span<const streamoff> const sizes) 
{
	putchar('¤');
	for (auto size : sizes)
		print("{:->{}}", '¤', size + 1);
	nextline();
}

constexpr char CD = '¦'; //разделитель столбцов


export template <std::ranges::range T>
void table_out(T const &arg) noexcept
{
	if (begin(arg) == end(arg))
		return println("Тут пусто, к сожалению.\n");

	using el_type = std::remove_reference_t<decltype(*begin(T())->second)>;

	static const std::span<const std::streamoff> sizes = el_type::table_sizes();
	static const std::span<const std::string_view> names = el_type::table_names();
	static const size_t rows = size(sizes);
	static nullptr_t __check__ = [] { return sizes.size() == names.size() ? nullptr : throw; }();

	row(sizes);
	putchar(CD);
	for (size_t i = 0; i != rows; ++i)
		print("{:<{}}{}", names[i], sizes[i], CD);
	nextline();
	row(sizes);


	for (const auto &[id, data] : arg)
	{
		vector<string> values = data->table_members();
		values[0] = to_string(id);

		for (size_t ended = 0, iter = 0; ended != rows; ++iter, putchar(CD), nextline())
			for (size_t i = 0; i != rows; ++i)
			{
				const string &value = values[i];
				const streamoff wid = sizes[i];
				putchar(CD);
				if (size_t num = iter * wid; size(value) < num) print("{: >{}}", "", wid);
				else
				{
					string to_print = value.substr(num, wid); 
					if (size(to_print) != wid) ++ended;
					print("{: <{}}", to_print, wid);
				}
			}
		row(sizes);
	}
	nextline();
}


