#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <cassert>

using namespace std;

unordered_map<string, string> test_data{
		{ "x^2+x", "2*x+1" },
		{ "2*x^100+100*x^2", "200*x^99+200*x" },
		{ "x^10000+x+1", "10000*x^9999+1" }, { "-x^2-x^3", "-3*x^2-2*x" },
		{ "x+x+x+x+x+x+x+x+x+x", "10" },
		{ "2*x+100-50+34+2+3434*x+4+x-3*x^2+4", "-6*x+3437" },
		{ "2*x+100-50+34+2+3434*x+4-x-3*x^2", "-6*x+3435" },
		{ "2*x-3*x^2", "-6*x+2" },
		{ "4+6+3+7", "" },
		{ "x^100", "100*x^99" },
		{ "-x^100", "-100*x^99" },
		{ "5-x^100", "-100*x^99" },
		{ "-5-x^100", "-100*x^99" },
		{ "-5*x-x^100", "-100*x^99-5" },
};

struct Comp
{
	bool operator()(const int &a, const int &b) const
	{
		return b < a;
	}
};

string derivative(const string &polynomial)
{
	map<int, int, Comp> degree_to_count;
	int last_sign_pos = -1;
	int last_sign = 1;
	int pre = 0;
	bool was_x = false;
	bool was_sign = false;
	int last_x_pos = -1;
	int degree = 1;
	const auto iter_begin = polynomial.begin();
	for (int i = 0; i < polynomial.size(); ++i) {
		char ch = polynomial[i];

		if (i == 0) {
			if (ch == '-') {
				last_sign_pos = 0;
				last_sign = -1;
			} else if (ch == 'x') {
				pre = 1;
				was_x = true;
				was_sign = false;
				last_x_pos = i;
			}
		} else if (ch == 'x') {
			if (i == last_sign_pos + 1) {
				pre = last_sign;
			} else {
				pre = last_sign
						* stoi(string(iter_begin + last_sign_pos + 1,
								iter_begin + i - 1));
			}
			was_x = true;
			was_sign = false;
			last_x_pos = i;
		} else if (ch == '-' || ch == '+') {
			last_sign = (ch == '-') ? -1 : 1;
			last_sign_pos = i;

			if (was_sign) {
				continue;
			}
			if (was_x) {
				if (i == last_x_pos + 1) {
					degree = 1;
				} else {
					degree = stoi(string(
							iter_begin + last_x_pos + 2, iter_begin + i));
				}
				degree_to_count[degree] += pre;
			}
			was_x = false;
			was_sign = true;
		}
	}
	if (was_x) {
		if (last_x_pos + 1 == polynomial.size()) {
			degree = 1;
		} else {
			degree =
					stoi(string(iter_begin + last_x_pos + 2, polynomial.end()));
		}
		degree_to_count[degree] += pre;
	}

	stringstream out;
	bool is_first = true;
	for (const auto &it : degree_to_count) {
		int deg = it.first;
		const int prefix = it.second * deg;
		--deg;

		if (is_first) {
			out << prefix;
			is_first = false;
		} else {
			if (prefix > 0) {
				out << '+';
			}
			out << prefix;
		}

		if (deg > 0) {
			out << "*x";
			if (deg > 1) {
				out << '^' << deg;
			}
		}
	}
	return out.str();
}

int main()
{
	for (const auto &it : test_data) {
		assert(derivative(it.first) == it.second);
	}
}