export module current_menu;
import Menu;
import <cstdint>;

//���� �����������
export extern const Menu<uint32_t, void()>& current_menu() noexcept;