#include "Algorithm.hpp"
using Utils::Matrix;
using namespace DDM;

#include "Arithmetic.h"
#include "IOParams.h"
#include "MatrixIO.hpp"
using namespace UIUtils;
#include "tclap/CmdLine.h"
#include "tclap/ValuesConstraint.h"

#include <iostream>
#include <string>
#include <vector>

// Disable pointless Visual Studio warnings (recommending strcopy_s, etc.).
#pragma warning( disable : 4996 )


struct CommandLineArgs
{
    Arithmetic arithmetic;
    bool checkResult;
    IOParams ioParams;
    Parameters parameters;
    double zerotol;
};


/* Parse command line to get input-output, arith and quickhull params.
Return boolean value if launch should be performed (shouldn't in case of errors
in input or after printing help or version). */
void parseCommandLine(int argc, char *argv[], CommandLineArgs * args,
    bool * continueExecution);


/* Check result by solving dual task, return if it is correct.
Floating-point arithmetic may lead to failed test. */
template <typename T>
bool check(const Matrix<T>& inequalities,
    const Matrix<T>& extremeRays, const std::vector<size_t>& facets,
    Parameters& params, bool intArithmetic, const T& zerotol);

/* Process task: read input, run qkeleton, write output, and check result if
necessary. */
template <typename T>
void processTask(Parameters& params, IOParams& ioParams, bool intArithmetic,
    const T& zerotol, bool checkResult);


int main(int argc, char *argv[])
{
    // Parse command line.
    CommandLineArgs args;
    bool continueExecution;
    parseCommandLine(argc, argv, &args, &continueExecution);

    // Exit if necessary.
    if (!continueExecution)
        return 0;

    // Process task using chosen arithmetic.
    if (args.arithmetic == Arithmetic::Int)
        processTask<int>(args.parameters, args.ioParams, true, 0, args.checkResult);
    else if (args.arithmetic == Arithmetic::Double)
        processTask<double>(args.parameters, args.ioParams, false, args.zerotol, args.checkResult);
    else if (args.arithmetic == Arithmetic::Float)
        processTask<float>(args.parameters, args.ioParams, false, (float)args.zerotol, args.checkResult);

    return 0;
}


void parseCommandLine(int argc, char *argv[], CommandLineArgs * args,
    bool * continueExecution)
{
    using namespace TCLAP;
    using std::string;
    using std::vector;

    // initialize with default values
    args->zerotol = 1e-8;
    *continueExecution = false;
    try
    {
        CmdLine cmd(
            "You are running the implementation of the double-description method.\n"
            "The program is under the GNU Lesser General Public License 3, see COPYING.\n"
            "Copyright (C) Sergey Bastrakov, 2013\n",
            ' ',
            "0.1");

        vector<string> newArgs;
        for (int i = 0; i < argc; i++)
            newArgs.push_back(string(argv[i]));
        IOParamsReader reader;
        reader.prepare(cmd, newArgs);

        /* Here and below the default value is explicitly added to the text
        description, as it seems to be no prominent way to make TCLAP do it. */
        ValuesConstraint<string> arithmeticConstraint(Arithmetic::names());
        ValueArg<string> arithmetic("a", "arithmetic",
            "Arithmetic type, default = " + Arithmetic::names()[0] + ".", false,
            Arithmetic::names()[0], &arithmeticConstraint, cmd);

        ValuesConstraint<string> pivotingOrderConstraint(PivotingOrder::names());
        ValueArg<string> pivotingOrder("p", "pivoting",
            "Order of adding inequalities, default = " + PivotingOrder::names()[0]
            + ".", false, PivotingOrder::names()[0], &pivotingOrderConstraint, cmd);

        ValuesConstraint<string> setRepresentationConstraint
            (SetRepresentation::names());
        ValueArg<string> setRepresentation("", "setrepresentation",
            "Representation of sets, default = " + SetRepresentation::names()[0]
            + ".", false, SetRepresentation::names()[0],
            &setRepresentationConstraint, cmd);

        ValuesConstraint<string> adjacencyTestConstraint(AdjacencyTest::names());
        ValueArg<string> adjacencyTest("", "checkadj",
            "Way to check adjcency, default = " + AdjacencyTest::names()[0] + ".",
            false, AdjacencyTest::names()[0], &adjacencyTestConstraint, cmd);

        SwitchArg plusplusFlag("", "plusplus",
            "Enable plusplus for edge elimination.",
            cmd, false);

        SwitchArg checkResultFlag("", "check",
            "Check result after computation. Warning: it could take "
            "much more time and/or memory than computation itself,"
            "and could be not precise for floating-point arithmetic.",
            cmd, false);

        int newArgc = newArgs.size();
        char** newArgv = new char*[newArgc];
        for (int i = 0; i < newArgc; i++)
        {
            newArgv[i] = new char[newArgs[i].size() + 1];
            strcpy(newArgv[i], newArgs[i].c_str());
        } 
        cmd.parse(newArgc, newArgv);
        for (int i = 0; i < newArgc; i++)
            delete [] newArgv[i];
        delete [] newArgv;

        reader.read(cmd, &args->ioParams);
        args->parameters.logStream = &args->ioParams.logStream.get();
        args->parameters.summaryStream = &args->ioParams.summaryStream.get();
        args->parameters.verboseLog = args->ioParams.verboseLog;

        args->arithmetic = arithmetic.getValue();
        args->parameters.pivotingOrder = pivotingOrder.getValue();
        args->parameters.adjacencyTest = adjacencyTest.getValue();
        args->parameters.setRepresentation = setRepresentation.getValue();
        args->parameters.usePlusPlus = plusplusFlag.getValue();
        args->checkResult = checkResultFlag.getValue();
    }
    catch (ArgException & e)
    {
        std::cerr << "COMMAND LINE ERROR: " << e.error() << " for arg " << e.argId() << "\n";
        return;
    }
    catch(...)
    {
        std::cerr << "INTERNAL ERROR: unknown exception has been thrown.\n";
        return;
    }

    // write params to log
    *args->parameters.logStream << "Arithmetic: " << args->arithmetic << "\n";
    *args->parameters.logStream << args->ioParams;
    *args->parameters.summaryStream << args->parameters << "\n";

    *continueExecution = true;
}




template <typename T>
void processTask(Parameters& params, IOParams& ioParams, bool intArithmetic,
    const T& zerotol, bool checkResult)
{
    // Read input matrix.
    Matrix<T> inequalities;
    bool succeed = readMatrix(ioParams.inputStream.get(), inequalities);
    if (!succeed)
        return;

    // Run qskeleton and write output.
    time_t beginTime;
    time(&beginTime);
    std::cout << "Computation started: " << asctime(localtime(&beginTime)) << "\n";
    srand((unsigned int)beginTime);
    Matrix<T> extremeRays;
    std::vector<size_t> facets;
    ddm(inequalities, params, intArithmetic, zerotol, extremeRays, facets);
    writeMatrix(ioParams.outputStream.get(), extremeRays);
    time_t endTime;
    time(&endTime);
    std::cout << "\nComputation finished: " << asctime(localtime(&endTime));

    // Check result if neccesary.
    if (checkResult)
    {
        std::cout << "Checking result...";
        ioParams.logStream.setNull();
        params.logStream = &ioParams.logStream.get();
        ioParams.summaryStream.setNull();
        params.summaryStream = &ioParams.summaryStream.get();
        bool resultCorrect = check(inequalities, extremeRays, facets, params,
            intArithmetic, zerotol);
        if (resultCorrect)
            std::cout << "PASSED.\n";
        else
            std::cout << "FAILED.\n";
    }
}





template <typename T>
bool check(const Matrix<T>& inequalities,
    const Matrix<T>& extremeRays, const std::vector<size_t>& facets,
    Parameters& params, bool intArithmetic, const T& zerotol)
{
    // At first, simply check that all extreme rays satisfy all inequalities.
    for (size_t i = 0; i < extremeRays.nrows(); i++)
        for (size_t j = 0; j < inequalities.nrows(); j++)
        {
            T dotProduct = 0;
            for (size_t k = 0; k < extremeRays.ncols(); ++k)
                dotProduct += extremeRays(i, k) * inequalities(j, k);
            if (dotProduct < -zerotol)
                return false;
        }

    // After simple check passed, solve dual task for precise check.
    Matrix <T> inequalitiesFromDual;
    std::vector<size_t> facetsFromDual;
    ddm(extremeRays, params, intArithmetic, zerotol, inequalitiesFromDual,
        facetsFromDual);

    // Output of the dual task should be all facets of original task, check it.
    if (facets.size() != inequalitiesFromDual.nrows())
        return false;
    for (size_t i = 0; i < facets.size(); i++)
    {
        bool isPresent = false;
        for (size_t j = 0; j < inequalitiesFromDual.nrows(); j++)
        {
            isPresent = true;
            for (size_t k = 0; k < inequalitiesFromDual.ncols(); k++)
                if (abs(inequalities(facets[i], k) - inequalitiesFromDual(j, k))
                    > zerotol)
                {
                    isPresent = false;
                    break;
                }
            if (isPresent)
                break;
        }
        if (!isPresent)
            return false;
    }
    return true;
}
