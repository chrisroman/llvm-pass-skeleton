#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Attributes.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/AliasAnalysis.h"

#include <unordered_set>
#include <unordered_map>
#include <deque>
using namespace llvm;

namespace {
  enum class NullStatus {
    PossiblyNull,
    DefinitelyNonNull
  };

  // Add null checking function
  struct AddNullCheckFuncPass : public ModulePass {
    static char ID;
    AddNullCheckFuncPass() : ModulePass(ID) {}

    Function *getPrintfFunc(Module *M) {
      Function *printf_func = M->getFunction("printf");
      if (!printf_func) {
        FunctionType *FuncTy9 = FunctionType::get(IntegerType::get(M->getContext(), 32), true);

        printf_func = Function::Create(FuncTy9, GlobalValue::ExternalLinkage, "printf", M);
        //printf_func->setCallingConv(CallingConv::C);
      }

      // Type *return_type = Type::getInt32Ty(M->getContext());
      // ArrayRef<Type*> param_types = {static_cast<Type*>(Type::getInt8PtrTy(M->getContext()))};
      // FunctionType *FT = FunctionType::get(return_type, param_types, true);
      // //Function* func = Function::Create(FT, Function::ExternalLinkage, "exit", M);
      // Function *func = cast<Function>(M->getOrInsertFunction("printf", FT).getCallee());
      // if (func == nullptr) {
      //   errs() << "couldn't find printf\n";
      // }

      if (printf_func == nullptr) {
        errs() << "ERROR: Couldn't find printf function";
        throw std::runtime_error("");
      }
      return printf_func;
    }

    Function *getEscapeFunc(Module *M) {
      Function *escape_func = M->getFunction("_Z6escapePv");
      if (!escape_func) {
        FunctionType *FuncTy9 = FunctionType::get(Type::getVoidTy(M->getContext()),
                                                  {static_cast<Type*>(Type::getInt8PtrTy(M->getContext()))},
                                                  false);

        escape_func = Function::Create(FuncTy9, GlobalValue::ExternalLinkage, "_Z6escapePv", M);
        //escape_func->setCallingConv(CallingConv::C);
      }

      if (escape_func == nullptr) {
        errs() << "ERROR: Couldn't find escape function";
        throw std::runtime_error("");
      }
      errs() << "escape_func = " << *escape_func << "\n";
      return escape_func;
    }

    // Gets a Function* for `exit(...)`
    Function *getExitFunc(Module *M) {
      Type *return_type = Type::getVoidTy(M->getContext());
      ArrayRef<Type*> param_types = {static_cast<Type*>(Type::getInt32Ty(M->getContext()))};
      FunctionType *FT = FunctionType::get(return_type, param_types, false);
      //Function* func = Function::Create(FT, Function::ExternalLinkage, "exit", M);
      Function* func = cast<Function>(M->getOrInsertFunction("exit", FT).getCallee());
      //errs() << "func = " << func << "\n";
      return func;
    }

    // Creates a global variable called "nullp" and calls escape(&nullp) at the beginning of each function
    // The reason for this is that in our later null pointer analysis pass,
    // we will effectively replace all "nullptr" values with this "nullp"
    // global value. The call to "escape" makes sure that the optimizer
    // does not copy propagate the value of "nullptr" through the program.
    // Finally, we can use LLVM's alias analysis to see if any pointer aliases
    // "nullp". If so, then it can be null
    Value *createNullP(Module *M) {
      // Create the global
      GlobalVariable* nullp = new GlobalVariable(*M,
                                                 Type::getInt8PtrTy(M->getContext()),
                                                 false,
                                                 GlobalValue::CommonLinkage,
                                                 0,
                                                 "nullp");
      nullp->setAlignment(8);
      nullp->setInitializer(ConstantPointerNull::get(Type::getInt8PtrTy(M->getContext())));

      // Add an "escape(&nullp)" to the beginning of every function
      Function *escape_func = getEscapeFunc(M);
      for (Function& F : *M) {
        if (&F == escape_func) {
          continue;
        }
        IRBuilder<> builder(&*F.begin()->begin());
        // TODO: do a bitcast operation on nullp to an i8* instead of an i8**
        Value *cast = builder.CreateBitCast(nullp, Type::getInt8PtrTy(M->getContext()));
        builder.CreateCall(escape_func->getFunctionType(),
                           escape_func,
                           {cast});
      }

      return nullp;
    }

    void createNullCheckFunc(Module &M) {
      FunctionType *FT = FunctionType::get(Type::getVoidTy(M.getContext()), {Type::getInt8PtrTy(M.getContext())}, false);
      Function *nullcheck_func = Function::Create(FT, Function::ExternalLinkage, "nullcheck", M);

      BasicBlock *then = BasicBlock::Create(M.getContext(), "", nullcheck_func);
      {
        IRBuilder<> builder(then);

        // Add printf call
        Function *printf_func = getPrintfFunc(&M);
        Value *str = builder.CreateGlobalStringPtr("Found a null pointer. Exiting...\n");
        CallInst *call = builder.CreateCall(printf_func, {str}, "");

        // Add exit call
        Function *exit_func = getExitFunc(&M);
        Value *one = ConstantInt::getSigned(Type::getInt32Ty(M.getContext()), 1);
        builder.CreateCall(exit_func->getFunctionType(),
                           exit_func,
                           {one});
        builder.CreateUnreachable();
      }

      BasicBlock *end = BasicBlock::Create(M.getContext(), "", nullcheck_func);
      {
        IRBuilder<> builder(end);
        builder.CreateRetVoid();
      }

      BasicBlock *block = BasicBlock::Create(M.getContext(), "", nullcheck_func, then);
      {
        IRBuilder<> builder(block);
        Value *p = &*nullcheck_func->arg_begin();
        Value *is_null = builder.CreateICmpEQ(p, ConstantPointerNull::get(static_cast<PointerType*>(p->getType())));

        // If argument is nullptr, jump to then block; otherwise, jump to end block
        builder.CreateCondBr(is_null, then, end);
      }
    }

    virtual bool runOnModule(Module &M) {
      //errs() << "Found module " << M.getName() << "\n";

      // for (auto& F : M) {
      //   if (F.hasInternalLinkage()) {
      //     errs() << "Found internal function " << F.getName() << "\n";
      //   }
      // }

      // Add escape(&nullp) to every function
      // Function *escape_func = getEscapeFunc(&M);
      // Value *nullp = M.getGlobalVariable("nullp", true);
      // for (auto& F : M) {
      //   IRBuilder<> builder(&*F.begin()->begin());
      //   builder.CreateCall(escape_func->getFunctionType(),
      //                      escape_func,
      //                      {nullp});
      // }

      //createNullP(&M);
      createNullCheckFunc(M);

      //errs() << "module = " << M;

      return true;
    }
  };

  struct AddNullCheckPass : public FunctionPass {
    using LatticeElt = std::unordered_map<void*, NullStatus>;

    static char ID;
    using placeholderPtr = std::unique_ptr<int>; // acts as a unique identifier
    // Maps pointer values to the value contained inside the pointer
    std::unordered_map<Value*, placeholderPtr> deref_map;
    AddNullCheckPass() : FunctionPass(ID) {}

    void addNullCheck(Instruction *I, Value *p) {
      Module *M = I->getParent()->getParent()->getParent();
      IRBuilder<> builder(I);
      Function *nullcheck = M->getFunction("nullcheck");
      Value *arg = builder.CreatePointerCast(p, Type::getInt8PtrTy(M->getContext()));
      builder.CreateCall(nullcheck->getFunctionType(),
                         nullcheck,
                         {arg});
    }

    std::unordered_set<Value*> getPtrs(Function &F) {
      std::unordered_set<Value*> res;
      for (auto &B : F) {
        for (auto &I : B) {
          if (auto* load = dyn_cast<LoadInst>(&I)) {
            res.insert(load->getPointerOperand());
          } else if (auto* store = dyn_cast<StoreInst>(&I)) {
            res.insert(store->getPointerOperand());
            if(store->getValueOperand()->getType()->isPointerTy()) {
              res.insert(store->getValueOperand());
            }
          }
          // else {
          //   res.insert(&I);
          // }
        }
      }
      return res;
    }

    void printSet(const std::unordered_set<Value*>& ptrs) {
      for (Value *p : ptrs) {
        errs() << "(" << p << ") " << *p << "\n";
      }
    }

    std::unordered_set<Instruction*> getPredecessors(Instruction *I) {
      std::unordered_set<Instruction*> res;
      Instruction *prev = I->getPrevNonDebugInstruction();
      if (prev != nullptr) {
        res.insert(prev);
      } else {
        for (BasicBlock &blk : *I->getParent()->getParent()) {
          Instruction *term = blk.getTerminator();
          for (unsigned int i = 0; i < term->getNumSuccessors(); ++i) {
            BasicBlock *blk_succ = term->getSuccessor(i);
            res.insert(&*blk_succ->begin());
          }
        }
      }
      return res;
    }

    LatticeElt meet(LatticeElt a, LatticeElt b) {
      //using LatticeElt = std::unordered_map<Value*, NullStatus>;
      LatticeElt res = a;
      for (auto& pair : a) {
        auto& val = pair.first;
        auto& status_a = pair.second;
        NullStatus status_b = b[val];
        if (status_a == NullStatus::PossiblyNull || status_b == NullStatus::PossiblyNull) {
          res[val] = NullStatus::PossiblyNull;
        } else {
          res[val] = NullStatus::DefinitelyNonNull;
        }
      }
      return res;
    }

    LatticeElt meet(Function& F, std::unordered_map<Instruction*, LatticeElt>& out, const std::unordered_set<Instruction*>& preds) {
      if (preds.empty()) {
        return Top(F);
      }
      LatticeElt res = out[*preds.begin()];
      for (auto it = next(preds.begin()); it != preds.end(); ++it) {
        res = meet(res, out[*it]);
      }
      return res;
    }

    // Represents the value store in the pointer p
    void *deref(Value *p) {
      return deref_map[p].get();
    }

    LatticeElt transferFunc(AliasAnalysis *AA, Instruction *I, LatticeElt elt) {
      LatticeElt res = elt;
      if (auto* store = dyn_cast<StoreInst>(I)) {
        Value *v = store->getValueOperand();
        Value *p = store->getPointerOperand();
        if (v == ConstantPointerNull::get(static_cast<PointerType*>(v->getType()))) {
          // Instruction is p == nullptr.
          // TODO: Now everything that may alias p should be PossiblyNull
          for (Value *other_p : getPtrs(*I->getParent()->getParent())) {
            if (AA->alias(p, other_p) != NoAlias) {
              res[deref(other_p)] = NullStatus::PossiblyNull;
            }
          }
          res[deref(p)] = NullStatus::PossiblyNull;
        } else {
          res[deref(p)] = res[v];
        }
      } else if (auto* load = dyn_cast<LoadInst>(I)) {
        res[load] = res[deref(load->getPointerOperand())];
      } else if (auto* alloca = dyn_cast<AllocaInst>(I)) {
        res[alloca] = NullStatus::DefinitelyNonNull;
      } else if (auto* call = dyn_cast<CallInst>(I)) {
        // TODO: must account for calls to "new"
        if (call->getCalledFunction() && call->getCalledFunction()->getName() == "malloc") {
          res[call] = NullStatus::DefinitelyNonNull;
        } else {
          res[call] = NullStatus::PossiblyNull;
        }
      } else if (auto* cast = dyn_cast<CastInst>(I)) {
        res[cast] = res[cast->getOperand(0)];
        errs() << "***********************\n";
        errs() << "Found operation cast : " << *cast << "\n";
        for (auto& operand : cast->operands()) {
          errs() << *operand << "\n";
        }
        errs() << "***********************\n";
      }
      // TODO: figure out how malloc sets a pointer
      // TODO: figure out how referencing with & sets a pointer

      return res;
    }

    void init_deref_map(Function *F) {
      for (auto& B : *F) {
        for (auto& I : B) {
          errs() << "Instruction " << I << "    as a value: " << &I << "\n";
          //deref_map.insert({&I, make_unique<int>(-1)});
          deref_map[&I] = make_unique<int>(-1);
        }
      }
    }

    LatticeElt Top(Function& F) {
      LatticeElt top;
      for (auto &B : F) {
        for (auto &I : B) {
          top[&I] = NullStatus::PossiblyNull;
          //top[deref(&I)] = NullStatus::PossiblyNull;
        }
      }
      return top;
    }

    // Dataflow analysis
    std::unordered_map<Instruction*, LatticeElt> nullptr_analysis(Function& F, AliasAnalysis *AA) {
      // TODO: Set deref_map
      init_deref_map(&F);

      std::unordered_set<Instruction*> worklist_set;
      std::deque<Instruction*> worklist;

      // TODO: Initialize out to top for all instructions
      std::unordered_map<Instruction*, LatticeElt> in;
      std::unordered_map<Instruction*, LatticeElt> out;
      LatticeElt top = Top(F);

      for (auto &B : F) {
        for (auto &I : B) {
          worklist.push_back(&I);
          worklist_set.insert(&I);
          in[&I] = top;
          out[&I] = top;
        }
      }

      while (!worklist.empty()) {
        Instruction *I = worklist.front();
        worklist.pop_front();
        worklist_set.erase(I);

        auto preds = getPredecessors(I);
        if (preds.empty()) {
          errs() << "Skipping instruction " << *I << "\n";
        }

        in[I] = meet(F, out, preds);
        auto old_out = out[I];
        out[I] = transferFunc(AA, I, in[I]);
        if (old_out != out[I]) {
          // Add all predecessors to worklist (modulo those already in the worklist)
          for (Instruction *IPred : preds) {
            errs() << "IPred = " << *IPred << "\n";
            if (worklist_set.find(IPred) == worklist_set.end()) {
              worklist.push_back(IPred);
              worklist_set.insert(IPred);
            }
          }
        }
      }

      return in;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<AAResultsWrapperPass>();
    }

    std::unordered_set<Value*> findAllBitcasts(Function &F) {
      std::unordered_set<Value*> res;
      for (BasicBlock& B : F) {
        for (Instruction& I : B) {
          errs() << "Looking at instruction " << I << "\n";
          if (isa<BitCastInst>(I)) {
            errs() << "+++++++++\n";
            errs() << "Found bitcast " << I << "\n";
            errs() << "Operands:\n";
            for (auto& operand : I.operands()) {
              errs() << "operand = " << *operand << "\n";
            }
            errs() << "+++++++++\n";
            res.insert(&I);
          } else {

            for (auto& operand : I.operands()) {
              Value *val = operand.get();
              errs() << "Looking at operand " << *val << "\n";
              if (auto *Iop = dyn_cast<BitCastOperator>(val)) {
                errs() << "+++++++++\n";
                errs() << "Found bitcast operator " << *Iop << "\n";
                errs() << "Operands:\n";
                for (auto& operand : Iop->operands()) {
                  errs() << "operand = " << *operand << "\n";
                }
                errs() << "+++++++++\n";
                res.insert(&I);
              }
            }

          }
        }
      }
      return res;
    }

    virtual bool runOnFunction(Function &F) override {
      // TODO: Skipping main for now
      if (F.getName() == "main") {
        return false;
      }
      errs() << "Function body before: " << F << "\n";
      printSet(getPtrs(F));
      //errs() << "-------------\n";

      // Value *null_value = ConstantPointerNull::get(Type::getInt8PtrTy(F.getContext()));
      // errs() << "Uses:\n";
      // for (auto& u : null_value->uses()) {
      //   errs() << *u << "\n";
      // }

      Module *M = F.getParent();
      AliasAnalysis *AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
      std::unordered_map<Instruction*, LatticeElt> NPA = nullptr_analysis(F, AA);
      //Value *nullp = M->getGlobalVariable("nullp", true);
      Value *nullp = nullptr;
      //errs() << "nullp = " << *nullp << "\n";
      // std::unordered_set<Value*> bitcasts = findAllBitcasts(F);
      bool changed = false;
      for (auto B_it = F.begin(); B_it != F.end(); ++B_it) {
        BasicBlock *B = &*B_it;
        for (auto I_it = B->begin(); I_it != B->end(); ++I_it) {
          Instruction *I = &*I_it;

          Value *p;
          if (auto* load = dyn_cast<LoadInst>(I)) {
            p = load->getPointerOperand();
          } else if (auto* store = dyn_cast<StoreInst>(I)) {
            p = store->getPointerOperand();
          } else {
            continue;
          }

            // for (Value *cast : bitcasts) {
            //   errs() << "AA->alias(" << *p << ", " << *cast << ") = " << AA->alias(p, cast) << "\n";
            // }
          // if (nullp == nullptr) {
          //   addNullCheck(I, p);
          // } else if (AA->alias(p, nullp) != NoAlias) {
          //   errs() << "AA->alias(" << *p << ", " << *nullp << ") = " << AA->alias(p, nullp) << "\n";
          //   addNullCheck(I, p);
          // }
          if (NPA[I][p] == NullStatus::PossiblyNull) {
            errs() << "NPA[I][p] = PossiblyNull\n";
            addNullCheck(I, p);
          } else {
            errs() << "NPA[I][p] = DefinitelyNonNull\n";
          }
        }
      }
      errs() << "-------------\n";

      // errs() << "Function body after: " << F << "\n";
      // printSet(getPtrs(F));
      // errs() << "--------------------------------------------------\n";

      return changed;
    }
  };

  struct ReplaceNullPass : public FunctionPass {
    static char ID;
    ReplaceNullPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) override {
      if (F.getName() == "nullcheck") {
        return false;
      }
      Module *M = F.getParent();
      Value *nullp = M->getGlobalVariable("nullp", true);
      if (nullp == nullptr) {
        errs() << "-------------------------------\n";
        errs() << "ERROR: Please #include 'track_nullptr.h' and add 'escape(&nullp)' in the file you are compiling.\n";
        errs() << "-------------------------------\n";
        throw std::runtime_error("");
      }
      bool changed = false;
      for (auto B_it = F.begin(); B_it != F.end(); ++B_it) {
        BasicBlock *B = &*B_it;
        for (auto I_it = B->begin(); I_it != B->end(); ++I_it) {
          Instruction *I = &*I_it;

          // errs() << "Gettings operands of " << *I << "\n";
          for (auto& operand : I->operands()) {
            // errs() << "operand = " << *operand << "\n";
            Value *op_null = ConstantPointerNull::get(static_cast<PointerType*>(operand.get()->getType()));
            if (operand.get() == op_null) {
              // TODO: remove check for nullp. when we add it manually in llvm, it iwll always exist
              // errs() << "Replacing null with " << *nullp << " in instr " << *I << "\n";
              // TODO: Replace with nullp (with bitcast)
              IRBuilder<> builder(I);
              Value *cast = builder.CreateBitCast(nullp, static_cast<PointerType*>(operand.get()->getType()));
              operand.set(cast);
            }
          }
        }
      }
      return changed;
    }
  };

}


char ReplaceNullPass::ID = 0;
static RegisterPass<ReplaceNullPass> R1("replace-null", "replace uses of nullptr with global nullp");

char AddNullCheckFuncPass::ID = 0;
static RegisterPass<AddNullCheckFuncPass> R2("add-nullcheck-func", "add a function that does null checking");

char AddNullCheckPass::ID = 0;
static RegisterPass<AddNullCheckPass> R3("add-nullcheck", "a useless pass");

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
// static void registerSkeletonPass(const PassManagerBuilder &,
//                                  legacy::PassManagerBase &PM) {
//   PM.add(new MPass());
//   PM.add(new SkeletonPass());
// }

// static RegisterStandardPasses
//   RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
//                  registerSkeletonPass);

// static RegisterStandardPasses
// RegisterMyPass(PassManagerBuilder::EP_ModuleOptimizerEarly, registerSkeletonPass);

// static RegisterStandardPasses
// RegisterMyPass0(PassManagerBuilder::EP_EnabledOnOptLevel0, registerSkeletonPass);


// static RegisterStandardPasses Y(
//                                 PassManagerBuilder::EP_EnabledOnOptLevel0,
//                                 [](const PassManagerBuilder &Builder,
//                                    legacy::PassManagerBase &PM) { PM.add(new SkeletonPass()); });
