
#include <zsLib/eventing/tool/ICommandLine.h>
#include <zsLib/eventing/tool/OutputStream.h>

#include <iostream>

namespace zsLib { namespace eventing { namespace tool { ZS_DECLARE_SUBSYSTEM(zslib_eventing_tool) } } }

using namespace zsLib::eventing::tool;

int main(int argc, char * const argv[])
{
  ICommandLine::StringList arguments;

  if (argc > 0) {
    argv = &(argv[1]);
    --argc;
  }

  output().installStdOutput();

  arguments = ICommandLine::toList(argc, argv);
  return ICommandLine::performDefaultHandling(arguments);
}
