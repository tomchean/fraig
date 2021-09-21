/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"
#include <bitset>
using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;
extern CirGate** gate_all;
/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{

}

void
CirGate::reportFanin(int level) 
{
  assert (level >= 0);
  visit=true;
  cout<<getTypeStr()<<" "<<ID<<endl;
  vector<size_t>* tmp;
  vector<size_t>* tmp1;
  if( getfanin(tmp)) {
    for(unsigned i =0;i<(*tmp).size();i++){
      if( gate_all[(*tmp)[i]/2]->visit == false){
        report_fanin_recur( gate_all[(*tmp)[i]/2],1, level, (*tmp)[i]%2);  
      }
      else {
        cout<<"  ";
        if((*tmp)[i]%2) cout<<"!";
        cout<<gate_all[(*tmp)[i]/2]->getTypeStr()<<" "<<(*tmp)[i]/2;
        if(gate_all[(*tmp)[i]/2]->getfanin(tmp1) && level > 1) {cout <<" (*)";}
        cout<<endl;
      }
    }
  }
  reset_fanin(level);
}
void 
CirGate::report_fanin_recur(CirGate* gate,unsigned space,int level,bool inv) {
  for(unsigned i=0;i<space;i++){
    cout<<"  ";
  }
  if(inv) cout<<"!";
  cout<<gate->getTypeStr()<<" "<<gate->getid()<<endl;
  if(level==1) return;
  gate->visit=true;
  vector<size_t>* tmp;
  vector<size_t>* tmp1;
  if( gate->getfanin(tmp)) {
    for(unsigned i =0;i<(*tmp).size();i++){
      if( gate_all[(*tmp)[i]/2]->visit == false){
        report_fanin_recur(gate_all[(*tmp)[i]/2],space+1,level-1,(*tmp)[i]%2);  
      }
      else {
        for(unsigned i=0;i<=space;i++){
          cout<<"  ";
        }
        if((*tmp)[i]%2){cout<<"!";}
        cout<<gate_all[(*tmp)[i]/2]->getTypeStr()<<" "<<(*tmp)[i]/2;
        if(gate_all[(*tmp)[i]/2]->getfanin(tmp1) && level != 2) {cout <<" (*)";}
        cout<<endl;
      }
    }
  }
}

void
CirGate::reset_fanin(int level){
  visit=false;
  if(level ==0) return;
  vector<size_t>* tmp;
  if( getfanin(tmp)) {
    for(unsigned i =0;i<(*tmp).size();i++){
      gate_all[(*tmp)[i]/2]->reset_fanin(level-1);       
    }
  }
}
void
CirGate::reportFanout(int level) {
  assert (level >= 0);
  visit=true; 
  cout<<getTypeStr()<<" "<<ID<<endl;
  vector<size_t>* tmp;
  vector<size_t>* tmp1;
  if( getfanout(tmp)) {
    for(unsigned i =0;i<(*tmp).size();i++){
      if(gate_all[(*tmp)[i]/2]->visit == false){
        report_fanout_recur(gate_all[(*tmp)[i]/2],1,level,(*tmp)[i]%2);  
      }
      else {
        cout<<"  ";
        if((*tmp)[i]%2) cout<<"!";
        cout<< gate_all[(*tmp)[i]/2]->getTypeStr()<<" "<<(*tmp)[i]/2;
        if(gate_all[(*tmp)[i]/2]->getfanout(tmp1) && level > 1) {cout <<" (*)";}
        cout<<endl;
      }
    }
  }
  reset_fanout(level); 
}

void 
CirGate::report_fanout_recur(CirGate* gate,unsigned space,int level,bool inv) {
  for(unsigned i=0;i<space;i++){
    cout<<"  ";
  }
  if(inv) cout<<"!";
  cout<<gate->getTypeStr()<<" "<<gate->getid()<<endl;
  if(level==1) return;
  gate->visit=true;
  vector<size_t>* tmp;
  vector<size_t>* tmp1;
  if( gate->getfanout(tmp)) {
    for(unsigned i =0;i<(*tmp).size();i++){
      if(gate_all[(*tmp)[i]/2]->visit == false){
        report_fanout_recur( gate_all[(*tmp)[i]/2],space+1,level-1,(*tmp)[i]%2);  
      }
      else {
        for(unsigned i=0;i<=space;i++){
          cout<<"  ";
        }
        if((*tmp)[i]%2){cout<<"!";}
        cout<<gate_all[(*tmp)[i]/2]->getTypeStr()<<" "<<(*tmp)[i]/2;
        if(gate_all[(*tmp)[i]/2]->getfanout(tmp1) && level != 2) {cout <<" (*)";}
        cout<<endl;
      }
    }
  }
}
void
CirGate::reset_fanout(int level){
  visit=false;
  if(level ==0) return;
  vector<size_t>* tmp;
  if( getfanout(tmp)) {
    for(unsigned i =0;i<(*tmp).size();i++){
      gate_all[(*tmp)[i]/2]->reset_fanout(level-1);       
    }
  }
}




// about DFS
void
CirGate_AIG::DFS(vector<unsigned>& list_pi,vector<unsigned>& list_aig) {
  setviset(true);
  vector<size_t>* tmp;
  if( getfanin(tmp)){
    for(unsigned i =0;i<(*tmp).size();i++){
      if(gate_all[(*tmp)[i]/2]->isviset() == false){
        dfsviset(gate_all[(*tmp)[i]/2],list_pi,list_aig);
      } 
    }
  }
  list_aig.push_back(ID);
}

void CirGate_AIG::dfsviset(CirGate* gate,vector<unsigned>& list_pi,vector<unsigned>& list_aig){
  gate->setviset(true);
  vector<size_t>* tmp;
  if( gate->getfanin(tmp)){
    for(unsigned i =0;i<(*tmp).size();i++){
      if(gate_all[(*tmp)[i]/2]->isviset() == false){
        dfsviset(gate_all[(*tmp)[i]/2],list_pi,list_aig);
      }
    }
    list_aig.push_back(gate->getid());
  }
  else{
    list_pi.push_back(gate->getid());    
  }
}



void 
CirGate::remove() {
  vector<size_t>* tmp;
  if( this->getfanin(tmp)){
    for(unsigned i =0;i<(*tmp).size();i++){
      gate_all[(*tmp)[i]/2]->remove_fanout(this);
    }
  }
  if( this->getfanout(tmp)){
    for(unsigned i =0;i<(*tmp).size();i++){
      gate_all[(*tmp)[i]/2]->remove_fanin(this);
    }
  }
} 

void 
CirGate::clear_fan(){
  vector<size_t>* tmp;
  if( this->getfanin(tmp)){
    tmp->clear();
  }
  if( this->getfanout(tmp)){
    tmp->clear();
  }
}

void 
CirGate::remove_fanin(CirGate* gate){
  vector<size_t>* tmp; 
  if( this->getfanin(tmp)){  
    for(unsigned i =0;i<(*tmp).size();i++){
      if( gate_all[(*tmp)[i]/2] == gate){
        tmp->erase(tmp->begin()+i);
        break;
      }
    } 
  }
} 
void 
CirGate::remove_fanout(CirGate* gate){
  vector<size_t>* tmp; 
  if( this->getfanout(tmp)){  
    for(unsigned i =0;i<(*tmp).size();i++){
      if( gate_all[(*tmp)[i]/2] == gate){
        tmp->erase(tmp->begin()+i);
        break;
      }
    } 
  }
} 


void
CirGate::simulation_gate(){
  vector<size_t>* tmp; 
  if( this->getfanin(tmp)){
    if((*tmp)[1]%2){
      sim = ~ gate_all[(*tmp)[1]/2]->get_simval();
    }
    else{
      sim =  gate_all[(*tmp)[1]/2]->get_simval();
    }
    if((*tmp)[0]%2){
      sim =  sim & ~ gate_all[(*tmp)[0]/2]->get_simval();
    }
    else{
      sim = sim & gate_all[(*tmp)[0]/2]->get_simval();
    }
  }
}
void
CirGate_PO::simulation_gate(){
  vector<size_t>* tmp; 
  if( this->getfanin(tmp)){
    if((*tmp)[0]%2){
      sim =  ~ gate_all[(*tmp)[0]/2]->get_simval();
    }
    else{
      sim =  gate_all[(*tmp)[0]/2]->get_simval();
    }
  }
}