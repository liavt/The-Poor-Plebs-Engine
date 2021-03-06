/*
Copyright (c) 2016-2019 Liav Turkia

See LICENSE.md for full copyright information
*/
#pragma once
#ifndef MACE__CORE_ERROR_H
#define MACE__CORE_ERROR_H
#include <string>
#include <stdexcept>
#include <MACE/Core/Constants.h>

namespace mc {
	//forward declaration for handleError();
	class Instance;

	/**
	Superclass that all exceptions in MACE extend.
	*/
	class Error: public std::runtime_error {
	public:
		static std::string getErrorDump(const std::exception& e, const Instance* i = nullptr);

		/**
		Stops MACE and prints an exception to console accordingly. This should be used every time a fatal exception is thrown.
		*/
		static void handleError MACE_NORETURN (const std::exception& e, Instance* i = nullptr);// i is not const because requestStop() will be called

		/**
		@copydoc Error::handleError(const std::exception& e, Instance*)
		*/
		static void handleError MACE_NORETURN (const std::exception& e, Instance& i);

		Error(const char* message, const unsigned int line, const std::string file);
		Error(const std::string message, const unsigned int line, const std::string file);
		Error(const char* message, const unsigned int line, const char* file);
		Error(const std::string message, const unsigned int line = 0, const char* file = "No file reported");
		Error() noexcept = default;
		~Error() noexcept = default;

		void handle MACE_NORETURN ();

		const unsigned int getLine() const;
		const char* getFile() const;
	private:
		const unsigned int line;
		const char* file;
	};

#define MACE__GET_ERROR_NAME(name) name##Error
#define MACE__DECLARE_ERROR(name) class MACE__GET_ERROR_NAME(name) : public Error{public: using Error::Error;}
#define MACE__THROW_CUSTOM_LINE(name, message, line, file) do{throw MACE__GET_ERROR_NAME(name) ( std::string(__func__) + ": " + std::string(message), line, file);}while(0)
#define MACE__THROW(name, message) MACE__THROW_CUSTOM_LINE(name, message, __LINE__, __FILE__)

	/**
	Thrown when an error from an unknown source occured
	*/
	MACE__DECLARE_ERROR(Unknown);

	/**
	Thrown when a pointer is equal to NULL
	*/
	MACE__DECLARE_ERROR(NullPointer);

	/**
	Thrown when an assertion fails.
	@see MACE::assertModule(std::string)
	*/
	MACE__DECLARE_ERROR(AssertionFailed);

	/**
	Thrown when a resource already exists and you are attempting to recreate it
	*/
	MACE__DECLARE_ERROR(AlreadyExists);

	/**
	Thrown when an object wasn't initializaed or initializtion failed
	*/
	MACE__DECLARE_ERROR(InitializationFailed);

	/**
	Thrown when an error occured trying to read or write a file
	@see FileNotFoundError
	*/
	MACE__DECLARE_ERROR(BadFile);

	/**
	Thrown when a file was not found on the filesystem
	@see BadFileError
	*/
	MACE__DECLARE_ERROR(FileNotFound);

	/**
	Thrown when a function looks for an object, but doesn't find it.
	*/
	MACE__DECLARE_ERROR(ObjectNotFound);

	/**
	Thrown when something is of the wrong type
	*/
	MACE__DECLARE_ERROR(InvalidType);

	/**
	Thrown when an index is provided for an array, but it is outside the valid bounds of the array.
	*/
	MACE__DECLARE_ERROR(OutOfBounds);

	/**
	Thrown when something was in the wrong state at the time of an operation
	*/
	MACE__DECLARE_ERROR(InvalidState);

	/**
	Thrown when the process has no remaining memory available
	*/
	MACE__DECLARE_ERROR(OutOfMemory);

	/**
	Thrown when the operating system throws an error
	*/
	MACE__DECLARE_ERROR(System);

	class MultipleErrors: public Error {
	public:
		using Error::Error;

		MultipleErrors(const Error errs[], const std::size_t errorSize, const unsigned int line, const char* file);

		const char* what() const noexcept override;
	private:
		std::string message;
	};
}

#endif