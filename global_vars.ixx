export module global_vars;
export import User;
export import Tour;
export import <cstdint>;
export import <map>;
export import <memory>;

export namespace global_vars
{
	extern std::map<uint64_t, std::unique_ptr<User>> users;
	extern std::map<uint64_t, std::unique_ptr<Tour>> tours;
	extern std::map<uint64_t, std::unique_ptr<User>>::iterator current_user;
	void save() noexcept;
}
