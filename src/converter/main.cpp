#include <fstream>
#include <iostream>
#include <string>
using namespace std;


int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cout << "Not enough arguments.\n"
            << "USAGE: " << argv[0] << " inputfile outputfile\n";
        return 1;
    }

    string inputName = argv[1];
    ifstream input(inputName.c_str(), std::ios::in);
    if (!input)
    {
        cout << "Can not open input file " << inputName << "\n";
        return 1;
    }
    string outputName = argv[2];
    ofstream output(outputName.c_str(), std::ios::out);
    if (!output)
    {
        cout << "Can not open output file " << outputName << "\n";
        return 1;
    }

    // Now only qskeleton -> PORTA and only for integers
    int n, d;    
    input >> n >> d;
    output << "DIM = " << d << "\n\n";
    output << "CONV_SECTION\n\n";
    output << "CONE_SECTION\n";
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < d; ++j)
        {
            int a;
            input >> a;
            output << a << " ";
        }
        output << "\n";
    }
    output << "\nEND\n";
    output << "DIMENSION OF THE POLYHEDRON : " << d << "\n";

    return 0;
}