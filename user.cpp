#include "user.h"

void user :: setValues(string contact , string name, string password)
{
    this->contact = contact;
    this->name = name;
    this->password = password;
}
string user :: getContact(){
    return contact;
}
string user :: getName(){
    return name;
}
string user :: getPassword(){
    return password;
}
string user :: getType(){
    return type;
}
