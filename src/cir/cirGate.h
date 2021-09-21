/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"
#include "cirMgr.h"
using namespace std;
#include <bitset>

// TODO: Feel free to define your own classes, variables, or functions.
extern CirGate** gate_all;
//class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
/*
class fan{
  public:
  fan(){}
  fan(CirGate* g,bool b): gate(g),invert(b){}
  CirGate* gate;
  bool invert;
};
*/

class CirGate
{
public:
   CirGate(){}
   CirGate(unsigned ID,unsigned line) :line(line),ID(ID),visit(false) { sim =0;}
   virtual ~CirGate() {}

   // Basic access methods
   virtual string getTypeStr() const { return ""; }
   unsigned getLineNo() const { return line; }
   virtual bool isAig() const { return false; }
   bool isviset() const { return visit ;}
   void setviset(bool v) {visit =v;}
   // Printing functions
   virtual void printGate() const {};
   virtual void reportGate() const ;
   virtual void reportFanin(int level)   ;
   virtual void reportFanout(int level)  ;
   
   //self define
   //basic access function
   void setline(unsigned i) { line =i; }
   unsigned getid(){ return ID; }
   virtual string getsymbol(){ return "";}
   
   //basic set function
   virtual void setsymbol(string s) {};
   void reset(){ visit=false;}

   //about fanin fanout access
   virtual void push_back_fi( size_t i) {};
   virtual void push_back_fo( size_t i) {};
   virtual unsigned getfanin_size(){ return 0;}
   virtual unsigned getfanout_size(){ return 0;}
   virtual bool getfanin( vector<size_t>*& vec)   { return false;}
   virtual bool getfanout( vector<size_t>*& vec)  {return false;}

   //about DFS
   virtual void DFS(vector<unsigned>&,vector<unsigned>&){;}
   virtual void dfsviset(CirGate* gate,vector<unsigned>&,vector<unsigned>&){;}


   // about fanin fanout report
   void report_fanin_recur(CirGate* gate,unsigned space,int level,bool inv) ;  
   void report_fanout_recur(CirGate* gate,unsigned space,int level,bool inv);
   void reset_fanin(int level);
   void reset_fanout(int level);
   void clear_fan();

   //about remove gate
   void remove() ;
   void remove_fanin(CirGate*) ;
   void remove_fanout(CirGate*) ;

   // about simulation 
   void set_simval(size_t i) { sim = i;};
   size_t get_simval() {return sim;}
   void update_sim(int i) { sim = (sim <<1) + i;}
   virtual void simulation_gate();

   //about fecprgs
   virtual vector<unsigned>* getfec() { return 0; };
   virtual  void setfec( vector<unsigned>* fec) { };
private:

protected:
  protected:
  unsigned line;
  unsigned ID;
  bool visit;//visit and report
  size_t sim;
};

class CirGate_PI : public CirGate{
  public:
    CirGate_PI(){};
    CirGate_PI(unsigned ID,unsigned line) :CirGate(ID,line){}
    // Basic access methods
     string getTypeStr() const { return "PI"; }

   // Printing functions
     void printGate() const {
      cout<<"PI  "<<ID;
      if(symbol.size()!=0) cout<<" ("<<symbol<<")";
      cout<<endl;
     };
     void reportGate() const {
       cout<<"================================================================================"<<endl;
       cout<<"= PI("<<ID<<")";
       if(symbol.size()!=0) cout<<"\""<<symbol<<"\"";
       cout<<", line "<<line<<endl;
       cout<<"= FECS:";
       cout<<endl;
       cout<<"= Value: ";
       bitset<8*sizeof(SIZE_MAX)> b(sim);
       for(unsigned i =0; i <8*sizeof(SIZE_MAX);i++){
         cout<<b[i];
         if(i%8 == 7){
           if(i!=8*sizeof(SIZE_MAX)-1)
           cout<<"_";
         }
       }
       cout<<endl;
       cout<<"================================================================================"<<endl;

     };
   
   //self define
   //basic access function
     string getsymbol(){ return symbol;}
   
   //basic set function
     void setsymbol(string s){ symbol = s;}

   //about fanin fanout
     void push_back_fo( size_t i) {fan_out.push_back(i);}
     unsigned getfanout_size(){ return fan_out.size();}
     bool getfanout( vector<size_t>* &vec)  { vec = &fan_out; return true;}

   protected:
    string symbol;
    vector<size_t> fan_out;
};
class CirGate_AIG : public CirGate{
  public:
    CirGate_AIG(){}
    CirGate_AIG(unsigned ID,unsigned line) :CirGate(ID,line){ fecgrps =0;}
      
    // Basic access methods
      string getTypeStr() const { return "AIG"; }
      bool isAig() const { return true; }

   // Printing functions
     void printGate() const {
      cout<<"AIG "<<ID<<" ";
      for (unsigned i =0;i<fan_in.size()-1;i++){
        if( gate_all[(fan_in[i]/2)]->getTypeStr() == "UNDEF") cout<<"*";
        if( fan_in[i]%2 == 1 ) cout<<"!";
        cout<<fan_in[i]/2<<" ";
      }
      if( gate_all[fan_in[fan_in.size()-1]/2]->getTypeStr() == "UNDEF") cout<<"*";
      if( fan_in[fan_in.size()-1] %2) cout<<"!";
        cout<< fan_in[fan_in.size()-1]/2;
      cout<<endl;
     };
     void reportGate() const {
       cout<<"================================================================================"<<endl;
       cout<<"= AIG("<<ID<<"), line "<<line<<endl;
       cout<<"= FECS:";
       if(fecgrps != 0){
         for(unsigned i =0,n = fecgrps->size(); i<n;i++){
           if((*fecgrps)[i] != ID){
             cout<<" ";
             if(gate_all[(*fecgrps)[i]]->get_simval() != sim) cout<<"!";
             cout<<(*fecgrps)[i];
           }
         }
       }
       cout<<endl;
       cout<<"= Value: ";
       bitset<8*sizeof(SIZE_MAX)> b(sim);
       for(unsigned i =0; i <8*sizeof(SIZE_MAX);i++){
         cout<<b[i];
         if(i%8 == 7){
           if(i!=8*sizeof(SIZE_MAX)-1)
           cout<<"_";
         }
       }
       cout<<endl;
       cout<<"================================================================================"<<endl;
     }
   
   //self define
   //about fanin fanout
     void push_back_fi( size_t i) {fan_in.push_back(i);}
     void push_back_fo( size_t i) {fan_out.push_back(i);}
     unsigned getfanin_size(){ return fan_in.size();}
     unsigned getfanout_size(){ return fan_out.size();}
     bool getfanout( vector<size_t>* &vec)  { vec = &fan_out; return true;}
     bool getfanin( vector<size_t>* &vec)   { vec = &fan_in; return true;}

  //about write agg
     void DFS(vector<unsigned>&,vector<unsigned>&);
     void dfsviset(CirGate* gate,vector<unsigned>&,vector<unsigned>&);

   // about fecgrps
     vector<unsigned>* getfec() {return fecgrps;}
     void setfec( vector<unsigned>* fec) {  fecgrps = fec;}

  protected:
    vector<size_t> fan_out , fan_in;
    vector<unsigned>* fecgrps;
};
class CirGate_PO : public CirGate{
  public:
    CirGate_PO(){}
    CirGate_PO(unsigned ID,unsigned line) :CirGate(ID,line){}
    // Basic access methods
     string getTypeStr() const { return "PO"; }

   // Printing functions
     void printGate() const {
      cout<<"PO  "<<ID;
      cout<<" ";
      for (unsigned i =0;i<fan_in.size();i++){
        if( gate_all[(fan_in[i]/2)]->getTypeStr() == "UNDEF") cout<<"*";
        if( fan_in[i]%2) cout<<"!";
        cout<<fan_in[i]/2;
      }
      if(symbol.size()!=0) cout<<" ("<<symbol<<")";
      cout<<endl;
     };
     void reportGate() const {
       cout<<"================================================================================"<<endl;
       cout<<"= PO("<<ID<<")";
       if(symbol.size()!=0) cout<<"\""<<symbol<<"\"";
       cout<<", line "<<line<<endl;
       cout<<"= FECS:";
       cout<<endl;
       cout<<"= Value: ";
       bitset<8*sizeof(SIZE_MAX)> b(sim);
       for(unsigned i =0; i <8*sizeof(SIZE_MAX);i++){
         cout<<b[i];
         if(i%8 == 7){
           if(i!=8*sizeof(SIZE_MAX)-1)
           cout<<"_";
         }
       }
       cout<<endl;
       cout<<"================================================================================"<<endl;

     };
   
   //self define
   //basic access function
     string getsymbol(){ return symbol;}
   
   //basic set function
     void setsymbol(string s){ symbol =s;}

   //about fanin fanout
     void push_back_fi( size_t i) {fan_in.push_back(i);}
     unsigned getfanin_size(){ return fan_in.size();}
     bool getfanin( vector<size_t>* &vec)   { vec = &fan_in; return true;}
   //
   void simulation_gate();
  protected:
    string symbol;
    vector<size_t> fan_in;
};
class CirGate_CONS : public CirGate{
  public:
    CirGate_CONS(){}
    CirGate_CONS(unsigned ID,unsigned line) :CirGate(ID,line){ fecgrps =0 ;}
    // Basic access methods
     string getTypeStr() const { return "CONST"; }

   // Printing functions
     void printGate() const {
      cout<<"CONST"<<ID;
      cout<<endl;
     };
     void reportGate() const {
       cout<<"================================================================================"<<endl;
       cout<<"= CONST("<<ID<<"), line "<<line<<endl;
       cout<<"= FECS:";
       if(fecgrps != 0){
         for(unsigned i =0,n = fecgrps->size(); i<n;i++){
           cout<<" "<<(*fecgrps)[i];
         }
       }
       cout<<endl;
       cout<<"= Value: ";
       bitset<8*sizeof(SIZE_MAX)> b(sim);
       for(unsigned i =0; i <8*sizeof(SIZE_MAX);i++){
         cout<<b[i];
         if(i%8 == 7){
           if(i!=8*sizeof(SIZE_MAX)-1)
           cout<<"_";
         }
       }
       cout<<endl;
       cout<<"================================================================================"<<endl;
 
     };

   //about fanin fanout
     void push_back_fo( size_t i) {fan_out.push_back(i); }
     unsigned getfanout_size(){ return fan_out.size();}
     bool getfanout( vector<size_t>* &vec)  { vec = &fan_out; return true;}

    vector<unsigned>* getfec() {return fecgrps;}
     void setfec( vector<unsigned>* fec) {  fecgrps = fec;}

  protected:
    vector<unsigned>* fecgrps;
    vector<size_t> fan_out;
};
class CirGate_UNDEF : public CirGate{
  public:
    CirGate_UNDEF(){}
    CirGate_UNDEF(unsigned ID,unsigned line) :CirGate(ID,line){}

    // Basic access methods
     string getTypeStr() const { return "UNDEF"; } 

   // Printing functions
     void printGate() const {
      cout<<"UNDEF "<<ID;
      cout<<endl;
     };
     void reportGate() const {
       cout<<"================================================================================"<<endl;
       cout<<"= UNDEF("<<ID<<"), line "<<line<<endl;
       cout<<"= FECS:";
       cout<<endl;
       cout<<"= Value: ";
       bitset<8*sizeof(SIZE_MAX)> b(sim);
       for(unsigned i =0; i <8*sizeof(SIZE_MAX);i++){
         cout<<b[i];
         if(i%8 == 7){
           if(i!=8*sizeof(SIZE_MAX)-1)
           cout<<"_";
         }
       }
       cout<<endl;
       cout<<"================================================================================"<<endl;
 
     };


   //about fanin fanout
     void push_back_fo( size_t i) {fan_out.push_back(i); }
     unsigned getfanout_size(){ return fan_out.size();}
     bool getfanout( vector<size_t>* &vec)  { vec = &fan_out; return true;}

  protected:
    vector<size_t> fan_out;
};
#endif // CIR_GATE_H
