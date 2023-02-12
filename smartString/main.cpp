#include <iostream>
#include <string>

class StringPointer
{
public:
	std::string *operator->()
	{
		if (Pointer_) {
			return Pointer_;
		} else {
			return &empty_string;
		}
	}

	std::string &operator*()
	{
		if (Pointer_) {
			return *Pointer_;
		} else {
			return empty_string;
		}
	}
	explicit StringPointer(std::string *Pointer): Pointer_(Pointer) {}
	~StringPointer() = default;
private:
	std::string *Pointer_{ nullptr };
	std::string empty_string;
};


int main()
{
	std::string s1 = "Hello, world!";

	StringPointer sp1(&s1);
	StringPointer sp2(NULL);

	std::cout << sp1->length() << std::endl;
	std::cout << *sp1 << std::endl;
	std::cout << sp2->length() << std::endl;
	std::cout << *sp2 << std::endl;
}