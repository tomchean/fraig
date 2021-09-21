/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "myHashMap.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;
extern CirGate** gate_all;


class CirMgr
{
public:
   CirMgr() { simulated = false;  myfecgrps =0;  _dfsList_fec =0; }
   ~CirMgr() ; 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const ;

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() ;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;

   // self define 
   void DFS(CirGate* gate) ;
   void dfsviset(CirGate* gate) ;
   void resetvisit();
   
   //about sweep
   void sweepGate(CirGate*,unsigned);
   void update();
  
   //about opt
   void remove_one(CirGate*);
   void remove_zero(CirGate*);
   void neglect_gate(CirGate*);
   void check_opt(CirGate*);
   
   // about strash
   void merge(CirGate*,CirGate*);
   void merge_invert(CirGate*,CirGate*);
   // about simulation
   void initsim();
   void simulation();
   void simulation_po();
   void simulation_stream(size_t*);

   //about fec
   size_t update_fec(size_t&);
   void assign_fec();
   void reset_fec();
   void set_dfsfec();
   void init_dfsfec();
   void set_sim();
   void sort_fec();
   
   // about fraig
   void construct_fraig(SatSolver& ,unsigned*&);

   class AIG_REC{
    friend CirMgr;
    public:
      AIG_REC(unsigned i1,unsigned i2,unsigned i){  ip[0] = i1; ip[1] =i2 ; id =i; };
    private:
      unsigned ip[2];
      unsigned id;
    };
private:
   ofstream  *_simLog;
   unsigned cir_para[5];
   vector<unsigned> ID_IP,ID_OP,ID_AIG,ID_UNDEF,_dfsList,_dfsList_nopio;
   
   // FOR FEC
   vector<vector<unsigned>*>* myfecgrps;
   vector<unsigned>* _dfsList_fec;
   bool simulated; 
   vector<size_t*> useful_sim;
   
   // FOR SOLVE SAT
   SatSolver solver;
};




#endif // CIR_MGR_H
