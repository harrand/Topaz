//
// Created by Harrand on 25/06/2018.
//

#ifndef TOPAZ_TEST_FAILURE_EXCEPTION_HPP
#define TOPAZ_TEST_FAILURE_EXCEPTION_HPP

#include <exception>
#include <string>

class TestFailureException : public std::exception
{
public:
    TestFailureException(std::string msg);
    virtual const char* what() const throw() override;
private:
    std::string msg;
};


#endif //TOPAZ_TEST_FAILURE_EXCEPTION_HPP
