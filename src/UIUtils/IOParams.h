#ifndef IO_PARAMS_H
#define IO_PARAMS_H


#include "GenericIOStream.h"

#include <tclap/CmdLine.h>

#include <iostream>
#include <string>


namespace UIUtils
{


/* Input and output parameters: genetic streams for input, output, log and
summary, verbose log flag. */
struct IOParams
{
    GenericIStream inputStream;
    GenericOStream outputStream;
    GenericOStream logStream;
    GenericOStream summaryStream;
    bool verboseLog;

    friend std::ostream& operator <<(std::ostream& os,
        const IOParams& params);
};


class IOParamsReader
{
public:

    IOParamsReader();
    void prepare(TCLAP::CmdLine& cmd, std::vector<std::string>& args);
    void read(TCLAP::CmdLine& cmd, IOParams* params);

private:

    TCLAP::ValueArg<std::string> inputFilename, outputFilename, logFilename,
        summaryFilename;
    TCLAP::SwitchArg inputFromStdin, outputToStdout, noOutput, logToStdout,
        noLog, verboseLog, summaryToStdout, noSummary;
    TCLAP::UnlabeledValueArg<std::string> unlabeledInputFilename;
};


}  // namespace UIUtils


#endif
