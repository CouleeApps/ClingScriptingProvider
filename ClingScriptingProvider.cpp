//
// Created by Glenn Smith on 10/20/23.
//

#include "ClingScriptingProvider.h"
#include <cling/Interpreter/Value.h>
#include <cling/Utils/Output.h>
#include <llvm/Support/CrashRecoveryContext.h>

using namespace BinaryNinja;


//region Scripting Instance

BNOutStream::BNOutStream(ClingScriptingInstance* instance) : llvm::raw_ostream(true), m_instance(instance) {

}

void BNOutStream::write_impl(const char *Ptr, size_t Size) {
    m_instance->Output(std::string(Ptr, Size));
}

uint64_t BNOutStream::current_pos() const {
    return 0;
}

BNErrStream::BNErrStream(ClingScriptingInstance* instance) : llvm::raw_ostream(true), m_instance(instance) {

}

void BNErrStream::write_impl(const char *Ptr, size_t Size) {
    m_instance->Error(std::string(Ptr, Size));
}

uint64_t BNErrStream::current_pos() const {
    return 0;
}

ClingScriptingInstance::ClingScriptingInstance(ClingScriptingProvider* provider):
BinaryNinja::ScriptingInstance(provider) {
    m_output = std::make_unique<BNOutStream>(this);
    m_error = std::make_unique<BNErrStream>(this);

    cling::utils::setOuts(m_output.get());
    cling::utils::setErrs(m_error.get());

    initInterpreter();
}

void ClingScriptingInstance::initInterpreter() {
    std::vector<std::string> cppOptions;
    cppOptions.push_back("cling");
    cppOptions.push_back("-std=c++17");
    cppOptions.push_back("-isystem");
    cppOptions.push_back(LLVM_HEADERS_PATH);

    std::vector<char*> options;

    for (auto& option: cppOptions) {
        options.push_back(option.data());
    }

    m_interpreter = std::make_unique<cling::Interpreter>(options.size(), options.data());
    const auto& opts = m_interpreter->getOptions();

    if (!m_interpreter->isValid()) {
        Error("Could not start interpreter\n");
        InputReadyStateChanged(NotReadyForInput);
        m_interpreter = nullptr;
        return;
    }
    m_interpreter->AddIncludePath(".");

    for (const auto& lib: opts.LibsToLoad)
        m_interpreter->loadFile(lib);

    m_processor = std::make_unique<cling::MetaProcessor>(*m_interpreter, *m_output);

    m_interpreter->printIncludedFiles(*m_output.get());
    m_interpreter->DumpIncludePath(m_output.get());
    m_interpreter->DumpDynamicLibraryInfo(m_output.get());
    InputReadyStateChanged(ReadyForScriptProgramInput);
}


BNScriptingProviderExecuteResult
ClingScriptingInstance::ExecuteScriptInput(const std::string &input) {
    llvm::CrashRecoveryContext::Enable();
    llvm::CrashRecoveryContext context;

    BNScriptingProviderExecuteResult success = ScriptExecutionCancelled;

    if (!context.RunSafely([&]() {
        cling::Interpreter::CompilationResult compRes;
        cling::Value result;

        if (m_processor->awaitingMoreInput())
            m_processor->cancelContinuation();
        m_processor->process(llvm::StringRef(input), compRes, &result);

        switch (compRes)
        {
            case cling::Interpreter::kSuccess:
                success = SuccessfulScriptExecution;
                break;
            case cling::Interpreter::kFailure:
                success = SuccessfulScriptExecution;
                break;
            case cling::Interpreter::kMoreInputExpected:
                success = IncompleteScriptInput;
                break;
        }
    }))
    {
        llvm::CrashRecoveryContext::Disable();
        Error("Error in execution [crash recovered]\n");

        llvm::CrashRecoveryContext::Enable();
        llvm::CrashRecoveryContext context2;
        if (!context2.RunSafely([&]() {
            initInterpreter();
        }))
        {
            Error("Error in execution [crashed restarting interpreter too, need restart]\n");
            InputReadyStateChanged(NotReadyForInput);
            llvm::CrashRecoveryContext::Disable();
            return SuccessfulScriptExecution;
        }
        llvm::CrashRecoveryContext::Disable();
        return SuccessfulScriptExecution;
    }
    llvm::CrashRecoveryContext::Disable();
    return success;
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
