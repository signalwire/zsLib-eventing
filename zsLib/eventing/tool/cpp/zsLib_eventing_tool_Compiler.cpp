/*

Copyright (c) 2016, Robin Raymond
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

*/

#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_Compiler.h>

#include <zsLib/eventing/tool/OutputStream.h>

#include <zsLib/eventing/IHelper.h>
#include <zsLib/eventing/IHasher.h>
#include <zsLib/eventing/IEventingTypes.h>

#include <zsLib/Exception.h>
#include <zsLib/Numeric.h>

#include <sstream>
#include <list>
#include <set>

#define ZS_EVENTING_PREFIX "ZS_EVENTING_"

#define ZS_EVENTING_TOOL_INVALID_CONTENT (-2)
#define ZS_EVENTING_TOOL_FILE_FAILED_TO_LOAD (-3)
#define ZS_EVENTING_TOOL_METHOD_NOT_UNDERSTOOD (-4)
#define ZS_EVENTING_TOOL_SYSTEM_ERROR (-5)
#define ZS_EVENTING_TOOL_INTERNAL_ERROR (-99)

#define ZS_EVENTING_METHOD_PROVIDER "PROVIDER"
#define ZS_EVENTING_METHOD_ALIAS "ALIAS"
#define ZS_EVENTING_METHOD_INCLUDE "INCLUDE"
#define ZS_EVENTING_METHOD_SOURCE "SOURCE"
#define ZS_EVENTING_METHOD_CHANNEL "CHANNEL"
#define ZS_EVENTING_METHOD_TASK "TASK"
#define ZS_EVENTING_METHOD_OPCODE "OPCODE"
#define ZS_EVENTING_METHOD_TASK_OPCODE "TASK_OPCODE"
#define ZS_EVENTING_METHOD_INIT "INIT"
#define ZS_EVENTING_METHOD_DEINIT "DEINIT"


namespace zsLib { namespace eventing { namespace tool { ZS_DECLARE_SUBSYSTEM(zsLib_eventing_tool) } } }

namespace zsLib
{
  namespace eventing
  {
    ZS_DECLARE_TYPEDEF_PTR(IEventingTypes::Provider, Provider);

    namespace tool
    {
      ZS_DECLARE_TYPEDEF_PTR(eventing::IHelper, UseEventingHelper);
      ZS_DECLARE_TYPEDEF_PTR(eventing::IHasher, UseHasher);
      typedef std::set<String> HashSet;
      typedef std::map<size_t, String> ArgumentMap;
      typedef std::set<size_t> IndexSet;

      namespace internal
      {
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        #pragma mark
        #pragma mark Helpers
        #pragma mark

        struct ParseState
        {
          const char *mPos {};
          bool mStartOfLine {true};
        };

        //-----------------------------------------------------------------------
        static ICompilerTypes::Config &prepareProvider(ICompilerTypes::Config &config)
        {
          if (config.mProvider) return config;
          config.mProvider = Provider::create();
          return config;
        }

        //-----------------------------------------------------------------------
        static bool isLikelyJSON(const char *p)
        {
          while ('\0' != *p)
          {
            if (iswspace(*p)) {
              ++p;
              continue;
            }
            if (('{' == *p) || ('[' == *p)) return true;
            break;
          }
          return false;
        }

        //-----------------------------------------------------------------------
        static bool isNumber(const char *p)
        {
          while ('\0' != *p)
          {
            if (!isdigit(*p)) return false;
            ++p;
          }
          return true;
        }

        //-----------------------------------------------------------------------
        static bool skipWhitespaceExceptEOL(const char * &p)
        {
          bool found = false;

          while ('\0' != *p)
          {
            if (('\r' == *p) || ('\n' == *p)) break;
            if (!isspace(*p)) break;
            ++p;
          }
          return found;
        }

        //-----------------------------------------------------------------------
        static bool skipCComments(const char * &p)
        {
          if ('/' != *p) return false;
          if ('*' != *(p + 1)) return false;

          p += 2;

          while ('\0' != *p)
          {
            if ('*' != *p) continue;
            if ('/' != *(p + 1)) continue;

            p += 2;
            break;
          }

          return true;
        }

        //---------------------------------------------------------------------
        static bool skipEOL(const char * &p)
        {
          if (('\r' != *p) && ('\n' != *p)) return false;

          do
          {
            ++p;
          } while (('\r' == *p) || ('\n' == *p));

          return true;
        }

        //---------------------------------------------------------------------
        static void skipToEOL(const char * &p)
        {
          while ('\0' != *p)
          {
            if (!skipEOL(p)) {
              ++p;
              continue;
            }
            break;
          }
        }

        //---------------------------------------------------------------------
        static bool skipCPPComments(const char * &p)
        {
          if ('/' != *p) return false;
          if ('/' != *(p + 1)) return false;

          p += 2;

          skipToEOL(p);
          return true;
        }

        //---------------------------------------------------------------------
        static bool skipPreprocessorDirective(const char * &p)
        {
          if ('#' != *p) return false;

          while ('\0' != *p)
          {
            if (skipCComments(p)) break;
            if (skipCPPComments(p)) continue;
            if (skipEOL(p)) break;

            ++p;
          }
          return true;
        }

        //---------------------------------------------------------------------
        static bool skipQuote(const char * &p)
        {
          if ('\"' != *p) return false;

          ++p;
          while ('\0' != *p) {
            switch (*p) {
              case '\"': {
                ++p;
                return true;
              }
              case '\\': {
                ++p;
                if ('\0' == *p) return true;
                ++p;
                continue;
              }
            }
            ++p;
          }

          return true;
        }

        //---------------------------------------------------------------------
        static String getEventingLine(ParseState &state) throw (InvalidContent)
        {
          auto prefixLength = strlen(ZS_EVENTING_PREFIX);

          const char * &p = state.mPos;

          while ('\0' != *p)
          {
            if (skipWhitespaceExceptEOL(p)) continue;
            if (skipEOL(p)) {
              state.mStartOfLine = true;
              continue;
            }
            if (skipCComments(p)) continue;
            if (skipCPPComments(p)) continue;

            if (state.mStartOfLine) {
              // start of line
              if (skipPreprocessorDirective(p)) continue;
            }

            state.mStartOfLine = false;
            if (0 != strncmp(ZS_EVENTING_PREFIX, p, prefixLength)) {
              skipToEOL(p);
              state.mStartOfLine = true;
              continue;
            }

            p += prefixLength;

            const char *startPos = p;

            bool foundBracket = false;
            size_t bracketDepth = 0;

            while ('\0' != *p)
            {
              if (skipWhitespaceExceptEOL(p)) continue;
              if (skipEOL(p)) {
                state.mStartOfLine = true;
                continue;
              }
              if (skipCComments(p)) continue;
              if (skipCPPComments(p)) continue;
              if (state.mStartOfLine) {
                // start of line
                if (skipPreprocessorDirective(p)) continue;
              }
              state.mStartOfLine = true;
              if (skipQuote(p)) continue;

              char value = *p;
              ++p;

              if (')' == value) {
                if (0 == bracketDepth) {
                  ZS_THROW_CUSTOM(InvalidContent, String("Eventing mechanism found but closing bracket \')\' prematurely found"));
                }
                --bracketDepth;
                if (0 == bracketDepth) {
                  break;
                }
                continue;
              }
              if ('(' == *p) {
                ++bracketDepth;
                foundBracket = true;
                continue;
              }
            }
            if (0 != bracketDepth) {
              ZS_THROW_CUSTOM(InvalidContent, String("Eventing mechanism found but closing bracket \')\' not found"));
            }
            if (!foundBracket) {
              ZS_THROW_CUSTOM(InvalidContent, String("Eventing mechanism found but opening bracket \'(\' not found"));
            }

            return String(startPos, static_cast<size_t>(p - startPos));
          }

          return String();
        }

        //---------------------------------------------------------------------
        static void parseLine(
                              const char *p,
                              String &outMethod,
                              ArgumentMap &outArguments
                              ) throw (InvalidContent)
        {
          const char *startPos = p;

          while ('\0' != *p)
          {
            if (isalnum(*p)) {
              ++p;
              continue;
            }
            if ('_' == *p) {
              ++p;
              continue;
            }
            break;
          }

          outMethod = String(startPos, static_cast<size_t>(p - startPos));

          bool startOfLine = false;
          bool lastWasSpace = true;
          bool foundBracket = false;
          size_t bracketDepth = 0;
          bool done = false;
          size_t index = 0;

          std::stringstream ss;

          while ('\0' != *p)
          {
            {
              if (skipWhitespaceExceptEOL(p)) goto found_space;
              if (skipEOL(p)) {
                startOfLine = true;
                goto found_space;
              }
              if (skipCComments(p)) goto found_space;
              if (skipCPPComments(p)) goto found_space;
              if (startOfLine) {
                // start of line
                if (skipPreprocessorDirective(p)) goto found_space;
              }

              startOfLine = false;
              lastWasSpace = false;

              char value = *p;
              ++p;

              switch (value) {
                case '(':
                {
                  foundBracket = true;
                  ++bracketDepth;
                  if (1 != bracketDepth) {
                    ss << value;
                  }
                  break;
                }
                case ')':
                {
                  if (!foundBracket) {
                    ZS_THROW_CUSTOM(InvalidContent, String("Eventing mechanism closing bracket \')\' prematurely found"));
                  }
                  if (bracketDepth > 1) {
                    ss << value;
                  }
                  --bracketDepth;
                  if (0 == bracketDepth) {
                    done = true;
                    goto found_result;
                  }
                  break;
                }
                case ',':
                {
                  if (!foundBracket) {
                    ZS_THROW_CUSTOM(InvalidContent, String("Eventing mechanism found illegal comma \',\'"));
                  }
                  if (1 != bracketDepth) {
                    ss << value;
                    break;
                  }
                  break;

                  goto found_result;
                }
                default: break;
              }
              continue;
            }

          found_space:
            {
              if (!lastWasSpace) {
                ss << ' ';
                lastWasSpace = true;
              }
              continue;
            }

          found_result:
            {
              if (!done) continue;
              String result = ss.str();
              result.trim();

              if (result.hasData()) {
                outArguments[index] = result;
                ++index;
              }
              ss.swap(std::stringstream());
              lastWasSpace = true;
              break;
            }
          }
        }

        //---------------------------------------------------------------------
        static bool isQuotes(const String &str)
        {
          if (str.length() < 2) return false;

          const char *start = str.c_str();

          if (('L' == *start) ||
              ('u' == *start)) {
            ++start;
            if (str.length() < 3) return false;
          }

          if ('\"' != (*start)) return false;
          if ('\"' != (*(str.c_str() + str.length() - 1))) return false;
          return true;
        }

        //---------------------------------------------------------------------
        static String decodeQuotes(const String &str) throw (Failure)
        {
          if (!isQuotes(str)) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Failed to parse quoted string: " + str);
          }

          bool wstr = false;

          std::stringstream ss;

          const char *pos = str.c_str()+1;
          if ('L' == *(str.c_str())) {
            ++pos;
            wstr = true;
          }
          const char *stopPos = str.c_str() + str.length() - 1;

          bool isEscape = false;
          size_t pendingDigits = 0;
          size_t filledDigits = 0;
          bool isOctal = false;
          char digits[9];

          while (pos < stopPos) {
            {
              char value = *pos;
              ++pos;
              if (!isEscape) {
                if ('\\' == value) {
                  isEscape = true;
                  continue;
                }
                ss << value;
                continue;
              }

              if (pendingDigits > 0) {
                if (!isOctal) {
                  if (!(((value >= '0') && (value <= '9')) ||
                        ((value >= 'a') && (value <= 'f')) ||
                        ((value >= 'A') && (value <= 'F')))) {
                    --pos;  // backup once
                    goto digit_sequence_ended;
                  }
                } else {
                  if (!((value >= '0') && (value < '8'))) {
                    --pos;  // backup once
                    goto digit_sequence_ended;
                  }
                }

                digits[filledDigits] = value;

                ++filledDigits;
                --pendingDigits;
                continue;
              }

              switch (value)
              {
                case '\\': ss << '\\'; break;
                case '\'': ss << '\''; break;
                case '\"': ss << '\"'; break;
                case '?': ss << '\?'; break;
                case 'a': ss << '\a'; break;
                case 'b': ss << '\b'; break;
                case 'f': ss << '\f'; break;
                case 'n': ss << '\n'; break;
                case 'r': ss << '\r'; break;
                case 't': ss << '\t'; break;
                case 'v': ss << '\v'; break;
                case 'u': pendingDigits = 4; goto found_digit_sequence;
                case 'U': pendingDigits = 8; goto found_digit_sequence;
                case 'x': pendingDigits = 2; goto found_digit_sequence;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9': {
                  isOctal = true;
                  pendingDigits = 2;
                  goto found_digit_sequence;
                }
                default: {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Invalid digit sequence: " + str);
                }
              }

              isEscape = false;
              continue;
            }

          found_digit_sequence:
            {
              filledDigits = 0;
              memset(&(digits[0]), 0, sizeof(digits));
              continue;
            }
          digit_sequence_ended:
            {
              if (0 == filledDigits) {
                ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Invalid digit sequence: " + str);
              }

              try {
                if (!isOctal) {
                  wchar_t temp[2] = {};
                  temp[0] = static_cast<wchar_t>(Numeric<wchar_t>(&(digits[0]), true, 16));
                  ss << String(&(temp[0]));
                } else {
                  char temp = static_cast<char>(Numeric<unsigned char>(&(digits[0]), true, 8));
                  ss << temp;
                }
              } catch (const Numeric<unsigned char>::ValueOutOfRange &) {
                ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Invalid digit sequence: " + str);
              } catch (const Numeric<wchar_t>::ValueOutOfRange &) {
                ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Invalid digit sequence: " + str);
              }

              isOctal = false;
              pendingDigits = 0;
              filledDigits = 0;
              isEscape = false;
              continue;
            }
          }

          if (isEscape) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Invalid string escape sequence: " + str);
          }

          return ss.str();
        }

        //---------------------------------------------------------------------
        static bool insert(
                           IndexSet &indexes,
                           size_t index,
                           bool throwIfFound = true
                           ) throw (InvalidArgument)
        {
          if (0 == index) return false;
          
          auto found = indexes.find(index);
          if (found == indexes.end()) {
            indexes.insert(index);
            return true;
          }

          if (throwIfFound) {
            ZS_THROW_INVALID_ARGUMENT(String("Duplicate value: ") + string(index));
          }
          return false;
        }

        //---------------------------------------------------------------------
        static String toSymbol(const String &str)
        {
          String temp(str);
          temp.replaceAll("-", "_");
          temp.replaceAll("/", "_");
          temp.trim();
          temp.toUpper();
          return temp;
        }

        //---------------------------------------------------------------------
        static ElementPtr createStringEl(const String &id, const char *value)
        {
          ElementPtr stringEl = Element::create("string");
          stringEl->setAttribute("id", id);
          stringEl->setAttribute("value", value);
          return stringEl;
        }

        //---------------------------------------------------------------------
        static ElementPtr createDataEl(const String &inType, const char *name)
        {
          ElementPtr dataEl = Element::create("string");
          dataEl->setAttribute("inType", "win:" + inType);
          dataEl->setAttribute("name", name);
          return dataEl;
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark Compiler
        #pragma mark

        //---------------------------------------------------------------------
        Compiler::Compiler(
                         const make_private &,
                         const Config &config
                         ) :
          mConfig(config)
        {
        }

        //---------------------------------------------------------------------
        Compiler::~Compiler()
        {
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark Compiler => ICompiler
        #pragma mark

        //---------------------------------------------------------------------
        CompilerPtr Compiler::create(const Config &config)
        {
          CompilerPtr pThis(std::make_shared<Compiler>(make_private{}, config));
          pThis->mThisWeak = pThis;
          return pThis;
        }

        //---------------------------------------------------------------------
        void Compiler::process() throw (Failure)
        {
          read();
          prepareIndex();
          validate();
          if ((mConfig.mOutputName.hasData()) &&
              (mConfig.mProvider)) {
            writeXML(mConfig.mOutputName + ".man", generateManifest());
            writeXML(mConfig.mOutputName + ".wprp", generateWprp());
            writeJSON(mConfig.mOutputName + ".jman", generateJsonMan());
            String outputXPlatformNameStr = mConfig.mOutputName + "_events.h";
            String outputWindowsNameStr = mConfig.mOutputName + "_events_win.h";
            writeBinary(outputXPlatformNameStr, generateXPlatformEventsHeader(outputXPlatformNameStr, outputWindowsNameStr));
            writeBinary(outputWindowsNameStr, generateWindowsEventsHeader(outputXPlatformNameStr, outputWindowsNameStr));
          }
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark Compiler => (internal)
        #pragma mark

        //---------------------------------------------------------------------
        void Compiler::read() throw (Failure)
        {
          HashSet processedHashes;

          ProviderPtr &provider = mConfig.mProvider;

          auto configRaw = UseEventingHelper::loadFile(mConfig.mConfigFile);
          if (!configRaw) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_FILE_FAILED_TO_LOAD, String("Failed to load main configuration file: ") + mConfig.mConfigFile);
          }
          processedHashes.insert(UseHasher::hashAsString(configRaw));
          auto rootEl = UseEventingHelper::read(configRaw);

          try {
            provider = Provider::create(rootEl);
          } catch (const InvalidContent &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Failed to parse main configuration: " + e.message());
          }

          StringList sources = mConfig.mSourceFiles;
          mConfig.mSourceFiles.clear();

          ElementPtr sourcesEl = rootEl->findFirstChildElement("includes");
          if (sourcesEl) {
            ElementPtr sourceEl = sourcesEl->findFirstChildElement("include");
            while (sourceEl) {
              auto source = UseEventingHelper::getElementTextAndDecode(sourceEl);

              if (source.hasData()) {
                mConfig.mSourceFiles.push_back(source);
              }
              sourceEl = sourceEl->findNextSiblingElement("include");
            }
          }

          // put back the original configuration files
          for (auto iter = sources.begin(); iter != sources.end(); ++iter) {
            mConfig.mSourceFiles.push_back(*iter);
          }

          ElementPtr includesEl = rootEl->findFirstChildElement("sources");
          if (includesEl) {
            ElementPtr includeEl = includesEl->findFirstChildElement("source");
            while (includeEl) {
              auto source = UseEventingHelper::getElementTextAndDecode(includeEl);

              if (source.hasData()) {
                mConfig.mSourceFiles.push_back(source);
              }
              includeEl = includeEl->findNextSiblingElement("source");
            }
          }

          while (mConfig.mSourceFiles.size() > 0)
          {
            String fileName = mConfig.mSourceFiles.front();
            mConfig.mSourceFiles.pop_front();

            auto file = UseEventingHelper::loadFile(fileName);
            if (!file) {
              ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_FILE_FAILED_TO_LOAD, String("Failed to load file: ") + fileName);
            }
            auto hashResult = UseHasher::hashAsString(file);
            auto found = processedHashes.find(hashResult);
            if (found != processedHashes.end()) {
              tool::output() << "[Info] Duplicate file found thus ignoring: " << fileName << "\n";
              continue;
            }
            const char *fileAsStr = reinterpret_cast<const char *>(file->BytePtr());
            auto isJSON = internal::isLikelyJSON(fileAsStr);

            if (isJSON) {
              try {
                tool::output() << "\n[Info] Reading JSON configuration: " << fileName << "\n\n";
                auto rootEl = UseEventingHelper::read(file);
                if (!rootEl) {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_FILE_FAILED_TO_LOAD, String("Failed to load file as JSON: ") + fileName);
                }
                if (!provider) {
                  provider = Provider::create(rootEl);
                } else {
                  provider->parse(rootEl);
                }
              } catch (const InvalidContent &e) {
                ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Failed to parse JSON configuration: " + e.message());
              }
              continue;
            }
            tool::output() << "\n[Info] Reading C/C++ source file: " << fileName << "\n\n";

            ParseState state;
            state.mPos = reinterpret_cast<const char *>(file->BytePtr());
            while ('\0' != *(state.mPos))
            {
              String line = getEventingLine(state);
              if (line.isEmpty()) continue;
              
              String method;
              ArgumentMap args;
              parseLine(line.c_str(), method, args);

              if (method.isEmpty()) continue;

              if (isNumber(method.c_str())) {
                size_t totalParams {};
                try
                {
                  totalParams = Numeric<decltype(totalParams)>(method);
                } catch (const Numeric<decltype(totalParams)>::ValueOutOfRange &) {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INTERNAL_ERROR, "Event value out of range: " + line);
                }

                if (args.size() != (6 + (3 * totalParams))) {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Number of parameters mismatch: " + line);
                }

                prepareProvider(mConfig);

                auto event = IEventingTypes::Event::create();
                event->mSubsystem = provider->aliasLookup(args[0]);
                try {
                  event->mSeverity = Log::toSeverity(provider->aliasLookup(args[1]));
                } catch (const InvalidArgument &) {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Event has invalid severity: " + line);
                }
                try {
                  event->mLevel = Log::toLevel(provider->aliasLookup(args[2]));
                } catch (const InvalidArgument &) {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Event has invalid level: " + line);
                }

                String channelID = provider->aliasLookup(args[3]);
                String taskID = provider->aliasLookup(args[4]);
                String opCode = provider->aliasLookup(args[5]);

                // map channel
                {
                  auto found = provider->mChannels.find(channelID);
                  if (found == provider->mChannels.end()) {
                    ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Event has invalid channel: " + line);
                  }
                  event->mChannel = (*found).second;
                }

                // map task + task opcode
                {
                  // map task
                  {
                    auto found = provider->mTasks.find(taskID);
                    if (found == provider->mTasks.end()) {
                      ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Event has invalid task: " + line);
                    }
                    event->mTask = (*found).second;
                  }
                  // map task opcode (if revelant)
                  {
                    auto found = event->mTask->mOpCodes.find(opCode);
                    if (found != event->mTask->mOpCodes.end()) {
                      event->mOpCode = (*found).second;
                    }
                  }
                }

                // map opcode
                if (!event->mOpCode)
                {
                  auto found = provider->mOpCodes.find(opCode);
                  if (found == provider->mOpCodes.end()) {
                    try {
                      auto predefinedOpCode = IEventingTypes::toPredefinedOpCode(opCode);
                      event->mOpCode = IEventingTypes::OpCode::create();
                      event->mOpCode->mName = opCode;
                      event->mOpCode->mValue = predefinedOpCode;
                      provider->mOpCodes[opCode] = event->mOpCode;
                      goto found_predefined_opcode;
                    } catch (const InvalidArgument &) {
                    }
                    ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Event has invalid opCode: " + line);
                  }
                  event->mOpCode = (*found).second;
                }

              found_predefined_opcode: {}

                IEventingTypes::DataTemplatePtr dataTemplate;
                if (totalParams > 0) {
                  dataTemplate = IEventingTypes::DataTemplate::create();
                }

                for (decltype(totalParams) loop = totalParams; loop < totalParams; ++loop)
                {
                  String type = provider->aliasLookup(args[6 + (loop * 3)]);
                  String name = provider->aliasLookup(args[6 + (loop * 3) + 1]);

                  auto dataType = IEventingTypes::DataType::create();
                  dataType->mValueName = name;

                  // check if there's a typedef
                  {
                    auto found = provider->mTypedefs.find(type);
                    if (found != provider->mTypedefs.end()) {
                      dataType->mType = (*found).second->mType;
                    } else {
                      try {
                        dataType->mType = IEventingTypes::toPredefinedTypedef(type);
                      } catch (const InvalidArgument &) {
                        ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, String("Event has invalid type \"") + type + "\" in line: " + line);
                      }
                    }
                  }

                  dataTemplate->mDataTypes.push_back(dataType);
                }

                if (dataTemplate) {
                  String hash = dataTemplate->hash();

                  {
                    auto found = provider->mDataTemplates.find(hash);
                    if (found == provider->mDataTemplates.end()) {
                      provider->mDataTemplates[hash] = dataTemplate; // remember new template
                    } else {
                      dataTemplate = (*found).second; // replace with existing template
                    }
                  }
                }

                event->mDataTemplate = dataTemplate;

                {
                  auto found = provider->mEvents.find(event->mName);
                  if (found != provider->mEvents.end()) {
                    ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, String("Event has duplicate symbol \"") + event->mName + "\" in line: " + line);
                  }
                }

                tool::output() << "[Info] Found event: " << event->mName << "\n";
                provider->mEvents[event->mName] = event;
                continue;
              }

              if (ZS_EVENTING_METHOD_PROVIDER == method) {
                if (5 != args.size()) {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Invalid number of arguments in provider \"" + string(args.size()) + "\" in line: " + line);
                }

                prepareProvider(mConfig);

                String uuidStr = decodeQuotes(provider->aliasLookup(args[0]));
                String nameStr = provider->aliasLookup(args[1]);
                String symbolNameStr = decodeQuotes(provider->aliasLookup(args[2]));
                String descriptionStr = decodeQuotes(provider->aliasLookup(args[3]));
                String resouceNameStr = decodeQuotes(provider->aliasLookup(args[4]));

                try {
                  decltype(provider->mID) id = Numeric<decltype(provider->mID)>(uuidStr);
                  if (!(!(provider->mID))) {
                    if (id != provider->mID) {
                      ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, String("Provider ID has been redefined \"") + uuidStr + "\" in line: " + line);
                    }
                  }
                  provider->mID = id;
                } catch (const Numeric<decltype(provider->mID)>::ValueOutOfRange &) {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, String("Provider ID is invalid \"") + uuidStr + "\" in line: " + line);
                }

                if (provider->mName.hasData()) {
                  if (nameStr != provider->mName) {
                    ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, String("Provider name has been redefined \"") + nameStr + "\" in line: " + line);
                  }
                }
                provider->mName = nameStr;

                if (provider->mSymbolName.hasData()) {
                  if (symbolNameStr != provider->mSymbolName) {
                    ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, String("Provider symbol name has been redefined \"") + symbolNameStr + "\" in line: " + line);
                  }
                }
                provider->mSymbolName = symbolNameStr;

                if (provider->mDescription.hasData()) {
                  if (descriptionStr != provider->mDescription) {
                    ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, String("Provider description has been redefined \"") + descriptionStr + "\" in line: " + line);
                  }
                }
                provider->mDescription = descriptionStr;

                if (provider->mResourceName.hasData()) {
                  if (resouceNameStr != provider->mResourceName) {
                    ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, String("Provider resource name has been redefined \"") + resouceNameStr + "\" in line: " + line);
                  }
                }
                provider->mResourceName = resouceNameStr;
                continue;
              }

              if (ZS_EVENTING_METHOD_ALIAS == method) {
                if (2 != args.size()) {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Invalid number of arguments in alias \"" + string(args.size()) + "\" in line: " + line);
                }
                prepareProvider(mConfig);

                {
                  auto found = provider->mAliases.find(args[0]);
                  if (found != provider->mAliases.end()) {
                    auto existingAlias = (*found).second;
                    if (existingAlias != args[1]) {
                      ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, String("Alias has been redefined \"") + args[0] + "\" in line: " + line);
                    }
                    continue;
                  }
                }

                provider->mAliases[args[0]] = args[1];
                tool::output() << "[Info] Found alias: " << args[0] << " -> " << args[1] << "\n";
                continue;
              }
              if (ZS_EVENTING_METHOD_SOURCE == method) {
                if (1 != args.size()) {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Invalid number of arguments in source \"" + string(args.size()) + "\" in line: " + line);
                }
                prepareProvider(mConfig);
                mConfig.mSourceFiles.push_back(decodeQuotes(provider->aliasLookup(args[0])));
                tool::output() << "[Info] Found source: " << args[0] << "\n";
                continue;
              }
              if (ZS_EVENTING_METHOD_INCLUDE == method) {
                if (1 != args.size()) {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Invalid number of arguments in source \"" + string(args.size()) + "\" in line: " + line);
                }
                prepareProvider(mConfig);
                mConfig.mSourceFiles.push_front(decodeQuotes(provider->aliasLookup(args[0])));
                tool::output() << "[Info] Found include: " << args[0] << "\n";
                continue;
              }
              if (ZS_EVENTING_METHOD_CHANNEL == method) {
                if (3 != args.size()) {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Invalid number of arguments in channel \"" + string(args.size()) + "\" in line: " + line);
                }
                prepareProvider(mConfig);

                auto channel = IEventingTypes::Channel::create();
                channel->mID = provider->aliasLookup(args[0]);
                channel->mName = decodeQuotes(provider->aliasLookup(args[1]));
                try {
                  channel->mType = IEventingTypes::toOperationalType(provider->aliasLookup(args[2]));
                } catch (const InvalidArgument &) {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Invalid operational type: " + line);
                }

                {
                  auto found = provider->mChannels.find(channel->mID);
                  if (found != provider->mChannels.end()) {
                    auto existingChannel = (*found).second;
                    if ((existingChannel->mID != channel->mID) ||
                        (existingChannel->mName != channel->mName) ||
                        (existingChannel->mType != channel->mType)) {
                      ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, String("Channel has been redefined \"") + channel->mID + "\" in line: " + line);
                    }
                    channel = existingChannel;
                  } else {
                    provider->mChannels[channel->mID] = channel;
                  }
                }
                tool::output() << "[Info] Found channel: " << channel->mID << "\n";
                continue;
              }
              if (ZS_EVENTING_METHOD_TASK == method) {
                if (1 != args.size()) {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Invalid number of arguments in task \"" + string(args.size()) + "\" in line: " + line);
                }
                prepareProvider(mConfig);

                auto task = IEventingTypes::Task::create();
                task->mName = provider->aliasLookup(args[0]);

                {
                  auto found = provider->mTasks.find(task->mName);
                  if (found == provider->mTasks.end()) {
                    provider->mTasks[task->mName] = task;
                  }
                }
                tool::output() << "[Info] Found task: " << task->mName << "\n";
                continue;
              }
              if (ZS_EVENTING_METHOD_OPCODE == method) {
                if (1 != args.size()) {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Invalid number of arguments in opcode \"" + string(args.size()) + "\" in line: " + line);
                }
                prepareProvider(mConfig);

                auto opCode = IEventingTypes::OpCode::create();
                opCode->mName = provider->aliasLookup(args[0]);

                {
                  auto found = provider->mOpCodes.find(opCode->mName);
                  if (found == provider->mOpCodes.end()) {
                    provider->mOpCodes[opCode->mName] = opCode;
                  }
                }
                tool::output() << "[Info] Found opcode: " << opCode->mName << "\n";
                continue;
              }

              if (ZS_EVENTING_METHOD_TASK_OPCODE == method) {
                if (2 != args.size()) {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Invalid number of arguments in task opcode \"" + string(args.size()) + "\" in line: " + line);
                }
                prepareProvider(mConfig);

                auto opCode = IEventingTypes::OpCode::create();
                String taskName = provider->aliasLookup(args[0]);
                opCode->mName = provider->aliasLookup(args[1]);

                IEventingTypes::TaskPtr task;
                {
                  auto found = provider->mTasks.find(taskName);
                  if (found == provider->mTasks.end()) {
                    ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, String("Task is missing \"") + taskName + "\" in line: " + line);
                  }
                  task = (*found).second;
                }

                {
                  auto found = task->mOpCodes.find(opCode->mName);
                  if (found == task->mOpCodes.end()) {
                    provider->mOpCodes[opCode->mName] = opCode;
                  }
                }
                tool::output() << "[Info] Found task opcode: " << task->mName << ", " << opCode->mName << "\n";
                continue;
              }

              if ((ZS_EVENTING_METHOD_INIT == method) ||
                  (ZS_EVENTING_METHOD_DEINIT == method)) {
                prepareProvider(mConfig);
                if (provider->mName.hasData()) {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, String("INIT does not match provider name \"") + provider->mName + "\" in line: " + line);
                }
                continue;
              }

              ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_METHOD_NOT_UNDERSTOOD, "Method is not valid: " + method);
            }
          }
        }

        //---------------------------------------------------------------------
        void Compiler::prepareIndex() throw (Failure)
        {
          if (!mConfig.mProvider) return;

          ProviderPtr &provider = mConfig.mProvider;

          IndexSet consumedOpCodeIndexes;

          try {
            IndexSet consumedIndexes;

            for (auto iter = provider->mChannels.begin(); iter != provider->mChannels.end(); ++iter)
            {
              auto channel = (*iter).second;
              if (0 == channel->mValue) continue;
              if ((channel->mValue < 1) && (channel->mValue > (sizeof(BYTE)<<8))) {
                ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Channel index is not valid: " + string(channel->mValue));
              }
              insert(consumedIndexes, channel->mValue);
            }
          } catch (const InvalidArgument &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Channel index is not valid: " + e.message());
          }

          try {
            IndexSet consumedIndexes;

            for (auto iter = provider->mTasks.begin(); iter != provider->mTasks.end(); ++iter)
            {
              auto task = (*iter).second;
              if (0 == task->mValue) continue;
              if ((task->mValue < 1) && (task->mValue > 239)) {
                ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Task index is not valid: " + string(task->mValue));
              }
              insert(consumedIndexes, task->mValue);
            }

            size_t current = 1;
            for (auto iter = provider->mTasks.begin(); iter != provider->mTasks.end(); ++iter)
            {
              auto task = (*iter).second;
              if (0 != task->mValue) continue;
              
              while (!insert(consumedIndexes, current, false)) { ++current; }
              task->mValue = current;
            }
            if (current > 239) {
              ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Task index is too large: " + string(current));
            }
          } catch (const InvalidArgument &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Task index is not valid: " + e.message());
          }

          try {
            // make sure to consume every index from every task
            for (auto iter = provider->mTasks.begin(); iter != provider->mTasks.end(); ++iter)
            {
              auto task = (*iter).second;

              for (auto iterOp = provider->mOpCodes.begin(); iterOp != provider->mOpCodes.end(); ++iterOp)
              {
                auto opCode = (*iterOp).second;
                if (0 == opCode->mValue) continue;
                insert(consumedOpCodeIndexes, opCode->mValue, false);
              }
            }

            for (auto iter = provider->mOpCodes.begin(); iter != provider->mOpCodes.end(); ++iter)
            {
              auto opCode = (*iter).second;
              if (0 == opCode->mValue) continue;
              if ((opCode->mValue < 10) && (opCode->mValue > 239)) {
                try {
                  auto predefinedType = IEventingTypes::toPredefinedOpCode(opCode->mName);
                  goto found_predefined_opcode;
                } catch (const InvalidArgument &) {
                }
                ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "OpCode index is not valid: " + string(opCode->mValue));
              }
            found_predefined_opcode: {}
              insert(consumedOpCodeIndexes, opCode->mValue);
            }

            size_t current = 10;
            for (auto iter = provider->mOpCodes.begin(); iter != provider->mOpCodes.end(); ++iter)
            {
              auto opCode = (*iter).second;
              if (0 != opCode->mValue) continue;

              while (!insert(consumedOpCodeIndexes, current, false)) { ++current; }
              opCode->mValue = current;
            }
            if (current > 239) {
              ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Task index is too large: " + string(current));
            }
          } catch (const InvalidArgument &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Task index is not valid: " + e.message());
          }

          try {
            for (auto iter = provider->mTasks.begin(); iter != provider->mTasks.end(); ++iter)
            {
              auto task = (*iter).second;

              IndexSet opIndexes(consumedOpCodeIndexes);  // never use a global opcode value

              for (auto iterOp = provider->mOpCodes.begin(); iterOp != provider->mOpCodes.end(); ++iterOp)
              {
                auto opCode = (*iterOp).second;
                if (0 == opCode->mValue) continue;
                if ((opCode->mValue < 10) && (opCode->mValue > 239)) {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "OpCode index is not valid: " + string(opCode->mValue));
                }
                insert(opIndexes, opCode->mValue);
              }

              size_t current = 10;
              for (auto iterOp = provider->mOpCodes.begin(); iterOp != provider->mOpCodes.end(); ++iterOp)
              {
                auto opCode = (*iterOp).second;
                while (!insert(opIndexes, current, false)) { ++current; }
                opCode->mValue = current;
              }
              if (current > 239) {
                ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Task OpCode index is too large: " + string(current));
              }
            }
          } catch (const InvalidArgument &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Task OpCode index is not valid: " + e.message());
          }

          try {
            IndexSet consumedIndexes;

            for (auto iter = provider->mEvents.begin(); iter != provider->mEvents.end(); ++iter)
            {
              auto event = (*iter).second;
              if (0 == event->mValue) continue;
              insert(consumedIndexes, event->mValue);
            }

            size_t current = 1000;
            for (auto iter = provider->mEvents.begin(); iter != provider->mEvents.end(); ++iter)
            {
              auto event = (*iter).second;
              if (0 != event->mValue) continue;

              while (!insert(consumedIndexes, current, false)) { ++current; }
              event->mValue = current;
            }
          } catch (const InvalidArgument &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Event index is not valid: " + e.message());
          }
        }

        //---------------------------------------------------------------------
        void Compiler::validate() throw (Failure)
        {
          ProviderPtr &provider = mConfig.mProvider;
        }

        //---------------------------------------------------------------------
        DocumentPtr Compiler::generateManifest() const throw (Failure)
        {
          const ProviderPtr &provider = mConfig.mProvider;

          DocumentPtr doc = Document::create();

          {
            // <?xml version='1.0' encoding='utf-8' standalone='yes'?>
            auto decl = Declaration::create();
            decl->setAttribute("version", "1.0");
            decl->setAttribute("encoding", "utf-8");
            decl->setAttribute("standalone", "yes");
            doc->adoptAsLastChild(decl);
          }

          {
            CommentPtr comment = Comment::create();
            comment->setValue(" Generated by zsLibEventingTool ");
            doc->adoptAsLastChild(comment);
          }

          ElementPtr instrumentationManifestEl = Element::create("instrumentationManifest");
          ElementPtr instrumentationEl = Element::create("instrumentation");
          ElementPtr outerEventsEl = Element::create("events");
          ElementPtr providerEl = Element::create("provider");
          ElementPtr channelsEl = Element::create("channels");
          ElementPtr tasksEl = Element::create("tasks");
          ElementPtr opCodesEl = Element::create("opcodes");
          ElementPtr dataTemplatesEl = Element::create("templates");
          ElementPtr innerEventsEl = Element::create("events");

          ElementPtr localizationEl = Element::create("localization");
          ElementPtr resourcesEl = Element::create("resources");
          ElementPtr stringTableEl = Element::create("stringTable");

          //<instrumentationManifest xmlns="http://schemas.microsoft.com/win/2004/08/events">
          {
            instrumentationManifestEl->setAttribute("xmlns", "http://schemas.microsoft.com/win/2004/08/events");
            doc->adoptAsLastChild(instrumentationManifestEl);
          }
          //<instrumentation
          //  xmlns:win = "http://manifests.microsoft.com/win/2004/08/windows/events"
          //  xmlns:xs = "http://www.w3.org/2001/XMLSchema"
          //  xmlns:xsi = "http://www.w3.org/2001/XMLSchema-instance">
          {
            instrumentationEl->setAttribute("xmlns:win", "http://manifests.microsoft.com/win/2004/08/windows/events");
            instrumentationEl->setAttribute("xmlns:xs", "http://www.w3.org/2001/XMLSchema");
            instrumentationEl->setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
            instrumentationManifestEl->adoptAsLastChild(instrumentationEl);
          }
          //<events xmlns="http://schemas.microsoft.com/win/2004/08/events">
          {
            outerEventsEl->setAttribute("xmlns", "http://schemas.microsoft.com/win/2004/08/events");
            instrumentationEl->adoptAsLastChild(outerEventsEl);
          }

          //<provider
          //  guid = "{6586be19-7cf9-44f1-996b-3751e3549ccd}"
          //  name = "zsLib"
          //  message = "$(string.zsLib.ProviderMessage)"
          //  symbol = "ZSLIB_PROVIDER_GUID"
          //  messageFileName = "zsLib_ETWTracing.dll"
          //  resourceFileName = "zsLib_ETWTracing.dll" >
          {
            providerEl->setAttribute("guid", "{"+string(provider->mID) + "}");
            providerEl->setAttribute("name", provider->mName);
            providerEl->setAttribute("message", "$(string.Provider)");
            providerEl->setAttribute("symbol", "PROVIDER_" + toSymbol(provider->mSymbolName));
            providerEl->setAttribute("messageFileName", provider->mResourceName + ".dll");
            providerEl->setAttribute("resourceFileName", provider->mResourceName + ".dll");
            outerEventsEl->adoptAsLastChild(providerEl);

            stringTableEl->adoptAsLastChild(createStringEl("Provider", provider->mName));
          }

          //<channels>
          //  <channel chid="zs"
          //    name="zsLib/Debug"
          //    type="Debug"
          //    symbol="CHANNEL_ZSLIB_DEBUG"
          //    message="$(string.Channel.zsLibDebug)" / >
          //</channels>
          if (provider->mChannels.size() > 0)
          {
            for (auto iter = provider->mChannels.begin(); iter != provider->mChannels.end(); ++iter)
            {
              auto channel = (*iter).second;
              ElementPtr channelEl = Element::create("channel");
              channelEl->setAttribute("chid", channel->mID);
              channelEl->setAttribute("name", channel->mName);
              channelEl->setAttribute("symbol", "CHANNEL_" + toSymbol(channel->mName));
              if (0 != channel->mValue) {
                channelEl->setAttribute("value", string(channel->mValue));
              }
              channelEl->setAttribute("message", "$(string.Channel." + channel->mID + ")");
              channelsEl->adoptAsLastChild(channelEl);

              stringTableEl->adoptAsLastChild(createStringEl("Channel." + channel->mID, channel->mName));
            }
            providerEl->adoptAsLastChild(channelsEl);
          }

          //<tasks>
          //  <task name="Exception" symbol="TASK_EXCEPTION" value="1" message="$(string.Task.Exception)">
          //  <opcodes>
          //    <opcode name="Exception" symbol="OPCODE_EXCEPTION_EXCEPTION" value = "11" message="$(string.Opcode.Exception.Exception)" / >
          //  </opcodes>
          //</task>
          if (provider->mTasks.size() > 0) {
            for (auto iter = provider->mTasks.begin(); iter != provider->mTasks.end(); ++iter)
            {
              auto task = (*iter).second;
              ElementPtr taskEl = Element::create("channel");
              taskEl->setAttribute("name", task->mName);
              taskEl->setAttribute("symbol", "TASK_" + toSymbol(task->mName));
              if (0 != task->mValue) {
                taskEl->setAttribute("value", string(task->mValue));
              }
              taskEl->setAttribute("message", "$(string.Task." + task->mName + ")");
              if (task->mOpCodes.size() > 0) {
                ElementPtr taskOpCodesEl = Element::create("opcodes");
                for (auto iterOpCode = task->mOpCodes.begin(); iterOpCode != task->mOpCodes.end(); ++iterOpCode)
                {
                  auto opCode = (*iterOpCode).second;
                  ElementPtr opCodeEl = Element::create("opcode");
                  opCodeEl->setAttribute("name", opCode->mName);
                  opCodeEl->setAttribute("symbol", "TASK_" + toSymbol(task->mName) + "_OPCODE_" + toSymbol(opCode->mName));
                  if (0 != opCode->mValue) {
                    opCodeEl->setAttribute("value", string(opCode->mValue));
                  }
                  opCodeEl->setAttribute("message", "$(string.Task." + task->mName + ".OpCode." + opCode->mName + ")");
                  taskOpCodesEl->adoptAsLastChild(opCodeEl);

                  stringTableEl->adoptAsLastChild(createStringEl("Task." + task->mName + ".OpCode." + opCode->mName, opCode->mName));
                }
                taskEl->adoptAsFirstChild(taskOpCodesEl);
              }
              channelsEl->adoptAsLastChild(taskEl);

              stringTableEl->adoptAsLastChild(createStringEl("Task." + task->mName, task->mName));
            }
            providerEl->adoptAsLastChild(tasksEl);
          }

          if (provider->mOpCodes.size() > 0) {
            for (auto iter = provider->mOpCodes.begin(); iter != provider->mOpCodes.end(); ++iter)
            {
              auto opCode = (*iter).second;

              for (IEventingTypes::PredefinedOpCodes index = IEventingTypes::PredefinedOpCode_First; index <= IEventingTypes::PredefinedOpCode_Last; index = static_cast<IEventingTypes::PredefinedOpCodes>(static_cast<std::underlying_type<IEventingTypes::PredefinedOpCodes>::type>(index) + 1)) {
                if (opCode->mValue == index) goto skip_next;
              }

              {
                ElementPtr opCodeEl = Element::create("opcode");
                opCodeEl->setAttribute("name", opCode->mName);
                opCodeEl->setAttribute("symbol", "OPCODE_" + toSymbol(opCode->mName));
                if (0 != opCode->mValue) {
                  opCodeEl->setAttribute("value", string(opCode->mValue));
                }
                opCodeEl->setAttribute("message", "$(string.OpCode." + opCode->mName + ")");
                opCodesEl->adoptAsLastChild(opCodeEl);

                stringTableEl->adoptAsLastChild(createStringEl("OpCode." + opCode->mName, opCode->mName));
              }

            skip_next: {}
            }
            providerEl->adoptAsLastChild(opCodesEl);
          }

          if (provider->mDataTemplates.size() > 0) {
            for (auto iter = provider->mDataTemplates.begin(); iter != provider->mDataTemplates.end(); ++iter)
            {
              auto dataTemplate = (*iter).second;
              String hash = dataTemplate->hash();

              ElementPtr dataTemplateEl = Element::create("template");
              dataTemplateEl->setAttribute("tid", "T_" + hash);

              dataTemplateEl->adoptAsLastChild(createDataEl("AnsiString", "function"));
              dataTemplateEl->adoptAsLastChild(createDataEl("UInt32", "line"));

              IEventingTypes::DataTypePtr lastDataType;
              for (auto iterDataTemplate = dataTemplate->mDataTypes.begin(); iterDataTemplate != dataTemplate->mDataTypes.end(); ++iterDataTemplate)
              {
                auto dataType = (*iterDataTemplate);

                if (("function" == dataType->mValueName) ||
                    ("line" == dataType->mValueName)) {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Function parameter cannot be named: " + dataType->mValueName);
                }

                {
                  if (lastDataType) {
                    if (IEventingTypes::PredefinedTypedef_binary == lastDataType->mType) {
                      if (IEventingTypes::PredefinedTypedef_size != dataType->mType) {
                        ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Template binary missing size: T_" + hash);
                        dataTemplateEl->adoptAsLastChild(createDataEl("UInt32", dataType->mValueName));
                        dataTemplateEl->adoptAsLastChild(createDataEl("Binary", lastDataType->mValueName));
                      }
                      goto next;
                    }
                  }
                  switch (dataType->mType) {
                    case IEventingTypes::PredefinedTypedef_bool:        dataTemplateEl->adoptAsLastChild(createDataEl("Boolean", dataType->mValueName)); break;

                    case IEventingTypes::PredefinedTypedef_byte:
                    case IEventingTypes::PredefinedTypedef_uint8:
                    case IEventingTypes::PredefinedTypedef_uchar:       dataTemplateEl->adoptAsLastChild(createDataEl("UInt8", dataType->mValueName)); break;
                    case IEventingTypes::PredefinedTypedef_int8:
                    case IEventingTypes::PredefinedTypedef_sint8:
                    case IEventingTypes::PredefinedTypedef_char:
                    case IEventingTypes::PredefinedTypedef_schar:       dataTemplateEl->adoptAsLastChild(createDataEl("Int8", dataType->mValueName)); break;
                    case IEventingTypes::PredefinedTypedef_word:
                    case IEventingTypes::PredefinedTypedef_uint16:
                    case IEventingTypes::PredefinedTypedef_ushort:      dataTemplateEl->adoptAsLastChild(createDataEl("UInt16", dataType->mValueName)); break;
                    case IEventingTypes::PredefinedTypedef_int16:
                    case IEventingTypes::PredefinedTypedef_sint16:
                    case IEventingTypes::PredefinedTypedef_short:
                    case IEventingTypes::PredefinedTypedef_sshort:      dataTemplateEl->adoptAsLastChild(createDataEl("Int16", dataType->mValueName)); break;
                    case IEventingTypes::PredefinedTypedef_int:
                    case IEventingTypes::PredefinedTypedef_sint:        
                    case IEventingTypes::PredefinedTypedef_int64:
                    case IEventingTypes::PredefinedTypedef_sint64:
                    case IEventingTypes::PredefinedTypedef_long:
                    case IEventingTypes::PredefinedTypedef_longlong:
                    case IEventingTypes::PredefinedTypedef_slonglong:   
                    case IEventingTypes::PredefinedTypedef_slong:       dataTemplateEl->adoptAsLastChild(createDataEl("Int64", dataType->mValueName)); break;
                    case IEventingTypes::PredefinedTypedef_size:
                    case IEventingTypes::PredefinedTypedef_uint:
                    case IEventingTypes::PredefinedTypedef_qword:
                    case IEventingTypes::PredefinedTypedef_uint64:
                    case IEventingTypes::PredefinedTypedef_ulong:       
                    case IEventingTypes::PredefinedTypedef_ulonglong:   dataTemplateEl->adoptAsLastChild(createDataEl("UInt64", dataType->mValueName)); break;
                    case IEventingTypes::PredefinedTypedef_dword:
                    case IEventingTypes::PredefinedTypedef_uint32:      dataTemplateEl->adoptAsLastChild(createDataEl("UInt32", dataType->mValueName)); break;
                    case IEventingTypes::PredefinedTypedef_int32:
                    case IEventingTypes::PredefinedTypedef_sint32:      dataTemplateEl->adoptAsLastChild(createDataEl("Int32", dataType->mValueName)); break;


                    case IEventingTypes::PredefinedTypedef_float32:
                    case IEventingTypes::PredefinedTypedef_float:       dataTemplateEl->adoptAsLastChild(createDataEl("Float", dataType->mValueName)); break;
                    case IEventingTypes::PredefinedTypedef_float64:
                    case IEventingTypes::PredefinedTypedef_ldouble:
                    case IEventingTypes::PredefinedTypedef_double:      dataTemplateEl->adoptAsLastChild(createDataEl("Double", dataType->mValueName)); break;

                    case IEventingTypes::PredefinedTypedef_pointer:     dataTemplateEl->adoptAsLastChild(createDataEl("Pointer", dataType->mValueName)); break;

                    case IEventingTypes::PredefinedTypedef_binary:      goto next;

                    case IEventingTypes::PredefinedTypedef_string:      
                    case IEventingTypes::PredefinedTypedef_astring:     dataTemplateEl->adoptAsLastChild(createDataEl("AnsiString", dataType->mValueName)); break;
                    case IEventingTypes::PredefinedTypedef_wstring:     dataTemplateEl->adoptAsLastChild(createDataEl("UnicodeString", dataType->mValueName)); break;
                  }
                }

              next: {}
                lastDataType = dataType;
              }
              if (lastDataType) {
                if (IEventingTypes::PredefinedTypedef_binary == lastDataType->mType) {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Template binary missing size: T_" + hash);
                }
              }
            }
          }

          // template basic
          {
            ElementPtr dataTemplateEl = Element::create("template");
            dataTemplateEl->setAttribute("tid", "T_Basic");
            dataTemplateEl->adoptAsLastChild(createDataEl("AnsiString", "function"));
            dataTemplateEl->adoptAsLastChild(createDataEl("UInt32", "line"));

            dataTemplatesEl->adoptAsLastChild(dataTemplateEl);
          }
          providerEl->adoptAsLastChild(dataTemplatesEl);

          if (provider->mEvents.size() > 0) {
            /*
            <events>
              <event symbol="ZsExceptionEventFired" channel="zs" template="T_Exception" task="Exception" opcode="Exception" value="1001" level="win:Error" message="$(string.Event.ZsExceptionEventFired)" />
              <event symbol="ZsMessageQueueCreate" channel="zs" template="T_BasicThis" task="MessageQueue" opcode="win:Start" value="1101" level="win:Informational" message="$(string.Event.ZsMessageQueueCreate)" />
            */
            for (auto iter = provider->mEvents.begin(); iter != provider->mEvents.end(); ++iter)
            {
              auto event = (*iter).second;
              auto eventEl = Element::create("event");
              if (event->mName.hasData()) {
                eventEl->setAttribute("symbol", event->mName);
              }
              if (event->mChannel) {
                eventEl->setAttribute("channel", event->mChannel->mID);
              }
              if (event->mDataTemplate) {
                eventEl->setAttribute("template", "T_" + event->mDataTemplate->hash());
              } else {
                eventEl->setAttribute("template", "T_Basic");
              }
              if (event->mTask) {
                eventEl->setAttribute("task", event->mTask->mName);
              }
              if (event->mOpCode) {
                eventEl->setAttribute("opcode", event->mOpCode->mName);
              }
              if (0 != event->mValue) {
                eventEl->setAttribute("value", string(event->mValue));
              }
              eventEl->setAttribute("level", String("win:") + IEventingTypes::toString(IEventingTypes::toPredefinedLevel(event->mSeverity, event->mLevel)));
              if (event->mName.hasData()) {
                eventEl->setAttribute("message", "$(string.Event." + event->mName + ")");
                stringTableEl->adoptAsLastChild(createStringEl("Event." + event->mName, event->mName));
              }
              innerEventsEl->adoptAsLastChild(eventEl);
            }

            providerEl->adoptAsLastChild(innerEventsEl);
          }


          //<localization>
          //  <resources culture="en-US">
          //    <stringTable>
          //      <string id="zsLib.ProviderMessage" value="zsLib Provider" / >
          //      <string id = "level.Critical" value="Critical" / >
          //      <string id = "level.Error" value="Error" / >
          //      <string id = "level.Warning" value="Warning" / >
          //      <string id = "level.Informational" value="Informational" / >
          //      <string id = "level.Verbose" value="Verbose" / >
          {
            resourcesEl->setAttribute("culture", "en-US");
            resourcesEl->adoptAsLastChild(stringTableEl);
            localizationEl->adoptAsLastChild(resourcesEl);
            instrumentationManifestEl->adoptAsLastChild(localizationEl);

            stringTableEl->adoptAsLastChild(createStringEl(String("level.") + IEventingTypes::toString(IEventingTypes::PredefinedLevel_Critical), IEventingTypes::toString(IEventingTypes::PredefinedLevel_Critical)));
            stringTableEl->adoptAsLastChild(createStringEl(String("level.") + IEventingTypes::toString(IEventingTypes::PredefinedLevel_Error), IEventingTypes::toString(IEventingTypes::PredefinedLevel_Error)));
            stringTableEl->adoptAsLastChild(createStringEl(String("level.") + IEventingTypes::toString(IEventingTypes::PredefinedLevel_Warning), IEventingTypes::toString(IEventingTypes::PredefinedLevel_Warning)));
            stringTableEl->adoptAsLastChild(createStringEl(String("level.") + IEventingTypes::toString(IEventingTypes::PredefinedLevel_Informational), IEventingTypes::toString(IEventingTypes::PredefinedLevel_Informational)));
            stringTableEl->adoptAsLastChild(createStringEl(String("level.") + IEventingTypes::toString(IEventingTypes::PredefinedLevel_Verbose), IEventingTypes::toString(IEventingTypes::PredefinedLevel_Verbose)));
          }

          return doc;
        }

        //---------------------------------------------------------------------
        DocumentPtr Compiler::generateWprp() const throw (Failure)
        {
          const ProviderPtr &provider = mConfig.mProvider;

          DocumentPtr doc = Document::create();

          {
            // <?xml version='1.0' encoding='utf-8' standalone='yes'?>
            auto decl = Declaration::create();
            decl->setAttribute("version", "1.0");
            decl->setAttribute("encoding", "utf-8");
            decl->setAttribute("standalone", "yes");
            doc->adoptAsLastChild(decl);
          }

          {
            CommentPtr comment = Comment::create();
            comment->setValue(" Generated by zsLibEventingTool ");
            doc->adoptAsLastChild(comment);
          }

          ElementPtr windowsPerformanceRecorderEl = Element::create("WindowsPerformanceRecorder");
          ElementPtr profilesEl = Element::create("Profiles");

          {
            //<WindowsPerformanceRecorder
            // Author="Robin Raymond"
            // Comments="zsLib Custom Provider Profile"
            // Version="1.0">
            windowsPerformanceRecorderEl->setAttribute("Author", mConfig.mAuthor.hasData() ? mConfig.mAuthor : "Unknown");
            windowsPerformanceRecorderEl->setAttribute("Comments", provider->mName);
            windowsPerformanceRecorderEl->setAttribute("Version", "1.0");
          }

          {
            //<EventCollector Id="EventCollector_zsLib_Verbose" Name="zsLib Event Collector" Private="false" ProcessPrivate="false" Secure="false" Realtime="false">
            //  <BufferSize Value="1024"/>
            //  <Buffers Value="40"/>
            //</EventCollector>
            {
              ElementPtr eventCollectorEl = Element::create("EventCollector");
              eventCollectorEl->setAttribute("Id", "EventCollector_" + provider->mSymbolName + "_Verbose");
              eventCollectorEl->setAttribute("Name", provider->mSymbolName + " Event Collector");
              eventCollectorEl->setAttribute("Private", "false");
              eventCollectorEl->setAttribute("ProcessPrivate", "false");
              eventCollectorEl->setAttribute("Secure", "false");
              eventCollectorEl->setAttribute("Realtime", "false");
              {
                ElementPtr bufferSizeEl = Element::create("BufferSize");
                bufferSizeEl->setAttribute("Value", "1024");
                eventCollectorEl->adoptAsLastChild(bufferSizeEl);
              }
              {
                ElementPtr bufferEl = Element::create("Buffers");
                bufferEl->setAttribute("Value", "1024");
                eventCollectorEl->adoptAsLastChild(bufferEl);
              }
              profilesEl->adoptAsLastChild(eventCollectorEl);
            }

            //<EventCollector Id="EventCollector_zsLib_Light" Name="zsLib Event Collector" Private="false" ProcessPrivate="false" Secure="false" Realtime="false">
            //  <BufferSize Value="128"/>
            //  <Buffers Value="40"/>
            //</EventCollector>
            {
              ElementPtr eventCollectorEl = Element::create("EventCollector");
              eventCollectorEl->setAttribute("Id", "EventCollector_" + provider->mSymbolName + "_Light");
              eventCollectorEl->setAttribute("Name", provider->mSymbolName + " Event Collector");
              eventCollectorEl->setAttribute("Private", "false");
              eventCollectorEl->setAttribute("ProcessPrivate", "false");
              eventCollectorEl->setAttribute("Secure", "false");
              eventCollectorEl->setAttribute("Realtime", "false");
              {
                ElementPtr bufferSizeEl = Element::create("BufferSize");
                bufferSizeEl->setAttribute("Value", "1024");
                eventCollectorEl->adoptAsLastChild(bufferSizeEl);
              }
              {
                ElementPtr bufferEl = Element::create("Buffers");
                bufferEl->setAttribute("Value", "1024");
                eventCollectorEl->adoptAsLastChild(bufferEl);
              }
              profilesEl->adoptAsLastChild(eventCollectorEl);
            }
          }

          {
            // <EventProvider Id="EventProvider_zsLib" Name="6586be19-7cf9-44f1-996b-3751e3549ccd" />
            ElementPtr eventProviderEl = Element::create("EventProvider");

            eventProviderEl->setAttribute("Id", "EventProvider_" + provider->mSymbolName);
            eventProviderEl->setAttribute("Name", string(provider->mID));

            profilesEl->adoptAsLastChild(eventProviderEl);
          }

          // Profile - Verbose.Memory
          {
            //<Profiles>
            //  <Profile
            //    Description="GeneralProfileForLargeServers"
            //    DetailLevel="Verbose"
            //    Id="GeneralProfileForLargeServers.Verbose.Memory"
            //    LoggingMode="Memory"
            //    Name="GeneralProfileForLargeServers"
            //    >
            //    <Collectors>
            //      <!-- EventCollectorId must match the EventCollector ID specified above -->
            //      <EventCollectorId Value="EventCollector_zsLib_Verbose">
            //        <EventProviders>
            //          <EventProviderId Value="EventProvider_zsLib"/>
            //        </EventProviders>
            //      </EventCollectorId>
            //    </Collectors>
            ElementPtr profileEl = Element::create("Profile");

            profileEl->setAttribute("Description", provider->mDescription);
            profileEl->setAttribute("DetailLevel", "Verbose");
            profileEl->setAttribute("Id", provider->mSymbolName + ".Verbose.Memory");
            profileEl->setAttribute("LoggingMode", "Memory");
            profileEl->setAttribute("Name", provider->mSymbolName);

            ElementPtr collectorsEl = Element::create("Collectors");
            ElementPtr eventCollectorIdEl = Element::create("EventCollectorId");
            eventCollectorIdEl->setAttribute("Value", "EventCollector_" + provider->mSymbolName + "_Verbose");
            ElementPtr eventProvidersEl = Element::create("EventProviders");
            eventProvidersEl->setAttribute("Value", "EventCollector_" + provider->mSymbolName + "_Verbose");
            ElementPtr eventProviderIdEl = Element::create("EventProviderId");
            eventProviderIdEl->setAttribute("Value",  "EventProvider_" + provider->mSymbolName);

            eventCollectorIdEl->adoptAsLastChild(eventProvidersEl);
            collectorsEl->adoptAsLastChild(eventCollectorIdEl);
            profileEl->adoptAsLastChild(collectorsEl);

            profilesEl->adoptAsLastChild(profileEl);
          }

          //<Profile
          //  Id="zsLibProfile.Light.File"
          //  LoggingMode="File"
          //  Name="zsLibProfile"
          //  DetailLevel="Light"
          //  Description="zsLib Provider for Diagnostic trace">
          //  <Collectors>
          //    <!-- EventCollectorId must match the EventCollector ID specified above -->
          //    <EventCollectorId Value="EventCollector_zsLib_Light">
          //      <EventProviders>
          //        <EventProviderId Value="EventProvider_zsLib"/>
          //      </EventProviders>
          //    </EventCollectorId>
          //  </Collectors>
          //</Profile>
          {
            ElementPtr profileEl = Element::create("Profile");

            profileEl->setAttribute("Description", provider->mDescription);
            profileEl->setAttribute("DetailLevel", "Light");
            profileEl->setAttribute("Id", provider->mSymbolName + ".Light.Memory");
            profileEl->setAttribute("LoggingMode", "Memory");
            profileEl->setAttribute("Name", provider->mSymbolName);

            ElementPtr collectorsEl = Element::create("Collectors");
            ElementPtr eventCollectorIdEl = Element::create("EventCollectorId");
            eventCollectorIdEl->setAttribute("Value", "EventCollector_" + provider->mSymbolName + "_Light");
            ElementPtr eventProvidersEl = Element::create("EventProviders");
            eventProvidersEl->setAttribute("Value", "EventCollector_" + provider->mSymbolName + "_Light");
            ElementPtr eventProviderIdEl = Element::create("EventProviderId");
            eventProviderIdEl->setAttribute("Value", "EventProvider_" + provider->mSymbolName);

            eventCollectorIdEl->adoptAsLastChild(eventProvidersEl);
            collectorsEl->adoptAsLastChild(eventCollectorIdEl);
            profileEl->adoptAsLastChild(collectorsEl);

            profilesEl->adoptAsLastChild(profileEl);
          }

          {
            //<Profile
            //  Id="zsLibProfile.Verbose.Memory"
            //  LoggingMode="Memory"
            //  Name="zsLibProfile"
            //  DetailLevel="Verbose"
            //  Description="zsLib Provider for Diagnostic trace"
            //  Base="zsLibProfile.Verbose.File"/>
            ElementPtr profileEl = Element::create("Profile");

            profileEl->setAttribute("Description", provider->mDescription);
            profileEl->setAttribute("DetailLevel", "Verbose");
            profileEl->setAttribute("Id", provider->mSymbolName + ".Verbose.File");
            profileEl->setAttribute("LoggingMode", "File");
            profileEl->setAttribute("Name", provider->mSymbolName);
            profileEl->setAttribute("Base", provider->mSymbolName + ".Verbose.Memory");

            profilesEl->adoptAsLastChild(profileEl);
          }

          {
            //<Profile
            //  Id="zsLibProfile.Light.Memory"
            //  LoggingMode="Memory"
            //  Name="zsLibProfile"
            //  DetailLevel="Light"
            //  Description="zsLib Provider for Diagnostic trace"
            //  Base="zsLibProfile.Light.File"/>
            ElementPtr profileEl = Element::create("Profile");

            profileEl->setAttribute("Description", provider->mDescription);
            profileEl->setAttribute("DetailLevel", "Light");
            profileEl->setAttribute("Id", provider->mSymbolName + ".Light.File");
            profileEl->setAttribute("LoggingMode", "File");
            profileEl->setAttribute("Name", provider->mSymbolName);
            profileEl->setAttribute("Base", provider->mSymbolName + ".Light.Memory");

            profilesEl->adoptAsLastChild(profileEl);
          }

          return doc;
        }

        //---------------------------------------------------------------------
        DocumentPtr Compiler::generateJsonMan() const throw (Failure)
        {
          DocumentPtr doc = Document::create();
          ElementPtr providerEl = mConfig.mProvider->createElement();

          ElementPtr commentEl = UseEventingHelper::createElementWithTextAndJSONEncode("comment", "Generated by zsLibEventingTool");
          providerEl->adoptAsFirstChild(commentEl);
          doc->adoptAsLastChild(providerEl);
          return doc;
        }

        //---------------------------------------------------------------------
        static const char *getFunctions()
        {
          static const char *functions =
          "\n"
          "    template <typename TWriteType>\n"
          "    void eventWriteBuffer(BYTE * &p, TWriteType value)\n"
          "    {\n"
          "      memcpy(&p, &value, sizeof(value));\n"
          "      p += sizeof(value);\n"
          "    }\n"

          "    inline void eventWriteBuffer(const BYTE ** &p, const BYTE *buffer, size_t * &bufferSizes, size_t size)\n"
          "    {\n"
          "      (*p) = buffer;\n"
          "      (*bufferSizes) = size;\n"
          "      ++p;\n"
          "      ++bufferSizes;\n"
          "    }\n"

          "    inline void eventWriteBuffer(const BYTE ** &p, const char *str, size_t * &bufferSizes)\n"
          "    {\n"
          "      (*p) = reinterpret_cast<const BYTE *>(str);\n"
          "      (*bufferSizes) = (NULL == str ? 0 : strlen(str)) * sizeof(char);\n"
          "      ++p;\n"
          "      ++bufferSizes;\n"
          "    }\n"

          "    inline void eventWriteBuffer(const BYTE ** &p, const wchar_t *str, size_t * &bufferSizes)\n"
          "    {\n"
          "      (*p) = reinterpret_cast<const BYTE *>(str);\n"
          "      (*bufferSizes) = (NULL == str ? 0 : wcslen(str)) * sizeof(wchar_t);\n"
          "      ++p;\n"
          "      ++bufferSizes;\n"
          "    }\n"
          "\n";

          return functions;
        }

        //---------------------------------------------------------------------
        SecureByteBlockPtr Compiler::generateXPlatformEventsHeader(
                                                                   const String &outputNameXPlatform,
                                                                   const String &outputNameWindows
                                                                   ) const throw (Failure)
        {
          std::stringstream ss;

          const ProviderPtr &provider = mConfig.mProvider;
          if (!provider) return SecureByteBlockPtr();

          ss << "// Generated by zsLibEventingTool\n\n";
          ss << "#pragma once\n\n";
          ss << "#include <stdint.h>\n\n";
          ss << "#include <zsLib/eventing/Log.h>\n\n";
          ss << "#include \"" << outputNameWindows << "\"";
          ss << "#ifndef _WIN32\n\n";
          ss << "namespace zsLib {\n";
          ss << "  namespace eventing {\n";

          ss << getFunctions();

          ss << "#define ZS_INTERNAL_INIT_EVENTING_" << provider->mName << "() EventRegister" << provider->mName;
          ss << "#define ZS_INTERNAL_DEINIT_EVENTING_" << provider->mName << "() EventRegister" << provider->mName;

          for (auto iter = provider->mEvents.begin(); iter != provider->mEvents.end(); ++iter)
          {
            auto event = (*iter).second;

            {
              ss << "#define ZS_INTERNAL_EVENTING_EVENT_" << event->mName << "(xSubsystem, xSeverity, xLevel";

              if (!event->mDataTemplate) goto declaration_done;
              if (event->mDataTemplate->mDataTypes.size() < 1) goto declaration_done;

              for (size_t loop = 1; loop <= event->mDataTemplate->mDataTypes.size(); ++loop)
              {
                ss << ", xType" << string(loop) << ", xName" << string(loop) << ", xValue" << string(loop);
              }
            }

          declaration_done:
            {
              ss << ") ";
            }

            {
              if ("x" == event->mSubsystem) {
                ss << "if (ZS_EVENTING_IS_LOGGING(xLevel)) { \\\n";
              } else {
                ss << "if (ZS_EVENTING_IS_SUBSYSTEM_LOGGING(xSubsystem, xLevel)) { \\\n";
              }

              size_t totalPointers = 0;
              size_t maxSize = 0;
              for (auto iterDataType = event->mDataTemplate->mDataTypes.begin(); iterDataType != event->mDataTemplate->mDataTypes.end(); ++iterDataType) {
                auto dataType = (*iterDataType);
                maxSize += IEventingTypes::getMaxBytes(dataType->mType);
                switch (IEventingTypes::getBaseType(dataType->mType))
                {
                  case IEventingTypes::BaseType_Boolean:
                  case IEventingTypes::BaseType_Integer:
                  case IEventingTypes::BaseType_Float:
                  case IEventingTypes::BaseType_Pointer:  break;
                  case IEventingTypes::BaseType_Binary: 
                  case IEventingTypes::BaseType_String:   ++totalPointers; break;
                }
              }

              ss << "  zsLib::BYTE xxOutputBuffer[" << maxSize << "];BYTE *xxPOutputBuffer = &(xxOutputBuffer[0]); \\\n";

              if (totalPointers > 0) {
                ss << "  const BYTE *xxIndirectBuffer[" << totalPointers << "];BYTE **xxPIndirectBuffer = &(xxIndirectBuffer[0]); \\\n";
                ss << "  size_t xxIndirectSize[" << totalPointers << "];size_t *xxPIndirectBuffer = &(xxIndirectSize[0]); \\\n";
              }
              
              bool nextMustBeSize = false;
              size_t loop = 1;
              for (auto iterDataType = event->mDataTemplate->mDataTypes.begin(); iterDataType != event->mDataTemplate->mDataTypes.end(); ++iterDataType, ++loop) {
                auto dataType = (*iterDataType);

                switch (dataType->mType)
                {
                  case IEventingTypes::PredefinedTypedef_bool: {
                    ss << "  eventWriteBuffer<bool>(xxPOutputBuffer, (xValue" << string(loop) << ")); \\\n";
                    break;
                  }

                  case IEventingTypes::PredefinedTypedef_uchar:
                  case IEventingTypes::PredefinedTypedef_ushort:
                  case IEventingTypes::PredefinedTypedef_uint:
                  case IEventingTypes::PredefinedTypedef_ulong:
                  case IEventingTypes::PredefinedTypedef_ulonglong:
                  case IEventingTypes::PredefinedTypedef_uint8:
                  case IEventingTypes::PredefinedTypedef_uint16:
                  case IEventingTypes::PredefinedTypedef_uint32:
                  case IEventingTypes::PredefinedTypedef_uint64:
                  case IEventingTypes::PredefinedTypedef_byte:
                  case IEventingTypes::PredefinedTypedef_word:
                  case IEventingTypes::PredefinedTypedef_dword:
                  case IEventingTypes::PredefinedTypedef_qword: {
                    String typeStr = String("uint") + string(IEventingTypes::getMaxBytes(dataType->mType) * 8) + "_t";
                    ss << "  eventWriteBuffer<" << typeStr << ">(xxPOutputBuffer, static_cast<" << typeStr << ">(xValue" << string(loop) << ")); \\\n";
                    break;
                  }

                  case IEventingTypes::PredefinedTypedef_char:
                  case IEventingTypes::PredefinedTypedef_schar:
                  case IEventingTypes::PredefinedTypedef_short:
                  case IEventingTypes::PredefinedTypedef_sshort:
                  case IEventingTypes::PredefinedTypedef_int:
                  case IEventingTypes::PredefinedTypedef_sint:
                  case IEventingTypes::PredefinedTypedef_long:
                  case IEventingTypes::PredefinedTypedef_slong:
                  case IEventingTypes::PredefinedTypedef_longlong:
                  case IEventingTypes::PredefinedTypedef_slonglong:
                  case IEventingTypes::PredefinedTypedef_int8:
                  case IEventingTypes::PredefinedTypedef_sint8:
                  case IEventingTypes::PredefinedTypedef_int16:
                  case IEventingTypes::PredefinedTypedef_sint16:
                  case IEventingTypes::PredefinedTypedef_int32:
                  case IEventingTypes::PredefinedTypedef_sint32:
                  case IEventingTypes::PredefinedTypedef_int64:
                  case IEventingTypes::PredefinedTypedef_sint64: {
                    String typeStr = String("int") + string(IEventingTypes::getMaxBytes(dataType->mType) * 8) + "_t";
                    ss << "  eventWriteBuffer<" << typeStr << ">(xxPOutputBuffer, static_cast<" << typeStr << ">(xValue" << string(loop) << ")); \\\n";
                    break;
                  }

                  case IEventingTypes::PredefinedTypedef_float:
                  case IEventingTypes::PredefinedTypedef_double:
                  case IEventingTypes::PredefinedTypedef_ldouble:
                  case IEventingTypes::PredefinedTypedef_float32:
                  case IEventingTypes::PredefinedTypedef_float64: {
                    String typeStr = (IEventingTypes::getMaxBytes(dataType->mType) <= 4 ? "float" : "double");
                    ss << "  eventWriteBuffer<" << typeStr << ">(xxPOutputBuffer, static_cast<" << typeStr << ">(xValue" << string(loop) << ")); \\\n";
                    break;
                  }

                  case IEventingTypes::PredefinedTypedef_pointer: {
                    ss << "  eventWriteBuffer<uint_64>(xxPOutputBuffer, static_cast<uint_64>(xValue" << string(loop) << ")); \\\n";
                    break;
                  }

                  case IEventingTypes::PredefinedTypedef_binary: {
                    ss << "  eventWriteBuffer(xxPIndirectBuffer, reinterpret_cast<const BYTE *>(xValue" << string(loop) << "), xxIndirectSize, (xValue" << string(loop + 1) << ")); \\\n";
                    if (loop + 1 > event->mDataTemplate->mDataTypes.size()) {
                      ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, String("Binary data missing size"));
                    }
                    nextMustBeSize = true;
                    break;
                  }
                  case IEventingTypes::PredefinedTypedef_size: {
                    nextMustBeSize = false;
                    goto next_loop;
                  }

                  case IEventingTypes::PredefinedTypedef_string: {
                    ss << "  eventWriteBuffer(xxPIndirectBuffer, (xValue" << string(loop) << "), xxIndirectSize); \\\n";
                    break;
                  }
                  case IEventingTypes::PredefinedTypedef_astring: {
                    ss << "  eventWriteBuffer(xxPIndirectBuffer, (xValue" << string(loop) << "), xxIndirectSize); \\\n";
                    break;
                  }
                  case IEventingTypes::PredefinedTypedef_wstring: {
                    ss << "  eventWriteBuffer(xxPIndirectBuffer, (xValue" << string(loop) << "), xxIndirectSize); \\\n";
                    break;
                  }
                }

                {
                  if (nextMustBeSize) {
                    ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, String("Binary data missing size"));
                  }
                }

              next_loop:
                {
                }
              }

              if (totalPointers > 0) {
                ss << "  ZS_EVENTING_WRITE_EVENT_WITH_BUFFERS(__func__, __LINE__, &(xxOutputBuffer[0]), &(xxIndirectBuffer[0]), &(xxIndirectSize[0]), " << string(totalPointers) << "); \\\n";
              } else {
                ss << "  ZS_EVENTING_WRITE_EVENT(__func__, __LINE__, &(xxOutputBuffer[0], " << string(maxSize) << ")); \\\n";
              }

              ss << "}\n";
            }
          }

          ss << "\n";
          ss << "  } // namespace eventing\n";
          ss << "} // namespace zsLib\n\n";
          ss << "#endif // ndef _WIN32\n\n";

          return UseEventingHelper::convertToBuffer(ss.str());
        }

        //---------------------------------------------------------------------
        SecureByteBlockPtr Compiler::generateWindowsEventsHeader(
                                                                 const String &outputNameXPlatform,
                                                                 const String &outputNameWindows
                                                                 ) const throw (Failure)
        {
          std::stringstream ss;

          const ProviderPtr &provider = mConfig.mProvider;
          if (!provider) return SecureByteBlockPtr();

          ss << "// Generated by zsLibEventingTool\n\n";
          ss << "#pragma once\n\n";
          ss << "#include <stdint.h>\n\n";
          ss << "#include <zsLib/eventing/Log.h>\n\n";
          ss << "#ifdef _WIN32\n\n";
          ss << "namespace zsLib {\n";
          ss << "  namespace eventing {\n";

          ss << "#define ZS_INTERNAL_INIT_EVENTING_" << provider->mName << "() EventRegister" << provider->mName;
          ss << "#define ZS_INTERNAL_DEINIT_EVENTING_" << provider->mName << "() EventRegister" << provider->mName;

          ss << getFunctions();

          for (auto iter = provider->mEvents.begin(); iter != provider->mEvents.end(); ++iter)
          {
            auto event = (*iter).second;

            {
              ss << "#define ZS_INTERNAL_EVENTING_EVENT_" << event->mName << "(xSubsystem, xSeverity, xLevel";

              if (!event->mDataTemplate) goto declaration_done;
              if (event->mDataTemplate->mDataTypes.size() < 1) goto declaration_done;

              for (size_t loop = 1; loop <= event->mDataTemplate->mDataTypes.size(); ++loop)
              {
                ss << ", xType" << string(loop) << ", xName" << string(loop) << ", xValue" << string(loop);
              }
            }

          declaration_done:
            {
              ss << ") ";
            }

            {
              if ("x" == event->mSubsystem) {
                ss << "if (ZS_EVENTING_IS_LOGGING(xLevel)) { ";
              }
              else {
                ss << "if (ZS_EVENTING_IS_SUBSYSTEM_LOGGING(xSubsystem, xLevel)) { ";
              }

              ss << "EventWrite" << event->mName << "(__func__, __LINE__";

              size_t index = 1;
              bool nextMustBeSize = false;
              for (auto iterDataType = event->mDataTemplate->mDataTypes.begin(); iterDataType != event->mDataTemplate->mDataTypes.end(); ++iterDataType, ++index) {
                auto dataType = (*iterDataType);
                switch (IEventingTypes::getBaseType(dataType->mType))
                {
                  case IEventingTypes::BaseType_Boolean:  goto output_next;
                  case IEventingTypes::BaseType_Integer: {
                    if (IEventingTypes::PredefinedTypedef_size == dataType->mType) {
                      nextMustBeSize = false;
                      goto skip_next;
                    }
                    goto output_next;
                  }
                  case IEventingTypes::BaseType_Float:    goto output_next;
                  case IEventingTypes::BaseType_Pointer:  goto output_next;
                  case IEventingTypes::BaseType_Binary: {
                    ss << ", static_cast<size_t>(xValue" << (index+1) << ")";
                    ss << ", reinterpret_cast<const BYTE *>(xValue" << index << ")";
                    nextMustBeSize = true;
                    goto skip_next;
                  }
                  case IEventingTypes::BaseType_String:   goto output_next;
                }

              output_next:
                {
                  if (nextMustBeSize) {
                    ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, String("Binary data missing size"));
                  }

                  switch (dataType->mType)
                  {
                    case IEventingTypes::PredefinedTypedef_bool: {
                      ss << ", (bool)(xValue" << index << ")";
                      break;
                    }

                    case IEventingTypes::PredefinedTypedef_uchar:
                    case IEventingTypes::PredefinedTypedef_ushort:
                    case IEventingTypes::PredefinedTypedef_uint:
                    case IEventingTypes::PredefinedTypedef_ulong:
                    case IEventingTypes::PredefinedTypedef_ulonglong:
                    case IEventingTypes::PredefinedTypedef_uint8:
                    case IEventingTypes::PredefinedTypedef_uint16:
                    case IEventingTypes::PredefinedTypedef_uint32:
                    case IEventingTypes::PredefinedTypedef_uint64:
                    case IEventingTypes::PredefinedTypedef_byte:
                    case IEventingTypes::PredefinedTypedef_word:
                    case IEventingTypes::PredefinedTypedef_dword:
                    case IEventingTypes::PredefinedTypedef_qword:
                    case IEventingTypes::PredefinedTypedef_size:
                    {
                      String typeStr = String("uint") + string(IEventingTypes::getMaxBytes(dataType->mType) * 8) + "_t";
                      ss << ", static_cast<" << typeStr << ">(xValue" << string(index) << ")";
                      break;
                    }

                    case IEventingTypes::PredefinedTypedef_char:
                    case IEventingTypes::PredefinedTypedef_schar:
                    case IEventingTypes::PredefinedTypedef_short:
                    case IEventingTypes::PredefinedTypedef_sshort:
                    case IEventingTypes::PredefinedTypedef_int:
                    case IEventingTypes::PredefinedTypedef_sint:
                    case IEventingTypes::PredefinedTypedef_long:
                    case IEventingTypes::PredefinedTypedef_slong:
                    case IEventingTypes::PredefinedTypedef_longlong:
                    case IEventingTypes::PredefinedTypedef_slonglong:
                    case IEventingTypes::PredefinedTypedef_int8:
                    case IEventingTypes::PredefinedTypedef_sint8:
                    case IEventingTypes::PredefinedTypedef_int16:
                    case IEventingTypes::PredefinedTypedef_sint16:
                    case IEventingTypes::PredefinedTypedef_int32:
                    case IEventingTypes::PredefinedTypedef_sint32:
                    case IEventingTypes::PredefinedTypedef_int64:
                    case IEventingTypes::PredefinedTypedef_sint64: {
                      String typeStr = String("int") + string(IEventingTypes::getMaxBytes(dataType->mType) * 8) + "_t";
                      ss << ", static_cast<" << typeStr << ">(xValue" << string(index) << ")";
                      break;
                    }

                    case IEventingTypes::PredefinedTypedef_float:
                    case IEventingTypes::PredefinedTypedef_double:
                    case IEventingTypes::PredefinedTypedef_ldouble:
                    case IEventingTypes::PredefinedTypedef_float32:
                    case IEventingTypes::PredefinedTypedef_float64: {
                      String typeStr = (IEventingTypes::getMaxBytes(dataType->mType) <= 4 ? "float" : "double");
                      ss << ", static_cast<" << typeStr << ">(xValue" << string(index) << ")";
                      break;
                    }

                    case IEventingTypes::PredefinedTypedef_pointer: {
                      ss << ", reinterpret_cast<uintptr_t>(xValue" << string(index) << ")";
                      break;
                    }

                    case IEventingTypes::PredefinedTypedef_binary: break;

                    case IEventingTypes::PredefinedTypedef_string: {
                      ss << ", (xValue" << string(index) << ")";
                      break;
                    }
                    case IEventingTypes::PredefinedTypedef_astring: {
                      ss << ", (xValue" << string(index) << ")";
                      break;
                    }
                    case IEventingTypes::PredefinedTypedef_wstring: {
                      ss << ", (xValue" << string(index) << ")";
                      break;
                    }
                  }
                  continue;
                }
              skip_next:
                {
                }
              }

              if (nextMustBeSize) {
                ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, String("Binary data missing size"));
              }

              ss << ")\n";


              ss << "}\n";
            }
          }

          ss << "\n";
          ss << "  } // namespace eventing\n";
          ss << "} // namespace zsLib\n\n";
          ss << "#endif // _WIN32\n\n";

          return UseEventingHelper::convertToBuffer(ss.str());
        }

        //---------------------------------------------------------------------
        void Compiler::writeXML(const String &outputName, const DocumentPtr &doc) const throw (Failure)
        {
          if (!doc) return;
          try {
            auto output = UseEventingHelper::writeXML(*doc);
            UseEventingHelper::saveFile(outputName, *output);
          } catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to save XML file \"" + outputName + "\": " + e.message());
          }
        }

        //---------------------------------------------------------------------
        void Compiler::writeJSON(const String &outputName, const DocumentPtr &doc) const throw (Failure)
        {
          if (!doc) return;
          try {
            auto output = UseEventingHelper::writeJSON(*doc);
            UseEventingHelper::saveFile(outputName, *output);
          } catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to save JSON file \"" + outputName + "\": " + e.message());
          }
        }

        //---------------------------------------------------------------------
        void Compiler::writeBinary(const String &outputName, const SecureByteBlockPtr &buffer) const throw (Failure)
        {
          if ((!buffer) ||
              (0 == buffer->SizeInBytes())) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to save file \"" + outputName + "\": file is empty");
          }
          try {
            UseEventingHelper::saveFile(outputName, *buffer);
          } catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to save file \"" + outputName + "\": " + e.message());
          }
        }

      } // namespace internal

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICompiler
      #pragma mark

      //-----------------------------------------------------------------------
      ICompilerPtr ICompiler::create(const Config &config)
      {
        return internal::Compiler::create(config);
      }

    } // namespace tool
  } // namespace eventing
} // namespace zsLib
