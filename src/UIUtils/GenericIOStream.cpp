#include "GenericIOStream.h"


#include <iostream>
#include <sstream>


namespace UIUtils
{


GenericIStream::GenericIStream():
    stream(&std::cin),
    name("stdin")
{}

GenericIStream::~GenericIStream()
{
    if (file.is_open())
        file.close();
}

bool GenericIStream::setFile(const std::string& filename)
{
    if (file.is_open())
        file.close();
    file.open(filename.c_str(), std::ios::in);
    stream = &file;
    name = filename;
    return (file != 0);
}

std::istream& GenericIStream::get() const
{
    return *stream;
}

const std::string GenericIStream::getName() const
{ 
    return name;
}


GenericOStream::GenericOStream():
    stream(&std::cout),
    name("stdout")
{}

GenericOStream::~GenericOStream()
{
    if (file.is_open())
        file.close();
}

bool GenericOStream::setFile(const std::string& filename)
{
    if (file.is_open())
        file.close();
    file.open(filename.c_str(), std::ios::out);
    stream = &file;
    name = filename;
    return (file != 0);
}

void GenericOStream::setNull()
{
    // Here empty stringstream serves as a 'black hole'.
    static std::stringstream nullStream;
    stream = &nullStream;
    name = "-";
}

std::ostream& GenericOStream::get() const
{ 
    return *stream;
}

const std::string GenericOStream::getName() const
{ 
    return name;
}


} // namespace UIUtils
