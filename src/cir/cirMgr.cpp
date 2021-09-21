/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert> 
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;
// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;
CirGate** gate_all =0 ;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

CirMgr::~CirMgr() {
  if(gate_all !=0){
  for(unsigned i=0;i<cir_para[0]+cir_para[3]+1;i++){
    if(gate_all[i]!= 0){
      delete gate_all[i];
    }
  }
  delete [] gate_all;
  gate_all =0;
  }
  if(simulated){
    for(size_t i =0 ,n = myfecgrps->size() ; i< n;i++){
      delete (*myfecgrps)[i];     
    }
    delete myfecgrps; 
    myfecgrps =0;
  }
  delete _dfsList_fec;
  _dfsList_fec =0;
  for(unsigned i=0,n=useful_sim.size();i<n;i++){
    delete[]  useful_sim[i];
  }
} 

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{

  lineNo=0;
  int i =0;
  int tmp[3];
  fstream ifile;
  char cbuf;
  CirGate* gate;
  vector<AIG_REC>  aig_rec;
  gate_all=0;
  ifile.open(fileName,ios::in);
  if(!ifile){
    cerr<<"Cannot open design \""<<fileName<<"\"!!"<<endl;
    return false;
  }
  ifile.get(buf,1024,' '); // get header
  while(i<4){  // get para
    ifile.get(cbuf);
    ifile.get(buf,1024,' ');
    myStr2Int((string)buf,tmp[0]);
    cir_para[i] = tmp[0];
    i++;
  }
  ifile.get(cbuf);
  ifile.getline(buf,1024);
  myStr2Int((string)buf,tmp[0]);
  cir_para[4] = tmp[0];  
  lineNo+=1;
  gate_all =new CirGate*[cir_para[0]+1+cir_para[3]];
  for(unsigned i=0;i<cir_para[0]+1+cir_para[3];i++){
    gate_all[i]=0;
  }
  gate_all[0] = new CirGate_CONS(0,0);

  for(unsigned i =0; i<cir_para[1];i++,lineNo++){ // get input id
    ifile.getline(buf,1024);
    myStr2Int((string)buf,tmp[0]);
    ID_IP.push_back((unsigned)tmp[0]/2);
    gate = new CirGate_PI(ID_IP[i],lineNo+1);
    (gate_all)[tmp[0]/2] = gate;
  }
  for(unsigned i =0; i<cir_para[3];i++,lineNo++){ //get output id
    ifile.getline(buf,1024);
    myStr2Int((string)buf,tmp[0]);
    ID_OP.push_back(tmp[0]);
  }  
  for(unsigned i =0; i<cir_para[4];i++,lineNo++){ // get aig gate
    for(unsigned j =0; j<2;j++){  
      ifile.get(buf,1024,' ');
      myStr2Int((string)buf,tmp[j]);
      ifile.get(cbuf);
    }   
    ifile.getline(buf,1024);
    myStr2Int((string)buf,tmp[2]);
    AIG_REC a = AIG_REC((unsigned)tmp[1],(unsigned)tmp[2],(unsigned)tmp[0]/2); 
    aig_rec.push_back(a);
    ID_AIG.push_back((unsigned)tmp[0]/2);
    gate = new CirGate_AIG(aig_rec[i].id,lineNo+1);
    (gate_all)[tmp[0]/2] = gate;
  }

  
  for(unsigned i =0; i<cir_para[4];i++){ // connect node;
    for(int k =0;k<2;k++){
      if(gate_all[aig_rec[i].ip[k]/2] != 0){
        gate_all[aig_rec[i].id]->push_back_fi( aig_rec[i].ip[k]);
        gate_all[aig_rec[i].ip[k]/2]->push_back_fo( 2*aig_rec[i].id + aig_rec[i].ip[k]%2);
      }
      else{
        gate = new CirGate_UNDEF(aig_rec[i].ip[k]/2,0);
        ID_UNDEF.push_back(aig_rec[i].ip[k]/2);
        gate_all[aig_rec[i].id]->push_back_fi(aig_rec[i].ip[k]);
        gate->push_back_fo( 2*aig_rec[i].id + aig_rec[i].ip[k]%2);
        gate_all[aig_rec[i].ip[k]/2] = gate;
      }      
    }     
  }
  for(unsigned i =0; i<cir_para[3];i++){ //connect output
    gate = new CirGate_PO(cir_para[0]+1+i,cir_para[1]+i+2);
    gate_all[cir_para[0]+1+i] = gate;
    if(gate_all[ID_OP[i]/2] != 0){
      gate->push_back_fi( ID_OP[i]);
      gate_all[ID_OP[i]/2]->push_back_fo( 2*(cir_para[0]+1+i)+ ID_OP[i]%2);
    }
    else{
      gate = new CirGate_UNDEF(ID_OP[i]/2,0);
      ID_UNDEF.push_back(ID_OP[i]/2);
      gate_all[cir_para[0]+1+i]->push_back_fi(ID_OP[i] );
      gate->push_back_fo( 2*(cir_para[0]+1+i) + ID_OP[i]%2);
      gate_all[ID_OP[i]/2] = gate;
    }       
  }
  while( ifile.peek() != 'c' && ifile.peek() != EOF){
    ifile.get(cbuf);
    ifile.get(buf,1024,' ');
    if(cbuf =='i'){
      myStr2Int((string)buf,tmp[0]);
      ifile.get(cbuf);
      ifile.getline(buf,1024);
      gate_all[ID_IP[tmp[0]]]->setsymbol((string)buf);    
    }
    else if(cbuf == 'o'){
      myStr2Int((string)buf,tmp[0]);
      ifile.get(cbuf);
      ifile.getline(buf,1024);
      gate_all[cir_para[0]+tmp[0]+1]->setsymbol((string)buf);
    }
    lineNo++;
  }
  // set dfslist
  for(unsigned i=0;i<cir_para[3];i++){
    DFS(gate_all[cir_para[0]+1+i]);
  }
  resetvisit();

  return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
  cout<<endl;
  cout<<"Circuit Statistics\n"
        <<"==================\n"
        <<"  PI  "<<setw(10)<<cir_para[1]<<"\n"
        <<"  PO  "<<setw(10)<<cir_para[3]<<"\n"
        <<"  AIG "<<setw(10)<<ID_AIG.size()<<"\n"
        <<"------------------\n"
        <<"  Total"<<setw(9)<<cir_para[1]+cir_para[3]+ID_AIG.size()<<"\n";

}

void
CirMgr::printNetlist() 
{
  // about DFS
  cout<<endl;
  for(unsigned i =0,J =0;i < _dfsList.size();i++){    
    if(gate_all[_dfsList[i]]->getTypeStr() != "UNDEF"){
      cout<<"["<<J<<"] ";
      gate_all[_dfsList[i]]->printGate();
      J++;
    }    
  }  
  
}

void
CirMgr::DFS(CirGate* gate) {
  gate->setviset(true);
  vector<size_t>* tmp;
  if( gate->getfanin(tmp)){
    for(unsigned i =0;i<(*tmp).size();i++){
      if(gate_all[(*tmp)[i]/2]->isviset() == false){
        dfsviset(gate_all[(*tmp)[i]/2]);
      }
    }
    _dfsList.push_back(gate->getid());
  }
}
void 
CirMgr::dfsviset(CirGate* gate) {
  gate->setviset(true);
  vector<size_t>* tmp;
  if( gate->getfanin(tmp)){
    for(unsigned i =0;i<(*tmp).size();i++){
      if(gate_all[(*tmp)[i]/2]->isviset() == false){
        dfsviset(gate_all[(*tmp)[i]/2]);
      }
    }
    _dfsList.push_back(gate->getid());
    _dfsList_nopio.push_back(gate->getid());
  }
  else{
      _dfsList.push_back(gate->getid());       
  }
}

void 
CirMgr::resetvisit() {
  for(unsigned i=0;i<=cir_para[0]+cir_para[3];i++){
    if(gate_all[i]!=0){
      gate_all[i]->reset();
    }
  }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for(unsigned i=0;i<cir_para[1];i++){
     cout<<" "<<ID_IP[i];
   }   
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for(unsigned i=0; i<cir_para[3]; i++){
     cout<<" "<<cir_para[0]+1+i;
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
  vector<unsigned> rec;
  vector<size_t>* tmp;
  for(unsigned i=0;i<ID_UNDEF.size();i++){
    if( gate_all[ID_UNDEF[i]]->getfanout(tmp)){
      for(unsigned j =0;j<(*tmp).size();j++){
        rec.push_back((*tmp)[j]/2);
      }
    }
  }
  std::sort(rec.begin(),rec.end());
  vector<unsigned>::iterator pos;
  rec.erase(unique(rec.begin(),rec.end()),rec.end());
  if(!rec.empty()){
    cout<<"Gates with floating fanin(s):";
    for(unsigned i=0;i<rec.size();i++){
      cout<<" "<<rec[i]; 
    }
    cout<<endl;
    rec.clear();
  }
  for(unsigned i=1; i<cir_para[0]+1; i++){
    if(gate_all[i]!=0){
      if(gate_all[i]->getfanout_size() == 0){
      rec.push_back(gate_all[i]->getid());
      }
    }
  }
  std::sort(rec.begin(),rec.end());
  if(!rec.empty()){
    cout<<"Gates defined but not used  :";
    for(unsigned i=0;i<rec.size();i++){
      cout<<" "<<rec[i]; 
    }
    cout<<endl;
    rec.clear();
  }   
}

void
CirMgr::printFECPairs() const
{
  if(myfecgrps != 0){
    for(size_t i =0;i<(*myfecgrps).size();i++){
      cout <<"["<<i<<"]";
      for(size_t j =0, val =gate_all[(*(*myfecgrps)[i])[0]]->get_simval() ; j<(*(*myfecgrps)[i]).size();j++){
        cout<<" ";
        if( gate_all[(*(*myfecgrps)[i])[j]]->get_simval() != val ){
          cout<<"!";
        }
        cout<< (*(*myfecgrps)[i])[j];
      } 
      cout<<endl; 
    }
  }  
}

void
CirMgr::writeAag(ostream& outfile) const
{
  outfile.write("aag",3);
  outfile.put(' ');
  
  for(unsigned i=0;i<4;i++){
    outfile<<cir_para[i];
    outfile.put(' ');
  }
  outfile<<_dfsList_nopio.size();
  outfile.put('\n');
  for(unsigned i=0;i<ID_IP.size();i++){
    outfile<<2*ID_IP[i];
    outfile.put('\n');
  }
  for(unsigned i=0;i<ID_OP.size();i++){
    outfile<<ID_OP[i];
    outfile.put('\n');
  }
  for(unsigned i=0 ,n = _dfsList_nopio.size();i< n;i++){
    outfile<<2* (gate_all[_dfsList_nopio[i]]->getid());
    vector<size_t>* tmp;
    if( gate_all[_dfsList_nopio[i]]->getfanin(tmp))
    for(unsigned j =0;j<(*tmp).size();j++){
      outfile.put(' ');
      outfile<<((*tmp)[j]);
    }
    outfile.put('\n');
  }
  for(unsigned i=0;i<cir_para[1];i++){
    if(gate_all[ID_IP[i]]->getsymbol() != ""){
      outfile<<'i'<<i<<' '<<gate_all[ID_IP[i]]->getsymbol()<<'\n';
    }
  }
  for(unsigned i=0;i<cir_para[3];i++){
    if(gate_all[cir_para[0]+i+1]->getsymbol() != ""){
      outfile<<'o'<<i<<' '<<gate_all[cir_para[0]+i+1]->getsymbol()<<'\n';
    }
  }  
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
  vector<unsigned> list_pi,list_aig;
  g->DFS(list_pi,list_aig);
  outfile.write("aag",3);
  outfile.put(' ');
  outfile<<(list_aig.size()+list_pi.size());
  outfile.put(' ');
  outfile<<list_pi.size();
  outfile.put(' ');
  outfile<<0;
  outfile.put(' ');
  outfile<<1;  
  outfile.put(' ');
  outfile<<list_aig.size();
  outfile.put('\n');
  for(unsigned i=0;i<list_pi.size();i++){
    outfile<<2*list_pi[i];
    outfile.put('\n');
  }
  outfile<< 2*g->getid();
  outfile.put('\n');
  for(unsigned i=0 ,n = list_aig.size();i< n;i++){
    outfile<<2* (gate_all[list_aig[i]]->getid());
    vector<size_t>* tmp;
    if( gate_all[list_aig[i]]->getfanin(tmp))
    for(unsigned j =0;j<(*tmp).size();j++){
      outfile.put(' ');
      outfile<<((*tmp)[j]);
    }
    outfile.put('\n');
  }
  for(unsigned i=0,n =list_pi.size();i<n;i++){
    if(gate_all[list_pi[i]]->getsymbol() != ""){
      outfile<<'i'<<i<<' '<<gate_all[ID_IP[i]]->getsymbol()<<'\n';
    }
  }
  outfile.write("o0 Gate_",8);
  outfile<<g->getid()<<'\n';



}

CirGate* 
CirMgr::getGate(unsigned gid) const {
  if(gid<cir_para[0]+cir_para[3]+1) return gate_all[gid];
  return 0;
}