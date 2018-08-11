#include "include/Sudoku/TString.h"

TString::TString(){
    index = 0;
    ispp = false;
}

TString::TString(std::string s){
    index = 0;
    ispp = false;
    data = s;
    str = s;
}

TString &TString::operator << (const char *s){
    if(!ispp){
        data.append(s);
    }else{
        data2.append(s);
    }
    str = "";
    str = data+constant_data+data2;
    return *this;
}

TString &TString::operator << (const std::string &s){
    if(!ispp){
        data.append(s);
    }else{
        data2.append(s);
    }
    str = "";
    str = data+constant_data+data2;
    return *this;
}

TString &TString::operator << (int s){
    char a[sizeof(int)];
    sprintf(a,"%d",s);
    if(!ispp){
        data.append(a);
    }else{
        data2.append(a);
    }
    str = "";
    str = data+constant_data+data2;
    return *this;
}

TString &TString::operator ++(){
    ispp = true;
    index++;
    char a[sizeof(int)];
    sprintf(a,"%d",index);
    constant_data = "";
    constant_data.append(a);
    str = "";
    str = data+constant_data+data2;
    return *this;
}

TString TString::operator ++(int){
    ispp = true;
    char a[sizeof(int)];
    sprintf(a,"%d",index);
    index++;
    constant_data = "";
    constant_data.append(a);
    str = "";
    str = data+constant_data+data2;
    return *this;
}

TString &TString::operator --(){
    ispp = true;
    index--;
    char a[sizeof(int)];
    sprintf(a,"%d",index);
    constant_data = "";
    constant_data.append(a);
    str = "";
    str = data+constant_data+data2;
    return *this;
}

TString TString::operator --(int){
    ispp = true;
    char a[sizeof(int)];
    sprintf(a,"%d",index);
    index--;
    constant_data = "";
    constant_data.append(a);
    str = "";
    str = data+constant_data+data2;
    return *this;
}

TString TString::operator +(TString _second){
    char a[sizeof(int)];
    sprintf(a,"%d",index);
    constant_data = "";
    constant_data.append(a);
    this->str = "";
    this->str = data+constant_data+data2;
    TString s(*this);
    s.str += _second.str;
    return s;
}

TString& TString::operator +=(TString _second){
    this->str += _second.str;
    return *this;
}

void TString::clear(){
    data = "";
    index = 0;
    ispp = false;
    str = "";
    data2 = "";
}


