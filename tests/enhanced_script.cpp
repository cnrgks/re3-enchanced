#include "../src/extras/EnhancedScript.h"
#include <cstdio>
#include <cstdlib>
static void require(bool condition, const char *message)
{
    if (!condition) { std::fprintf(stderr,"FAILED: %s\n",message); std::exit(1); }
}
int main()
{
    uint8_t data[64]={};
    data[20]=6; const int16_t fixed=-4897; std::memcpy(data+21,&fixed,2);
    size_t at=20; float value=0;
    require(EnhancedScript::Number(data,64,16,at,value,true) && value==-306.0625f && at==23,"GTA III fixed-point float");
    data[23]=4; data[24]=255; at=23;
    require(EnhancedScript::Number(data,64,16,at,value,false) && value==-1,"signed byte literal");
    const float global=893.5f; std::memcpy(data+8,&global,4);
    data[25]=2; data[26]=8; data[27]=0; at=25;
    require(EnhancedScript::Number(data,64,16,at,value,true) && value==global,"global float resolution");
    data[26]=15; at=25;
    require(!EnhancedScript::Number(data,64,16,at,value,true),"global read cannot cross variable block");
    data[26]=0; at=25;
    require(!EnhancedScript::Number(data,64,16,at,value,true),"script header is not a global variable");
    data[63]=1; at=63;
    require(!EnhancedScript::Number(data,64,16,at,value,true),"truncated literal bounds");
    data[30]=3; at=30;
    require(!EnhancedScript::Number(data,64,16,at,value,true),"do not guess local variables");
    at=64;
    require(!EnhancedScript::Number(data,64,16,at,value,true),"end of script bounds");
    std::puts("PASS: SCM constants, fixed-point floats, global bounds and malformed operands");
}
