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

Value* resolveGetElementPtr(GetElementPtrInst *GI,DataLayout *D,LLVMContext &Context) {

	int offset = 0;
	Value *Offset,*temp;
	int c = 0;

	//errs() << *GI << "\n" ;

	if(ConstantInt *CI = dyn_cast<ConstantInt>(GI->getOperand(GI->getNumOperands()-1))){ // get the last operand of the getelementptr
		c = CI->getZExtValue () ;
	}else{
		//suppose the last index was not a constant then set 'c' to some special value and get the last operand.
		c = -999;
		// Instruction *I = dyn_cast<Instruction>(GI->getOperand(GI->getNumOperands()-1));
		// Offset = I->getOperand(0);
		Offset = GI->getOperand(GI->getNumOperands()-1) ;
	}
	
	Type *type = GI->getSourceElementType(); //get the type of getelementptr
	if(StructType *t = dyn_cast<StructType>(type) ){ //check for struct type
			
		const StructLayout *SL = D->getStructLayout(t);
		offset+= SL->getElementOffset(c);

	}else if(ArrayType *t = dyn_cast<ArrayType>(type) ) {

		if(c==-999){
			temp= llvm::ConstantInt::get(Type::getInt64Ty(Context), D->getTypeAllocSize(t->getElementType()));
			IRBuilder<> builder(GI);
			Offset = builder.CreateBinOp(Instruction::Mul,Offset, temp, "tmp");

		}else
			offset+=c*D->getTypeAllocSize(t->getElementType()) ; //D->getTypeAllocSize(t)/t->getArrayNumElements();
				
	}
	/*else if(UnionType *t = dyn_cast<ArrayType>(type)){
		errs() << "inside union \n" ;
	}
*/

	else{//basic pointer increment or decrements

		if(c==-999){
			temp= llvm::ConstantInt::get(Type::getInt64Ty(Context), D->getTypeAllocSize(type));
			IRBuilder<> builder(GI);
			Offset = builder.CreateBinOp(Instruction::Mul,Offset, temp, "tmp");

		}else
			offset+=c*D->getTypeAllocSize(type);
	}

	if(c!=-999)
		Offset = llvm::ConstantInt::get(Type::getInt32Ty(Context),offset);
	
	return Offset;
}

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

				Module *m = F.getParent();
				Function *val = Intrinsic::getDeclaration(m, Intrinsic::riscv_validate);	// get hash intrinsic declaration

				for (auto &B : F) {
					//for (auto &I : B) {
					for(BasicBlock::iterator i = B.begin(), e = B.end(); i != e; ++i) {
						Instruction *I = dyn_cast<Instruction>(i);

						if (auto *op = dyn_cast<StoreInst>(I)) {
							if(op->getOperand(1)->getType() != Type::getInt128Ty(Ctx))
								continue;
							modified = true;
							TruncInst *tr_lo = new TruncInst(op->getOperand(1), Type::getInt64Ty(Ctx),"fpr_low", op);	// alloca stack cookie
							Value* shamt = llvm::ConstantInt::get(Type::getInt128Ty(Ctx),64);
							BinaryOperator *shifted =  BinaryOperator::Create(Instruction::LShr, op->getOperand(1), shamt , "fpr_hi_big", op);
							TruncInst *tr_hi = new TruncInst(shifted, Type::getInt64Ty(Ctx),"fpr_hi", op);	// alloca stack cookie

							// Set up intrinsic arguments
							std::vector<Value *> args;

							args.push_back(tr_hi);
							args.push_back(tr_lo);
							ArrayRef<Value *> args_ref(args);

							// Create call to intrinsic
							IRBuilder<> Builder(I);
							Builder.SetInsertPoint(I);
							Builder.CreateCall(val, args_ref,"");

							Type *storetype = op->getOperand(0)->getType();//->getPointerElementType();
							Type *storeptrtype = storetype->getPointerTo();

							//errs()<<"\n*******\nStoretype: "<<*storeptrtype<<"\n********\n";

							IntToPtrInst *ptr = new IntToPtrInst(tr_lo,storeptrtype,"ptr",op);

							new StoreInst(op->getOperand(0),ptr,op);

							--i;
							op->dropAllReferences();
						    op->removeFromParent();

						}

						else if (auto *op = dyn_cast<LoadInst>(I)) {
							//errs()<<"\n*******\nOp0: "<<*op->getOperand(0)<<"\n********\n";
							if(op->getOperand(0)->getType() != Type::getInt128Ty(Ctx))
								continue;
							modified = true;
							TruncInst *tr_lo = new TruncInst(op->getOperand(0), Type::getInt64Ty(Ctx),"fpr_low", op);	// alloca stack cookie
							Value* shamt = llvm::ConstantInt::get(Type::getInt128Ty(Ctx),64);
							BinaryOperator *shifted =  BinaryOperator::Create(Instruction::LShr, op->getOperand(0), shamt , "fpr_hi_big", op);
							TruncInst *tr_hi = new TruncInst(shifted, Type::getInt64Ty(Ctx),"fpr_hi", op);	// alloca stack cookie

							// Set up intrinsic arguments
							std::vector<Value *> args;

							args.push_back(tr_hi);
							args.push_back(tr_lo);
							ArrayRef<Value *> args_ref(args);

							// Create call to intrinsic
							IRBuilder<> Builder(I);
							Builder.SetInsertPoint(I);
							Builder.CreateCall(val, args_ref,"");

							Type *loadtype = op->getType();//->getType();//->getPointerElementType();
							Type *loadptrtype = loadtype->getPointerTo();

							//errs()<<"\n*******\nStoretype: "<<*storeptrtype<<"\n********\n";

							IntToPtrInst *ptr = new IntToPtrInst(tr_lo,loadptrtype,"ptr",op);
							//op->mutateType(Type::getIntNPtrTy(Ctx,128));

							op->setOperand(0,ptr);

							//errs()<<"\n*******\nModified load: "<<*op<<"\n********\n";

							//LoadInst *safeload = new LoadInst(loadtype,ptr,"safe_load",op);

							//--i;
							//op->dropAllReferences();
						    //op->removeFromParent();

						}

						else if (auto *op = dyn_cast<GetElementPtrInst>(I)) {
							modified=true;
							Value *offset = resolveGetElementPtr(op,D,Ctx);
							//errs()<<"\n***\nOFFSET: "<<*offset<<"\n*****\n";
							ZExtInst *zext_binop = new ZExtInst(offset, Type::getInt128Ty(Ctx), "zextarrayidx", op);
							BinaryOperator *binop =  BinaryOperator::Create(Instruction::Add, op->getOperand(0), zext_binop , "arrayidx", op);
							//errs()<<"===============================\n"<<F<<"\n===============================\n";
							
							std::stack <User *> users;
							std::stack <int> pos;

							for (auto &U : op->uses()) {
								User *user = U.getUser();
								users.push(user);
								pos.push(U.getOperandNo());
								//errs()<<"\n------\n"<<*op<<"\nused at:\n" << *user << "\n----\n" ; 
							}

							while(users.size())
						    {
						    	User *u = users.top();
						    	users.pop();
						    	int index = pos.top();
						    	pos.pop();
						    	u->setOperand(index, binop);	
						    }

						    --i;
						    op->dropAllReferences();
						    op->removeFromParent();
						}

						else if (auto *op = dyn_cast<AllocaInst>(I)) {
							modified=true;
							if(op->getAllocatedType()->isPointerTy())
							{
								op->setAllocatedType(Type::getInt128Ty(Ctx));
								op->mutateType(Type::getIntNPtrTy(Ctx,128));
								//errs()<<"\nPointer: "<<*(op->getType())<<"\n";
								//errs()<<"\nAllocatedtype: "<<*(op->getAllocatedType())<<"\n";
								
								//made_fpr=true;
							}

							if (op->getName() == "stack_cookie")
							{
								st_hash = dyn_cast<CallInst>(op->getNextNode());
								//errs()<<"\n"<<*(st_hash)<<"\n";
								continue;
							}
							//errs() << "\n***Found array or struct***\n"<<*op<<"\n";
							//errs()<<"===============================\n"<<F<<"\n===============================\n";
							PtrToIntInst *trunc = new PtrToIntInst(op, Type::getInt32Ty(Ctx),"",op->getNextNode());
							//errs()<<"\n***"<<*(trunc->getOperand(0))<<"***\n";
							//trunc->setOperand(0,op);
							//errs()<<"===============================\n"<<F<<"\n===============================\n";

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

							IRBuilder<> Builder(I);
							Builder.SetInsertPoint(trunc->getNextNode());
							//errs()<<"\nNAME: "<<op->getName()<<"\n";
							Value *fpr = Builder.CreateCall(craftFunc, args_ref,op->getName()+"fpr");

							bool flag = false;
							//int count = 0;
							std::stack <User *> users;
							std::stack <int> pos;

							//User *ptrtoint;
							//int ptrtointindex = 0;

							for (auto &U : op->uses()) {
								//count++;
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
				//errs()<<"===============================\n"<<F<<"\n===============================\n";
			}

			return modified;
		}
	};
}

char cookiePass::ID = 0;
static RegisterPass<cookiePass> X("cookie", "Sample program for checking GetElementPtr instructions ");
