#ifndef GENERICIOSTREAM_H
#define GENERICIOSTREAM_H


#include <fstream>
#include <string>


namespace UIUtils
{


/* Class for generic istream being either stdin or file. */
class GenericIStream
{
public:
    // Default constructor creates istream being stdin.
    GenericIStream();
    ~GenericIStream();
    // Set stream to given file, return whether operation succeed.
    bool setFile(const std::string& filename);
    // Get istream.
    std::istream& get() const;
    // Get string name: stdin or name of file.
    const std::string getName() const;

private:
    std::istream* stream;
    std::ifstream file;
    std::string name;

    // Copy constructor and assignment are disallowed.
    GenericIStream(const GenericIStream& );
    GenericIStream& operator =(const GenericIStream& );
};


/* Class for generic ostream being either stdout, file, or null stream
('black hole' to output anything with no effect). */
class GenericOStream
{
public:
    // Default constructor creates ostream being stdout.
    GenericOStream();
    ~GenericOStream();
    // Set stream to given file, return whether operation succeed.
    bool setFile(const std::string& filename);
    // Set stream to null stream.
    void setNull();
    // Get ostream.
    std::ostream& get() const;
    // Get string name: stdout, name of file, or "-" for null stream.
    const std::string getName() const;

private:
    std::ostream * stream;
    std::ofstream file;
    std::string name;

    // Copy constructor and assignment are disallowed.
    GenericOStream(const GenericOStream& );
    GenericOStream& operator =(const GenericOStream& );
};


} // namespace UIUtils


#endif
