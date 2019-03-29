#ifndef __HASHER__
#define __HASHER__
#include "MurmurHash3.h"

#include <google/dense_hash_map>
#include <set>
#include "sysflow/sysflow.hh"

using namespace std;
using namespace sysflow;
//typedef boost::array<uint8_t, 16> OID;
struct NFKey {
   //OID oid;
   uint32_t ip1;
   uint16_t port1;
   uint32_t ip2;
   uint16_t port2;
};

class DataFlowObj {
    public:
      time_t exportTime;
      time_t lastUpdate;
      bool isNetworkFlow;
      DataFlowObj(bool inf) : exportTime(0), lastUpdate(0), isNetworkFlow(inf) {
           
      }
      

};
   
class NetFlowObj : public DataFlowObj {
    public:
       NetworkFlow netflow;
       bool operator ==(const NetFlowObj& nfo) {
         if(exportTime != nfo.exportTime) {
            return false;
         }

       //  cout << netflow.procOID.createTS << " " << nfo.netflow.procOID.createTS << " " << netflow.procOID.hpid <<  " " << nfo.netflow.procOID.hpid << endl;
         return (netflow.procOID.createTS == nfo.netflow.procOID.createTS && 
            netflow.procOID.hpid == nfo.netflow.procOID.hpid &&
            netflow.sip == nfo.netflow.sip &&
            netflow.dip == nfo.netflow.dip &&
            netflow.sport == nfo.netflow.sport &&
            netflow.dport == nfo.netflow.dport &&
            netflow.ts == nfo.netflow.ts);
       // cout << "Result: " << result << endl;
        //return result;
      
      }
      NetFlowObj() : DataFlowObj(true) {
         
      }
 

};

class FileFlowObj : public DataFlowObj  {
    public:
        FileFlow fileflow;
        FileFlowObj() : DataFlowObj(false) {
         
        }
};



// simple hash adapter for types without pointers
template<typename T> 
struct MurmurHasher {
    size_t operator()(const T& t) const {
        size_t hash;
        MurmurHash3_x86_32(&t, sizeof(t), 0, &hash);
        return hash;
    }    
};

/*template<> 
struct MurmurHasher<OID*> {
    size_t operator()(const OID* t) const {
        size_t hash;
        MurmurHash3_x86_32(t->begin(), t->size(), 0, &hash);
        return hash;
    }    
};*/
template<> 
struct MurmurHasher<OID*> {
    size_t operator()(const OID* t) const {
        size_t hash;
        MurmurHash3_x86_32((void*)t, sizeof(OID), 0, &hash);
        return hash;
    }    
};

/*struct eqoid
{
  bool operator()(const OID* s1, const OID* s2) const
  {
    return (*s1 == *s2);
  }
};*/
struct eqoidptr
{
  bool operator()(const OID* s1, const OID* s2) const
  {
    //cout << s1->hpid << " " << s2->hpid << " " << s1->createTS << " " << s2->createTS << " " << (s1->hpid == s2->hpid && s1->createTS == s2->createTS) << endl;
    return (s1->hpid == s2->hpid && s1->createTS == s2->createTS);
  }
};
struct eqoid
{
  bool operator()(const OID& s1, const OID& s2) const
  {
    //cout << s1->hpid << " " << s2->hpid << " " << s1->createTS << " " << s2->createTS << " " << (s1->hpid == s2->hpid && s1->createTS == s2->createTS) << endl;
    return (s1.hpid == s2.hpid && s1.createTS == s2.createTS);
  }
};
// specialization for strings
template<> 
struct MurmurHasher<string> {
    size_t operator()(const string& t) const {
        size_t hash;
        MurmurHash3_x86_32(t.c_str(), t.size(), 0, &hash);
        return hash;
    }    
};

struct eqstr
{
  bool operator()(const string& s1, const string& s2) const
  {
    return (s1.compare(s2) == 0);
  }
};

template<> 
struct MurmurHasher<NFKey> {
    size_t operator()(const NFKey& t) const {
        size_t hash;
        MurmurHash3_x86_32((void*)&t, sizeof(NFKey), 0, &hash);
        return hash;
    }    
};

struct eqnfkey {
  bool operator()(const NFKey& n1, const NFKey& n2) const
  {
    return (n1.ip1 == n2.ip1 && n1.ip2 == n2.ip2 && 
           n1.port1 == n2.port1 && n1.port2 == n2.port2);
           //&& n1.oid.hpid == n2.oid.hpid && n1.oid.createTS == n2.oid.createTS);
  }
};

struct eqdfobj {
  bool operator()(const DataFlowObj* df1, const DataFlowObj* df2) {
      return (df1->exportTime < df2->exportTime);  
 }


};



typedef google::dense_hash_map<int, string> ParameterMapping;
typedef google::dense_hash_map<string, Container*, MurmurHasher<string>, eqstr> ContainerTable;
//typedef google::dense_hash_map<OID*, ProcessFlow*, MurmurHasher<OID*>, eqoid> ProcessFlowTable;
typedef google::dense_hash_map<NFKey, NetFlowObj*, MurmurHasher<NFKey>, eqnfkey> NetworkFlowTable;
typedef google::dense_hash_map<string, FileFlowObj*, MurmurHasher<string>, eqstr> FileFlowTable;
typedef google::dense_hash_map<OID, NetworkFlowTable*,MurmurHasher<OID>, eqoid> OIDNetworkTable;
//typedef multiset<NetFlowObj*, eqnfobj>  NetworkFlowSet;
typedef multiset<DataFlowObj*, eqdfobj>  DataFlowSet;

class ProcessObj {
    private:
        static NFKey m_nfdelkey;
        static NFKey m_nfemptykey;
    public:
        bool written;
        Process proc;
        NetworkFlowTable netflows;
        FileFlowTable    fileflows;
        ProcessObj() : written(false) {
            m_nfdelkey.ip1 = 1;
            m_nfdelkey.ip2 = 1;
            m_nfdelkey.port1 = 1;
            m_nfdelkey.port2 = 1;
            m_nfemptykey.ip1 = 1;
            m_nfemptykey.ip2 = 0;
            m_nfemptykey.port1 = 1;
            m_nfemptykey.port2 = 1;
            netflows.set_empty_key(m_nfemptykey);
            netflows.set_deleted_key(m_nfdelkey);
      }
};

typedef google::dense_hash_map<OID*, ProcessObj*, MurmurHasher<OID*>, eqoidptr> ProcessTable;


#endif
