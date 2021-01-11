#pragma once
#include <exception>
#include <system_error>
#include <future>
#include <iostream>

// This class was taken from the book
// The C++ Standard Library Second Edition: A Tutorial and Reference
// Author: Nicolai M. Josuttis
// pages 50-51
class ExceptionHandler
{
public:
	void ProcessException()
	{
		try
		{
			throw; // rethrow exception
		}
		catch (const std::ios_base::failure& e)
		{
			std::cerr << "I/O EXCEPTION: " << e.what() << std::endl;
			ProcessCodeException(e);
		}
		catch (const std::system_error& e)
		{
			std::cerr << "SYSTEM EXCEPTION: " << e.what() << std::endl;
			ProcessCodeException(e);
		}
		catch (const std::future_error& e)
		{
			std::cerr << "FUTURE EXCEPTION: " << e.what() << std::endl;
			ProcessCodeException(e);
		}
		catch (const std::bad_alloc& e)
		{
			std::cerr << "BAD ALLOC EXCEPTION: " << e.what() << std::endl;
		}
		catch (const std::exception& e)
		{
			std::cerr << "EXCEPTION: " << e.what() << std::endl;
		}
		catch (...)
		{
			std::cerr << "EXCEPTION (unkwown)" << std::endl;
		}
	}

private:
	template<typename T>
	void ProcessCodeException(const T& e)
	{
		auto c = e.code();
		std::cerr << "- category:     " << c.category().name() << std::endl;
		std::cerr << "- value:        " << c.value() << std::endl;
		std::cerr << "- msg:          " << c.message() << std::endl;
		std::cerr << "- def category: " << c.default_error_condition().category().name() << std::endl;
		std::cerr << "- def value:    " << c.default_error_condition().value() << std::endl;
		std::cerr << "- def msg:      " << c.default_error_condition().message() << std::endl;
	}
};