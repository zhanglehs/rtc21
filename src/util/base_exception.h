#pragma once

#include <exception>
#include <string>

namespace live_stream_sdk
{
  class BaseException : public std::exception
  {
  public:
    BaseException(const std::string& name);
    virtual ~BaseException()  throw(){}
    void SetMessage(const std::string& msg);
    const std::string GetErrorMessage();
    const std::string GetNameAndError();

    virtual const std::string& Name();

  protected:
    std::string _message;
    std::string _name;
  };
}
