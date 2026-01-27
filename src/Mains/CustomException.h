#pragma once

#include <exception>

class CustomException : public std::exception {
public:
	CustomException(const char* msg) : message(msg) {}
	const char* what() {
		return message;
	}

private:
	const char* message;
};