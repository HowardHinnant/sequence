import std;

#include "Sequence.h"

using namespace std;

template<typename T>
using my_vector = sequence<T,
	sequence_traits<> {
		.dynamic = true,
		.variable = true,
		.growth = sequence_lits::VECTOR,
	}>;

template<typename T, size_t S>
using my_inplace_vector = sequence<T,
	sequence_traits<size_t, S> {
		.dynamic = false,
		.variable = false,
	}>;

template<typename T, size_t S>
using my_small_vector = sequence<T,
	sequence_traits<size_t, S> {
		.dynamic = true,
		.variable = true,
	}>;


struct immovable {
	immovable() = default;
	immovable(const immovable&) = delete;
	immovable(immovable&&) = delete;
	immovable& operator=(const immovable&) = delete;
	immovable& operator=(immovable&&) = delete;
};

struct copy_only {
	copy_only() = default;
	copy_only(const copy_only&) = default;
	copy_only(copy_only&&) = delete;
	copy_only& operator=(const copy_only&) = default;
	copy_only& operator=(copy_only&&) = delete;
};

struct unmakeable {
	unmakeable() = delete;
	char c;
};

struct base {
	void foo() { println("base::foo"); }
};
struct has_foo : public base {
	void foo() { println("has_foo::foo"); }
};
struct no_foo : public base {
};

int main()
{
	//println("---- default --------------------------------");
	//sequence<int> s;
	//show(s);

	//println("---- vector ---------------------------------");
	//my_vector<int> s5;
	//show(s5);

	//println("---- inplace_vector -------------------------");
	//my_inplace_vector<int, 10> s6;
	//show(s6);


	//println("---- small_vector ---------------------------");
	//my_small_vector<int, 15> s7;
	//show(s7);

	println("---- test -----------------------------------");
	sequence<int,
		sequence_traits<> {
			.dynamic = false,
			.variable = false,
			.capacity = 10,
			.location = sequence_lits::BACK,
		}> s3;
	show(s3);

	std::println("Size of s3:\t{}", sizeof(s3));
	println("");

	for (int i = 1; i < 6; ++i)
		s3.push_front(i);

	//try {
	//	s3.push_back(42);
	//}
	//catch (std::bad_alloc& e)
	//{
	//	println("Oops!");
	//}

	for (auto&& e : s3)
		print("{}\t", e);
	println("");

	println("---------------------------------------------");

}




/*
* 

	base b;
	b.foo();
	has_foo hf;
	hf.foo();
	no_foo nf;
	nf.foo();

	immovable i1;
	immovable i2;
	vector<immovable> v;

	println("std::move_constructible<int> = {}", std::move_constructible<int>);
	println("std::move_constructible<immovable> = {}", std::move_constructible<immovable>);
	println("std::move_constructible<string> = {}", std::move_constructible<string>);
	println("std::move_constructible<copy_only> = {}", std::move_constructible<copy_only>);

	copy_only c1;
	copy_only c2;
	vector<copy_only> c;
	c2 = c1;
	//c.emplace_back();


	union fred {
		unmakeable m[5];
		unsigned char s;
	};

	std::println("Size of unmakeable:\t{}", sizeof(unmakeable));
	std::println("Size of fred:\t\t{}", sizeof(fred));
 
template<bool B>
struct thing {
};

struct base1 {
	void id() const
	{
		std::println("base1");
	}
};

struct base2 {
	void id() const
	{
		std::println("base2");
	}
};

template<>
struct thing<true> : base1 {
	void id() const
	{
		std::println("thing<true>");
		base1::id();
	}
};
template<>
struct thing<false> : base2 {
	void id() const
	{
		std::println("thing<false>");
		base2::id();
	}
};
	thing<true> tt;
	tt.id();
	thing<false> tf;
	tf.id();

	struct foo {
		char c;
	};
	struct bar {
		size_t size;
		foo data[15];
		unsigned char start;
	};
	struct pho {
		bar b1, b2;
	};

	std::println("Size of foo:\t{}", sizeof(foo));
	std::println("Size of bar:\t{}", sizeof(bar));
	std::println("Size of pho:\t{}", sizeof(pho));

	constexpr sequence_traits<unsigned char> st{
		.dynamic = false,
		.variable = false,
		.capacity = 128,
		.location = sequence_lits::MIDDLE,
	};

	println("---- default --------------------------------");
	sequence<int, st> s2;
	show(s2);
	println("--------------------------------");

	sequence<int,
		sequence_traits<unsigned short> {
			.dynamic = false,
			.variable = false,
			.capacity = 16,
			.location = sequence_lits::BACK,
		}> s3;
	show(s3);
	println("--------------------------------");

	sequence<int,
		sequence_traits<> {
			.location = sequence_lits::MIDDLE,
			.growth = sequence_lits::LINEAR,
			.increment = 256,
		}> s4;
	show(s4);
	println("--------------------------------");
*/