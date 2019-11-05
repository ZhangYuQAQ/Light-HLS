# Light-HLS: Fast, Accurate and Convenient 

*Let's try to make HLS developemnt easier for everyone~ ^\_^.*
     


Light-HLS is a light weight high-level synthesis (HLS) framework for academic exploration and evaluation, which can be called to perform various design space exploration (DSE) for FPGA-based HLS design. It covers the abilities of previous works, overcomes the existing limitations and brings more practical features. Light-HLS is modularized and portable so designers can use the components of Light-HLS to conduct various DSE procedures.  Light-HLS gets rid of RTL code generation so it will not suffer from the time-consuming synthesis of commercial HLS tools like VivadoHLS, which involves many detailed operations in both its frond-end and back-end.

If Light-HLS helps for your works, please cite our paper in ICCAD 2019 ^_^: 

    Hi-ClockFlow: Multi-Clock Dataflow Automation and Throughput Optimization in High-Level Synthesis. IEEE/ACM 2019 International Conference On Computer Aided Design (ICCAD) 


<img src="https://github.com/zslwyuan/Light-HLS/blob/master/Images/Light-HLS-Overview.png" width="800"> 

## Light-HLS Frond-End

The goal of Light-HLS frond-end is to generate IR code close enough to those generated via commercial tools, like VivadoHLS, for DSE purpose. In the front-end of Light-HLS, initial IR codes generated via Clang will be processed by HLS optimization passes consisted of three different levels: (a) At instruction level, Light-HLS modifies, removes or reorders the instructions, e.g. reducing bitwidth,  removing redundant instruction  and reordering computation. (b) At loop/function level, functions will be instantiated and loops may be extracted into sub-functions. (c) As for memory access level, redundant load/store instructions will be removed based on dependency analysis.

## Light-HLS Back-End

The back-end of Light-HLS is developed to schedule and bind for the optimized IR codes, so it can predict the resultant performance and resource cost accurately based on the given settings. The IR instructions can be automatically characterized by Light-HLS and a corresponding library, which records the timing and resource of different types of operations, will be generated. For scheduling, based on the generated library, Light-HLS maps most operations to corresponding cycles based on as-soon-as-possible (ASAP) strategy. For some pipelined loops, the constraints of the port number of the BRAMs and loop-carried dependencies are considered. Moreover, some operations might be scheduled as late as possible (ALAP) to lower the II. 	As for resource binding, Light-HLS accumulates the resource cost by each operation and the chaining of operations is considered. The reusing of hardware resource is an important feature in HLS and Light-HLS reuses resources based on more detailed rules, e.g. the source and type of input.

## Light-HLS Application Scenariors

Let's first see what we can do with Light-HLS in the research about HLS.

1. HLS designs can be set with various configurations, leading to different results of performance and resource. To find the optmial solution, designers can determine the configuration and call Light-HLS to predict the result in tens milliseconds, which will be close to the result in VivadoHLS. An example is **[Hi-ClockFlow](https://github.com/zslwyuan/Hi-ClockFlow)**, a tool which searches for the configuration of clock settings and HLS directives for the multi-clock dataflow.

2. HLS designs are sensitive to the source codes, some of which are friendly to FPGA while the others are not. If researchers want to analyze and optimize the design at source code level, Light-HLS have accomplished the back-tracing from back-end, to front-end, to source code, so researchers can find out which part of source code have some interesting behaviors causing problems. In the example **[Hi-ClockFlow](https://github.com/zslwyuan/Hi-ClockFlow)**, Light-HLS helps to partition the source code and map the performance and resource to the different parts of the source code.

3. In the front-end of HLS, source code will be processed by a series of LLVM Passes for analysis and optimization. However, for most of researchers, even if they come up with an idea for the front-end processing, they can hardly estimate the exact outcome of the solution if it can be applied to the commercial HLS tools. Currently, Light-HLS can generate IR code similar to the one generated by VivadoHLS and provide accurate scheduling and resource binding in back-end. Therefore, researchers might implement a Pass, plug it into the front-end of Light-HLS (Yes, just plug ^_^), and evaluate it with the back-end of Light-HLS to see the effect.

4. In the back-end of HLS, the IR instructions/blocks/loops/functions are scheduled and binded to specific hardware resource on FPGA. Based on the IR code similar to the one generated by commercial tools, how to properly schedule the source code and bind the resource can be tested and analyzed with Light-HLS. Currently, Light-HLS can provide the estimated performance and resource cost close to those from VivadoHLS 2018.2. (We will catch up the version of 2019.2 recently.) Light-HLS can generate the library of the timing and resource cost of all the IR instructions, e.g. add, fmul, MAC, fptoui and etc, for a specified devive, like Zedboard. Researchers can change the original scheme of Light-HLS's scheduling and binding to see the effect.


## Category:

**[Installation of Light-HLS](https://github.com/zslwyuan/Light-HLS#installation-of-light-hls)**

**[Usage of Light-HLS](https://github.com/zslwyuan/Light-HLS#usage-of-light-hls)**

**[Implementation of Light-HLS and Further development](https://github.com/zslwyuan/Light-HLS#implementation-of-light-hls-and-further-development)**


***

## [Installation of Light-HLS](https://github.com/zslwyuan/Light-HLS#installation-of-light-hls)

1. Install LLVM-9.0.0: Download LLVM-9.0.0, make and install it. If you want to support arbitary precision integer, please apply **[the batch](https://github.com/zslwyuan/Light-HLS/tree/master/Patch_for_LLVM)** to Clang before make and build. 
If you have problems in building the LLVM package or applying the patch, we provide **[a source code repository](https://github.com/zslwyuan/LLVM-9-for-Light-HLS)** with the patch applied. 
2. In the directory **[Tests](https://github.com/zslwyuan/Light-HLS/tree/master/Tests)**, a series of experiments are conducted during development. The standard Light-HLS is implemented in **[Light_HLS_Top](https://github.com/zslwyuan/Light-HLS/tree/master/Tests/Light_HLS_Top)**
3. In the directory **[Light_HLS_Top](https://github.com/zslwyuan/Light-HLS/tree/master/Tests/Light_HLS_Top)**, a bash script **[Build.sh](https://github.com/zslwyuan/Light-HLS/blob/master/Tests/Light_HLS_Top/Build.sh)** will help to build the project.
4. You can find the built Light-HLS in the directory "build", in which you can test it with the following command:

            ./Light_HLS_Top ../../../App/conv/conv.cc convs ../config_conv.txt DEBUG
   
 
## [Usage of Light-HLS](https://github.com/zslwyuan/Light-HLS#usage-of-light-hls)

1. download the repository (entire project)
2. the organization of this repository is: (1) basic functiones and passes are implemented in the directory **["Implementations"](https://github.com/zslwyuan/Light-HLS/tree/master/Implementations)**. Nearly all the directories have their own README file to explain the directory. (2) experiments are tested in the directory **["Test"](https://github.com/zslwyuan/Light-HLS/tree/master/Tests)**. (3) by making a "build" directory and using CMake in each experiment directory (e.g. **[this one](https://github.com/zslwyuan/Light-HLS/tree/master/Tests/LLVM_exp5_SimpleTimingAnalysis/)**), executable can be generated and tried. (hint: cmake .. & make)  (4) for user's convenience, we prepare some scripts for example, **BuildAllFiles.sh**, which will build all the projects, **CleanBuiltFiles.sh**, which will clean all the built files to shrink the size of the directories, and **Build.sh** in test directory, which will just build one test project. All these scripts can be run directly.
3. looking into the source code with detailed comments, reader can trace the headers and functions to understand how the experiment work.
4. in the directory **[HLS_Data_Lib](https://github.com/zslwyuan/Light-HLS/tree/master/HLS_Data_Lib)**, there is the library of the IR instructions on FPGA. Currently, the library is built for Zedboard (Xilinx Platform ID: xc7z020clg484-1). If you want to use Light-HLS for another platform, you can regenerate the library by using the **[LibGen.py](https://github.com/zslwyuan/Light-HLS/blob/master/HLS_Lib_Generator/LibGen.py)**, which will collect the information from VivadoHLS and designers can overwrite the library in HLS_Data_Lib with the new one. The LibGen.py supports multiple processes to accelerate the procedure. An example is shown below. 

         python LibGen.py -n 12 -p xc7z020clg484-1

8. Light-HLS supports HLS directives for the design, including loop unrolling, loop pipelining, array partitioning, static array setting, dataflow and clock settings, which can be set in a configuration file. An example is shown in **[config_2mm.txt](https://github.com/zslwyuan/Light-HLS/blob/master/Tests/Light_HLS_Top/config_2mm.txt)** and **[config_conv.txt](https://github.com/zslwyuan/Light-HLS/blob/master/Tests/Light_HLS_Top/config_conv.txt)**. If you want to check the loops' labels, please run Light-HLS with loop configurations first and the source code with labels for loop, "tmp_loopLabeled.cc", will be generated in the directory. If you need to set array partition for the arrays, please note that in Light-HLS, the definition of the order of dimentsion is shown like   \[...\]\[second dimension\]\[first dimension\].

9. As you can notice in the usage examples, Light-HLS can run with DEBUG flag and lots of information during the HLS procedure will be dumped for analysis.


***



**usage example**

      When built, most test executables can be used like below but please check the source code for confirmation.
      (1) ./Light_HLS_Top  <C/C++ FILE> <top\_function\_name>  <configuration\_file> [DEBUG]
      or
      (2) ./LLVM\_expXXXXX  <C/C++ FILE> <top\_function\_name>   
      or
      (3) ./LLVM\_expXXXXX  <IR FILE>


***



***

## [Implementation of Light-HLS and Further development](https://github.com/zslwyuan/Light-HLS#implementation-of-light-hls-and-further-development)

### The Overview of Light-HLS Implementation

As shown in **[Light_HLS_Top.cc](https://github.com/zslwyuan/Light-HLS/blob/master/Tests/Light_HLS_Top/Light_HLS_Top.cc)**, the procedure of Light-HLS contains mainly four steps:

1. [Clang part for Front-End Processing](https://github.com/zslwyuan/Light-HLS#1-clang-part-for-front-end-processing): 

         extract the information of arrays in the source code   
         set labels for each loop in the source code   

2. [Front-End Passes for FPGA-Oriented Optimizations and Transformation](https://github.com/zslwyuan/Light-HLS#2-front-end-passes-for-fpga-oriented-optimizations-and-transformation):     

         GEPLowering      
         Loop Extraction                                                               
         Loop Simplification                                                           
         Loop Stregnth Reducation                                                      
         Duplicated Instruction Removal                                                
         Function Instantiation                                                              
         Instruction Optimization: Multiplication / Instruction Hoisting / Redundant Access Removal / Bitwidth Reduction/ etc.. 

3. Front-End Passes just before Back-End Analysis:

         map the IR loops to the loop labels in the source code for the configurations of loops. 
         mainly account for inserting MUX for the accesses to array partitions.   

4. Backend-end Pass

         for scheduling and binding       
         After running this Pass, the information will be stored in
         The public variables of the Pass pointer.                 
         such as function latencies, loop tripcounts and etc.  

### 1. Clang part for Front-End Processing

In this part, we need to extract the information of arrays in the source code, which might be lost in the IR code after the Clang processing. For example, if declared in the interface of functions, the information of the arrays' first dimension might be removed in the IR code. This may lead to the situation that we cannot apply the partitioning to specific dimensions properly. In the processing at Clang level, we can [extract such informantion from the Abstract Syntax Tree (AST) with HI_FunctionInterfaceInfo](https://github.com/zslwyuan/Light-HLS/tree/master/Implementations/HI_FunctionInterfaceInfo).

From another perspective, the loops in IR codes are named according to LLVM rules, which might not be easy to map them to the original source code. Therefore, we need to [set label for each loop with Hi_LoopLabeler](https://github.com/zslwyuan/Light-HLS/tree/master/Implementations/HI_LoopLabeler), so designers can easier specify the loops for configurations.
    
### 2. Front-End Passes for FPGA-Oriented Optimizations and Transformation

In this part, Light-HLS will transform the IR code according to the FPGA characteristics for optimization. 

(1) GEPLowering: GEP is an operation in LLVM to get the element pointer for the accesses to arrays. An array could have multiple dimensions and GEP helps to map the accesses to array to the exact memory address. However, the on-chip memory of FPGA are mainly BRAMs, which are actually "single-dimension". In order to ensure that the instructions can get data from BRAMs, Light-HLS lowers the GEP to those exact operations of address calculation. For example, for the access B\[i\]\[j\] to the array B\[70\]\[20\], Light-HLS will [transform the GEP operation into the multiplication and addition](https://github.com/zslwyuan/Light-HLS/tree/master/Implementations/HI_SeparateConstOffsetFromGEP), e.g. i*20+j.

### Further development

If you want to do your own works based on this project, following hints might be useful.

1. user can add their passes according to the examples in the directory  **["Implementations"](https://github.com/zslwyuan/Light-HLS/tree/master/Implementations)**. 
2. Note for development: (a) remember to define unique marco for header files (like #ifndef _HI_HI_POLLY_INFO);  (b) Modify the CMakelists.txt file in the 4 directories: **the pass directory([example](https://github.com/zslwyuan/Light-HLS/tree/master/Implementations/HI_SimpleTimingEvaluation/CMakeLists.txt)), Implementation directory([example](https://github.com/zslwyuan/Light-HLS/tree/master/Implementations/CMakeLists.txt)), LLVM_Learner_Libs directory([example](https://github.com/zslwyuan/Light-HLS/tree/master/Tests/LLVM_Learner_Libs/CMakeLists.txt)) and the test directory([example](https://github.com/zslwyuan/Light-HLS/blob/master/Tests/LLVM_exp7_DuplicateInstRemove/CMakeLists.txt))**. The modification should add subdirectory and consider the including path/library name.

## Good Good Study Day Day Up \(^o^)/~
