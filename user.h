#ifndef USER_H
#define USER_H
#include<string>
using namespace std;

class user
{
protected :
    string contact , name, password, type;

public : 
    virtual ~user() = default;
    virtual void setType() = 0;
    void setValues(const string& contact, const string& name, const string& password);
    string getContact() const;
    string getName() const;
    string getPassword() const;
    string getType() const;
};

#endif