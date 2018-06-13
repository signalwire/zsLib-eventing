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

#pragma once

#include <zsLib/eventing/tool/internal/types.h>

#include <zsLib/eventing/tool/ICompiler.h>
#include <zsLib/eventing/IIDLTypes.h>

#include <stack>

namespace zsLib
{
  namespace eventing
  {
    namespace tool
    {
      namespace internal
      {
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //
        // IDLCompiler
        //

        class IDLCompiler : public ICompiler,
                            public IIDLTypes
        {
          struct make_private {};

        public:
          ZS_DECLARE_TYPEDEF_PTR(IIDLTypes::Project, Project);
          
          typedef std::list<ElementPtr> ElementList;
          
          typedef std::map<Name, StringList> ModifierValueMap;

          ZS_DECLARE_STRUCT_PTR(Token);
          typedef std::list<TokenPtr> TokenList;
          ZS_DECLARE_PTR(TokenList);

          typedef std::stack<TokenPtr> TokenStack;
          typedef std::stack<TokenListPtr> TokenListStack;
          typedef std::list<TokenListPtr> TokenListList;

          enum TokenTypes
          {
            TokenType_First,
            
            TokenType_Unknown = TokenType_First,

            TokenType_Directive,
            TokenType_Documentation,
            TokenType_Char,
            TokenType_Quote,

            TokenType_Number,
            TokenType_Identifier,

            TokenType_SemiColon,

            TokenType_Brace,        // (
            TokenType_CurlyBrace,   // {
            TokenType_SquareBrace,  // [
            TokenType_AngleBrace,   // <

            TokenType_Operator,
            TokenType_ScopeOperator,
            TokenType_CommaOperator,
            TokenType_ColonOperator,
            TokenType_EqualsOperator,

            TokenType_Last = TokenType_EqualsOperator,
          };

          typedef std::set<TokenTypes> TokenTypeSet;

          struct Token
          {
            TokenTypes mTokenType {TokenType_First};
            String mToken;
            ULONG mLineCount {1};

            bool isBrace() const noexcept;
            bool isOpenBrace() const noexcept;
            bool isCloseBrace() const noexcept;
            bool isOpenBrace(TokenTypes type) const noexcept;
            bool isCloseBrace(TokenTypes type) const noexcept;
            bool isIdentifier(const char *identifier) const noexcept;
          };

        public:
          //-------------------------------------------------------------------
          IDLCompiler(
                      const make_private &,
                      const Config &config
                      ) noexcept;
          ~IDLCompiler() noexcept;

          IDLCompiler(const Noop &) noexcept;

          static void installDefaultTargets() noexcept;

          //-------------------------------------------------------------------
          //
          // IDLCompiler => ICompiler
          //

          static IDLCompilerPtr create(const Config &config) noexcept;

          void process() noexcept(false) override; // throws Failure, FailureWithLine

        protected:
          //-------------------------------------------------------------------
          //
          // IDLCompiler => (internal)
          //

          void outputSkeleton() noexcept;
          void read() noexcept(false); // throws Failure, FailureWithLine
          void validate() noexcept(false); // throws Failure

          bool parseNamespace(NamespacePtr parent) noexcept(false); // throws FailureWithLine
          void parseNamespaceContents(NamespacePtr namespaceObj) noexcept(false); // throws FailureWithLine

          bool parseUsing(NamespacePtr namespaceObj) noexcept(false); // throws FailureWithLine
          bool parseTypedef(ContextPtr context) noexcept(false); // throws FailureWithLine
          bool parseStruct(ContextPtr context) noexcept(false); // throws FailureWithLine
          bool parseEnum(ContextPtr context) noexcept(false); // throws FailureWithLine
          bool parseProperty(StructPtr context) noexcept(false); // throws FailureWithLine
          bool parseMethod(StructPtr context) noexcept(false); // throws FailureWithLine

          bool parseDocumentation() noexcept;
          bool parseSemiColon() noexcept;
          bool parseComma() noexcept;
          bool parseModifiers() noexcept(false); // throws FailureWithLine
          bool parseDirective() noexcept(false); // throws FailureWithLine
          bool pushDirectiveTokens(TokenPtr token) noexcept(false); // throws FailureWithLine
          bool parseDirectiveExclusive(bool &outIgnoreMode) noexcept(false); // throws FailureWithLine

          ElementPtr getDocumentation() noexcept;
          ElementPtr getDirectives() noexcept;
          void mergeDocumentation(ElementPtr &existingDocumentation) noexcept;
          void mergeDirectives(ElementPtr &existingDocumentation) noexcept;
          void mergeModifiers(ContextPtr context) noexcept(false); // throws FailureWithLine
          void fillContext(ContextPtr context) noexcept;

          static String makeTypenameFromTokens(const TokenList &tokens) noexcept(false); // throws InvalidContent

          void pushTokens(const TokenList &tokens) noexcept;
          void pushTokens(TokenListPtr tokens) noexcept;
          TokenListPtr getTokens() const noexcept;
          TokenListPtr popTokens() noexcept;

          bool hasMoreTokens() const noexcept;
          TokenPtr peekNextToken(const char *whatExpectingMoreTokens) noexcept(false); // throws FailureWithLine
          TokenPtr extractNextToken(const char *whatExpectingMoreTokens) noexcept(false); // throws FailureWithLine
          void putBackToken(TokenPtr token) noexcept;
          void putBackTokens(const TokenList &tokens) noexcept;
          ULONG getLastLineNumber() const noexcept;
          
          static void insertBefore(
                                   TokenList &tokens,
                                   const TokenList &insertTheseTokens
                                   ) noexcept;
          static void insertAfter(
                                  TokenList &tokens,
                                  const TokenList &insertTheseTokens
                                  ) noexcept;

          bool extractToClosingBraceToken(
                                          const char *whatExpectingClosingToken,
                                          TokenList &outTokens,
                                          bool includeOuterBrace = false
                                          ) noexcept(false); // throw FailureWithLine

          bool extractToComma(
                              const char *whatExpectingComma,
                              TokenList &outTokens
                              ) noexcept(false); // throw FailureWithLine
          
          bool extractToEquals(
                               const char *whatExpectingComma,
                               TokenList &outTokens
                               ) noexcept(false); // throw FailureWithLine
          
          bool extractToTokenType(
                                  const char *whatExpectingComma,
                                  TokenTypes searchTokenType,
                                  TokenList &outTokens,
                                  bool includeFoundToken = false,
                                  bool processBrackets = true
                                  ) noexcept(false); // throw FailureWithLine

          TokenPtr peekAheadToFirstTokenOfType(const TokenTypeSet &tokenTypes) noexcept;

          void processUsingNamespace(
                                     NamespacePtr currentNamespace,
                                     NamespacePtr usingNamespace
                                     ) noexcept;
          void processUsingType(
                                NamespacePtr currentNamespace,
                                TypePtr usingType
                                ) noexcept;
          void processTypedef(
                              ContextPtr context,
                              const TokenList &typeTokens,
                              const String &typeName
                              ) noexcept(false); /// throws FailureWithLine
          StructPtr processStructForward(
                                         ContextPtr context,
                                         const String &typeName,
                                         bool *wasCreated = NULL
                                         ) noexcept(false); // throws FailureWithLine
          void processRelated(
                              StructPtr structObj,
                              const TokenList &typeTokens
                              ) noexcept(false); // throws FailureWithLine

          TypePtr findTypeOrCreateTypedef(
                                          ContextPtr context,
                                          const TokenList &tokens,
                                          TypedefTypePtr &outCreatedTypedef
                                          ) noexcept(false); // throws FailureWithLine

          static void writeXML(const String &outputName, const DocumentPtr &doc) noexcept(false); // throws Failure
          static void writeJSON(const String &outputName, const DocumentPtr &doc) noexcept(false); // throws Failure
          static void writeBinary(const String &outputName, const SecureByteBlockPtr &buffer) noexcept(false); // throws Failure

        private:
          //-------------------------------------------------------------------
          //
          // IDLCompiler => (data)
          //

          IDLCompilerWeakPtr mThisWeak;

          Config mConfig;
          TokenList mPendingDocumentation;
          ElementList mPendingDirectives;
          ModifierValueMap mPendingModifiers;

          TokenListStack mTokenListStack;
          TokenStack mLastTokenStack;
          TokenPtr mLastToken;
        };

      } // namespace internal
    } // namespace tool
  } // namespace eventing
} // namespace zsLib
