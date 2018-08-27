
#include <zsLib/eventing/tool/ICommandLine.h>
#include <zsLib/eventing/tool/OutputStream.h>

#include <iostream>

namespace zsLib { namespace eventing { namespace tool { ZS_DECLARE_SUBSYSTEM(zslib_eventing_tool) } } }

using namespace zsLib::eventing::tool;

int main(int argc, char * const argv[])
{
  try {
     ICommandLine::StringList arguments;

	  if (argc > 0) {
		argv = &(argv[1]);
		--argc;
	  }

	  output().installStdOutput();

	  arguments = ICommandLine::toList(argc, argv);
	  return ICommandLine::performDefaultHandling(arguments);
   } catch (const std::exception &e) {
	   std::cout << "Uncaught exception: " << e.what() << std::endl;
	   return 255;
   }
   } catch (...) {
	   std::cout << "Uncaught exception of unknown type" << std::endl;
	   return 255;
   }
}
