module;
#include <format>
#include <stdexcept>

module input;
import <string>;

using
    std::string,
    std::string_view,
    std::istream,
    std::invalid_argument,
    std::out_of_range;


template <> [[nodiscard]] string input<string>(string_view str, istream &in)
{
    std::cout << str;
    string el;
    if (in.fail() and not in.bad()) in.clear();
    try { getline(in, el); } 
    catch (std::ios_base::failure&)
    {
        throw invalid_argument("Пустой ввод");
    }
    while (not el.empty() and isblank(el.back())) el.pop_back();
    if (el.empty()) { throw invalid_argument("Пустой ввод"); }
    size_t new_begin = 0;
    while (isblank(el[new_begin])) ++new_begin;
    return move(el).substr(new_begin);
}

template <> [[nodiscard]] uint32_t input<uint32_t>(string_view str, istream &in)
{
    auto el = input<string>(str, in);
    if (el[0] == '-') throw out_of_range("Негативный ввод");
    size_t size;
    auto ul = stoul(el, &size, 0);
    if (size != el.size()) throw invalid_argument("Введено не только число");
    if ((uint32_t)ul != ul) throw out_of_range("Введено слишком большое число");
    return ul;
}
template <> [[nodiscard]] uint8_t input<uint8_t>(string_view str, istream &in)
{
    auto el = input<string>(str, in);
    if (el[0] == '-') throw out_of_range("Негативный ввод");
    size_t size;
    auto ul = stoul(el, &size, 0);
    if (size != el.size()) throw invalid_argument("Введено не только число");
    if ((uint8_t)ul != ul) throw out_of_range("Введено слишком большое число");
    return ul;
}
template <> [[nodiscard]] uint64_t input<uint64_t>(string_view str, istream &in)
{
    auto el = input<string>(str, in);
    if (el[0] == '-') throw out_of_range("Негативный ввод");
    size_t size;
    auto ull = stoull(el, &size, 0);
    if (size != el.size()) throw invalid_argument("Введено не только число");
    if ((uint64_t)ull != ull) throw out_of_range("Введено слишком большое число");
    return ull;
}
template <> [[nodiscard]] double input<double>(string_view str, istream &in)
{
    size_t size;
    auto el = input<string>(str, in);
    auto ret = stod(el, &size);
    if (size != el.size()) throw invalid_argument("Введено не только число");
    return ret;
}