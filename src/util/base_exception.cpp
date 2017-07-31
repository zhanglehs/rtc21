#include "../util/base_exception.h"

using namespace std;

namespace live_stream_sdk
{

  BaseException::BaseException(const string& name)
  {
    _name = name;
  }

  void BaseException::SetMessage(const string& msg)
  {
    _message = msg;
  }

  const string BaseException::GetErrorMessage()
  {
    return _message;
  }

  const string BaseException::GetNameAndError()
  {
    return Name() + ": " + GetErrorMessage();
  }

  const string& BaseException::Name()
  {
    return _name;
  }
}

