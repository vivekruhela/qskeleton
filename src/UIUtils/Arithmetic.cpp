#include "Arithmetic.h"

#include <algorithm>


namespace UIUtils
{


Arithmetic::Arithmetic(Type _type):
    type(_type)
{
}

Arithmetic::Arithmetic(const std::string& s)
{
    std::vector<std::string> ns = names();
    type = Type(distance(ns.begin(), find(ns.begin(), ns.end(), s)));
}

bool Arithmetic::operator ==(const Arithmetic& a) const
{
    return type == a.type;
}

bool Arithmetic::isInteger() const
{
    return (type == Int);
}

std::vector<std::string> Arithmetic::names()
{
    std::vector<std::string> ns(numTypes);
    ns[Int]= "int";
    ns[Double] = "double";
    ns[Float] = "float";
    return ns;
}

std::ostream& operator <<(std::ostream& os, const Arithmetic& a)
{
    return os << Arithmetic::names()[int(a.type)];
}


} // namespace UIUtils
