#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "HI_print.h"
#include "HI_IR2SourceCode.h"
#include <stdio.h>
#include <string>
#include <ios>
#include <stdlib.h>
#include "HI_StringProcess.h"

using namespace llvm;
 
bool HI_IR2SourceCode::runOnModule(Module &M) // The runOnModule declaration will overide the virtual one in ModulePass, which will be executed for each Module.
{
    if (DEBUG) *IR2Src_Log << M;
    if (DEBUG) *IR2Src_Log << "\n\n============================================================\n\n\n";
    for (auto &F : M)
    {
        if (F.getName().find("llvm.")!=std::string::npos)
            continue;
        if (DEBUG) *IR2Src_Log << "\n\n============================================================\n";
        if (DEBUG) *IR2Src_Log << "============================================================\n";
        if (DEBUG) *IR2Src_Log << "============================================================\n";

        if (DEBUG) *IR2Src_Log << "processing IR code of Function : [" << F.getName() << "]\n";

        if (DEBUG) *IR2Src_Log << "============================================================\n";
        if (DEBUG) *IR2Src_Log << "============================================================\n";
        if (DEBUG) *IR2Src_Log << "============================================================\n";

        auto &LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
        auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(F).getSE();

        traceInstructionSourceCode(F);
        if (DEBUG) *IR2Src_Log << "\n\n============================================================\n\n\n";

        traceBasicBlockSourceCode(LI, F);
        if (DEBUG) *IR2Src_Log << "\n\n============================================================\n\n\n";

        traceLoopSourceCode(LI, SE, F);   
        if (DEBUG) *IR2Src_Log << "\n\n============================================================\n\n\n";

        traceFunctionSourceCode(F);
        if (DEBUG) *IR2Src_Log << "\n\n============================================================\n\n\n";
    }

    if (DEBUG) *IR2Src_Log << "\n\n============================================================\n\n\n";
    if (DEBUG) *IR2Src_Log << "mappingLoopIR2LoopLabel\n\n============================================================\n\n\n";
    mappingLoopIR2LoopLabel(M);
    if (DEBUG) *IR2Src_Log << "\n\n============================================================\n\n\n";

    for (auto funcName2beginline_pair : IRFunc2BeginLine)
    {
        if (DEBUG) *IR2Src_Log << "printing function: [" << funcName2beginline_pair.first << "] 's possible beginlines:\n";
        for (auto line : funcName2beginline_pair.second)
        {
            if (DEBUG) *IR2Src_Log << "      line--->" << line << "\n";
        }
        if (DEBUG) *IR2Src_Log << "\n";
    }
    IR2Src_Log->flush();
    return false;
}



char HI_IR2SourceCode::ID = 0;  // the ID for pass should be initialized but the value does not matter, since LLVM uses the address of this variable as label instead of its value.

void HI_IR2SourceCode::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.setPreservesCFG();
}

void HI_IR2SourceCode::traceInstructionSourceCode(Function &F)
{
    for (auto &B : F)
    {
        for(auto &I : B) 
        {
            if (DEBUG) *IR2Src_Log << "Instruction: " << I;
            SmallVector<std::pair<unsigned, MDNode *>, 4> I_MDs;
            I.getAllMetadata(I_MDs);
            for (auto &MD : I_MDs) 
            {
                if (MDNode *N = MD.second) 
                {
                    if (DEBUG) *IR2Src_Log << " ==>  Details: " << *N;
                    if (auto DILoc = dyn_cast<DILocation>(N) )
                    {
                        while (DILoc->getInlinedAt() != nullptr)
                        {
                            DILoc = dyn_cast<DILocation>(DILoc->getInlinedAt());
                        }                        
                        if (DEBUG) *IR2Src_Log << " ==>  SourceRange: " << DILoc->getDirectory() << "/" << DILoc->getFilename() << ":" <<DILoc->getLine();
                    }
                }
            }
            if (DEBUG) *IR2Src_Log << "\n";
        } 
    }
}


void HI_IR2SourceCode::traceBasicBlockSourceCode(LoopInfo &LI, Function &F)
{
    for (auto &B : F)
    {
        if (DEBUG) *IR2Src_Log << "BasicBlock: " << B.getName() << ": SourceRange = ";
        std::string path = "";
        int begin_line = 1000000;
        int end_line = 1;
        for(auto &I : B) 
        {
            SmallVector<std::pair<unsigned, MDNode *>, 4> I_MDs;
            I.getAllMetadata(I_MDs);
            for (auto &MD : I_MDs) 
            {
                if (MDNode *N = MD.second) 
                {
                    if (auto DILoc = dyn_cast<DILocation>(N) )
                    {
                        while (DILoc->getInlinedAt() != nullptr)
                        {
                            DILoc = dyn_cast<DILocation>(DILoc->getInlinedAt());
                        }   
                        if (path == "")
                        {
                            path = DILoc->getDirectory().str() + "/" + DILoc->getFilename().str();
                        }

                        if (auto callI = dyn_cast<CallInst>(&I))
                        {
                            if (callI->getCalledFunction()->getName().str().find("llvm.") != std::string::npos)
                            {
                                break;
                            }
                        }

                        if (DILoc->getLine() > end_line)
                            end_line = DILoc->getLine();

                        if (DILoc->getLine() < begin_line && DILoc->getLine()>0)
                            begin_line = DILoc->getLine();
                        
                    }
                }
            }
        } 
        Block2Line[&B] = std::pair<int, int> (begin_line, end_line);
        Block2Path[&B] = path;
        if (DEBUG) *IR2Src_Log << path << ":" << begin_line << "--" << end_line << "\n";
        if (DEBUG) *IR2Src_Log << "BasicBlock: " << B.getName() << ":\n";
        for(auto &I : B) 
        {
            if (DEBUG) *IR2Src_Log << "  " << I << ":  line:";
            SmallVector<std::pair<unsigned, MDNode *>, 4> I_MDs;
            I.getAllMetadata(I_MDs);
            for (auto &MD : I_MDs) 
            {
                if (MDNode *N = MD.second) 
                {
                    if (auto DILoc = dyn_cast<DILocation>(N) )
                    {
                        if (DEBUG) *IR2Src_Log << DILoc->getLine();
                    }                    
                }
            }
            if (DEBUG) *IR2Src_Log << "\n";
        } 
        if (DEBUG) *IR2Src_Log << "\n\n";
    }   
}

void HI_IR2SourceCode::traceLoopSourceCode(LoopInfo &LI, ScalarEvolution &SE, Function &F)
{
    for (auto cur_Loop : LI.getLoopsInPreorder())
    {
        if (DEBUG) *IR2Src_Log << "Loop: " << cur_Loop->getName() << " includes basic blocks:\n   ";
        std::string path = "";
        int begin_line = 1000000;
        int end_line = 1;
        for (auto B : cur_Loop->getBlocks())
        {
            if (DEBUG) *IR2Src_Log << B->getName() << ", ";
            if (path == "")
            {
                path = Block2Path[B];
            }

            if (Block2Line[B].second > end_line)
                end_line = Block2Line[B].second;

            if (Block2Line[B].first < begin_line)
                begin_line = Block2Line[B].first;
        }
        if (DEBUG) *IR2Src_Log << "\n";
        if (DEBUG) *IR2Src_Log << "    SourceRang: " << path << ":" << begin_line << "--" << end_line << "\n\n";
        Loop2Line[cur_Loop->getHeader()] = std::pair<int, int>(begin_line, end_line);
        Loop2Path[cur_Loop->getHeader()] = path;
        IRFunc2BeginLine[demangleFunctionName(F.getName())].push_back(begin_line);
        std::string tmp_loop_name = cur_Loop->getHeader()->getParent()->getName();
        tmp_loop_name += "-";
        tmp_loop_name += cur_Loop->getHeader()->getName();
        
        IRLoop2OriginTripCount[tmp_loop_name] = SE.getSmallConstantMaxTripCount(cur_Loop);
    }

}


void HI_IR2SourceCode::traceFunctionSourceCode(Function &F)
{
    SmallVector<std::pair<unsigned, MDNode *>, 4> MDs;

    if (DEBUG) *IR2Src_Log << "Function: " << F.getName() << " includes basic blocks:\n   ";
    std::string path = "";
    int begin_line = 1000000;
    int end_line = 1;
    for (auto &B : F)
    {
        if (DEBUG) *IR2Src_Log << B.getName() << ", ";
        if (path == "")
        {
            path = Block2Path[&B];
        }

        if (Block2Line[&B].second > end_line)
            end_line = Block2Line[&B].second;

        if (Block2Line[&B].first < begin_line)
            begin_line = Block2Line[&B].first;
    }


    F.getAllMetadata(MDs);
    for (auto &MD : MDs) 
    {
        if (MDNode *N = MD.second) 
        {
            if (DEBUG) *IR2Src_Log << "Total instructions in function " << F.getName() << " - " << *N << "\n";
            if (auto DFLoc = dyn_cast<DISubprogram>(N) )
            {
                if (DEBUG) *IR2Src_Log << "     DebugInfo: : " << DFLoc->getDirectory() << "/" << DFLoc->getFilename() << ":" << DFLoc->getLine() << "\n";
                IRFunc2BeginLine[demangleFunctionName(F.getName())].push_back(DFLoc->getLine());
                if (begin_line > DFLoc->getLine())
                    begin_line = DFLoc->getLine();
                for (auto RMD : DFLoc->getRetainedNodes())
                {
                    if (DEBUG) *IR2Src_Log << "     retained #" << *RMD << " : \n";                    
                }
                
            }
        }
    }
    if (DEBUG) *IR2Src_Log << "\n";
    if (DEBUG) *IR2Src_Log << "    SourceRang: " << path << ":" << begin_line << "--" << end_line << "\n\n";

    Function2Line[&F] = std::pair<int, int>(begin_line, end_line);
    //if (F.getName().find(".") == std::string::npos)
    IRFunc2BeginLine[demangleFunctionName(F.getName())].push_back(begin_line);
    
    Function2Path[&F] = path;
}

void HI_IR2SourceCode::traceModuleMetadata(Module &M)
{
    SmallVector<std::pair<unsigned, MDNode *>, 4> MDs;

    
    // for (auto &MD : M.meta()) 
    // {
    //     if (MDNode *N = MD) 
    //     {
    //         if (DEBUG) *IR2Src_Log << "Total instructions in function " << F.getName() << " - " << *N << "\n";
    //         if (auto DFLoc = dyn_cast<DISubprogram>(N) )
    //         {
    //             if (DEBUG) *IR2Src_Log << "     DebugInfo: : " << DFLoc->getDirectory() << "/" << DFLoc->getFilename() << ":" << DFLoc->getLine() << "\n";
    //         }
    //         else if (auto DLLoc = dyn_cast<DILabel>(N))
    //         {
    //             if (DEBUG) *IR2Src_Log << "     LoopLable: : " << DLLoc->getFile() << ":" << DLLoc->getLine() << "\n";
    //         }
    //     }
    // }
}

void HI_IR2SourceCode::mappingLoopIR2LoopLabel(llvm::Module &M)
{
    for (auto &F : M)
    {
        SmallVector<std::pair<unsigned, MDNode *>, 4> MDs;
        F.getAllMetadata(MDs);
        for (auto &MD : MDs) 
        {
            if (MDNode *N = MD.second) 
            {
                if (DEBUG) *IR2Src_Log << "Total instructions in function " << F.getName() << " - " << *N << "\n";
                if (auto DFLoc = dyn_cast<DISubprogram>(N) )
                {
                    if (DEBUG) *IR2Src_Log << "     DebugInfo: : " << DFLoc->getDirectory() << "/" << DFLoc->getFilename() << ":" << DFLoc->getLine() << "\n";
                    for (auto RMD : DFLoc->getRetainedNodes())
                    {
                        if (DEBUG) *IR2Src_Log << "     retained #" << *RMD << " : \n";
                            
                        if (auto DLLoc = dyn_cast<DILabel>(RMD))
                        {
                            if (DEBUG) *IR2Src_Log << "           LoopLable: " << DLLoc->getName() <<" : " << DLLoc->getFile()->getDirectory() << "/" << DLLoc->getFile()->getName() << ":" << DLLoc->getLine() << "\n";
                            std::string tmp_path(DFLoc->getDirectory());
                            tmp_path +=  "/";
                            tmp_path += DFLoc->getFilename();
                            bool find = 0;
                            for (auto itLine : Loop2Line)
                            {
                                if (itLine.second.first == DLLoc->getLine() && Loop2Path[itLine.first] == tmp_path)
                                {
                                    if (DEBUG) *IR2Src_Log << "           mapping to IR loop: " << itLine.first->getName() << "in Function : " << itLine.first->getParent()->getName() << "\n";
                                    std::string tmp_loop_name = itLine.first->getParent()->getName();
                                    tmp_loop_name += "-";
                                    tmp_loop_name += itLine.first->getName();
                                    IRLoop2LoopLabel[tmp_loop_name] = DLLoc->getName();
                                    find = 1;
                                }
                            }
                            IR2Src_Log->flush();
                            // assert(find && "mapping for each loop label should be found!!!");
                        }
                        
                    }
                    
                }
            }
        }
        if (DEBUG) *IR2Src_Log << "\n";
    }

}