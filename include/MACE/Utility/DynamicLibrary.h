/*
Copyright (c) 2016-2019 Liav Turkia

See LICENSE.md for full copyright information
*/
#pragma once
#ifndef MACE__UTILITY_DYNAMIC_LIBRARY_H
#define MACE__UTILITY_DYNAMIC_LIBRARY_H

#include <MACE/Core/Constants.h>

#include <string>

namespace mc {
	class DynamicLibrary {
	public:
		static DynamicLibrary getRunningProcess();

		~DynamicLibrary();
		DynamicLibrary();
		DynamicLibrary(const std::string& path);
		DynamicLibrary(const char* path);

		void init(const std::string& path);
		void init(const char* path);

		void destroy();

		void* getSymbol(const std::string& name) const;
		void* getSymbol(const char* name) const;

		void* operator[](const char* name) const;
		void* operator[](const std::string& name) const;

		bool isCreated() const;

#if defined(MACE_WINAPI)&&defined(MACE_EXPOSE_WINAPI)
		void* getHandle() const {
			return dll;
		}
#elif defined(MACE_POSIX)&&defined(MACE_EXPOSE_POSIX)
		void* getDescriptor() const {
			return dll;
		}
#endif
	private:
		bool created;

		void* dll;
		};//DynamicLibrary
	}//mc

#endif//MACE__UTILITY_DYNAMIC_LIBRARY_H
