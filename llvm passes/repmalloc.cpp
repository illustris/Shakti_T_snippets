#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/InstrTypes.h"

using namespace llvm;

namespace {
	struct repfunPass : public FunctionPass {
		static char ID;
		repfunPass() : FunctionPass(ID) {}

		virtual bool runOnFunction(Function &F) {
			bool modified=false;
			// Get the safemalloc function to call from the new library.
			LLVMContext &Ctx = F.getContext();
			// Malloc arg is just one unsigned long
			std::vector<Type*> mallocParamTypes = {Type::getInt64Ty(Ctx)};
			// Free arg is i8*
			std::vector<Type*> freeParamTypes = {Type::getInt8PtrTy(Ctx)};
			// Return type of malloc is i8*
			Type *mallocRetType = Type::getInt8PtrTy(Ctx);
			// Return type of free is void
			Type *freeRetType = Type::getVoidTy(Ctx);
			// Put everything together to make function type i8*(i64)
			FunctionType *mallocFuncType = FunctionType::get(mallocRetType, mallocParamTypes, false);
			// Put everything together to make function type void*(i8*)
			FunctionType *freeFuncType = FunctionType::get(freeRetType, freeParamTypes, false);
			// Get function pointer for malloc
			Value *mallocFunc = F.getParent()->getOrInsertFunction("malloc", mallocFuncType);
			// Get function pointer for free
			Value *freeFunc = F.getParent()->getOrInsertFunction("free", freeFuncType);
			// Make safemalloc declaration, get pointer to it
			Value *safemallocFunc = F.getParent()->getOrInsertFunction("safemalloc", mallocFuncType);
			// Make safefree declaration, get pointer to it
			Value *safefreeFunc = F.getParent()->getOrInsertFunction("safefree", freeFuncType);

			// Iterate over BBs in F
			for (auto &B : F) {
				// Iterate over Instrs in BB
				for (auto &I : B) {
					// If instruction is of type call
					if (auto *op = dyn_cast<CallInst>(&I)) {
						// If call invokes malloc
						if((op->getCalledValue()) == (mallocFunc)) {
							errs()<<"\n----------------\nReplacing:\n";
								op->print(llvm::errs(), NULL);
							errs()<<"\nwith:\n";
							// Replace malloc with safemalloc in call
							op->setCalledFunction (safemallocFunc);
							op->print(llvm::errs(), NULL);
							errs()<<"\n----------------\n";
							// Set flag if modified
							modified=true;
						}
						else if((op->getCalledValue()) == (freeFunc)) {
							errs()<<"\n----------------\nReplacing:\n";
							op->print(llvm::errs(), NULL);
							errs()<<"\nwith:\n";
							// Replace free with safefree in call
							op->setCalledFunction (safefreeFunc);
							op->print(llvm::errs(), NULL);
							errs()<<"\n----------------\n";
							// Set flag if modified
							modified=true;
						}
					}
				}
			}
			return (modified?true:false);
		}
	};
}

char repfunPass::ID = 0;
static RegisterPass<repfunPass> X("repmalloc", "Sample program for checking GetElementPtr instructions ");
