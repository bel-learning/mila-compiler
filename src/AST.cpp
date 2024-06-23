//
// Created by Bilguudei Baljinnyam on 14.06.2024.
//

#include "AST.hpp"


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
