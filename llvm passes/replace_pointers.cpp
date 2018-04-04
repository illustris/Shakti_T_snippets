#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include <stack>

using namespace llvm;

namespace {
	struct cookiePass : public ModulePass {
		static char ID;
		cookiePass() : ModulePass(ID) {}

		virtual bool runOnModule(Module &M) {
			bool modified=false;
			for (auto &F : M) {
				DataLayout *D = new DataLayout(&M);

				LLVMContext &Ctx = F.getContext();
				std::vector<Type*> craftParamTypes = {Type::getInt32Ty(Ctx),Type::getInt32Ty(Ctx),Type::getInt32Ty(Ctx),Type::getInt32Ty(Ctx)};
				Type *craftRetType = Type::getInt128Ty(Ctx);
				FunctionType *craftFuncType = FunctionType::get(craftRetType, craftParamTypes, false);
				Value *craftFunc = F.getParent()->getOrInsertFunction("craft", craftFuncType);
				CallInst *st_hash;
		
				for (auto &B : F) {
					for (auto &I : B) {
						if (auto *op = dyn_cast<AllocaInst>(&I)) {
							if (op->getName() == "stack_cookie")
							{
								st_hash = dyn_cast<CallInst>(op->getNextNode());
								//errs()<<"\n"<<*(st_hash)<<"\n";
								continue;
							}
							//errs() << "\n***Found array or struct***\n"<<*op<<"\n";
							
							PtrToIntInst *trunc = new PtrToIntInst(op, Type::getInt32Ty(Ctx),"",op->getNextNode());

							std::vector<Value *> args;
							args.push_back(trunc);
							args.push_back(trunc);
							//errs()<<"\nsize = "<<D->getTypeAllocSize(op->getAllocatedType())<<"\n";
							args.push_back(
								llvm::ConstantInt::get(
									Type::getInt32Ty(Ctx),
									(D->getTypeAllocSize(op->getAllocatedType()))
								)
							);
							args.push_back(st_hash);
							ArrayRef<Value *> args_ref(args);

							IRBuilder<> Builder(&I);
							Builder.SetInsertPoint(trunc->getNextNode());
							//errs()<<"\nNAME: "<<op->getName()<<"\n";
							Value *fpr = Builder.CreateCall(craftFunc, args_ref,op->getName()+"fpr");

							bool flag = false;
							int count = 0;
							std::stack <User *> users;
							std::stack <int> pos;
							for (auto &U : op->uses()) {
								count++;
								if(!flag) {
									flag = true;
									continue;
								}

								User *user = U.getUser();
								users.push(user);
								pos.push(U.getOperandNo());
								//errs()<<"\n------\nused at:\n" << *U << "\n----\n" ; 
								//U->replaceUsesOfWith(op,fpr);
								//user->setOperand(U.getOperandNo(), fpr); //binop - cannot use binop dirctly as it is a i32 value and not a pointer
						    }
						    while(users.size())
						    {
						    	User *u = users.top();
						    	users.pop();
						    	int index = pos.top();
						    	pos.pop();
						    	u->setOperand(index, fpr);	
						    }
						    //errs() << "\n================\nUsecount = "<<count<<"\n========\n";
						}
					}
				}
				errs()<<"===============================\n"<<F<<"\n===============================\n";
			}

			return modified;
		}
	};
}

char cookiePass::ID = 0;
static RegisterPass<cookiePass> X("cookie", "Sample program for checking GetElementPtr instructions ");
