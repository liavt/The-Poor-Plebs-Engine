/*
The MIT License (MIT)

Copyright (c) 2016 Liav Turkia and Shahar Sandhaus

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/
#include <MC-System/Utility/Math.h>
#include <cmath>

namespace mc{

	namespace math
	{
		long double pi() {
			return 3.14159265358979323846264338327950288419716939937510l;
		}
		long double tau()
		{
			return 6.28318530717958647692528676655900576839433879875021l;
		}
		long double gamma()
		{
			return 0.57721566490153286060651209008240243104215933593992l;
		}
		long double e() {
			return 2.71828182845904523536028747135266249775724709369995l;
		}
		long double root2()
		{
			return 1.41421356237309504880168872420969807856967187537694l;
		}
		long double root3()
		{
			return 1.73205080756887729352744634150587236694280525381038l;
		}
		long double root5()
		{
			return 2.23606797749978969640917366873127623544061835961152l;
		}
		long double phi()
		{
			return 1.61803398874989484820458683436563811772030917980576l;
		}
		constexpr double abs(const double value)
		{
			return value < 0 ? -value : value;
		}

		int ceil(const double value)
		{
			int out = floor(value);
			if (out != value)out++;//ceil(5) should equal 5, not 6. check to make sure it is actually a whole number
			return out;//so cheap, but it works
		}
		int floor(const double value)
		{
			int out = static_cast<int>(value);
			if (out < 0)out--;//so floor(-5.7) equals -6, not -5. compilers will round towards zero, floor() shouldnt
			return out;
		}

		bool isPrime(const int n)
		{
			if (n % 2 == 0) {
				return false;
			}
			for (int i = 3; i * i <= n; i += 2) {
				if (n % i == 0) {
					return false;
				}
			}
			return true;
		}

		constexpr bool isEven(const int value)
		{
			return value % 2 == 0;
		}

		constexpr bool isOdd(const int value)
		{
			return !isEven(value);
		}

		//pow pow is having fun tonight
		double pow(const double value, const int power)
		{
			if (power == 0)return 1.0;//any number to the power of 0 is 1. even 0. 0^0=1. axioms are hilarious.

			double out = value;
			//boom! pow!
			for (unsigned int i = 0; i < abs(power) - 1; i++) {
				out = out * value;
			}

			if (power < 0) {
				out = (1.0 / out);
			}

			return out;
		}

		constexpr double sqr(const double value)
		{
			return value*value;
		}

		constexpr double cube(const double value)
		{
			return value*value*value;
		}

		double toRadians(const double degrees)
		{
			return degrees*(pi() / 180.0);
		}

		double toDegrees(const double radians)
		{
			return radians*(180.0 / pi());
		}


		Matrix4f getProjectionMatrix(float FOV, float NEAR_PLANE, float FAR_PLANE, float aspectRatio)
		{
			const float y_scale = (float)((1.0f / tan(toRadians(FOV / 2.0f))) * aspectRatio);
			const float x_scale = y_scale / aspectRatio;
			const float frustum_length = FAR_PLANE - NEAR_PLANE;

			Matrix4f projectionMatrix = Matrix4f();
			projectionMatrix[0][0] = x_scale;
			projectionMatrix[1][1] = y_scale;
			projectionMatrix[2][2] = -((FAR_PLANE + NEAR_PLANE) / frustum_length);
			projectionMatrix[2][3] = -1;
			projectionMatrix[3][2] = -((2 * NEAR_PLANE * FAR_PLANE) / frustum_length);
			projectionMatrix[3][3] = 0;

			return projectionMatrix;
		}
	}
}