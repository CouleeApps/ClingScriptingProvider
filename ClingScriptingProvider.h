//
// Created by Glenn Smith on 10/20/23.
//

#ifndef CLINGSCRIPTINGPROVIDER_CLINGSCRIPTINGPROVIDER_H
#define CLINGSCRIPTINGPROVIDER_CLINGSCRIPTINGPROVIDER_H

#include "binaryninjaapi.h"
#include <memory>
#include <cling/Interpreter/Interpreter.h>
#include <cling/MetaProcessor/MetaProcessor.h>
#include <llvm/Support/raw_ostream.h>
#include <cling/Interpreter/InterpreterCallbacks.h>

class BNOutStream: public llvm::raw_ostream {
    class ClingScriptingInstance* m_instance;

public:
    explicit BNOutStream(class ClingScriptingInstance* instance);
    void write_impl(const char *Ptr, size_t Size) override;
    uint64_t current_pos() const override;
};


class ClingScriptingInstance: public BinaryNinja::ScriptingInstance {
    friend class ClingScriptingProvider;

    std::unique_ptr<cling::Interpreter> m_interpreter;
    std::unique_ptr<BNOutStream> m_output;
    std::unique_ptr<cling::MetaProcessor> m_processor;

    explicit ClingScriptingInstance(class ClingScriptingProvider* provider);

public:
    BNScriptingProviderExecuteResult
    ExecuteScriptInput(const std::string &input) override;

    BNScriptingProviderExecuteResult
    ExecuteScriptInputFromFilename(const std::string &filename) override;
};


class ClingScriptingProvider: public BinaryNinja::ScriptingProvider {
    static ClingScriptingProvider* g_instance;

    ClingScriptingProvider();

public:
    static void init();

    BinaryNinja::Ref<BinaryNinja::ScriptingInstance> CreateNewInstance() override;

    bool LoadModule(const std::string &repository, const std::string &module,
                    bool force) override;

    bool InstallModules(const std::string &modules) override;
};


#endif //CLINGSCRIPTINGPROVIDER_CLINGSCRIPTINGPROVIDER_H
