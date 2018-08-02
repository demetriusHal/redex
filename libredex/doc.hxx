
/**
  \mainpage

  \section intro Introduction

  This is the doxygen documentation of redex code. Unfortunately, the codebase is not documented in any
  useful way, although several pieces of code have documenting comments in them. 

  \subsection intro_exec_redex Executing redex

  The \b redex command is actually a self-extracting file, which extracts into a python wrapper and the
  \b redex-all binary. The python wrapper is responsible for unpacking an .apk file, exposing the .dex file(s) in it.
  Then, \b redex-all analyses and rewrites the .dex files, and finally the python wrapper reassemples the 
  .apk file. The complete logic of the python wrapper is contained in file redex.py and in the pyredex package.

  \subsection intro_redex_all The execution of redex-all

  The file containing the \c main() function is  tools/redex-all/main.cpp. At a high level, the tasks performed
  in the main function are the following

    1. Arguments are parsed.
    2. The RedexContext global object, `g_redex`, is created
    3. The %ProGuard configuration is read
    4. The root DexStore object is created. It corresponds to the first .dex file on the command line.
        If more .dex files are there on the command line (a so-called multidex .apk is processed), then more
        DexStore objects are created (one per .dex file). <br/>
		Note that the file on the command line must end in '.dex', or else it must be a so-called "metadata"
		file (see DexMetadata for more). This requires further examination...
	5. A list of external .jar files ("library jars") is also read into a Scope obect 
	6. Names are de-obfuscated by using ProGuard map files.
	7. A PassManager is created and passes are executed according to configuration
	8. Optimized .dex files are written to disk.

\section passes Passes

A pass is defined as a subclass of abstract class Pass. The execution of passes happens in two steps.
In the first step, method Pass::eval_pass() is executed for each pass. In the second step, method Pass::run_pass()
is executed. The idea is that during the first step, each pass can see the original input in the dex stores
whereas in the second pass the order of passes affects what each pass encounters.

Each Pass subclass is a singleton class and it is instantiated as a static. Note: care must be taken when linking, but
apparently the current build machinery has this nailed down.

\subsection passes_implementing Implementing a pass

\section dex_file_model The DEX file model

- DEX file model DexClass.h
  - DexClasses and Scope are both aliases for std::vector<DexClass*>
- walkers in Walkers.h
- dex refs and rebinding ReBindRefs


\section testing_the_code Testing the code

In order to write unit tests for new code, there is a number of facilities. Since we are using GNU autotools,
the test execution happens by 
> make check     
to run all tests, and
> make recheck   
to run only tests that failed previously.

N.B. At this time [Aug/18] the facebook-redex test code is not maintained well, therefore only a subset of
the tests compile and run. 

 \subsection testing_tracing Tracing execution

   Inside the code, one can use the @ref TRACE macro to generate messages as the code executes.
   >  TRACE(MODULE, trace_level, fmt, ...)
   The above printf-ish call designates a module and a level. Modules are listed in libredex/Trace.h
   and you can add your own modules there. The level is a positive integer (higher means more detail).
   Typical values are 1 and 2. The `fmt` and the following args are passed to a printf formatter.

   Tracing is enabled by environment variables:
   > TRACE= level
   The above must be defined for any tracing to happen.
   Finer configuration of tracing can be done with 
   - TRACEFILE: filename to send trace output to
   - SHOW_TIMESTAMPS: if defined, show timestamps
   - SHOW_TRACEMODULE: if defined, show module name
   - TRACE_METHOD_FILTER: if defined, list of module names to trace

 \subsection testing_assertions Adding assertions

   There are several useful assertion macros in libredex/Debug.h. Follow the link to read them.
   
   N.B. There is also the @ref CHECK macro in shared/Util.h. This macro is not currently used in the code,
   so don't use it (it may be removed from facebook-redex later)!

 \subsection testing_writing Writing tests for clue-redex

 Tests can be written using the Google Test library. See https://github.com/google/googletest for the documentation.
 The facebook-redex code uses this library as well. 

 Tests can be added to directory test/clue. In order to write a test suite you can start by copying file 
 test/clue/SampleTest.cpp to a new filename. Also, add your new test file to test/clue/Makefile.am, following
 the pattern for sample_test in that file (note: Makefile.am is a template for generating a Makefile using GNU automake. 
 For more features of GNU automake, see the 
 [automake documentation](https://www.gnu.org/software/automake/manual/automake.html) )

*/

/** @file tools/redex-all/main.cpp */

// These declarations declare header files in libredex as public

/** \file ApkManager.h */
/** \file CallGraph.h */
/** \file ClassHierarchy.h */
/** \file ConcurrentContainers.h */
/** \file ConfigFiles.h */
/** \file ControlFlow.h */
/** \file Creators.h */
/** \file Dataflow.h */
/* \file Debug.h */
/** \file DexAccess.h */
/** \file DexAnnotation.h */
/** \file DexAsm.h */
/** \file DexClass.h */
/** \file DexDebugInstruction.h */
/** \file DexIdx.h */
/** \file DexInstruction.h */
/** \file DexLoader.h */
/** \file DexMemberRefs.h */
/** \file DexOpcode.h */
/** \file DexOutput.h */
/** \file DexPosition.h */
/** \file DexStore.h */
/** \file DexUtil.h */
/** \file Gatherable.h */
/** \file ImmutableSubcomponentAnalyzer.h */
/** \file Inliner.h */
/** \file InstructionAnalyzer.h */
/** \file InstructionLowering.h */
/** \file IRAssembler.h */
/** \file IRCode.h */
/** \file IRInstruction.h */
/** \file IRList.h */
/** \file IROpcode.h */
/** \file IRTypeChecker.h */
/** \file JarLoader.h */
/** \file Match.h */
/** \file MethodDevirtualizer.h */
/** \file Mutators.h */
/** \file Pass.h */
/** \file PassManager.h */
/** \file PassRegistry.h */
/** \file PluginRegistry.h */
/** \file PointsToSemantics.h */
/** \file PointsToSemanticsUtils.h */
/** \file PrintSeeds.h */
/** \file ProguardConfiguration.h */
/** \file ProguardLexer.h */
/** \file ProguardMap.h */
/** \file ProguardMatcher.h */
/** \file ProguardParser.h */
/** \file ProguardPrintConfiguration.h */
/** \file ProguardRegex.h */
/** \file ProguardReporting.h */
/** \file ReachableClasses.h */
/** \file ReachableObjects.h */
/** \file RedexContext.h */
/** \file RedexResources.h */
/** \file ReferencedState.h */
/** \file Resolver.h */
/** \file Show.h */
/** \file SimpleReflectionAnalysis.h */
/** \file SimpleValueAbstractDomain.h */
/** \file StringBuilder.h */
/** \file Timer.h */
/** \file Trace.h */
/** \file Transform.h */
/** \file TypeSystem.h */
/** \file Vinfo.h */
/** \file VirtualScope.h */
/** \file Walkers.h */
/** \file Warning.h */
/** \file WorkQueue.h */

