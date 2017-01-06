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
#include <zsLib/eventing/IWrapperTypes.h>

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
        #pragma mark
        #pragma mark WrapperCompiler
        #pragma mark

        class WrapperCompiler : public ICompiler,
                                public IWrapperTypes
        {
          struct make_private {};

        public:
          ZS_DECLARE_TYPEDEF_PTR(IWrapperTypes::Project, Project);

          ZS_DECLARE_STRUCT_PTR(Token);
          typedef std::list<TokenPtr> TokenList;
          ZS_DECLARE_PTR(TokenList);

          typedef std::stack<TokenPtr> TokenStack;
          typedef std::stack<TokenListPtr> TokenListStack;

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
            
            TokenType_Brace,
            TokenType_CurlyBrace,
            TokenType_SquareBrace,
            TokenType_AngleBrace,

            TokenType_Operator,
            TokenType_ScopeOperator,
            TokenType_CommaOperator,
            TokenType_ColonOperator,
            TokenType_EqualsOperator,
            
            TokenType_PointerOperator,
            TokenType_AddressOperator,
            TokenType_CarotOperator,
            
            TokenType_Last = TokenType_CarotOperator,
          };

          struct Token
          {
            TokenTypes mTokenType {TokenType_First};
            String mToken;
            ULONG mLineCount {1};
            
            bool isOpenBrace() const;
            bool isCloseBrace() const;
          };
          
        public:
          //-------------------------------------------------------------------
          WrapperCompiler(
                           const make_private &,
                           const Config &config
                           );
          ~WrapperCompiler();

          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark WrapperCompiler => ICompiler
          #pragma mark

          static WrapperCompilerPtr create(const Config &config);

          virtual void process() throw (Failure, FailureWithLine);

        protected:
          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark WrapperCompiler => (internal)
          #pragma mark

          void outputSkeleton();
          void read() throw (Failure, FailureWithLine);
          void validate() throw (Failure);
          
          bool parseNamespace(NamespacePtr parent) throw (FailureWithLine);
          void parseNamespaceContents(NamespacePtr namespaceObj) throw (FailureWithLine);
          
          bool parseUsing(NamespacePtr namespaceObj) throw (FailureWithLine);

          bool parseDocumentation();
          bool parseSemiColon();
          bool parseDirective() throw (FailureWithLine);
          bool pushDirectiveTokens(TokenPtr token) throw (FailureWithLine);
          bool parseDirectiveExclusive(bool &outIgnoreMode) throw (FailureWithLine);

          ElementPtr getDocumentation();
          void mergeDocumentation(ElementPtr &existingDocumentation);

          void pushTokens(const TokenList &tokens);
          void pushTokens(TokenListPtr tokens);
          TokenListPtr getTokens() const;
          TokenListPtr popTokens();

          bool hasMoreTokens() const;
          TokenPtr peekNextToken(const char *whatExpectingMoreTokens) throw (FailureWithLine);
          TokenPtr extractNextToken(const char *whatExpectingMoreTokens) throw (FailureWithLine);
          void putBackToken(TokenPtr token);
          void putBackTokens(const TokenList &tokens);

          void processUsingNamespace(
                                     NamespacePtr currentNamespace,
                                     NamespacePtr usingNamespace
                                     );
          void processUsingType(
                                NamespacePtr currentNamespace,
                                TypePtr usingType
                                );

          TypePtr findTypeOrCreateTypedef(
                                          const TokenList &tokens,
                                          TypedefPtr &outCreatedTypedef
                                          );

          void writeXML(const String &outputName, const DocumentPtr &doc) const throw (Failure);
          void writeJSON(const String &outputName, const DocumentPtr &doc) const throw (Failure);
          void writeBinary(const String &outputName, const SecureByteBlockPtr &buffer) const throw (Failure);

        private:
          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark WrapperCompiler => (data)
          #pragma mark

          WrapperCompilerWeakPtr mThisWeak;

          Config mConfig;
          TokenList mPendingDocumentation;

          TokenListStack mTokenListStack;
          TokenStack mLastTokenStack;
          TokenPtr mLastToken;
        };

      } // namespace internal
    } // namespace tool
  } // namespace eventing
} // namespace zsLib
