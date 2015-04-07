#include "Elimination.hpp"
#include "GaussianElimination.hpp"
using Utils::Matrix;
using namespace Elimination;

#include "Arithmetic.h"
#include "IOParams.h"
#include "MatrixIO.hpp"
using namespace UIUtils;
#include <tclap/CmdLine.h>
#include <tclap/ValuesConstraint.h>

#include <iostream>
#include <string>
using std::string;
#include <vector>

// Disable pointless Visual Studio warnings (recommending strcopy_s, etc.).
#pragma warning( disable : 4996 )


struct CommandLineArgs
{
    Arithmetic arithmetic;
    IOParams ioParams;
    Parameters parameters;
    std::vector<size_t> eliminationVariables;
    bool computeDualDescription;
};


/* Parse command line to get input-output, arith and quickhull params.
Return boolean value if launch should be performed (shouldn't in case of errors
in input or after printing help or version). */
void parseCommandLine(int argc, char *argv[], CommandLineArgs * args,
    bool * continueExecution);

/* Process task: read input, run eliminaion, write output. */
template <typename T>
void processTask(const CommandLineArgs& args);

/* Transform inequality matrix to find dual description via elimination. */
template <typename T>
void prepareDoubleDescriptionInput(const Parameters& parameters,
    Matrix<T>& inequalities, std::vector<size_t>& eliminationOrder,
    Matrix<T>& bas);

/* Transform result of finding dual description to original format. */
template <typename T>
void prepareDoubleDescriptionOutput(size_t dim, Matrix<T>& result,
    const Matrix<T>& bas);


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
    args.parameters.zerotol = 1e-6;
    if (args.arithmetic == Arithmetic::Int)
    {
        args.parameters.zerotol = 0;
        processTask<int>(args);
    }
    else if (args.arithmetic == Arithmetic::Double)
        processTask<double>(args);
    else if (args.arithmetic == Arithmetic::Float)
        processTask<float>(args);

    return 0;
}


void parseCommandLine(int argc, char *argv[], CommandLineArgs * args,
    bool * continueExecution)
{
    using namespace TCLAP;
    using std::string;
    using std::vector;

    *continueExecution = false;
    try
    {
        CmdLine cmd(
            "You are running the implementation of the Fourier-Motzkin elimination.\n"
            "The program is under the GNU Lesser General Public License 3, see COPYING.\n"
            "Copyright (C) Sergey Bastrakov, 2013\n",
            ' ',
            "0.2");

        vector<string> newArgs;
        for (int i = 0; i < argc; i++)
            newArgs.push_back(string(argv[i]));
        IOParamsReader reader;
        reader.prepare(cmd, newArgs);
        ValueArg<string> eliminationFilename("e", "elimination",
            "File with variables to eliminate in the format: \n"
            "N \n"
            "i1 i2 ... iN\n"
            "where N is number of variables, "
            "i1, ..., iN are indexes of variables, indexes start from 0.",
            false, "",
            "filename", cmd);

        /* Here and below the default value is explicitly added to the text
        description, as it seems to be no prominent way to make TCLAP do it. */
        ValuesConstraint<string> arithmeticConstraint(Arithmetic::names());
        ValueArg<string> arithmetic("a", "arithmetic",
            "Arithmetic type, default = " + Arithmetic::names()[0],
            false, Arithmetic::names()[0], &arithmeticConstraint, cmd);

        ValuesConstraint<string> chernikovTestConstraint(ChernikovTest::names());
        ValueArg<string> chernikovTest("t", "test",
            "Way to perform Chernikov test, default = "
            + ChernikovTest::names()[0] + ".",
            false, ChernikovTest::names()[0], &chernikovTestConstraint, cmd);        

        ValuesConstraint<string> eliminationOrderingConstraint(
            EliminationOrdering::names());
        ValueArg<string> eliminationOrdering("", "ordering",
            "Ordering of eliminated variables, default = " +
            EliminationOrdering::names()[0] + ".", false,
            EliminationOrdering::names()[0], &eliminationOrderingConstraint,
            cmd);   

         SwitchArg computeDualDescriptionFlag("d", "dualdescription",
            "Use elimination to compute dual description of a given cone.",
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
        args->parameters.intArithmetic = args->arithmetic.isInteger();
        args->parameters.chernikovTest = chernikovTest.getValue();
        args->parameters.eliminationOrdering = eliminationOrdering.getValue();
        args->computeDualDescription = computeDualDescriptionFlag.getValue();

        if (eliminationFilename.isSet() && args->computeDualDescription)
        {
            std::cerr << "ERROR: --" << eliminationFilename.getName()
                << " and --" << computeDualDescriptionFlag.getName()
                << " are incompatible.\n";
            return;
        }
        if (eliminationFilename.isSet())
        {
            string filename = eliminationFilename.getValue();
            std::ifstream eliminationFile(filename.c_str(),
                std::ios::in);
            if (!eliminationFile)
            {
                std::cerr << "ERROR: could not open elimination file "
                    << filename << ".\n";
                return;
            }
            size_t n = 0;
            eliminationFile >> n;
            for (size_t i = 0; i < n; ++i)
            {
                size_t var;
                eliminationFile >> var;
                args->eliminationVariables.push_back(var);
            }
        }
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

    *args->parameters.logStream << "Arithmetic: " << args->arithmetic << "\n";
    if (args->computeDualDescription)
    {
        *args->parameters.logStream << "Find dual description using elimination\n";
        args->parameters.variableName = "y";
    }
    else
    {
        if (args->eliminationVariables.empty())
            *args->parameters.logStream << "Eliminate all variables\n";
        else
            *args->parameters.logStream << "Eliminate specified "
                << args->eliminationVariables.size() << " variables\n";
        args->parameters.variableName = "x";
    }
    *args->parameters.logStream << args->ioParams;
    *args->parameters.summaryStream << args->parameters << "\n";

    *continueExecution = true;
}


template <typename T>
void processTask(const CommandLineArgs& args)
{
    Matrix<T> inequalities;
    bool succeed = readMatrix(args.ioParams.inputStream.get(), inequalities);
    if (!succeed)
        return;
    size_t dim = inequalities.ncols();

    std::vector<size_t> eliminationVariables = args.eliminationVariables;
    Matrix<T> bas;
    if (args.computeDualDescription)
        prepareDoubleDescriptionInput(args.parameters, inequalities,
            eliminationVariables, bas);
    else
        if (eliminationVariables.empty())
            for (size_t i = 0; i < dim; ++i)
                eliminationVariables.push_back(i);

    time_t beginTime;
    time(&beginTime);
    std::cout << "Computation started: " << asctime(localtime(&beginTime))
        << "\n";
    srand((unsigned int)beginTime);
    Matrix<T> result;
    elimination(inequalities, eliminationVariables, args.parameters, result);
    time_t endTime;
    time(&endTime);
    std::cout << "\nComputation finished: " << asctime(localtime(&endTime));

    if (args.computeDualDescription)
        prepareDoubleDescriptionOutput(dim, result, bas);
    writeMatrix(args.ioParams.outputStream.get(), result);
}


template <typename T>
void prepareDoubleDescriptionInput(const Parameters& parameters,
    Matrix<T>& inequalities, std::vector<size_t>& eliminationVariables,
    Matrix<T>& bas)
{
    /* To compute the dual description of the cone Ax >= 0 with n x d matrix A:
    - introduce new variables y1, ..., yn,
      the dual cone is defined by x = tr(A)*y, y >= 0
    - eliminate y1, ..., yn from the system of n + d variables:
        (tr(A) | -E) (y)  = 0    (1) d equations
        (  E   |  0) (x) >= 0    (2) n inequalities
    - we could apply elimination directly to this system, but it is better to
      first remove some y's using (1) to reduce the number of iterations and
      avoid redundant output
    - let r be rank(tr(A)), B be r x d column base matrix of tr(A),
      tr(A) = (B | N), yB and yN - corresponding y variables, then multiplying (1)
      by inv(B) yields
      E*yB + inv(B)*N*yN - inv(B)*x = 0 => yB = -inv(B)*N*yN + inv(B)*x 
    - remove yB by putting this to y >=0, get the system of rank + n - d inequalities
      of n variables:
        (-inv(B)*N | inv(B)) (yN) >= 0    (1')
        (     E    |    0  ) (x)  >= 0    (2')
    - eliminate yN from this system using Fourier-Motzkin elimination
    - take C = the last d columns of the result (those, corresponding to x),
      then the dual cone is Cx >= 0 and the rows of C are coordiantes of the
      extreme rays of the original cone, i.e. {x: Ax >= 0} = cone(tr(C)).
    */
    size_t n = inequalities.nrows();
    size_t d = inequalities.ncols();

    // Set equations = (tr(A) | -E), the matrix of (1).
    Matrix<T> equations(d, n + d, 0);
    for (size_t i = 0; i < d; ++i)
    {
        for (size_t j = 0; j < n; ++j)
            equations(i, j) = inequalities(j, i);
        equations(i, i + n) = (T)(-1);
    }

    // Gaussian elimination on equations matrix.
    Matrix<T> invB;
    std::vector<size_t> perm;
    size_t rank;
    Utils::gauss<T>(transpose(equations), n, invB, bas, rank, perm,
        parameters.intArithmetic, (T)parameters.zerotol);
    std::vector<size_t> yB(perm.begin(), perm.begin() + rank);

    /* First, form matrix of system (1')-(2') with all variables, including yB:
        (-inv(B)*N | inv(B)) (yN) = yB
        (     E    |    0  ) (x)  >= 0
    */
    Matrix<T> extendedInequalities = mmult(invB, equations);
    for (size_t i = 0; i < rank; ++i)
        if (extendedInequalities(i, yB[i]) > 0)
            extendedInequalities.mult_row(i, (T)(-1));
    for (size_t i = 0; i < n; ++i)
        if (std::find(yB.begin(), yB.end(), i) == yB.end())
        {
            extendedInequalities.insert_row(extendedInequalities.nrows());
            extendedInequalities(extendedInequalities.nrows() - 1, i) = 1;
        }

    // Remove columns corresponding to yB, the remaining part is >= 0.
    inequalities = transpose(extendedInequalities);
    std::sort(yB.begin(), yB.end(), std::greater<size_t>());
    for (size_t i = 0; i < yB.size(); ++i)
        inequalities.erase_row(yB[i]);
    inequalities = transpose(inequalities);
    // Eliminate all variables except the last d.
    eliminationVariables.clear();
    for (size_t i = 0; i < inequalities.ncols() - d; ++i)
        eliminationVariables.push_back(i);
}


template <typename T>
void prepareDoubleDescriptionOutput(size_t dim, Matrix<T>& result,
    const Matrix<T>& bas)
{
    // Leave last d columns of result plus all equations.
    size_t numEquations = bas.nrows();
    Matrix<T> newResult(2 * numEquations + result.nrows(), dim);
    // Add all equation as pairs of inequalities.
    for (size_t i = 0; i < numEquations; ++i)
        for (size_t j = 0; j < dim; ++j)
        {
            newResult(2 * i, j) = bas(i, j);
            newResult(2 * i + 1, j) = -bas(i, j);
        }
    size_t shift = result.ncols() - dim;
    for (size_t i = 0; i < result.nrows(); ++i)
        for (size_t j = 0; j < dim; ++j)
            newResult(2 * numEquations + i, j) = result(i, j + shift);
    result = newResult;
}
