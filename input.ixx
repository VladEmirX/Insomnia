export module input;
import <iostream>;
import <string_view>;


export template <class C> C input(std::string_view = {}, std::istream & = std::cin) { static_assert([] { return false; }()); };
export template <> extern std::string input<std::string>(std::string_view str, std::istream &in);
export template <> extern uint32_t input<uint32_t>(std::string_view str, std::istream &in);
export template <> extern uint64_t input<uint64_t>(std::string_view str, std::istream &in);
export template <> extern uint8_t input<uint8_t>(std::string_view str, std::istream &in);
export template <> extern double input<double>(std::string_view str, std::istream &in);