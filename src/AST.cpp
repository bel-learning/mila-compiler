//
// Created by Bilguudei Baljinnyam on 14.06.2024.
//

#include "AST.hpp"

BlockAST::BlockAST(std::vector<std::unique_ptr<AST>> body) : m_Body(std::move(body)) {}

void BlockAST::print(std::ostream &out, int indent) const {
    out << std::string(indent, ' ') << "{\n";
    for (const auto &stmt : m_Body) {
        if(!stmt)
            std::clog << "Nullptr in: " << " block" << std::endl;
        else
            stmt->print(out, indent + 2);
        out << ",\n";
    }
    out << std::string(indent, ' ') << "}";
}
llvm::Value * BlockAST::codegen(GenContext& gen) {
    for(auto & expression : m_Body) {
        expression->codegen(gen);
    }
    return nullptr;
};

TypeAST::TypeAST(Type type) : m_type(type) {};
//    void print(std::ostream& os, unsigned indent = 0) const override;
//    llvm::Value* codegen(GenContext& gen) const override;
void TypeAST::print(std::ostream &out, int indent ) const {
    out << std::string(indent, ' ') << " Type: " << "INT" << "\n";
}
llvm::Value * TypeAST::codegen(GenContext& gen) { return nullptr;};


NumberExprAST::NumberExprAST(int val) : m_Val(val) {}
const std::string & NumberExprAST::getName() const { return ""; };

void NumberExprAST::print(std::ostream &out, int indent) const {
    out << std::string(indent, ' ') << "{\n";
    out << std::string(indent + 2, ' ') << "\"type\": \"NumberExprAST\",\n";
    out << std::string(indent + 2, ' ') << "\"value\": " << m_Val << "\n";
    out << std::string(indent, ' ') << "}";
}
llvm::Value * NumberExprAST::codegen(GenContext& gen) {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.ctx), m_Val, true);
}


DeclRefAST::DeclRefAST(std::string var): m_Var(var) {};

const std::string & DeclRefAST::getName() const { return m_Var; };

void DeclRefAST::print(std::ostream &out, int indent) const {
    out << std::string(indent, ' ') << "{\n";
    out << std::string(indent + 2, ' ') << "\"type\": \"DeclRefASTNode\",\n";
    out << std::string(indent + 2, ' ') << "\"name\": \"" << m_Var << "\"\n";
    out << std::string(indent, ' ') << "}";
};
llvm::Value * DeclRefAST::codegen(GenContext& gen) {
    // Look up the variable in the symbol table
//        std::clog << "Codegening DeclRefAST: " << m_Var << std::endl;
    auto searchIt = gen.symbTable.find(m_Var);
    if (searchIt == gen.symbTable.end()) {
        throw std::runtime_error("Unknown variable name: " + m_Var);
    }
    // Return the stored LLVM Value for the variable
    llvm::Value * val = gen.builder.CreateLoad(llvm::Type::getInt32Ty(gen.ctx), searchIt->second.store, m_Var);

    return val;
};
//    llvm::Value* codegen(GenContext& gen) const override;
//    llvm::AllocaInst* getStore(GenContext& gen) const;

VarDeclAST::VarDeclAST(std::string var, std::unique_ptr<TypeAST> type, std::unique_ptr<ExprAST> expr, bool constant) :
        m_var(var), m_type(std::move(type)), m_expr(std::move(expr)), m_constant(constant) {};
void VarDeclAST::print(std::ostream &out, int indent) const {
    out << std::string(indent, ' ') << "{\n";
    out << std::string(indent + 2, ' ') << "\"type\": \"VarDeclAST\",\n";
    out << std::string(indent + 2, ' ') << "\"mvar\": "<< m_var;
//        out << std::string(indent + 2, ' ') << "\"operator\": \"" << Op << "\",\n";
    out << "\n" << std::string(indent, ' ') << "}";
};
llvm::Value* VarDeclAST::codegen(GenContext &gen) {
    // Generate the type for the variable
//        llvm::Type* varType = m_type->codegen(gen);
    std::clog << "Codegening VarDeclAST: " << m_var << std::endl;
    llvm::Type * varType = llvm::Type::getInt32Ty(gen.ctx);

    if (!varType) {
        throw std::runtime_error("Unknown type for variable: " + m_var);
    }

    // Create an alloca instruction in the entry block of the function
    llvm::Function* function = gen.builder.GetInsertBlock()->getParent();
    llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(),
                                 function->getEntryBlock().begin());
    llvm::AllocaInst* alloca = tmpBuilder.CreateAlloca(varType, 0, m_var.c_str());

    // Initialize the variable if an initializer expression is provided
    if (m_expr) {
        llvm::Value* initVal = m_expr->codegen(gen);
        if (!initVal) {
            throw std::runtime_error("Failed to generate initializer for variable: " + m_var);
        }

        // Ensure the types match
//            if (initVal->getType() != varType) {
//                throw std::runtime_error("Type mismatch in initializer for variable: " + m_var);
//            }

        gen.builder.CreateStore(initVal, alloca);
    }

    // Add the variable to the symbol table
    auto searchIt = gen.symbTable.find(m_var);
    if(searchIt != gen.symbTable.end()) {
        throw std::runtime_error("Already exists var: " + m_var);
    }
    gen.symbTable[m_var] = {alloca, m_constant};

    return alloca;
}

//    llvm::Value* codegen(GenContext& gen) const override;

BinaryExprAST::BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
        : Op(Op), m_LHS(std::move(LHS)), m_RHS(std::move(RHS)) {}
const std::string & BinaryExprAST::getName() const { return ""; };

void BinaryExprAST::print(std::ostream &out, int indent) const {
    out << std::string(indent, ' ') << "{\n";
    out << std::string(indent + 2, ' ') << "\"type\": \"BinaryExprAST\",\n";
    out << std::string(indent + 2, ' ') << "\"operator\": \"" << Op << "\",\n";
    out << std::string(indent + 2, ' ') << "\"LHS\": ";
    m_LHS->print(out, indent + 2);
    out << ",\n";
    out << std::string(indent + 2, ' ') << "\"RHS\": ";
    m_RHS->print(out, indent + 2);
    out << "\n" << std::string(indent, ' ') << "}";
}
llvm::Value * BinaryExprAST::codegen(GenContext& gen)  {
    llvm::Value *L = m_LHS->codegen(gen);
    llvm::Value *R = m_RHS->codegen(gen);
    if (!L || !R)
        return nullptr;

    switch (Op) {
        case '+':
            return gen.builder.CreateAdd(L, R, "addtmp");
        case '-':
            return gen.builder.CreateSub(L, R, "subtmp");
        case '*':
            return gen.builder.CreateMul(L, R, "multmp");
        case '<':
            L = gen.builder.CreateICmpSLT(L, R, "cmptmp");
            // Convert bool 0/1 to int 0 or 1
            return gen.builder.CreateIntCast(L, llvm::Type::getInt32Ty(gen.ctx), false, "lesstmp");
        case '>':
            L = gen.builder.CreateICmpSGT(L, R, "cmptmp");
            // Convert bool 0/1 to int 0 or 1
            return gen.builder.CreateIntCast(L, llvm::Type::getInt32Ty(gen.ctx), false, "greatertmp");
        case tok_lessequal:
            L = gen.builder.CreateICmpSLE(L, R, "cmptmp");
            // Convert bool 0/1 to int 0 or 1
            return gen.builder.CreateIntCast(L, llvm::Type::getInt32Ty(gen.ctx), false, "lsetmp");
        case tok_greaterequal:
            L = gen.builder.CreateICmpSGE(L, R, "cmptmp");
            // Convert bool 0/1 to int 0 or 1
            return gen.builder.CreateIntCast(L, llvm::Type::getInt32Ty(gen.ctx), false, "gsetmp");
        case tok_equal:
            L = gen.builder.CreateICmpEQ(L, R, "cmptmp");
            // Convert bool 0/1 to int 0 or 1
            return gen.builder.CreateIntCast(L, llvm::Type::getInt32Ty(gen.ctx), false, "eqtmp");
        case tok_notequal:
            L = gen.builder.CreateICmpNE(L, R, "cmptmp");
            // Convert bool 0/1 to int 0 or 1
            return gen.builder.CreateIntCast(L, llvm::Type::getInt32Ty(gen.ctx), false, "netmp");
        case tok_or:
            L = gen.builder.CreateOr(L, R, "ortmp");
            // Convert bool 0/1 to int 0 or 1
            return gen.builder.CreateIntCast(L, llvm::Type::getInt32Ty(gen.ctx), false, "ortmp");
        case tok_and:
            L = gen.builder.CreateAnd(L, R, "andtmp");
            // Convert bool 0/1 to int 0 or 1
            return gen.builder.CreateIntCast(L, llvm::Type::getInt32Ty(gen.ctx), false, "andtmp");

        case tok_mod:
            return gen.builder.CreateSRem(L, R, "sremtmp");
        case tok_assign:
            return codegenAssignment(gen);


//            case tok_and:
//                L = gen.builder.CreateSRem(L, R, "sremtmp");
        default:
            std::clog << "Exception" << std::endl;
            return nullptr;
    };
}

llvm::Value * BinaryExprAST::codegenAssignment(GenContext & gen) {
    // Generate code for the LHS, which should be an address
    llvm::Value* lhs = m_LHS->codegen(gen);
    if (!lhs) {
        throw std::runtime_error("Failed to generate LHS for assignment.");
    }
    auto searchIter = gen.symbTable.find(m_LHS->getName());
    if(searchIter != gen.symbTable.end()) {
        if(searchIter->second.constant) {
            throw std::runtime_error("Trying to change const value");
        }
    }

    // Generate code for the RHS, which should be a value
    llvm::Value* rhs = m_RHS->codegen(gen);
    if (!rhs) {
        throw std::runtime_error("Failed to generate RHS for assignment.");
    }

    llvm::Value * variable = gen.symbTable[m_LHS->getName()].store;
    if(!variable) {
        std::clog << "Var name(from LLVM): " << m_LHS->getName() << std::endl;
        throw std::runtime_error("Unknown variable");
    }

    // Store the RHS value into the LHS address
    gen.builder.CreateStore(rhs, variable);

    // Return the stored value (RHS) for any further use
    return rhs;
}


CallExprAST::CallExprAST(std::string Callee, std::vector<std::unique_ptr<ExprAST>> Args)
        : Callee(std::move(Callee)), Args(std::move(Args)) {}
const std::string & CallExprAST::getName() const { return Callee; }

void CallExprAST::print(std::ostream &out, int indent) const {
    out << std::string(indent, ' ') << "{\n";
    out << std::string(indent + 2, ' ') << "\"type\": \"CallExprAST\",\n";
    out << std::string(indent + 2, ' ') << "\"callee\": \"" << Callee << "\",\n";
    out << std::string(indent + 2, ' ') << "\"args\": [\n";
    for (const auto &arg : Args) {
        arg->print(out, indent + 4);
        out << ",\n";
    }
    out << std::string(indent + 2, ' ') << "]\n";
    out << std::string(indent, ' ') << "}";
}
llvm::Value * CallExprAST::PredefinedFunctions(GenContext& gen) {
    if(Callee == "dec") {
        if(Args.empty()) return nullptr;
        llvm::AllocaInst * Var = gen.symbTable.find(Args[0]->getName())->second.store;
        llvm::Value * Val = gen.builder.CreateLoad(llvm::Type::getInt32Ty(gen.ctx), Var, Args[0]->getName());
        llvm::Value * Add = gen.builder.CreateSub(Val, NumberExprAST(1).codegen(gen));
        gen.builder.CreateStore(Add, Var);
        gen.symbTable[Args[0]->getName()] = {Var, false};
        return Add;
    }
    return nullptr;
}
llvm::Value * CallExprAST::codegen(GenContext& gen)  {
//        for (const llvm::Function &func : gen.module) {
//            if (!func.isDeclaration()) {
//                llvm::outs() << "Function: " << func.getName() << "\n";
//            }
//        }
//        Check preexisting functions
    llvm::Value * retValue = PredefinedFunctions(gen);
    if(retValue != nullptr) {
        return retValue;
    }

    llvm::Function * calleeF = gen.module.getFunction(Callee);
    if(calleeF == nullptr) {
        throw std::runtime_error("Unknown function referenced: " + Callee);
    }
    // Check if the argument count matches.
    if (calleeF->arg_size() != Args.size()) {
        throw std::runtime_error("Incorrect number of arguments passed to: " + Callee);
    }

    std::vector<llvm::Value *> argsV;
    for (unsigned i = 0; i < Args.size(); ++i) {

        llvm::Value *argValue = Args[i]->codegen(gen);

        // Special case for "readln" function
        if (Callee == "readln") {
            // Create a pointer to the argument if necessary
            llvm::AllocaInst * var = gen.symbTable[Args[i]->getName()].store;
            if(!var) {
                throw std::runtime_error("Var doesn't exist");
            }
            //                Args[i].
            argsV.push_back(var);
        } else {
            argsV.push_back(argValue);
        }

    }

    // Check if the callee function returns void
    if (calleeF->getReturnType()->isVoidTy()) {
        gen.builder.CreateCall(calleeF, argsV);

        return nullptr; // No value to return for void functions
    } else {
        return  gen.builder.CreateCall(calleeF, argsV, "callfunc");; // Return the call instruction for non-void functions
    }
};

PrototypeAST::PrototypeAST(const std::string &Name, std::vector<std::string> Args, std::unique_ptr<VarDeclAST> Return)
        : m_Name(Name), m_Args(std::move(Args)), m_Return(std::move(Return)) {}

const std::string & PrototypeAST::getName() const { return m_Name; }

void PrototypeAST::print(std::ostream &out, int indent ) const {
    out << std::string(indent, ' ') << "{\n";
    out << std::string(indent + 2, ' ') << "\"type\": \"PrototypeAST\",\n";
    out << std::string(indent + 2, ' ') << "\"name\": \"" << m_Name << "\",\n";
    out << std::string(indent + 2, ' ') << "\"args\": [\n";
    for (const auto &arg : m_Args) {
        out << std::string(indent + 4, ' ') << "\"" << arg << "\",\n";
    }
    out << std::string(indent + 2, ' ') << "]\n";
    out << std::string(indent, ' ') << "}";
}
llvm::Function * PrototypeAST::codegen(GenContext& gen)  {
    std::vector<llvm::Type *> INTS(m_Args.size(), llvm::Type::getInt32Ty(gen.ctx));
    llvm::FunctionType * FT = nullptr;
    if(m_Return == nullptr && m_Name != "main")
        FT = llvm::FunctionType::get(llvm::Type::getVoidTy(gen.ctx), INTS ,false);
    else
        FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(gen.ctx), INTS ,false);
    llvm::Function *F =
            llvm::Function::Create(FT, llvm::Function::ExternalLinkage, m_Name, gen.module);

    // Set names for all arguments.
    unsigned Idx = 0;
    for (auto &arg : F->args())
        arg.setName(m_Args[Idx++]);

    return F;
};

FunctionAST::FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::vector<std::unique_ptr<VarDeclAST>> Vars, std::unique_ptr<AST> Body)
        : m_Proto(std::move(Proto)), m_Vars(std::move(Vars)), m_Body(std::move(Body)) {}

void FunctionAST::print(std::ostream &out, int indent) const {
    out << std::string(indent, ' ') << "{\n";
    out << std::string(indent + 2, ' ') << "\"type\": \"FunctionAST\",\n";
    out << std::string(indent + 2, ' ') << "\"prototype\": ";
    m_Proto->print(out, indent + 2);
    out << ",\n";
    out << std::string(indent + 2, ' ') << "\"body\": ";
    if(m_Body)
        m_Body->print(out, indent + 2);
    out << "\n" << std::string(indent, ' ') << "}";
}
llvm::Value * FunctionAST::codegen(GenContext& gen)  {
    llvm::Function *TheFunction = gen.module.getFunction(m_Proto->getName());
    if (!TheFunction)
        TheFunction = m_Proto->codegen(gen);
    if(!m_Body) return TheFunction;
    if(m_Proto->getName() == "main") {
        llvm::BasicBlock *MainBB = llvm::BasicBlock::Create(gen.ctx, "entry", TheFunction);
        gen.builder.SetInsertPoint(MainBB);
        gen.symbTable.clear();

        for (auto &variable : m_Vars)
        {
            variable->codegen(gen);
        }

        m_Body->codegen(gen);
        // return 0
        gen.builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.ctx), 0));
        return TheFunction;
    }

    llvm::BasicBlock * BB = llvm::BasicBlock::Create(gen.ctx, m_Proto->getName(), TheFunction);
    gen.builder.SetInsertPoint(BB);

    gen.symbTable.clear();
    // Create return value
    llvm::AllocaInst *AllocaReturnVar = gen.builder.CreateAlloca(llvm::Type::getInt32Ty(gen.ctx), nullptr, m_Proto->getName());
    gen.symbTable[std::string(m_Proto->getName())] = {AllocaReturnVar, false};

    for (auto &Arg : TheFunction->args()) {
        // Create an alloca for this variable
        llvm::AllocaInst *Alloca = gen.builder.CreateAlloca(Arg.getType(), nullptr, Arg.getName());
        // Store the initial value into the alloca
        gen.builder.CreateStore(&Arg, Alloca);
        // Add the variable to the symbol table
        gen.symbTable[std::string(Arg.getName())] = {Alloca, false};
    }

    for(auto &Var : m_Vars)
        Var->codegen(gen);



    m_Body->codegen(gen);

    // If the function has a non-void return type, generate the return instruction
    if (TheFunction->getReturnType()->isVoidTy()) {
        gen.builder.CreateRetVoid();
    } else {
        llvm::Value *RetVal = gen.builder.CreateLoad(llvm::Type::getInt32Ty(gen.ctx), AllocaReturnVar, "return");
        gen.builder.CreateRet(RetVal);
    }

    // Validate the generated code, checking for consistency
    llvm::verifyFunction(*TheFunction);

    return TheFunction;

};

void FunctionExitAST::print(std::ostream &out, int indent ) const {
    out << std::string(indent + 2, ' ') << "\"type\": \">Function Exit<\",\n";
}
llvm::Value * FunctionExitAST::codegen(GenContext& gen) {
    llvm::Function *TheFunction = gen.builder.GetInsertBlock()->getParent();
    llvm::Type *ReturnType = TheFunction->getReturnType();
    std::string functionName = TheFunction->getName().str();
    std::clog << "FUNCTION NAME:::" << functionName << std::endl;
    if (ReturnType->isVoidTy()) {
    gen.builder.CreateRetVoid();
    } else {
    llvm::Value *RetVal = gen.builder.CreateLoad(llvm::Type::getInt32Ty(gen.ctx),
                                                 gen.symbTable[std::string(functionName)].store, functionName);
    gen.builder.CreateRet(RetVal);
    }
    return nullptr;
};

void LoopBreakAST::print(std::ostream &out, int indent) const {
    out << std::string(indent + 2, ' ') << "\"type\": \">Function Exit<\",\n";
}
llvm::Value * LoopBreakAST::codegen(GenContext& gen) {
    if (gen.loopExitBlocks.empty()) {
    std::cerr << "Error: 'break' used outside of loop" << std::endl;
    return nullptr;
    }
    llvm::BasicBlock *ExitBB = gen.loopExitBlocks.top();
    gen.builder.CreateBr(ExitBB);
    return nullptr;
};

IfStmtAST::IfStmtAST(std::unique_ptr<AST> cond, std::unique_ptr<AST> then,
          std::unique_ptr<AST> Else)
        : m_Cond(std::move(cond)), m_Then(std::move(then)), m_Else(std::move(Else)) {}


void IfStmtAST::print(std::ostream &out, int indent) const {
    out << std::string(indent, ' ') << "{\n";
    out << std::string(indent + 2, ' ') << "\"type\": \"IF\",\n";
    m_Cond->print(out, indent + 2);
    m_Then->print(out, indent + 2);

    if(m_Else) m_Else->print(out, indent + 2);
    out << std::string(indent, ' ') << "}";
}

llvm::Value * IfStmtAST::codegen(GenContext & gen) {
    llvm::Value *CondV = m_Cond->codegen(gen);
    if (!CondV)
        return nullptr;

    // Convert condition to a bool by comparing non-equal to 0
    CondV = gen.builder.CreateICmpNE(CondV, llvm::ConstantInt::get(CondV->getType(), 0, true), "ifcond");


    llvm::Function * TheFunction = gen.builder.GetInsertBlock()->getParent();

    // Create blocks for the then, else, and the continuation (merge) block
    llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(gen.ctx, "then", TheFunction);
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(gen.ctx, "ifcont");

    llvm::BasicBlock *ElseBB = nullptr;
    if (m_Else) {
        ElseBB = llvm::BasicBlock::Create(gen.ctx, "else");
        gen.builder.CreateCondBr(CondV, ThenBB, ElseBB);
    } else {
        gen.builder.CreateCondBr(CondV, ThenBB, MergeBB);
    }

    gen.builder.SetInsertPoint(ThenBB);

    m_Then->codegen(gen);

    gen.builder.CreateBr(MergeBB);

    if(m_Else) {
        TheFunction->getBasicBlockList().push_back(ElseBB);
        gen.builder.SetInsertPoint(ElseBB);
        m_Else->codegen(gen);
        gen.builder.CreateBr(MergeBB);
    }

    // Generate code for the merge block
    TheFunction->getBasicBlockList().push_back(MergeBB);
    gen.builder.SetInsertPoint(MergeBB);
    return MergeBB;
}

ForStmtAST::ForStmtAST(const std::string &Var, std::unique_ptr<ExprAST> Start,
           std::unique_ptr<ExprAST> End, std::unique_ptr<NumberExprAST> Step,
           std::unique_ptr<AST> Body)
        : m_Var(Var), m_Start(std::move(Start)), m_End(std::move(End)),
          m_Step(std::move(Step)), m_Body(std::move(Body)) {};

    void ForStmtAST::print(std::ostream &out, int indent ) const  {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"FOR\",\n";
        m_Start->print(out, indent + 2);
        m_End->print(out, indent + 2);
        m_Step->print(out, indent + 2);
        m_Body->print(out, indent + 2);
        out << std::string(indent + 2, ' ') << "}";
    }

    llvm::Value * ForStmtAST::codegen(GenContext & gen)  {
        llvm::Value *StartVal = m_Start->codegen(gen);
        if (!StartVal)
            return nullptr;

        // Make the new basic block for the loop header, inserting after current
// block.
        llvm::AllocaInst * VariableAlloca = gen.symbTable.find(m_Var)->second.store;
        gen.builder.CreateStore(StartVal, VariableAlloca);

        llvm::Function *TheFunction = gen.builder.GetInsertBlock()->getParent();

        llvm::BasicBlock *ConditionBB =
                llvm::BasicBlock::Create(gen.ctx, "condb");
        llvm::BasicBlock *LoopBB =
                llvm::BasicBlock::Create(gen.ctx, "loopb");
        llvm::BasicBlock *ExitBB = llvm::BasicBlock::Create(gen.ctx, "exitb");

        gen.builder.CreateBr(ConditionBB);
        gen.builder.SetInsertPoint(ConditionBB);

//        For I := 0 to 20 do then
//        m_End = <ExprAST>(20)
        llvm::Value *EndCond = m_End->codegen(gen);
        if (!EndCond)
            return nullptr;

        // Convert condition to a bool by comparing non-equal to 0.0.
        TheFunction->getBasicBlockList().push_back(ConditionBB);

        VariableAlloca = gen.symbTable.find(m_Var)->second.store;
        llvm::Value * VariableValue = gen.builder.CreateLoad(llvm::Type::getInt32Ty(gen.ctx), VariableAlloca, "for_assign");
        llvm::Value * condition = gen.builder.CreateICmpSLE(
                VariableValue, EndCond
        );

        gen.builder.CreateCondBr(condition, LoopBB, ExitBB);

        TheFunction->getBasicBlockList().push_back(LoopBB);

        gen.builder.SetInsertPoint(LoopBB);
        // To allow break
        gen.loopExitBlocks.push(ExitBB);
        m_Body->codegen(gen);
        gen.loopExitBlocks.pop();

        llvm::Value *StepVal = nullptr;
        if (m_Step) {
            StepVal = m_Step->codegen(gen);
            if (!StepVal)
                return nullptr;
        }
        VariableAlloca = gen.symbTable.find(m_Var)->second.store;
        VariableValue = gen.builder.CreateLoad(llvm::Type::getInt32Ty(gen.ctx), VariableAlloca, "for_assign");
        llvm::Value * NextVal = gen.builder.CreateAdd(VariableValue, StepVal, "nextvar");
        gen.builder.CreateStore(NextVal, VariableAlloca);

        gen.builder.CreateBr(ConditionBB);


        TheFunction->getBasicBlockList().push_back(ExitBB);
        gen.builder.SetInsertPoint(ExitBB);

        return nullptr;
    };

WhileStmtAST::WhileStmtAST(std::unique_ptr<ExprAST> cond, std::unique_ptr<AST> body)
            : m_Cond(std::move(cond)), m_Body(std::move(body)) {}

    void WhileStmtAST::print(std::ostream &out, int indent ) const {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"WHILE\",\n";
        m_Cond->print(out, indent + 2);
        m_Body->print(out, indent + 2);
        out << std::string(indent, ' ') << "}";
    }

    llvm::Value * WhileStmtAST::codegen(GenContext &gen) {
        llvm::Function *TheFunction = gen.builder.GetInsertBlock()->getParent();

        llvm::BasicBlock *CondBB = llvm::BasicBlock::Create(gen.ctx, "whilecond", TheFunction);
        llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(gen.ctx, "whileloop" );
        llvm::BasicBlock *ExitBB = llvm::BasicBlock::Create(gen.ctx, "whileexit");

        gen.builder.CreateBr(CondBB);
        gen.builder.SetInsertPoint(CondBB);

        llvm::Value *CondV = m_Cond->codegen(gen);
        if (!CondV)
            return nullptr;

        CondV = gen.builder.CreateICmpNE(CondV, llvm::ConstantInt::get(CondV->getType(), 0, true), "whilecond");
        gen.builder.CreateCondBr(CondV, LoopBB, ExitBB);

        TheFunction->getBasicBlockList().push_back(LoopBB);
        gen.builder.SetInsertPoint(LoopBB);

        gen.loopExitBlocks.push(ExitBB);
        m_Body->codegen(gen);
        gen.loopExitBlocks.pop();

        gen.builder.CreateBr(CondBB);

        TheFunction->getBasicBlockList().push_back(ExitBB);
        gen.builder.SetInsertPoint(ExitBB);

        return nullptr;
    }
