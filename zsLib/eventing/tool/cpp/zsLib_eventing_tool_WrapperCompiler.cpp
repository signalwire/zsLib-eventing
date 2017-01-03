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

#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_WrapperCompiler.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_Helper.h>

#include <zsLib/eventing/tool/OutputStream.h>

#include <zsLib/eventing/IHelper.h>
#include <zsLib/eventing/IHasher.h>
#include <zsLib/eventing/IEventingTypes.h>

#include <zsLib/Exception.h>
#include <zsLib/Numeric.h>

#include <sstream>
#include <list>
#include <set>

namespace zsLib { namespace eventing { namespace tool { ZS_DECLARE_SUBSYSTEM(zsLib_eventing_tool) } } }

namespace zsLib
{
  namespace eventing
  {
    ZS_DECLARE_TYPEDEF_PTR(IWrapperTypes::Project, Project);

    namespace tool
    {
      ZS_DECLARE_TYPEDEF_PTR(eventing::IHelper, UseHelper);
      ZS_DECLARE_TYPEDEF_PTR(eventing::IHasher, UseHasher);
      typedef std::set<String> HashSet;

      namespace internal
      {
        ZS_DECLARE_STRUCT_PTR(Token);
        typedef std::list<TokenPtr> TokenList;
        
        enum TokenTypes
        {
          TokenType_First,
          
          TokenType_Unknown = TokenType_First,
          
          TokenType_Documentation,
          TokenType_Char,
          TokenType_Quote,
          
          TokenType_Number,
          TokenType_Identifier,
          
          TokenType_SemiColon,
          
          TokenType_Brace,
          TokenType_CurlyBrace,
          TokenType_SquareBrace,
          
          TokenType_Operator,
          TokenType_ScopeOperator,
          TokenType_CommaOperator,
          TokenType_ColonOperator,
          TokenType_EqualsOperator,
          
          TokenType_PointerOperator,
          TokenType_AddressOperator,

          TokenType_Last = TokenType_AddressOperator,
        };
        
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        #pragma mark
        #pragma mark Helpers
        #pragma mark

        struct Token
        {
          TokenTypes mTokenType {TokenType_First};
          String mToken;
          ULONG mLineCount {1};
        };
        
        //---------------------------------------------------------------------
        static void skipPreprocessor(
                                     const char * &p,
                                     ULONG &ioLineCount
                                     )
        {
          const char *startPos = p;
          
          while (true)
          {
            Helper::skipToEOL(p);

            // see if this preprocessor statement is multi-line
            while (p != startPos)
            {
              --p;
              if (('\n' == *p) ||
                  ('\r' == *p)) {
                break;
              }

              if (isspace(*p)) continue;

              if ('\\' == *p) {
                Helper::skipToEOL(p);
                if (Helper::skipEOL(p, &ioLineCount)) goto next_line;
              }
              Helper::skipToEOL(p);
              Helper::skipEOL(p, &ioLineCount);
              return;
            }

          next_line:
            {
            }
          }
        }
        
        //---------------------------------------------------------------------
        static TokenPtr getCPPDocToken(
                                       const char *p,
                                       ULONG &ioLineCount
                                       )
        {
          if ('/' != *p) return TokenPtr();
          if ('/' != *(p+1)) return TokenPtr();
          if ('/' != *(p+2)) return TokenPtr();
          
          p += 3;

          const char *endPos = p;
          Helper::skipToEOL(endPos);
          
          String str(p, static_cast<size_t>(endPos - p));
          
          auto result = make_shared<Token>();
          result->mTokenType = TokenType_Documentation;
          result->mToken = str;
          result->mLineCount = ioLineCount;

          Helper::skipEOL(p, &ioLineCount);

          return result;
        }
        
        //---------------------------------------------------------------------
        static TokenPtr getQuoteToken(
                                      const char * &p,
                                      ULONG &ioLineCount
                                      )
        {
          ULONG currentLine = ioLineCount;
          
          const char *start = p;
          if (!Helper::skipQuote(p, &ioLineCount)) return TokenPtr();

          auto result = make_shared<Token>();
          result->mTokenType = TokenType_Quote;
          result->mToken = String(start, static_cast<size_t>(p - start));
          result->mLineCount = currentLine;
          return result;
        }

        //---------------------------------------------------------------------
        static TokenPtr getCharToken(
                                     const char * &p,
                                     ULONG &ioLineCount
                                     ) throw (FailureWithLine)
        {
          ULONG currentLine = ioLineCount;
          
          const char *start = p;
          if ('\'' != *p) return TokenPtr();
          
          ++p;
          if ('\\' == *p) {
            Helper::decodeCEscape(p, ioLineCount);
          } else {
            ++p;
          }
          if ('\'' != *p) return TokenPtr();
          ++p;

          auto result = make_shared<Token>();
          result->mTokenType = TokenType_Char;
          result->mToken = String(start, static_cast<size_t>(p - start));
          result->mLineCount = currentLine;
          return result;
        }

        //---------------------------------------------------------------------
        static TokenPtr getNumberToken(
                                       const char * &p,
                                       ULONG lineCount
                                       )
        {
          const char *start = p;
          
          bool foundNegative = false;
          bool foundDot = false;
          bool foundExponent = false;

          if ('-' == *start) {
            foundNegative = true;
            ++start;
            Helper::skipWhitespaceExceptEOL(start);
          }

          if (!isnumber(*start)) return TokenPtr();

          p = start;
          
          int base = 10;
          
          if ('0' == *p) {
            switch (*(p+1)) {
              case 'x':
              case 'X': {
                base = 16;
                p += 2;
                break;
              }
              case 'b':
              case 'B': {
                base = 2;
                p += 2;
                break;
              }
              case '.': {
                break;
              }
              case '0':
              case '1':
              case '2':
              case '3':
              case '4':
              case '5':
              case '6':
              case '7': {
                base = 8;
                ++p;
                break;
              }
            }
          }

          while ('\0' != *p)
          {
            switch (*p)
            {
              case '.': {
                if (10 != base) goto check_exponent;
                ++p;
                foundDot = true;
                continue;
              }
              case '0':
              case '1': {
                ++p;
                continue;
              }
              case '2':
              case '3':
              case '4':
              case '5':
              case '6':
              case '7': {
                if (base >= 8) {
                  ++p;
                  continue;
                }
                goto check_exponent;
              }
              case '8':
              case '9': {
                if (base >= 10) {
                  ++p;
                  continue;
                }
                goto check_exponent;
              }
              case 'a':
              case 'A':
              case 'b':
              case 'B':
              case 'c':
              case 'C':
              case 'd':
              case 'D':
              case 'e':
              case 'E':
              case 'f':
              case 'F': {
                if (base >= 10) {
                  ++p;
                  continue;
                }
                goto check_exponent;
              }
              default: goto check_exponent;
            }
          }
          
        check_exponent:
          {
            const char *exponentStart = p;
            
            if (('e' != *p) &&
                ('E' != *p)) {
              goto check_postfix;
            }
            if (10 != base) goto check_postfix;

            foundExponent = true;
            ++p;
            
            bool foundExponentNumber = false;
            if (('-' == *p) ||
                ('+' == *p)) {
              ++p;
            }

            while (isnumber(*p)) {
              foundExponentNumber = true;
              ++p;
            }

            if (!foundExponentNumber) {
              // the 'e' does not belong to the number
              p = exponentStart;
              goto done;
            }
          }
          
        check_postfix:
          {
            const char *postFixStart = p;
            
            bool foundUnsigned = false;
            bool foundFloat = false;
            size_t foundLongs = 0;
            bool lastWasLong = false;
            
            while (true)
            {
              switch (*p) {
                case 'u':
                case 'U':
                {
                  if (foundUnsigned) goto invalid_postfix;
                  if (foundFloat) goto invalid_postfix;
                  foundUnsigned = true;
                  goto not_long;
                }
                case 'l':
                case 'L':
                {
                  if (foundLongs > 0) {
                    if (foundFloat) goto invalid_postfix;
                    if (!lastWasLong) goto invalid_postfix;
                  }
                  ++foundLongs;
                  if (foundLongs > 2) goto invalid_postfix;
                  ++p;
                  lastWasLong = true;
                  continue;
                }
                case 'f':
                case 'F':
                {
                  if (10 != base) goto invalid_postfix;
                  if (foundUnsigned) goto invalid_postfix;
                  if (foundLongs > 1) goto invalid_postfix;
                  if (foundFloat) goto invalid_postfix;
                  foundFloat = true;
                  goto not_long;
                }
                default:
                {
                  goto done;
                }
              }
              
            not_long:
              {
                ++p;
                lastWasLong = false;
                continue;
              }
              
            invalid_postfix:
              {
                p = postFixStart;
                goto done;
              }
            }
          }
          
        done:
          {
          }
          
          auto result = make_shared<Token>();
          result->mTokenType = TokenType_Number;
          result->mToken = String(start, static_cast<size_t>(p - start));
          if (foundNegative) {
            result->mToken = String("-") + result->mToken;
          }
          result->mLineCount = lineCount;
          return result;
        }
        
        //---------------------------------------------------------------------
        static TokenPtr getIdentifierToken(
                                           const char * &p,
                                           ULONG lineCount
                                           )
        {
          if ((!isalpha(*p)) &&
              ('_' != *p)) return TokenPtr();
          
          const char *start = p;
          
          while ((isalnum(*p)) ||
                 ('_' == *p)) {
            ++p;
          }
          
          auto result = make_shared<Token>();
          result->mTokenType = TokenType_Identifier;
          result->mToken = String(start, static_cast<size_t>(p - start));
          result->mLineCount = lineCount;
          return result;
        }
        
        //---------------------------------------------------------------------
        static TokenPtr getBraceToken(
                                      const char * &p,
                                      ULONG lineCount
                                      )
        {
          TokenTypes type {TokenType_First};
          
          switch (*p) {
            case '{':
            case '}': type = TokenType_CurlyBrace; break;
            case '(':
            case ')': type = TokenType_Brace; break;
            case '[':
            case ']': type = TokenType_SquareBrace; break;
            default:  return TokenPtr();
          }
          
          auto result = make_shared<Token>();
          result->mTokenType = type;
          result->mToken = String(p, static_cast<size_t>(1));
          result->mLineCount = lineCount;

          ++p;

          return result;
        }

        //---------------------------------------------------------------------
        static TokenPtr getOperatorToken(
                                         const char * &p,
                                         ULONG lineCount
                                         )
        {
          static const char *operators[] =
          {
            ";",
            "::",
            "++",
            "--",
            ".",
            "->",
            "~",
            "!",
            "+",
            "-",
            "&",
            "*",
            ".*",
            "->*",
            "*",
            "/",
            "%",
            "<<",
            ">>",
            "<",
            ">",
            ">=",
            "<=",
            "==",
            "!=",
            "^",
            "|",
            "&&",
            "||",
            "=",
            "*=",
            "/=",
            "%=",
            "+=",
            "-=",
            ">>=",
            "<<=",
            "&=",
            "^=",
            "|=",
            "?",
            ":",
            ",",
            NULL
          };
          
          String valid;
          String test;

          while ('\0' != *p) {
            test = valid;
            test += String(p, static_cast<size_t>(1));
            
            for (int index = 0; NULL != operators[index]; ++index)
            {
              if (test == operators[index]) goto next;
            }
            goto done;
            
          next:
            {
              valid = test;
              ++p;
            }
          }

        done:
          {
          }
          
          auto result = make_shared<Token>();
          result->mTokenType = TokenType_Operator;
          if (";" == valid) {
            result->mTokenType = TokenType_SemiColon;
          } else if ("::" == valid) {
            result->mTokenType = TokenType_ScopeOperator;
          } else if ("," == valid) {
            result->mTokenType = TokenType_CommaOperator;
          } else if (":" == valid) {
            result->mTokenType = TokenType_ColonOperator;
          } else if ("=" == valid) {
            result->mTokenType = TokenType_EqualsOperator;
          } else if ("*" == valid) {
            result->mTokenType = TokenType_PointerOperator;
          } else if ("&" == valid) {
            result->mTokenType = TokenType_AddressOperator;
          }

          result->mToken = valid;
          result->mLineCount = lineCount;
          return result;
        }
        
        //---------------------------------------------------------------------
        static TokenPtr getUnknownToken(
                                        const char * &p,
                                        ULONG lineCount
                                        )
        {
          if (!p) return TokenPtr();
          
          if ('\0' == *p) return TokenPtr();
          
          auto result = make_shared<Token>();
          result->mTokenType = TokenType_Unknown;
          result->mToken = String(p, static_cast<size_t>(1));
          result->mLineCount = lineCount;
          ++p;
          return result;
        }

        //---------------------------------------------------------------------
        static TokenPtr getNextToken(
                                     const char * &p,
                                     bool &ioStartOfLine,
                                     ULONG &ioLineCount
                                     )
        {
          if (!p) return TokenPtr();
          
          while ('\0' != *p)
          {
            ULONG activeLine = ioLineCount;
            
            if (Helper::skipWhitespaceExceptEOL(p)) continue;
            if (Helper::skipEOL(p, &ioLineCount)) {
              ioStartOfLine = true;
              continue;
            }
            
            if (ioStartOfLine) {
              if ('#' == *p) {
                skipPreprocessor(p, ioLineCount);
                ioStartOfLine = true;
                continue;
              }
            }

            if (Helper::skipCComments(p, &ioLineCount)) {
              if (activeLine != ioLineCount) ioStartOfLine = true;
              continue;
            }

            {
              auto result = getCPPDocToken(p, ioLineCount);
              if (result) {
                ioStartOfLine = true;
                return result;
              }
            }

            if (Helper::skipCPPComments(p)) {
              Helper::skipEOL(p, &ioLineCount);
              ioStartOfLine = true;
              continue;
            }
            
            {
              auto result = getQuoteToken(p, ioLineCount);
              if (result) {
                ioStartOfLine = false;
                return result;
              }
            }
            
            {
              auto result = getNumberToken(p, ioLineCount);
              if (result) {
                ioStartOfLine = false;
                return result;
              }
            }
            
            {
              auto result = getIdentifierToken(p, ioLineCount);
              if (result) {
                ioStartOfLine = false;
                return result;
              }
            }
            
            {
              auto result = getBraceToken(p, ioLineCount);
              if (result) {
                ioStartOfLine = false;
                return result;
              }
            }

            {
              auto result = getOperatorToken(p, ioLineCount);
              if (result) {
                ioStartOfLine = false;
                return result;
              }
            }
            
            {
              auto result = getUnknownToken(p, ioLineCount);
              if (result) {
                ioStartOfLine = false;
                return result;
              }
            }
          }

          return TokenPtr();
        }

        //---------------------------------------------------------------------
        void tokenize(
                      const char *p,
                      TokenList &outTokens,
                      ULONG startLineNumber = 1
                      )
        {
          bool startOfLine = true;
          ULONG lineCount = startLineNumber;
          
          while (true)
          {
            auto token = getNextToken(p, startOfLine, lineCount);
            if (!token) break;

            outTokens.push_back(token);
          }
        }
        
        //---------------------------------------------------------------------
        void replaceAliases(
                            TokenList &ioTokens,
                            const IEventingTypes::AliasMap &aliases
                            )
        {
          for (auto iter_doNotUse = ioTokens.begin(); iter_doNotUse != ioTokens.end(); )
          {
            auto current = iter_doNotUse;
            ++iter_doNotUse;
            
            auto token = (*current);
            auto found = aliases.find(token->mToken);
            if (found == aliases.end()) continue;

            TokenList replacementTokens;
            tokenize((*found).second.c_str(), replacementTokens, token->mLineCount);
            
            for (auto iterReplace = replacementTokens.rbegin(); iterReplace != replacementTokens.rend(); ++iterReplace)
            {
              auto replaceToken = (*iterReplace);
              ioTokens.insert(current, replaceToken);
            }

            ioTokens.erase(current);
          }
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark WrapperCompiler
        #pragma mark

        //---------------------------------------------------------------------
        WrapperCompiler::WrapperCompiler(
                                           const make_private &,
                                           const Config &config
                                           ) :
          mConfig(config)
        {
        }

        //---------------------------------------------------------------------
        WrapperCompiler::~WrapperCompiler()
        {
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark WrapperCompiler => ICompiler
        #pragma mark

        //---------------------------------------------------------------------
        WrapperCompilerPtr WrapperCompiler::create(const Config &config)
        {
          WrapperCompilerPtr pThis(std::make_shared<WrapperCompiler>(make_private{}, config));
          pThis->mThisWeak = pThis;
          return pThis;
        }

        //---------------------------------------------------------------------
        void WrapperCompiler::process() throw (Failure, FailureWithLine)
        {
          outputSkeleton();
          read();
          validate();
          if ((mConfig.mOutputName.hasData()) &&
              (mConfig.mProject)) {
          }
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark WrapperCompiler => (internal)
        #pragma mark

        //---------------------------------------------------------------------
        void WrapperCompiler::outputSkeleton()
        {
        }

        //---------------------------------------------------------------------
        void WrapperCompiler::read() throw (Failure, FailureWithLine)
        {
          HashSet processedHashes;

          ProjectPtr &project = mConfig.mProject;
          
          SecureByteBlockPtr configRaw;

          try {
            configRaw = UseHelper::loadFile(mConfig.mConfigFile);
          } catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_FILE_FAILED_TO_LOAD, String("Failed to load main configuration file: ") + mConfig.mConfigFile + ", error=" + string(e.result()) + ", reason=" + e.message());
          }
          if (!configRaw) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_FILE_FAILED_TO_LOAD, String("Failed to load main configuration file: ") + mConfig.mConfigFile);
          }
          processedHashes.insert(UseHasher::hashAsString(configRaw));
          auto rootEl = UseHelper::read(configRaw);

          try {
            project = Project::create(rootEl);
          } catch (const InvalidContent &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Failed to parse main configuration: " + e.message());
          }

          StringList sources = mConfig.mSourceFiles;
          mConfig.mSourceFiles.clear();

          ElementPtr sourcesEl = rootEl->findFirstChildElement("includes");
          if (sourcesEl) {
            ElementPtr sourceEl = sourcesEl->findFirstChildElement("include");
            while (sourceEl) {
              auto source = UseHelper::getElementTextAndDecode(sourceEl);

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
              auto source = UseHelper::getElementTextAndDecode(includeEl);

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

            SecureByteBlockPtr file;
            try {
              file = UseHelper::loadFile(fileName);
            } catch (const StdError &e) {
              ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_FILE_FAILED_TO_LOAD, String("Failed to load main configuration file: ") + mConfig.mConfigFile + ", error=" + string(e.result()) + ", reason=" + e.message());
            }
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
            auto isJSON = Helper::isLikelyJSON(fileAsStr);

            if (isJSON) {
              try {
                tool::output() << "\n[Info] Reading JSON configuration: " << fileName << "\n\n";
                auto rootEl = UseHelper::read(file);
                if (!rootEl) {
                  ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_FILE_FAILED_TO_LOAD, String("Failed to load file as JSON: ") + fileName);
                }
                if (!project) {
                  project = Project::create(rootEl);
                } else {
                  project->parse(rootEl);
                }
              } catch (const InvalidContent &e) {
                ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Failed to parse JSON configuration: " + e.message());
              }
              continue;
            }

            tool::output() << "\n[Info] Reading C/C++ source file: " << fileName << "\n\n";

            try {
              const char *pos = reinterpret_cast<const char *>(file->BytePtr());
              
              TokenList tokens;
              tokenize(pos, tokens);
              
              replaceAliases(tokens, project->mAliases);

            } catch (const InvalidContent &e) {
              ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Invalid content found: " + e.message());
            } catch (const InvalidContentWithLine &e) {
              ZS_THROW_CUSTOM_PROPERTIES_2(FailureWithLine, ZS_EVENTING_TOOL_INVALID_CONTENT, e.lineNumber(), "Invalid content found: " + e.message());
            }
          }
        }

        //---------------------------------------------------------------------
        void WrapperCompiler::validate() throw (Failure)
        {
          auto &project = mConfig.mProject;
          if (!project) return;

//          if (project->mUniqueHash.isEmpty()) {
//            project->mUniqueHash = project->uniqueEventingHash();
//          }
        }

        //---------------------------------------------------------------------
        void WrapperCompiler::writeXML(const String &outputName, const DocumentPtr &doc) const throw (Failure)
        {
          if (!doc) return;
          try {
            auto output = UseHelper::writeXML(*doc);
            UseHelper::saveFile(outputName, *output);
          } catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to save XML file \"" + outputName + "\": " + " error=" + string(e.result()) + ", reason=" + e.message());
          }
        }

        //---------------------------------------------------------------------
        void WrapperCompiler::writeJSON(const String &outputName, const DocumentPtr &doc) const throw (Failure)
        {
          if (!doc) return;
          try {
            auto output = UseHelper::writeJSON(*doc);
            UseHelper::saveFile(outputName, *output);
          } catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to save JSON file \"" + outputName + "\": " + " error=" + string(e.result()) + ", reason=" + e.message());
          }
        }

        //---------------------------------------------------------------------
        void WrapperCompiler::writeBinary(const String &outputName, const SecureByteBlockPtr &buffer) const throw (Failure)
        {
          if ((!buffer) ||
              (0 == buffer->SizeInBytes())) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to save file \"" + outputName + "\": file is empty");
          }
          try {
            UseHelper::saveFile(outputName, *buffer);
          } catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to save file \"" + outputName + "\": " + " error=" + string(e.result()) + ", reason=" + e.message());
          }
        }

      } // namespace internal
    } // namespace tool
  } // namespace eventing
} // namespace zsLib
