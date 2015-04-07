#ifndef ARITHMETIC_H
#define ARITHMETIC_H


#include <string>
#include <vector>


namespace UIUtils
{


/* Supported arithmetic types. */
class Arithmetic
{
public:

    enum Type {Int, Double, Float, numTypes};

    Arithmetic(Type type = Type(0));
    Arithmetic(const std::string& s);
    bool operator ==(const Arithmetic& a) const;
    bool isInteger() const;
    static std::vector<std::string> names();
    friend std::ostream& operator <<(std::ostream& os, const Arithmetic& a);

private:

    Type type;
};


} // namespace UIUtils


#endif
