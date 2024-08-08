/*	Sequence.h
* 
*	Header file for general-purpose sequence container.
* 
*	Alan Talbot
*/

#include <assert.h>

// sequence_lits - Literals used to specify the values of various sequence traits.
// These are hoisted out of the class template to avoid template dependencies.
// See sequence_traits below for a detailed discussion of these values.

enum class sequence_lits {
	FRONT, MIDDLE, BACK,			// See sequence_traits::location.
	LINEAR, EXPONENTIAL, VECTOR,	// See sequence_traits::growth.
};

// sequence_traits - Structure used to supply the sequence traits. The default values have been chosen
// to exactly model std::vector so that sequence can be used as a drop-in replacement with no adjustments.

template<std::unsigned_integral SIZE = size_t, size_t CAP = 0>
struct sequence_traits
{
	// 'size_type' is the type of the size field for local storage. This allows small sequences of
	// small types to be made meaningfully more space efficient by representing the size with a
	// smaller type than size_t. This size reduction may (or may not) also result in faster run times.
	// (Note that this type is not used in this structure. Using it for 'capacity' complicates
	// sequence_storage without offering any real benefits, and it's not correct for 'increment'
	// because the SBO (see below) may be small but the dynamic size large.)

	using size_type = SIZE;

	// 'dynamic' is true to allow storage to be dynamically allocated; false if storage must always be local.

	bool dynamic = true;

	// 'variable' is true if the capacity is permitted to grow; false if the capacity must remain fixed.
	// Local storage ('dynamic' == false) implies a fixed capacity, so in this case 'variable' must be false.
	// (Allowing it to be true would be harmless since the field would be ignored, but this would always be
	// a programming error or a misunderstanding).

	bool variable = true;

	// 'capacity' is size of a fixed capacity. If this field is non-zero for dynamic storage, then it is the
	// size of the small object optimization buffer (SBO). If it is zero, no small object optimization is used.

	size_t capacity = CAP;		

	// 'location' specifies how the data are managed within the capacity:
	//		FRONT:	Data starts at the lowest memory location. This makes push_back most efficient (like std::vector).
	//		MIDDLE:	Data floats in the middle of the capacity. This makes both push_back and push_front efficient (similar to std::deque).
	//		BACK:	Data ends at the highest memory location. This makes push_front most efficient.

	sequence_lits location = sequence_lits::FRONT;

	// 'growth' indicates how the capacity grows when growth is necessary:
	//		LINEAR:			Capacity grows by a fixed amount specified by 'increment'.
	//		EXPONENTIAL:	Capacity grows exponentially by a factor specified by 'factor'.
	//		VECTOR:			Capacity grows in the same way as std::vector. This behavior is implementation dependent.
	//						It is provided so that sequence can be used as an implementation of std::vector and/or a
	//						drop-in replacement for std::vector with no changes in behavior, even if the std::vector
	//						growth behavoir cannot be otherwise modeled with LINEAR or EXPONENTIAL growth modes.

	sequence_lits growth = sequence_lits::VECTOR;

	// 'increment' specifies the linear capacity growth in elements. This must be greater than 0.

	size_t increment = 1;

	// 'factor' specifies the exponential capacity growth factor. This must be greater than 1.0.
	// The minimum growth will be one element, regardless of how close to 1.0 this is set.

	float factor = 1.5;
};

// sequence_storage - Base class for sequence which provides the different memory allocation strategies.
// The DYN and VAR boolean parameters are sequence_traits::dynamic and sequence_traits::variable respectively.
// The SIZE type parameter is sequence_traits::size_type. The CAP unsigned parameter is sequence_traits::capacity.

// The primary template. This is never instantiated.

template<bool DYN, bool VAR, typename T, typename SIZE, size_t CAP>
class sequence_storage
{
public:
	constexpr static size_t capacity() { return 0; }
	size_t size() { return 0; }
};

// Dynamic, variable memory allocation with small object optimization. This is like boost::small_vector.

template<typename T, typename SIZE, size_t CAP>
class sequence_storage<true, true, T, SIZE, CAP>
{
public:
	constexpr static size_t capacity() { return 0; }
	size_t size() { return 0; }
};

// Dynamic, variable memory allocation. This is like std::vector.
// The CAP parameter is ignored. The SIZE parameter is used only
// for the offset value when the management mode is MIDDLE.

template<typename T, typename SIZE>
class sequence_storage<true, true, T, SIZE, size_t(0)>
{
	using value_type = T;

public:

	void resize(size_t new_size)
	{}
	void reserve(size_t new_capacity)
	{}
	void shrink_to_fit()
	{}

	constexpr static size_t capacity() { return 0; }
	size_t size() { return 0; }

private:

	union storage_type {
		storage_type() {}
		value_type element;
		unsigned char unused;
	};


};

// Dynamic, fixed memory allocation. This is like std::vector if you pre-reserve memory,
// and never allow it to grow past the reserve, but it supports immovable objects.

template<typename T, typename SIZE, size_t CAP>
class sequence_storage<true, false, T, SIZE, CAP>
{
	using value_type = T;
	using size_type = SIZE;

public:

	constexpr static size_t capacity() { return CAP; }
	size_t size() { return m_size; }

private:

	size_type m_size = 0;
};

// Local, fixed memory allocation. This is like std::inplace_vector (or boost::static_vector).

template<typename T, typename SIZE, size_t CAP>
class sequence_storage<false, false, T, SIZE, CAP>
{
	using value_type = T;
	using size_type = SIZE;

public:

	constexpr static size_t capacity() { return CAP; }
	size_t size() const { return m_size; }

protected:

	value_type* capacity_start() { return m_storage.elements; }
	value_type* capacity_end() { return m_storage.elements + CAP; }
	const value_type* capacity_start() const { return m_storage.elements; }
	const value_type* capacity_end() const { return m_storage.elements + CAP; }

	void add(value_type* p, const value_type& e)
	{
		assert(size() < capacity());
		new(p) value_type(e);
		++m_size;
	}
	void shift(value_type* beg, value_type* end, ptrdiff_t dist)
	{
		if (dist > 0)
		{
			assert(beg >= capacity_start());
			assert(end + dist <= capacity_end());
			for (auto dst = end + dist; end > beg; --dst)
			{
				new(dst) value_type(*--end);
///				*end = 99999;	// !!!
				end->~value_type();
			}
			return;
		}
		if (dist < 0)
		{
			assert(beg - dist >= capacity_start());
			assert(end <= capacity_end());
			for (auto dst = beg + dist; beg < end; ++dst, ++beg)
			{
				new(dst) value_type(*beg);
///				*beg = 99999;	// !!!
				beg->~value_type();
			}
		}
	}
	void reallocate()
	{
		throw std::bad_alloc();
	}

private:

	size_type m_size = 0;
	union storage_type {
		storage_type() {}
		value_type elements[CAP];
		unsigned char unused;
	} m_storage;
};

// sequence_management - Base class for sequence which provides the different element management strategies.
// The LOC sequence_lits parameter specifies which element management strategy to use. The remaining parameters
// are passed on the sequence_storage base class.

// The primary template. This is never instantiated.

template<sequence_lits LOC, bool DYN, bool VAR, typename T, typename SIZE, size_t CAP>
class sequence_management
{
public:
	void id() const
	{
		std::println("sequence_management: primary template");
	}
};

// FRONT element location.

template<bool DYN, bool VAR, typename T, typename SIZE, size_t CAP>
class sequence_management<sequence_lits::FRONT, DYN, VAR, T, SIZE, CAP> : public sequence_storage<DYN, VAR, T, SIZE, CAP>
{
	using value_type = T;
	using inherited = sequence_storage<DYN, VAR, T, SIZE, CAP>;

	using inherited::capacity;
	using inherited::size;
	using inherited::capacity_start;
	using inherited::add;
	using inherited::shift;
	using inherited::reallocate;

public:
	void push_front(const value_type& e)
	{
		if (size() == capacity())
			reallocate();
		shift(data_start(), data_end(), 1);
		add(data_start(), e);
	}
	void push_back(const value_type& e)
	{
		if (size() == capacity())
			reallocate();
		add(data_end(), e);
	}

protected:

	value_type* data_start() { return capacity_start(); }
	value_type* data_end() { return capacity_start() + size(); }
	const value_type* data_start() const { return capacity_start(); }
	const value_type* data_end() const { return capacity_start() + size(); }
};

// MIDDLE element location.

template<bool DYN, bool VAR, typename T, typename SIZE, size_t CAP>
class sequence_management<sequence_lits::MIDDLE, DYN, VAR, T, SIZE, CAP> : public sequence_storage<DYN, VAR, T, SIZE, CAP>
{
	using value_type = T;
	using size_type = SIZE;
	using inherited = sequence_storage<DYN, VAR, T, SIZE, CAP>;

	using inherited::capacity;
	using inherited::size;
	using inherited::capacity_start;
	using inherited::add;
	using inherited::shift;
	using inherited::reallocate;

public:

	void push_front(const value_type& e)
	{
		if (size() == capacity())
			reallocate();
		if (m_offset == 0)
		{
			auto offset = (capacity() - size()) / 2u;
			shift(data_start(), data_end(), offset);
			m_offset = offset;
		}
		else --m_offset;
		add(data_start(), e);
	}
	void push_back(const value_type& e)
	{
		if (size() == capacity())
			reallocate();
		if (back_gap() == 0)
		{
			auto offset = m_offset / 2;
			shift(data_start(), data_end(), -ptrdiff_t(m_offset - offset));
			m_offset = offset;
		}
		add(data_end(), e);
	}

protected:

	value_type* data_start() { return capacity_start() + m_offset; }
	value_type* data_end() { return capacity_start() + m_offset + size(); }
	const value_type* data_start() const { return capacity_start() + m_offset; }
	const value_type* data_end() const { return capacity_start() + m_offset + size(); }
	size_type back_gap() const { return capacity() - (m_offset + size()); }

private:

	size_type m_offset = CAP / 2;
};

// BACK element location.

template<bool DYN, bool VAR, typename T, typename SIZE, size_t CAP>
class sequence_management<sequence_lits::BACK, DYN, VAR, T, SIZE, CAP> : public sequence_storage<DYN, VAR, T, SIZE, CAP>
{
	using value_type = T;
	using inherited = sequence_storage<DYN, VAR, T, SIZE, CAP>;

	using inherited::capacity;
	using inherited::size;
	using inherited::capacity_end;
	using inherited::add;
	using inherited::shift;
	using inherited::reallocate;

public:

	void push_front(const value_type& e)
	{
		if (size() == capacity())
			reallocate();
		add(data_start() - 1, e);
	}
	void push_back(const value_type& e)
	{
		if (size() == capacity())
			reallocate();
		shift(data_start(), data_end(), -1);
		add(data_end() - 1, e);
	}

protected:

	value_type* data_start() { return capacity_end() - size(); }
	value_type* data_end() { return capacity_end(); }
	const value_type* data_start() const { return capacity_end() - size(); }
	const value_type* data_end() const { return capacity_end(); }
};

// sequence - This is the main class template.

template<typename T, sequence_traits TRAITS = sequence_traits<size_t>()>
class sequence : public sequence_management<TRAITS.location, TRAITS.dynamic, TRAITS.variable,
											T, typename decltype(TRAITS)::size_type, TRAITS.capacity>
{
	using inherited = sequence_management<TRAITS.location, TRAITS.dynamic, TRAITS.variable,
										T, typename decltype(TRAITS)::size_type, TRAITS.capacity>;
	using inherited::data_start;
	using inherited::data_end;

public:

	using value_type = T;

	~sequence()
	{
		for (auto next(data_start()), end(data_end()); next != end; ++next)
		{
///			*next = 99999;	// !!!
			next->~value_type();
		}
	}

	using traits_type = decltype(TRAITS);
	static constexpr traits_type traits = TRAITS;

	// This combination is meaningless. See comment in sequence_traits.
	static_assert(!(traits.dynamic == false && traits.variable == true),
				  "A sequence with local storage must have a fixed capacity.");

	// Variable capacity means that the capacity must grow, and this growth must actually make progress.
	static_assert(traits.increment > 0,
				  "Linear capacity growth must be greater than 0.");
	static_assert(traits.factor > 1.0f,
				  "Exponential capacity growth must be greater than 1.0.");

	// Maintaining elements in the middle of the capacity is more or less useless without the ability to shift.
	static_assert(traits.location != sequence_lits::MIDDLE || std::move_constructible<T>,
				  "Middle element location requires move-constructible types.");

	const value_type* begin() const { return data_start(); }
	const value_type* end() const { return data_end(); }

private:


};

template<typename SEQ>
void show(const SEQ& seq)
{
	using traits_type = SEQ::traits_type;

	std::println("Size Type:\t{}", typeid(traits_type::size_type).name());

	std::println("Dynamic:\t{}", seq.traits.dynamic ? "yes" : "no");
	std::println("Variable:\t{}", seq.traits.variable ? "yes" : "no");
	std::println("Capacity:\t{}", seq.traits.capacity);

	std::print("Location:\t");
	switch (seq.traits.location)
	{
		case sequence_lits::FRONT:			std::println("FRONT");	break;
		case sequence_lits::MIDDLE:			std::println("MIDDLE");	break;
		case sequence_lits::BACK:			std::println("BACK");	break;
	}
	std::print("Growth:\t\t");
	switch (seq.traits.growth)
	{
		case sequence_lits::LINEAR:			std::println("LINEAR");			break;
		case sequence_lits::EXPONENTIAL:	std::println("EXPONENTIAL");	break;
		case sequence_lits::VECTOR:			std::println("VECTOR");			break;
	}

	std::println("Increment:\t{}", seq.traits.increment);
	std::println("Factor:\t\t{}", seq.traits.factor);
}
