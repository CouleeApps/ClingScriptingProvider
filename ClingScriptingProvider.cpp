//
// Created by Glenn Smith on 10/20/23.
//

#include "ClingScriptingProvider.h"
#include <cling/Interpreter/Value.h>

using namespace BinaryNinja;


//region Scripting Instance

BNOutStream::BNOutStream(ClingScriptingInstance* instance) : llvm::raw_ostream(true), m_instance(instance)
{

}

void BNOutStream::write_impl(const char *Ptr, size_t Size) {
    m_instance->Output(std::string(Ptr, Size));
}

uint64_t BNOutStream::current_pos() const {
    return 0;
}


class ClingInterperterCallbacks: public cling::InterpreterCallbacks
{

};


ClingScriptingInstance::ClingScriptingInstance(ClingScriptingProvider* provider):
BinaryNinja::ScriptingInstance(provider) {
    std::vector<char*> options;
    m_interpreter = std::make_unique<cling::Interpreter>(options.size(), options.data());
    m_output = std::make_unique<BNOutStream>(this);
    m_processor = std::make_unique<cling::MetaProcessor>(*m_interpreter, *m_output);
}


BNScriptingProviderExecuteResult
ClingScriptingInstance::ExecuteScriptInput(const std::string &input) {
    cling::Interpreter::CompilationResult compRes;
    cling::Value result;
    m_processor->process(llvm::StringRef(input), compRes, &result);

    switch (compRes)
    {
        case cling::Interpreter::kSuccess:
            return SuccessfulScriptExecution;
        case cling::Interpreter::kFailure:
            return InvalidScriptInput;
        case cling::Interpreter::kMoreInputExpected:
            return IncompleteScriptInput;
    }
}


BNScriptingProviderExecuteResult
ClingScriptingInstance::ExecuteScriptInputFromFilename(const std::string &filename) {
    cling::Value result;
    cling::Interpreter::CompilationResult compRes = m_processor->readInputFromFile(llvm::StringRef(filename), &result);

    switch (compRes)
    {
        case cling::Interpreter::kSuccess:
            return SuccessfulScriptExecution;
        case cling::Interpreter::kFailure:
            return InvalidScriptInput;
        case cling::Interpreter::kMoreInputExpected:
            return IncompleteScriptInput;
    }
}


//endregion

//---------------------------------------------------------------------------------------

//region Scripting Provider
ClingScriptingProvider* ClingScriptingProvider::g_instance = nullptr;


ClingScriptingProvider::ClingScriptingProvider(): ScriptingProvider("Cling", "cling") {

}


void ClingScriptingProvider::init() {
    g_instance = new ClingScriptingProvider();
    Register(g_instance);
}

Ref<ScriptingInstance> ClingScriptingProvider::CreateNewInstance() {
    return new ClingScriptingInstance(this);
}

bool ClingScriptingProvider::LoadModule(const std::string &repository,
                                        const std::string &module, bool force) {
    return false;
}

bool ClingScriptingProvider::InstallModules(const std::string &modules) {
    return false;
}

//endregion
