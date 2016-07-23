/*
The MIT License (MIT)

Copyright (c) 2016 Liav Turkia and Shahar Sandhaus

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/
#include <Catch.h>
#include <MC-System/Utility/Math.h>


namespace mc {
	TEST_CASE("Testing pow()", "[system][utility][math]") {
		REQUIRE(math::pow(2, 8) == 256);
		REQUIRE(math::pow(5, 2) == 25);
		REQUIRE(math::pow(4, 3) == 64);
		REQUIRE(math::pow(10, 0) == 1);
		REQUIRE(math::pow(-6, 3) == -216);
	}

	TEST_CASE("Testing ceil()", "[system][utility][math]") {
		REQUIRE(math::ceil(5.9) == 6);
		REQUIRE(math::ceil(2.1) == 3);
		REQUIRE(math::ceil(3) == 3);
	}

	TEST_CASE("Testing abs()", "[system][utility][math]") {
		REQUIRE(math::abs(5) == 5);
		REQUIRE(math::abs(-5) == 5);
		REQUIRE(math::abs(4.2) == 4.2);
		REQUIRE(math::abs(-4.2) == 4.2);
	}

	TEST_CASE("Testing floor()", "[system][utility][math]") {
		REQUIRE(math::floor(6.9) == 6);
		REQUIRE(math::floor(1.6) == 1);
		REQUIRE(math::floor(4.7) == 4);
		REQUIRE(math::floor(5) == 5);
		REQUIRE(math::floor(-5.7) == -6);
	}

	TEST_CASE("Testing isPrime()", "[system][utility][math]") {
		REQUIRE(math::isPrime(19));
		REQUIRE(!math::isPrime(360));
	}
}