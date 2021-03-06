/*
Copyright (c) 2016-2019 Liav Turkia

See LICENSE.md for full copyright information
*/
#pragma once
#ifndef MACE__UTILITY_COLOR_H
#define MACE__UTILITY_COLOR_H

#include <MACE/Core/Constants.h>
#include <MACE/Utility/Vector.h>
#include <array>
#include <ostream>

namespace mc {

	/**
	Represents a color in the RGBA space. Values are stored as a `float` between 0 and 1, where 1 is the brightest. Values can optionally be added as a `Byte` from 0 to 255.
	<p>
	Note that conversion from a `Byte` to a `float` (and vice-versa) is not 100% accurate and may produce slightly different results on different processors.
	<p>
	Examples:
	{@code
		Color color = {1.0f,1.0f,1.0f,1.0f};//create the color white

		float red = color.r;//retrive the red componenet, as a float from 0.0 to 1.0
		float green = color.g;//retrive the green componenet, as a float from 0.0 to 1.0
		float blue = color.b;//retrive the blue componenet, as a float from 0.0 to 1.0
		float alpha = color.a;//retrive the alpha componenet, as a float from 0.0 to 1.0

		red = color.getRed();//retrieve the red component as a byte from 0 to 255
		green = color.getGreen();//retrieve the red component as a byte from 0 to 255
		blue = color.getBlue();//retrieve the red component as a byte from 0 to 255
		alpha = color.getAlpha();//retrieve the red component as a byte from 0 to 255

		color.r = 0.5f;//set the red component to be 0.5f
		color.setRed(127); //set the red component to be 0.5f as a Byte from 0 to 255
	}
	*/
	struct Color{
		/**
		Color component, represented as a `float`. It is from 0.0 to 1.0, where 1.0 is the brightest it can get.
		*/
		float r, g, b, a;

		explicit Color(const int red, const int green, const int blue, const int alpha = 255) noexcept;

		explicit Color(const Byte red, const Byte green, const Byte blue, const Byte alpha = 255) noexcept;

		explicit Color(const unsigned int value) noexcept;

		/**
		@copydoc Color(const std::array<float, 4>&)
		*/
		explicit Color(const Vector<float, 4>& values);

		/**
		Creates a `Color` from specified values.
		@param red The red component from 0.0 to 1.0, where 1.0 is the most red.
		@param green The red component from 0.0 to 1.0, where 1.0 is the most red.
		@param blue The red component from 0.0 to 1.0, where 1.0 is the most red.
		@param alpha The alpha component from 0.0 to 1.0, where 1.0 is opaque and 0.0 is transparent. By default, this parameter is 1.0
		@see Color(std::array<float,4>)
		*/
		Color(const float red, const float green, const float blue, const float alpha = 1.0f) noexcept;
		/**
		Creates a `Color` from an array of `floats`.
		@param values An array where the first element is red, the second is green, etc
		@see Color(std::array<Byte,4>)
		@see Color(float,float,float,float)
		@see setValues(std::array<float,4>
		*/
		Color(const std::array<float, 4>& values);

		Color(const float values[4]);

		/**
		Clones a `Color`, coying it's color values into a new one.
		@param copy A `Color` to copy
		*/
		Color(const Color& copy) noexcept;
		/**
		@copydoc Color(const Color&)
		@param alpha New alpha value for this `Color`
		*/
		Color(const Color& copy, const float alpha) noexcept MACE_EXPECTS(alpha >= 0.0f && alpha <= 1.0f);
		/**
		Default constructor. Constructs a `Color` with all of it's color values as `0`, or black.
		*/
		Color() noexcept;

		/**
		Sets the RGB values of this `Color` from an array of `floats`. The alpha is unchanged
		@param rgba An array where the first element is red, the second is green, etc
		@see Color(std::array<Byte,4>)
		@see Color(std::array<float,4>)
		@see Color(float,float,float,float)
		@see setValues(std::array<float,4>)
		*/
		void setValues(const std::array<float, 4>& rgba);

		/**
		Retrieves the RGBA values in `std::array`
		@return An array where the first element is red, the second value is green, etc
		*/
		std::array<float, 4> getValues() const;

		Vector<float, 4> toVector() const;

		/**
		Retrieves the alpha component of this `Color`
		@return A `Byte` from 0 to 255, where 255 is fully opaque, and 0 is transparent
		@see convertFloatToRGBA(float)
		@see convertRGBAToFloat(Byte)
		@see a
		*/
		Byte getAlpha() const;

		/**
		Set the alpha value of this `Color` from a `Byte`. It will automatically be converted to `float` internally.
		@param a A `Byte` from 0 to 255, where 255 is opaque
		@see convertRGBAToFloat(Byte)
		@see getAlpha()
		@see a
		*/
		void setAlpha(const Byte& a);

		unsigned int toUnsignedIntNoAlpha() const;

		unsigned int toUnsignedInt() const;

		/**
		Creates a new `Color` that is slightly darker.
		<p>
		Due to rounding, `color.darken() != color.darken().lighten()`
		<p>
		The alpha is not affected
		@return A darker `Color`
		@see Color::lighten() const
		*/
		Color darken() const;
		/**
		Creates a new `Color` that is slightly lighter.
		<p>
		Due to rounding, `color.lighten() != color.lighten().darken()` in some systems
		<p>
		The alpha is not affected
		@return A lighter `Color`
		@see Color::darken() const
		*/
		Color lighten() const;

		/**
		Retrieves the red component of this `Color`
		@return A `Byte` from 0 to 255, where 255 is the brightest
		@see convertFloatToRGBA(float)
		@see convertRGBAToFloat(Byte)
		@see r
		*/
		Byte getRed() const;
		/**
		Retrieves the green component of this `Color`
		@return A `Byte` from 0 to 255, where 255 is the brightest
		@see convertFloatToRGBA(float)
		@see convertRGBAToFloat(Byte)
		@see g
		*/
		Byte getGreen() const;
		/**
		Retrieves the blue component of this `Color`
		@return A `Byte` from 0 to 255, where 255 is the brightest
		@see convertFloatToRGBA(float)
		@see convertRGBAToFloat(Byte)
		@see b
		*/
		Byte getBlue() const;

		/**
		Set the red value of this `Color` from a `Byte`. It will automatically be converted to `float` internally.
		@param r A `Byte` from 0 to 255, where 255 is the brightest
		@see convertRGBAToFloat(Byte)
		@see getRed()
		@see r
		*/
		void setRed(const Byte& r);
		/**
		Set the green value of this `Color` from a `Byte`. It will automatically be converted to `float` internally.
		@param g A `Byte` from 0 to 255, where 255 is the brightest
		@see convertRGBAToFloat(Byte)
		@see getGreen()
		@see g
		*/
		void setGreen(const Byte& g);
		/**
		Set the blue value of this `Color` from a `Byte`. It will automatically be converted to `float` internally.
		@param b A `Byte` from 0 to 255, where 255 is the brightest
		@see convertRGBAToFloat(Byte)
		@see getBlue()
		@see b
		*/
		void setBlue(const Byte& b);

		/**
		Sets the values of this `Color` from an array of `floats`.
		@param rgb An array where the first element is red, the second is green, etc
		@see Color(std::array<Byte,4>)
		@see Color(std::array<Byte,4>)
		@see Color(float,float,float,float)
		@see setValues(std::array<float,3>)
		*/
		void setValues(const std::array<float, 3>& rgb);

		float* end();
		const float* end() const;

		float& getComponent(const Index i) MACE_EXPECTS(i < size());
		const float& getComponent(const Index i) const MACE_EXPECTS(i < size());

		float& operator[](const Index i) MACE_EXPECTS(i < size());
		const float& operator[](const Index i) const MACE_EXPECTS(i < size());

		std::string toHex() const;

		/**
		Creates an array with the data of this `Color`
		@return Pointer to `arr`
		@param arr The array to fill
		*/
		const float* flatten(float arr[4]) const;

		Size size() const;

		float* begin();
		const float* begin() const;

		friend std::ostream& operator<<(std::ostream& output, const Color& v);

		/**
		Compares the color values of 2 `Colors`
		@param other Another `Color` object
		@return Whether the 2 `Colors` have the same RGBA
		@see operator!=(Color&) const
		*/
		bool operator==(const Color& other) const;
		/**
		Compares the color values of 2 `Colors`
		@param other Another `Color` object
		@return Whether the 2 `Colors` don't have the same RGBA
		@see operator==(Color&) const
		*/
		bool operator!=(const Color& other) const;

		/**
		Compares the color values of 2 'Colors'
		@param other Another `Color` object
		@return The result of the operation on both `Colors` r, g, b, and a values.
		*/
		bool operator>(const Color& other) const;
		/**
		Compares the color values of 2 'Colors'
		@param other Another `Color` object
		@return The result of the operation on both `Colors` r, g, b, and a values.
		*/
		bool operator>=(const Color& other) const;
		/**
		Compares the color values of 2 'Colors'
		@param other Another `Color` object
		@return The result of the operation on both `Colors` r, g, b, and a values.
		*/
		bool operator<(const Color& other) const;
		/**
		Compares the color values of 2 'Colors'
		@param other Another `Color` object
		@return The result of the operation on both `Colors` r, g, b, and a values.
		*/
		bool operator<=(const Color& other) const;
	};//Color

	Color fromHex(const std::string hexcode);

	/**
	Contains various `Color` constants for common colors.
	*/
	namespace Colors {
		/**
		Constant `Color` values for easy access to common colors
		*/
		const extern Color RED, LIGHT_RED, DARK_BLUE, BLUE, CYAN, LIGHT_BLUE,
			DARK_RED, DARK_GREEN, GREEN, LIGHT_GREEN, PURPLE, MAGENTA, WHITE,
			BLACK, YELLOW, DARK_GRAY, LIGHT_GRAY, ORANGE, GRAY, DARK_ORANGE,
			LIGHT_ORANGE, INVISIBLE;
	}
}//mc

#endif