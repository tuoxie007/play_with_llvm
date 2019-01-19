#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Bitcode/BitcodeWriter.h>


using namespace llvm;

static Module *makeLLVMModule(LLVMContext &ctx) {
    Module *mod = new Module("sum.c", ctx);
    mod->setDataLayout("e-m:o-i64:64-f80:128-n8:16:32:64-S128");
    mod->setTargetTriple("x86_64-apple-macosx10.14.0");

    SmallVector<Type*, 2> FuncTyArgs;
    FuncTyArgs.push_back(IntegerType::get(mod->getContext(),
     32));
    FuncTyArgs.push_back(IntegerType::get(mod->getContext(),
     32));
    FunctionType *FuncTy = FunctionType::get(
     IntegerType::get(mod->getContext(), 32), // return type
     FuncTyArgs, // arguments
     false); // is variable arguments

    Function *funcSum = Function::Create(
     FuncTy,
     GlobalValue::ExternalLinkage,
     "sum",
     mod);
    funcSum->setCallingConv(CallingConv::C);

    Function::arg_iterator args = funcSum->arg_begin();
    Value *int32_a = args++;
    int32_a->setName("a");
    Value *int32_b = args++;
    int32_b->setName("b");

    BasicBlock *bb = BasicBlock::Create(mod->getContext(), "entry", funcSum, 0);

    const DataLayout &DL = mod->getDataLayout();
    unsigned AddrSpace = DL.getAllocaAddrSpace();
    AllocaInst *ptrA = new AllocaInst(IntegerType::get(mod->getContext(), 32), AddrSpace, "a.addr", bb);
    ptrA->setAlignment(4);
    AllocaInst *ptrB = new AllocaInst(IntegerType::get(mod->getContext(), 32), AddrSpace, "b.addr", bb);
    ptrB->setAlignment(4);

    StoreInst *st0 = new StoreInst(int32_a, ptrA, false, bb);
    st0->setAlignment(4);
    StoreInst *st1 = new StoreInst(int32_b, ptrB, false, bb);
    st1->setAlignment(4);

    LoadInst *ld0 = new LoadInst(ptrA, "", false, bb);
    ld0->setAlignment(4);
    LoadInst *ld1 = new LoadInst(ptrB, "", false, bb);
    ld1->setAlignment(4);

    BinaryOperator *addRes = BinaryOperator::Create(Instruction::Add, ld0, ld1, "add", bb);

    ReturnInst::Create(mod->getContext(), addRes, bb);

    return mod;
}


int main() {
    LLVMContext ctx;
    Module *Mod = makeLLVMModule(ctx);
    verifyModule(*Mod);

    std::error_code EC;
    raw_fd_ostream OS("sum.bc", EC, sys::fs::F_None);
    if (EC)
        return -1;

    WriteBitcodeToFile(*Mod, OS);
    return 0;
}

