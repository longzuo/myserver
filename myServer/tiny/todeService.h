#include <stdio.h>
#include <string.h>

using namespace std;

class todeService
{
private:
      void execTode(const char* cmd, string& out);
public:
      void process(string& input, string& output); 
};

void todeService::execTode(const char* cmd, string& output)
{
    FILE *fp = popen(cmd, "r");
    if (!fp)
    {
        return;
    }
    char buff[100];
    int ret;
    do
    {
        ret = fread(buff, sizeof(char), sizeof(buff),fp);
        output += string(buff,ret);    
    }while(ret == sizeof(buff));
    pclose(fp);
    return;
}
void todeService::process(string& input, string& output)
{
    string header = "08.1/CRS/FS//1/0/CRS/1E/A/12345/0/CN/0/RES/0//";
    string cmd = "echo ";
    cmd += header;
    cmd += input;
    cmd += "|todecall //10.6.157.125:16764 SHOPCOMM -t";
    
    execTode(cmd.c_str(), output);    
}
