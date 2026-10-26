#ifndef USER_H
#define USER_H
#include<string>
using namespace std;

class user
{
protected :
    string contact , name, password, type;

public : 
    virtual void setType() = 0;
    void setValues(string contact , string name, string password);
    string getContact();
    string getName();
    string getPassword();
    string getType();
};

#endif