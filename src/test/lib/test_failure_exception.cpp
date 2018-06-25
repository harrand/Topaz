//
// Created by Harrand on 25/06/2018.
//

#include "test_failure_exception.hpp"

const char* TestFailureException::what() const throw()
{
    return this->msg.c_str();
}
