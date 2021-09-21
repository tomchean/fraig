/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include "myHashMap.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim() 
{
  if(!simulated) {
    initsim();
  }
  unsigned count =0 ;
  double rec =0 , baseline;
  unsigned size = 8*sizeof(SIZE_MAX);
  size_t num = cir_para[0];  
  simulation(); 
  num = update_fec(num);
  rec = num;
  baseline = rec/19000;
  count++;

  for(size_t i=0,n=useful_sim.size();i<n;i++){
    simulation_stream(useful_sim[i]);
    num = update_fec(num);
  }

  while (true){
    set_sim();
    simulation(); 
    num = update_fec(num);
    if(count %10 == 0){
      if( rec - num < baseline){  
        count++;
        break;
      }
      rec = num;
    }
    count++;
  }
  sort_fec();
  if(!simulated) {
    assign_fec();
    simulation_po();
    init_dfsfec();
    simulated = true;
  }
  else {
    reset_fec();
    assign_fec();
    simulation_po();
    set_dfsfec();
  }
  cout<< count*size <<" patterns simulated."<<endl;
}

void
CirMgr::fileSim(ifstream& patternFile)
{ 
  string buf;
  int count =0;
  unsigned size = 8*sizeof(SIZE_MAX);
  unsigned k =0; 
  size_t num = cir_para[0];
  size_t sim[cir_para[1]] ={};

  if(!simulated) {
    initsim();
  }
  while(patternFile>>buf){
    if(buf.length() == cir_para[1]){
      k++;
      for(unsigned l =0; l<cir_para[1] ;l++){
        if(buf[l] == '1'){
          sim[l] = (sim[l] << 1 )+ 1;
        }
        else if(buf[l] == '0'){
          sim[l] = (sim[l] << 1 );
        }
        else { // invalid char
          k =0;
          cout<<"Error: Pattern("<<buf<<") contains a non-0/1 character('"<<buf[l]<<"')."<<endl;
          goto end;
        }
      }
    }
    else{
      k =0;
      cout<<"Error: Pattern("<<buf<<") length("<<buf.length()
      <<") does not match the number of inputs("<<cir_para[1]<<") in a circuit!!"<<endl;
      goto end;
    }
    if(k == size){
      k=0;
      count++;
      simulation_stream(sim);
      num = update_fec(num);
      if(_simLog != 0){
        size_t rec = (1<<(size-1)) ;
        simulation_po();
        for(int n=size-1;n >=0 ;n--){
          for(unsigned m=0;m<cir_para[1];m++){
            *_simLog << ((sim[m] & rec) >> n);
          }
          *_simLog<<' ';
          for(unsigned m=0 ; m < cir_para[3] ; m++){
            *_simLog << ((gate_all[cir_para[0]+1+m]->get_simval() & rec) >> n);
          }
          *_simLog<<'\n';
          rec = rec >> 1;
        }
      }
    }
  }
  end:
    patternFile.close();
    if(k>0){
      for(unsigned l =0; l<cir_para[1] ;l++){
        sim[l] = (sim[l] << (size -k) ) ;
      }
      simulation_stream(sim);
      num = update_fec(num);
    }
    sort_fec();
    if(!simulated) {
      assign_fec();
      simulation_po();
      init_dfsfec();
      simulated = true;
    }
    else {
      reset_fec();
      assign_fec();
      simulation_po();
      set_dfsfec();
    }
    if(_simLog != 0){
      if(k>0){
        size_t rec = (1<<(size-1)) ;
        for(int n=size-1,u =size-1-k ; n > u ;n--){
          for(unsigned m=0;m<cir_para[1];m++){
            *_simLog << ((sim[m] & rec) >> n);
          }
          *_simLog<<' ';
          for(unsigned m=0 ; m < cir_para[3] ; m++){
            *_simLog << ((gate_all[cir_para[0]+1+m]->get_simval() & rec) >> n);
          }
          *_simLog<<'\n';
          rec = rec >> 1;
        }
      }
    }
    cout<< count*size+k <<" patterns simulated."<<endl;
    simulated = true;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/

void 
CirMgr::initsim(){
  myfecgrps = new vector<vector<unsigned>*> ;
  vector<unsigned>* tmp = new vector<unsigned>;
  tmp->push_back(0);
  for(unsigned i =0, n = _dfsList_nopio.size() ; i < n ;i++){
    tmp->push_back(_dfsList_nopio[i]);
  }
  myfecgrps->push_back(tmp); 

}

void 
CirMgr::simulation(){
  for(unsigned i =0, n = _dfsList_nopio.size() ; i < n ;i++){
    gate_all[_dfsList_nopio[i]]->simulation_gate();  
  } 
}
void 
CirMgr::simulation_stream(size_t* sim){
  for(unsigned i =0; i< cir_para[1] ; i++){
    gate_all[ID_IP[i]]->set_simval(sim[i]);
  }
  for(unsigned i =0, n = _dfsList_nopio.size() ; i < n ;i++){
    gate_all[_dfsList_nopio[i]]->simulation_gate();  
  }
}
void 
CirMgr::set_sim(){
  for(unsigned i =0; i< cir_para[1] ; i++){
    gate_all[ID_IP[i]]->set_simval(rnGen(SIZE_MAX));
  }
}

size_t 
CirMgr::update_fec(size_t& num){
  vector<vector<unsigned>*>* tmp_fecgrps = new vector<vector<unsigned>*>;
  vector<unsigned>* tmp;
  HashMap< HashKey_fec<size_t> ,vector<unsigned>*> fec_hash(getHashSize(size_t(num)));
  num =0;
  for(size_t i =0, n1 = myfecgrps->size() ; i < n1 ;i++){
    for(size_t j =0 ,n = (*(*myfecgrps)[i]).size() ; j < n ; j++){
      HashKey_fec<size_t> k( gate_all[(*(*myfecgrps)[i])[j]]->get_simval(),i);
      num++;
      if(fec_hash.query( k,tmp)){
        tmp->push_back((*(*myfecgrps)[i])[j]);
      } 
      else{ 
        tmp = new vector<unsigned>;
        tmp->push_back((*(*myfecgrps)[i])[j]);
        fec_hash.insert(k,tmp);
      }    
    }
  }    
  for( HashMap< HashKey_fec<size_t> ,vector<unsigned>*>::iterator it = fec_hash.begin() ,end = fec_hash.end()  ;it != end ; ++it){
    if( (*it).second->size() >= 2){
      tmp_fecgrps->push_back((*it).second);
    }
    else {
      delete (*it).second;  
    } 
  }
  
  for(size_t i =0 ,n = myfecgrps->size() ; i< n;i++){
    delete (*myfecgrps)[i];     
  }
  delete myfecgrps; 
  myfecgrps = tmp_fecgrps;
  return num;
}



bool cmp( vector<unsigned>* &a , vector<unsigned>* &b ) {
  return a[0] < b[0];
}
 
void
CirMgr::sort_fec(){
  for(size_t i =0 ,n1 = (*myfecgrps).size() ;i < n1 ;i++){
    std::sort( (*myfecgrps)[i]->begin(), (*myfecgrps)[i]->end());
  } 
  std::sort(myfecgrps->begin() , myfecgrps->end() , cmp);
}



void
CirMgr::assign_fec(){ 
  for(size_t i =0, n1 =myfecgrps->size() ; i < n1 ;i++){
    for(size_t j =0 ,n = (*(*myfecgrps)[i]).size() ; j < n ; j++){
      gate_all[(*(*myfecgrps)[i])[j]]->setfec((*myfecgrps)[i]);
    }
  }
}

void 
CirMgr::reset_fec(){
  for(size_t i=0,n = _dfsList_fec->size(); i<n ; i++){
    gate_all[(*_dfsList_fec)[i]]->setfec(0);
  }
}

void 
CirMgr::set_dfsfec(){
  vector<unsigned>* tmp =  new vector<unsigned>;
  for(size_t i=0,n = _dfsList_fec->size(); i<n ; i++){
    if(gate_all[ (*_dfsList_fec)[i]]->getfec()!=0)
    tmp->push_back((*_dfsList_fec)[i]) ;
  }
  delete _dfsList_fec;
  _dfsList_fec = tmp;
}

void 
CirMgr::init_dfsfec(){
  _dfsList_fec = new vector<unsigned>;
  for(size_t i=0,n = _dfsList_nopio.size(); i<n ; i++){
    if(gate_all[_dfsList_nopio[i]]->getfec()!=0)
    _dfsList_fec->push_back(_dfsList_nopio[i]) ;
  }
}


void 
CirMgr::simulation_po(){
  for(unsigned i=0; i<cir_para[3]; i++){
    gate_all[cir_para[0]+1+i]->simulation_gate();
  }
}