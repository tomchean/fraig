/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;
extern CirGate** gate_all;
// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{ 
  // get dfslist first
  _dfsList.clear();
  _dfsList_nopio.clear();
  for(unsigned i=0;i<cir_para[3];i++){
    DFS(gate_all[cir_para[0]+1+i]);
  }
  // if visit is false ->sweep 
  for(unsigned i =0; i< cir_para[0]+cir_para[3];i++){
    if(gate_all[i]!=0){
      if(gate_all[i]->isviset() == false){
        sweepGate(gate_all[i],i);
      }
    }
  }
  resetvisit();
  update();
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
  vector<size_t>* tmp; // const 0 or 1 -> replace
  vector<size_t> const_tmp;
  bool finish = false;
  while(!finish){
    finish = true;
    if( gate_all[0]->getfanout(tmp)){
      for(unsigned i =0;i< (*tmp).size();i++){
        if( gate_all[(*tmp)[i]/2]->getTypeStr() != "PO"){
          finish = false;
          if( (*tmp)[i]%2 ){ // input 1 -> remove this gate and connect it's input to it's output
            remove_one( gate_all[(*tmp)[i]/2]);
          }
          else{
            remove_zero(gate_all[(*tmp)[i]/2]);
          }
        }   
      }
    }
  }

  for(unsigned i =0; i< cir_para[0]+1;i++){ 
    if(gate_all[i]!=0){
      if(gate_all[i]->getfanin(tmp)){
        if( (*tmp)[0]/2 == (*tmp)[1]/2){
          if((*tmp)[0]%2 == (*tmp)[1]%2){
            neglect_gate(gate_all[i]);
          }
          else{
            remove_zero(gate_all[i]);
          }
        }
      }
    }
  }
  // update dfslist
  update();
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
void
CirMgr::sweepGate(CirGate* gate,unsigned index){
  gate->remove();
  if( gate->getTypeStr() != "PI" && gate->getTypeStr() != "CONST"){
    gate_all[index]=0;
    cout<<"Sweeping: "<<gate->getTypeStr()<<"("<<gate->getid()<<") removed..."<<endl;
    delete gate;
  }
  else{
    gate->clear_fan();
  }
}

void 
CirMgr::update(){
  ID_UNDEF.clear();
  ID_AIG.clear();
  for(unsigned i =0; i< cir_para[0]+1 ; i++){
    if(gate_all[i]!=0){
      if(gate_all[i]->getTypeStr() == "AIG"){
        ID_AIG.push_back(i);
      }
      if(gate_all[i]->getTypeStr() == "UNDEF"){
        ID_UNDEF.push_back(i);
      }
    }
  }
  _dfsList.clear();
  _dfsList_nopio.clear();
  for(unsigned i=0;i<cir_para[3];i++){
    DFS(gate_all[cir_para[0]+1+i]);
  } 
  resetvisit();  
}


void
CirMgr::remove_one(CirGate* gate){

  vector<size_t>* tmp_fanin =0;
  vector<size_t>* tmp_fanout =0;
  size_t tmp_gate =0;
  if( gate->getTypeStr() != "PO"){
    gate->getfanout(tmp_fanout);
    gate->getfanin(tmp_fanin);
    gate->remove();
    if( tmp_fanin != 0){
      for( unsigned i=0;i<(*tmp_fanin).size();i++){
        if( (*tmp_fanin)[i]/2 != 0){ // tmp_gate is the another input except const0
          tmp_gate = (*tmp_fanin)[i];
          cout<<"Simplifying: "<<gate_all[tmp_gate/2]->getid() <<" merging ";
          if(tmp_gate%2) cout<<"!";
          cout<<gate->getid()<<"..."<<endl;
          break;
        }
      }
      if( tmp_fanout != 0){
        for( unsigned i=0;i<(*tmp_fanout).size();i++){
          gate_all[tmp_gate/2]->push_back_fo( 2* ((*tmp_fanout)[i]/2) + ((*tmp_fanout)[i]%2 ^ (tmp_gate%2)) );
          gate_all[(*tmp_fanout)[i]/2]->push_back_fi( 2* (tmp_gate/2) + ((*tmp_fanout)[i]%2 ^ (tmp_gate%2)) );
          check_opt(gate_all[(*tmp_fanout)[i]/2]);
        }
      }
    } 
    gate_all[gate->getid()] =0;
    delete gate;
  }
  else{
    gate->remove();
    gate->clear_fan();
    gate->push_back_fi(1);
    gate_all[0]->push_back_fo(2*gate->getid()+1);
  }
}
 

void
CirMgr::remove_zero(CirGate* gate){

  if( gate->getTypeStr() != "PO"){
    vector<size_t>* tmp_fanin =0;
    vector<size_t>* tmp_fanout =0;
    gate->getfanout(tmp_fanout);
    gate->getfanin(tmp_fanin);
    gate->remove();
    if( tmp_fanout != 0){
      for( unsigned i=0; i<(*tmp_fanout).size();i++){
        if( (*tmp_fanout)[i]%2) {
          remove_one( gate_all[(*tmp_fanout)[i]/2]);
        }
        else{
          remove_zero(gate_all[(*tmp_fanout)[i]/2]);
        }
      }
    }
    gate_all[gate->getid()] =0;
    cout<<"Simplifying: 0 merging "<<gate->getid()<<"..."<<endl;
    delete gate;
  }
  else{
    gate->remove();
    gate->clear_fan();
    gate->push_back_fi(0);
    gate_all[0]->push_back_fo(2*gate->getid());
  }
}


void 
CirMgr::neglect_gate(CirGate* gate){
  vector<size_t>* tmp_fanin =0;
  vector<size_t>* tmp_fanout =0;

  gate->getfanout(tmp_fanout);
  gate->getfanin(tmp_fanin);
  gate->remove();

  cout<<"Simplifying: "<<gate_all[(*tmp_fanin)[0]/2]->getid()<<" merging ";
  if((*tmp_fanin)[0]%2) cout<<"!";
  cout<<gate->getid()<<"..."<<endl;

  if( tmp_fanout != 0){
    for( unsigned i=0;i<(*tmp_fanout).size();i++){
      gate_all[(*tmp_fanin)[0]/2]->push_back_fo( 2*((*tmp_fanout)[i]/2) + ( (*tmp_fanout)[i]%2 ^ (*tmp_fanin)[0]%2) );
      gate_all[(*tmp_fanout)[i]/2]->push_back_fi( 2*((*tmp_fanin)[0]/2) + ( (*tmp_fanout)[i]%2 ^ (*tmp_fanin)[0]%2) );
      check_opt(gate_all[(*tmp_fanout)[i]/2]);
    }
  }
  gate_all[gate->getid()] =0;
  delete gate;
}


void
CirMgr::check_opt(CirGate* gate){
  vector<size_t>* tmp; // const 0 or 1 -> replace
  vector<size_t> const_tmp;  
  if(gate->getfanin(tmp)){
    if(gate->getfanin_size() == 2){
      if( (*tmp)[0]/2 == (*tmp)[1]/2){
        if((*tmp)[0]%2 == (*tmp)[1]%2){
          neglect_gate(gate);
        }
        else{
          remove_zero(gate);
        }
      }
    }
  }
}