#include "SymbolTable.h"
#include <string>
#include <vector>
using namespace std;

const int HASH_TABLE_SIZE = 19997;

struct Info // Data type BUCKET will hold
{
	Info(string id, int lineNum, int scopeNum)
     : m_id(id), m_lineNum(lineNum), m_scopeNum (scopeNum)
        {item = nullptr;}
	string m_id;
	int m_lineNum;
	int m_scopeNum;
	Info* item; // next item if collision
};

struct BUCKET
{
	BUCKET() {item = nullptr;}
	Info* item;
};

class HashTable
{
public:
	void insert(const string& id, int lineNum, int scopeNum, Info** &record);
	bool search(const string& id, int& lineNum, int& scopeNum);
private:
	int hashFunc(const string& id) const;
	BUCKET m_buckets[HASH_TABLE_SIZE];
};

int HashTable::hashFunc(const string& id) const
// algorithm for turning strings into ints
{
	int total = 6;
	
	for (int i = 0; i < id.length(); i++)
    {
        total += id[i];
        total *= 2;
    }
    
    total = total % HASH_TABLE_SIZE;
    
    return (total);
}

bool HashTable::search(const string& id, int& lineNum, int& scopeNum)
{
	lineNum = -1;
	int h = hashFunc(id);
	Info* p = m_buckets[h].item;
	
	while (p!= nullptr)
	{
		if (p->m_id == id)
		{
			lineNum = p->m_lineNum; // output: the line and scope of the found item
			scopeNum = p->m_scopeNum;
		}
		p = p->item;
	}
	
	if (lineNum == -1)	// either no item in bucket, or no id found
		return false;
    
	return true;
}

void HashTable::insert(const string& id, int lineNum, int scopeNum, Info** &record)
{
	int h = hashFunc(id);
	Info* p = m_buckets[h].item;
	
	if (p == nullptr)   //no item in the bucket, dynamic allocation
	{
		m_buckets[h].item = new Info(id, lineNum, scopeNum);
		record = &m_buckets[h].item;
		return;
	}
	
	Info* q;
	while (p != nullptr)   //loop until empty spot found
	{
		q = p;
		p = p->item;
	}

	q->item = new Info(id, lineNum, scopeNum);
	record = &q->item;
    //output: pointer to pointer to new Info
}

// This class does the real work of the implementation.

class SymbolTableImpl
{
public:
    SymbolTableImpl() {m_scope = 0; Table = new HashTable;}
    ~SymbolTableImpl();
    void enterScope();
    bool exitScope();
    bool declare(const string& id, int lineNum);
    int find(const string& id) const;
private:
	HashTable* Table;
    vector<Info**> tableInfo;
    int m_scope;
};

SymbolTableImpl::~SymbolTableImpl()
{
    //need to free any leftover allocated Info memory and
    //free the allocated HashTable
    size_t i = tableInfo.size();
    
    while (i > 0)
    {
        i--;
        if (tableInfo[i] != nullptr)
            delete *(tableInfo[i]);
    }
    delete Table;
}

void SymbolTableImpl::enterScope()
{
	tableInfo.push_back(nullptr); // identify new scope
	m_scope++;
}

bool SymbolTableImpl::exitScope()
{
	size_t i = tableInfo.size();
	
	while (i > 0)
	{
		i--;
		if (tableInfo[i] == nullptr) // start of new scope found
			break;
        
		delete *(tableInfo[i]); // free memory and set pointer to null
        *(tableInfo[i]) = nullptr; //null allows us to recognize empty spot
		tableInfo.pop_back();
	}
    
	if (tableInfo.empty())  // unmatched }
		return false;
    
	tableInfo.pop_back();
	m_scope--;
	return true;
}

bool SymbolTableImpl::declare(const string& id, int lineNum)
{
    if (id.empty())
        return false;
    
	int scopeNum, line;
	if (Table->search(id, line, scopeNum) && scopeNum == m_scope)
		return false;   //found in same scope
	
	Info** p;
	Table->insert(id, lineNum, m_scope, p);
	tableInfo.push_back(p);
    return true;
}

int SymbolTableImpl::find(const string& id) const
{
    if (id.empty())
        return -1;
    
	int returnLine;
	int scopeNum;
	
	Table->search(id, returnLine, scopeNum);
    // if true, output returnLine; if false, output -1
    return returnLine;
}

//*********** SymbolTable functions **************

// For the most part, these functions simply delegate to SymbolTableImpl's
// functions.

SymbolTable::SymbolTable()
{
    m_impl = new SymbolTableImpl;
}

SymbolTable::~SymbolTable()
{
    delete m_impl;
}

void SymbolTable::enterScope()
{
    m_impl->enterScope();
}

bool SymbolTable::exitScope()
{
    return m_impl->exitScope();
}

bool SymbolTable::declare(const string& id, int lineNum)
{
    return m_impl->declare(id, lineNum);
}

int SymbolTable::find(const string& id) const
{
    return m_impl->find(id);
}
