#include "user.h"

void user :: setValues(const string& contact, const string& name, const string& password)
{
    this->contact = contact;
    this->name = name;
    this->password = password;
}
string user :: getContact() const {
    return contact;
}
string user :: getName() const {
    return name;
}
string user :: getPassword() const {
    return password;
}
string user :: getType() const {
    return type;
}