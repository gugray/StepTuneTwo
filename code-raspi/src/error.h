#ifndef ERROR_H
#define ERROR_H

#include <stdexcept>

#define THROWF(fmt, ...) throwf(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define THROWF_ERRNO(fmt, ...) throwf_errno(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)

class st_exception : public std::runtime_error
{
  private:
    const std::string _file;
    const int _line;
    const std::string _func;

  public:
    st_exception(const std::string &msg, const char *file, int line, const char *func)
        : std::runtime_error(msg)
        , _file(file)
        , _line(line)
        , _func(func)
    {
    }

    const char *file() const
    {
        return _file.c_str();
    }

    int line() const
    {
        return _line;
    }

    const char *func() const
    {
        return _func.c_str();
    }
};

void throwf(const char *file, int line, const char *func, const char *fmt, ...);
void throwf_errno(const char *file, int line, const char *func, const char *fmt, ...);

#endif
