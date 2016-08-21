/*
The MIT License (MIT)

Copyright (c) 2016 Liav Turkia and Shahar Sandhaus

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/
#pragma once
#include <MC-System/Constants.h>
#include <iostream>
#include <MC-System/Utility/Math.h>

namespace mc {
/**
Wrapper class for a primitive type that allows for easy bit manipulation. Can set individual bits. Additionally, it overrides almost every operator
to operate on it's wrapped value, to allow for you to manually manipulate it.
<p>
Examples:
{@code
	BitField<int> field = 15;//Creation
		
	field.setBit(i,state)//Set bit in position i to state

	field.getBit(i) //Get whether bit in position i is true or false
}
@tparam T What primitive the`BitField` should use to store the bits
*/
template<typename T>
struct BitField {
	/**
	The value this `BitField` will be acting upon
	*/
	T value;

	/**
	Default constructor.
	<p>
	Equal to calling {@code BitField(0)}
	*/
	BitField();

	/**
	Constructs a `BitField` with the specified value.
	<p>
	Equal to calling {@code BitField<T> = value}
	@param value Inital value
	*/
	BitField(const T val);

	//dont need a copy constructor, the above constructor acts like one. adding a copy constructor creates too many options when you do ByteField var = 0;

	/**
	Default destructor.
	*/
	~BitField();
	/**
	Retrieve the `const` value inside of this `BitField`
	@return The value represented by this `BitField`
	@see get()
	*/
	const T get() const;
	/**
	Retrieve the value inside of this `BitField`
	@return The value represented by this `BitField`
	@see get() const
	*/
	T get();

	/**
	Change the internal value.
	@param newValue New value for this `BitField` to operate on
	*/
	void set(const T newValue);

	/**
	Make the bit at a certain position toggled or untoggled
	@param position 0-indexed integer reprsenting which bit to set
	@param state `true` to make the specified bit 1, and `false` to make it 0
	@return `this` for chaining
	*/
	BitField& setBit(const Index position, const bool state);

	/**
	Turn bit at `position` to be `true` or `1`
	@param position 0-indexed integer representing which bit to toggle
	@return `this` for chaining
	*/
	inline BitField& toggleBit(Index position);
	/**
	Turn bit at `position` to be `false` or `0`
	@param position 0-indexed integer representing which bit to untoggle
	@return `this` for chaining
	*/
	inline BitField& untoggleBit(Index position);

	/**
	Retrieve the value of a specified bit
	@param position which bit to check
	@return `true` if the bit is 1, `false` otherwise
	*/
	inline bool getBit(Index position) const;
		
	/**
	Inverts a certain bit
	@param position Which bit to "flip," or invert
	@return `this` for chaining
	*/
	inline BitField& flipBit(Index position);

	/**
	Inverses every bit, making every 0 a 1 and every 1 a 0.
	<p>
	Equivelant to calling the ~ operator.
	@return `this` for chainign
	*/
	inline BitField& inverse();

	/**
	Gets how many bits this `BitField` is representing. Not to be confused with sizeof().
	<p>
	On most systems, this returns 8.
	@return Number of bits
	*/
	Size size() const;



	//i heard you like operators so i added some operators to your operators
	//dont worry i know how to operate

	/**
	Operator for `std::cout` to correctly print `BitField`
	*/
	template<typename T>
	friend std::ostream &operator<<(std::ostream &os, const BitField<T>& b);


	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	BitField operator|(const T value) {
		return BitField<T>(this->value | value);
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	void operator|=(const T value) {
		this->value |= value;
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	BitField operator&(const T value) {
		return BitField<T>(this->value & value);
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	void operator&=(const T value) {
		this->value &= value;
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	BitField operator^(const T value) {
		return BitField<T>(this->value ^ value);
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	void operator^=(const T value) {
		this->value ^= value;
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	BitField operator|(BitField& other) {
		return BitField<T>(this->value | other.value);
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	void operator|=(BitField& other) {
		this->value |= other.value;
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	BitField operator&(BitField& other) {
		return BitField<T>(this->value & other.value);
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	void operator&=(BitField& other) {
		this->value &= other.value;
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	BitField operator^(BitField& other) {
		return BitField<T>(this->value ^ other.value);
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	void operator^=(BitField& other) {
		this->value ^= other.value;
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	BitField operator>>(Index places) {
		return BitField<T>(value >> places);
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	BitField operator<<(Index places) {
		return BitField<T>(value << places);
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	void operator>>=(Index places) {
		value >>= places;
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	void operator<<=(Index places) {
		value <<= places;
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	BitField operator+(const T other) const {
		return BitField<T>(value + other);
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	BitField operator+(const BitField& other) const {
		return BitField<T>(value + other.value);
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	void operator+=(const BitField& other) {
		value += other.value;
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	void operator+=(const T other) {
		value += other;
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	BitField operator-(const T other) const {
		return BitField<T>(value - other);
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	BitField operator-(const BitField& other) const {
		return BitField<T>(value - other.value);
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	void operator-=(const BitField& other) {
		value -= other.value;
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	void operator-=(const T other) {
		value -= other;
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	BitField operator*(const T other) {
		return BitField<T>(value * other);
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	BitField operator*(const BitField& other) const {
		return BitField<T>(value * other.value);
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	void operator*=(const BitField& other) {
		value *= other.value;
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	void operator*=(const T other) {
		value *= other;
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	BitField operator/(const T other) const {
		return BitField<T>(value / other);
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	BitField operator/(const BitField& other) const {
		return BitField<T>(value / other.value);
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	void operator/=(const BitField& other) {
		value /= other.value;
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	void operator/=(const T other) {
		value /= other;
	}

	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	T operator%(const T other);
	/**
	Operator which acts upon the internal value.
	<p>
	Same as calling {@link inverse()}
	@see get() @see value
	*/
	BitField operator~();
	/***
	Operator which acts upon the internal value
	@see get() @see value
	*/
	void operator++() {
		value++;
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	void operator++(int dummy) {
		++value;
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	void operator--() {
		value--;
	}
	/**
	Operator which acts upon the internal value
	@see get() @see value
	*/
	void operator--(int dummy) {
		--value;
	}
	/**
	Gets a bit at a certain position
	@param position Which bit to retrieve. Zero-indexed
	@return `true` if the bit is 1, `false` otherwise.
	@see getBit(Index)
	*/
	bool operator[](const Index position);
	/**
	Compares this `BitField` to another value. Will return `true` if the bits represented are both equal.
	*/
	bool operator==(const T other) {
		return value == other;
	}
	/**
	Inverse for {@link operator==}
	*/
	bool operator!=(const T other) {
		return !(value == other);
	}
	/**
	Compares this `BitField` to another. Will return `true` if the bits represented are both equal.
	*/
	bool operator==(const BitField& other) {
		return value == other.value;
	}
	/**
	Inverse for {@link operator==}
	*/
	bool operator!=(const BitField& other) {
		return (value != other.value);
	}
};//BitField<T>

//for std::cout


/**
{@link BitField} which wraps around a {@link Byte byte,} meaning that it will have 8 bits.
*/
using ByteField = BitField<Byte>;

//include the actual definition
#include <MC-System/Utility/BitField.inl>

}//mc