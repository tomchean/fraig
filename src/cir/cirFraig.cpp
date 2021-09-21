/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;
extern CirGate** gate_all;
// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
  HashMap<HashKey<size_t>,size_t>  myhash(getHashSize(size_t(cir_para[0]))); 
  vector<size_t>* tmp ;
  size_t buf[2] = {};
  size_t mergegate;
  for(size_t i =0, n= _dfsList_nopio.size(); i<n ;i++){
    if(gate_all[_dfsList_nopio[i]]!=0){
      if(gate_all[_dfsList_nopio[i]]->getfanin(tmp)){
        if((*tmp)[0] > (*tmp)[1]){
          buf[0] = (*tmp)[1] ;
          buf[1] = (*tmp)[0] ; 
        }
        else{
          buf[0] = (*tmp)[0] ;
          buf[1] = (*tmp)[1] ; 
        }
      }
      HashKey<size_t> k(buf[0],buf[1]);  
      if(myhash.query(k,mergegate)){
        cout<<"Strashing: "<<mergegate<<" merging "<<_dfsList_nopio[i]<<"..."<<endl;
        merge( gate_all[_dfsList_nopio[i]],gate_all[mergegate]); 
        delete gate_all[_dfsList_nopio[i]];
        gate_all[_dfsList_nopio[i]] =0;
      }
      else{
        myhash.insert(k,_dfsList_nopio[i]);
      }       
    }
  }
  update();
}

void
CirMgr::fraig(){
  unsigned* SAT_ID;  
  size_t fec_num = (*myfecgrps).size();
  bool result;
  Var newV;
  vector<unsigned>* buf;
  size_t* sim =new size_t[cir_para[1]];;  
  unsigned count1 =0,count2 =0;
  HashMap< HashKey1<unsigned> ,vector<unsigned>*> fraig_hash(getHashSize(_dfsList_fec->size()));
  vector<unsigned>* tmp;


  solver.initialize();
  construct_fraig(solver,SAT_ID); 
restart:
  for(unsigned index = 0,size = _dfsList_fec->size();index <size;index++){
    buf = gate_all[ (*_dfsList_fec)[index]]->getfec();
    if( (*buf)[0] != (*_dfsList_fec)[index]){
      newV = solver.newVar();
      if( gate_all[(*buf)[0]]->get_simval() == gate_all[ (*_dfsList_fec)[index]]->get_simval()  ){
        solver.addXorCNF(newV, SAT_ID[(*buf)[0]], false , SAT_ID[(*_dfsList_fec)[index]], false);
      }
      else{
        solver.addXorCNF(newV, SAT_ID[(*buf)[0]], false , SAT_ID[(*_dfsList_fec)[index]], true);
      }
      solver.assumeRelease();
      solver.assumeProperty(newV, true);
      result = solver.assumpSolve();
      if(result){ //record pattern
        for(unsigned l =0; l<cir_para[1] ;l++){
            sim[l] = (sim[l] << 1 )+ solver.getValue( SAT_ID[ID_IP[l]]);
        }
        count1++;
        if( count1 == 8){
          count2++;
          count1 =0;
          simulation_stream(sim);
          reset_fec();
          fec_num = update_fec(fec_num);
          assign_fec();
          set_dfsfec();
          useful_sim.push_back(sim);
          sim = new size_t[cir_para[1]];
          goto restart;
        }          
      }
      else{ // merge gate
        for( vector<unsigned>::iterator ret = buf->begin() ; ret != buf->end(); ret++){
          if(*ret == (*_dfsList_fec)[index]){
            buf->erase(ret);
            break;
          } 
        }        
        HashKey1<unsigned> k((*buf)[0]);
        if(fraig_hash.query(k,tmp)){
          tmp->push_back((*_dfsList_fec)[index]);
        } 
        else{ 
          tmp = new vector<unsigned>;
          tmp->push_back((*_dfsList_fec)[index]);
          fraig_hash.insert(k,tmp);
        }    
      }
    }
  }
  simulation_stream(sim);
  reset_fec();
  fec_num = update_fec(fec_num);
  assign_fec();
  set_dfsfec();
  if( myfecgrps->size() > 0){
    goto restart;
  }

  delete myfecgrps;
  myfecgrps =0;
  reset_fec();
  gate_all[0]->setfec(0);
  delete _dfsList_fec;
  _dfsList_fec =0;
  simulated = false;      

  vector<unsigned> trash;
  for( HashMap< HashKey1<unsigned> ,vector<unsigned>*>::iterator it = fraig_hash.begin() ,end = fraig_hash.end()  ;it != end ; ++it){
    (*it).second->push_back((*it).first());
    for(unsigned i =0,j = (*it).second->size() ; i<j;i++){
      gate_all[(*(*it).second)[i]]->setfec((*it).second);
    }
  }
  tmp = gate_all[0]->getfec();
  if(tmp != 0){
    for(size_t j =0, n1 = tmp->size(); j<n1 ;j++ ){
      if((*tmp)[j] !=0){
        gate_all[(*tmp)[j]]->setfec(0);
        trash.push_back((*tmp)[j]);
        if( gate_all[(*tmp)[j]]->get_simval() == 0){
          merge(gate_all[(*tmp)[j]],gate_all[0]);
          cout<<"Fraig: 0 merging "<< (*tmp)[j]<<"..."<<endl;
        }
        else{
          merge_invert(gate_all[(*tmp)[j]],gate_all[0]);
          cout<<"Fraig: 0 merging !"<< (*tmp)[j]<<"..."<<endl;
        }        
      }
    }
    gate_all[0]->setfec(0);
  }
  for (size_t i = 0, n = _dfsList_nopio.size() ; i < n; ++i) {
    tmp = gate_all[_dfsList_nopio[i]]->getfec();
    if( tmp != 0){
      for(size_t j =0, n1 = tmp->size(); j<n1 ;j++ ){
        if((*tmp)[j] != _dfsList_nopio[i]){
          gate_all[(*tmp)[j]]->setfec(0);
          trash.push_back((*tmp)[j]);
          if( gate_all[(*tmp)[j]]->get_simval() ==gate_all[_dfsList_nopio[i]]->get_simval()){
            merge(gate_all[(*tmp)[j]],gate_all[_dfsList_nopio[i]]);
            cout<<"Fraig: "<<_dfsList_nopio[i]<<" merging "<< (*tmp)[j]<<"..."<<endl;;
          }
          else{
            merge_invert(gate_all[(*tmp)[j]],gate_all[_dfsList_nopio[i]]);
            cout<<"Fraig: "<<_dfsList_nopio[i]<<" merging !"<< (*tmp)[j]<<"..."<<endl;
          }        
        }
      }
      gate_all[_dfsList_nopio[i]]->setfec(0);
    }
  }
  for (size_t i = 0, n = trash.size() ; i < n; ++i){
    delete gate_all[trash[i]];
    gate_all[trash[i]] =0;
  }
  update();
}
/********************************************/
/*   Private member functions about fraig   */
/********************************************/


void
CirMgr::merge(CirGate* g1,CirGate* g2){
  //<<"merge"<<g1->getid()<<"  "<<g2->getid()<<endl;
  vector<size_t>* tmp_fanout =0;
  g1->getfanout(tmp_fanout);
  g1->remove();
  if(tmp_fanout !=0){
    for(size_t i =0; i< (*tmp_fanout).size();i++){
      g2->push_back_fo((*tmp_fanout)[i]);
      gate_all[(*tmp_fanout)[i]/2]->push_back_fi(2*g2->getid()+(*tmp_fanout)[i]%2);
    }
  }
}

void
CirMgr::merge_invert(CirGate* g1,CirGate* g2){
  //cout<<"merge"<<g1->getid()<<"  "<<g2->getid()<<endl;
  vector<size_t>* tmp_fanout =0;
  g1->getfanout(tmp_fanout);
  g1->remove();
  if(tmp_fanout !=0){
    for(size_t i =0; i< (*tmp_fanout).size();i++){
      g2->push_back_fo( (*tmp_fanout)[i] + ((*tmp_fanout)[i]+1)%2-1 );
      gate_all[(*tmp_fanout)[i]/2]->push_back_fi(2*g2->getid()+ (1-(*tmp_fanout)[i]%2));
    }
  }
}

void 
CirMgr::construct_fraig(SatSolver& s,unsigned*& SAT_ID ){
  SAT_ID = new unsigned[cir_para[0]+1];
  Var v = s.newVar();
  SAT_ID[0] = v;
  s.assertProperty(v,false);
  for (size_t i = 0, n = _dfsList_nopio.size() ; i < n; ++i) {
    Var v = s.newVar();
    SAT_ID[_dfsList_nopio[i]] = v;
  }
  for (size_t i = 0, n = ID_IP.size() ; i < n; ++i) {
    Var v = s.newVar();
    SAT_ID[ID_IP[i]] = v;
  }
  for (size_t i = 0, n = _dfsList_nopio.size() ; i < n; ++i){
   vector<size_t>* tmp ;
   gate_all[_dfsList_nopio[i]]->getfanin(tmp); 
   s.addAigCNF(SAT_ID[_dfsList_nopio[i]], SAT_ID[(*tmp)[0]/2], (*tmp)[0]%2 , SAT_ID[(*tmp)[1]/2], (*tmp)[1]%2);
  }
}
  
