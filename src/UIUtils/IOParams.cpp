#include "IOParams.h"

#include <tclap/ValuesConstraint.h>
using namespace TCLAP;

#include <iostream>
using std::ostream;
using std::string;
using std::vector;


namespace UIUtils
{


std::ostream& operator <<(ostream& os, const IOParams& params)
{
    os << "I/O parameters:\n";
    os << "    input: " + params.inputStream.getName() << "\n";
    os << "    output: " + params.outputStream.getName() << "\n";
    os << "    log: " + params.logStream.getName()
        << (params.verboseLog ? ", verbose\n" : "\n");
    os << "    summary: " + params.summaryStream.getName() << "\n";
    return os;
}


/* If args contains no arguments from given list, add default argument to args.
*/
void addDefaultXorValue(const vector<Arg*>& list,
    const Arg* defaultValue, vector<string>& args)
{
    bool argIsSet = false;
    for (vector<Arg*>::const_iterator li = list.begin();
        li != list.end(); li++)
    {
        for (size_t i = 0; i < args.size(); i++)
            if (!strcmp(args[i].c_str(), ("-" + (*li)->getFlag()).c_str()) ||
                !strcmp(args[i].c_str(), ("--" + (*li)->getName()).c_str()))
            {
                argIsSet = true;
            }
    }
    if (argIsSet)
        return;

    std::string newArg = (defaultValue->getName() != "") ?
        "--" + defaultValue->getName() : "-" + defaultValue->getFlag();
    args.push_back(newArg);
}


IOParamsReader::IOParamsReader():

    inputFilename("i", "ifile",
        "File with matrix of the input system of inequalities in form Ax >= 0: \n"
        "M N \n"
        "a11 a12 ... a1N\n"
        "a21 a22 ... a2N\n"
        "... \n"
        "aM1 aM2 ... aMN\n"
        "where M is number of inequalities, "
        "N is dimension (number of variables), "
        "A = { aIJ : I = 1, 2, ..., M, J = 1, 2, ... N }.",
        true, "", "filename"),
    unlabeledInputFilename("unlabeledInputFilename",
        "Input file, for file format see -" + inputFilename.getFlag() + ".",
        false, "", "filename"),
    inputFromStdin("", "istdin",
        "Read input from stdin, for format see -" + inputFilename.getFlag()
        + ".", false),

    outputFilename("o", "ofile",
        "File with the result matrix in the same format as input, "
        "for file format see -" + inputFilename.getFlag() + ".\n"
        "The interpretation of the result matrix depends on the task:\n"
        "* if you are finding dual description, the rows of the result matrix "
        "are the extreme rays of the input cone;\n"
        "* if you are doing elimination, the rows of the result matrix "
        "are the inequalities of the original system after the elimination.\n",
        true, "", "filename"),
    outputToStdout("", "ostdout",
        "Write result to stdout, for format see -" + outputFilename.getFlag()
        + ".", false),
    noOutput("", "nooutput", "Do not write output.", false),

    logFilename("l", "lfile", "Log output file.", true, "", "filename"),
    logToStdout("", "lstdout", "Write log to stdout.", false),
    noLog("", "nolog", "Do not write log.", false),
    verboseLog("V", "verbose", "Verbose log.", false),

    summaryFilename("s", "sfile", "Summary output file.", true, "", "filename"),
    summaryToStdout("", "sstdout", "Write summary to stdout.", false),
    noSummary("", "nosummary", "Do not write summary.", true)
{
}


void IOParamsReader::prepare(CmdLine& cmd, vector<string>& args)
{
    // input source: either input file or stdin
    std::vector<Arg *> inputXorList;
    inputXorList.push_back(&unlabeledInputFilename);
    inputXorList.push_back(&inputFilename);
    inputXorList.push_back(&inputFromStdin);
    cmd.xorAdd(inputXorList);

    // output destination: either to file or stdout or no output
    std::vector<Arg *> outputXorList;
    outputXorList.push_back(&outputFilename);
    outputXorList.push_back(&outputToStdout);
    outputXorList.push_back(&noOutput);
    addDefaultXorValue(outputXorList, &outputToStdout, args);
    cmd.xorAdd(outputXorList);

    // log output destination: either to file or stdout or no log output
    std::vector<Arg *> logXorList;
    logXorList.push_back(&logFilename);
    logXorList.push_back(&logToStdout);
    logXorList.push_back(&noLog);
    addDefaultXorValue(logXorList, &logToStdout, args);
    cmd.xorAdd(logXorList);

    // verbose log flag
    cmd.add(verboseLog);

    // summary output destination: either to file or stdout or no summary
    // output
    std::vector<Arg *> summaryXorList;
    summaryXorList.push_back(&summaryFilename);
    summaryXorList.push_back(&summaryToStdout);
    summaryXorList.push_back(&noSummary);
    addDefaultXorValue(summaryXorList, &summaryToStdout, args);
    cmd.xorAdd(summaryXorList);
}


void IOParamsReader::read(CmdLine& cmd, IOParams* params)
{
    // set input source
    if (!inputFromStdin.isSet())
    {
        std::string filename;
        if (inputFilename.isSet())
            filename = inputFilename.getValue();
        if (unlabeledInputFilename.isSet())
            filename = unlabeledInputFilename.getValue();
        bool succeed = params->inputStream.setFile(filename);
        if (!succeed)
        {
            std::cerr << "ERROR: could not open input file "
                 << filename << ".\n";
            return;
        }
    }

    // set output destination
    if (outputFilename.isSet())
    {
        bool succeed = params->outputStream.setFile(outputFilename.getValue());
        if (!succeed)
        {
            std::cerr << "ERROR: could not open output file "
                  << outputFilename.getValue() << ".\n";
            return;
        }
    }
    if (noOutput.isSet())
        params->outputStream.setNull();

     // set log output destination
    if (logFilename.isSet())
    {
        bool succeed = params->logStream.setFile(logFilename.getValue());
        if (!succeed)
        {
            std::cerr << "ERROR: could not open log output file "
                 << logFilename.getValue() << ".\n";
            return;
        }
    }
    if (noLog.isSet())
        params->logStream.setNull();
    else
        params->verboseLog = verboseLog.isSet();
        

    // set summary output destination
    if (summaryFilename.isSet())
    {
        bool succeed = params->summaryStream.setFile(summaryFilename.getValue());
        if (!succeed)
        {
            std::cerr << "ERROR: could not open summary output file "
                 << summaryFilename.getValue() << ".\n";
            return;
        }
    }
    if (noSummary.isSet())
       params->summaryStream.setNull();       
}


} // namespace UIUtils
