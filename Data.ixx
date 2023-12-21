module;
export module Data;
export import <iostream>;
import <vector>;
import <span>;

export class Data
{ 
public:
	virtual ~Data() = default;
	virtual void from_stream(std::istream&) = 0;
	virtual void to_stream(std::ostream&) const = 0;
	virtual void from_binary(std::istream&) = 0;
	virtual void to_binary(std::ostream&) const = 0;

	friend std::istream &operator>>(std::istream &in, Data &data) 
	{
		data.from_stream(in);
		return in;
	}
	friend std::ostream &operator<<(std::ostream &out, Data &data) 
	{
		data.to_stream(out);
		return out;
	}

	std::vector<std::string> table_members() const noexcept = delete;
	static std::span<const std::string_view> table_names() noexcept = delete;
	static std::span<const std::streamoff> table_sizes() noexcept = delete;
};
